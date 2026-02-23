#include "cef_render_handler.h"
#include "gstchromiumsrc.h"

#include <include/cef_app.h>
#include <include/cef_browser.h>
#include <include/cef_client.h>
#include <include/cef_command_line.h>
#include <include/wrapper/cef_helpers.h>

#include <glib.h>

static GMutex cef_init_mutex;
static gboolean cef_initialized = FALSE;

class CefRenderHandlerImpl : public CefRenderHandler {
public:
    CefRenderHandlerImpl(GstChromiumSrc *src, int width, int height)
        : src_(src), width_(width), height_(height) {}

    void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) override {
        rect.Set(0, 0, width_, height_);
    }

    void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
        const RectList &dirtyRects, const void *buffer, int width, int height) override {

        static int paint_count = 0;
        paint_count++;
        g_print("DEBUG: OnPaint #%d (page_loaded=%d)\n", paint_count, src_ ? src_->page_loaded : -1);

        if (!src_ || !src_->running || !src_->frame_buffer) {
            return;
        }

        if (width != width_ || height != height_) {
            g_warning("OnPaint: size mismatch: %dx%d vs %dx%d", width, height, width_, height_);
            return;
        }

        g_mutex_lock(&src_->frame_mutex);

        if (src_->running && src_->frame_buffer) {
            memcpy(src_->frame_buffer, buffer, src_->frame_size);
            src_->frame_ready = TRUE;
            g_cond_signal(&src_->frame_cond);
        }

        g_mutex_unlock(&src_->frame_mutex);
    }

private:
    GstChromiumSrc *src_;
    int width_;
    int height_;

    IMPLEMENT_REFCOUNTING(CefRenderHandlerImpl);
};

class CefLoadHandlerImpl : public CefLoadHandler {
public:
    CefLoadHandlerImpl(GstChromiumSrc *src) : src_(src) {}

    void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
        int httpStatusCode) override {
        if (frame->IsMain()) {
            g_print("DEBUG: Page loaded (HTTP %d)\n", httpStatusCode);
            src_->page_loaded = TRUE;
        }
    }

    void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
        ErrorCode errorCode, const CefString &errorText, const CefString &failedUrl) override {
        if (frame->IsMain()) {
            g_warning("Load error: %s (%d) - %s", errorText.ToString().c_str(),
                errorCode, failedUrl.ToString().c_str());
        }
    }

private:
    GstChromiumSrc *src_;
    IMPLEMENT_REFCOUNTING(CefLoadHandlerImpl);
};

class CefClientImpl : public CefClient {
public:
    CefClientImpl(CefRefPtr<CefRenderHandler> render_handler, CefRefPtr<CefLoadHandler> load_handler)
        : render_handler_(render_handler), load_handler_(load_handler) {}

    CefRefPtr<CefRenderHandler> GetRenderHandler() override {
        return render_handler_;
    }

    CefRefPtr<CefLoadHandler> GetLoadHandler() override {
        return load_handler_;
    }

private:
    CefRefPtr<CefRenderHandler> render_handler_;
    CefRefPtr<CefLoadHandler> load_handler_;

    IMPLEMENT_REFCOUNTING(CefClientImpl);
};

static gpointer cef_message_loop_thread(gpointer data) {
    GstChromiumSrc *src = (GstChromiumSrc *)data;
    int count = 0;

    while (src->running) {
        CefDoMessageLoopWork();
        count++;
        if (count % 60 == 0) {
            g_print("DEBUG: DoMessageLoopWork #%d\n", count);
        }

        if (src->page_loaded && src->cef_browser) {
            CefBrowser *browser = static_cast<CefBrowser *>(src->cef_browser);
            browser->GetHost()->Invalidate(PET_VIEW);
        }

        g_usleep(10000);
    }

    return NULL;
}

static gboolean initialize_cef(void) {
    g_mutex_lock(&cef_init_mutex);

    if (cef_initialized) {
        g_mutex_unlock(&cef_init_mutex);
        return TRUE;
    }

    CefMainArgs main_args;
    CefSettings settings;

    settings.no_sandbox = TRUE;
    settings.windowless_rendering_enabled = TRUE;
    settings.log_severity = LOGSEVERITY_WARNING;
    settings.multi_threaded_message_loop = FALSE;

    CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
    command_line->AppendSwitch("single-process");
    command_line->AppendSwitch("disable-gpu");
    command_line->AppendSwitch("disable-gpu-compositing");
    command_line->AppendSwitch("disable-software-rasterizer");
    command_line->AppendSwitchWithValue("log-severity", "warning");

    CefRefPtr<CefApp> app;

    if (!CefInitialize(main_args, settings, app, nullptr)) {
        g_mutex_unlock(&cef_init_mutex);
        g_warning("Failed to initialize CEF");
        return FALSE;
    }

    cef_initialized = TRUE;
    g_mutex_unlock(&cef_init_mutex);

    return TRUE;
}


extern "C" {

gboolean cef_browser_start(GstChromiumSrc *src, const gchar *url, gint width, gint height) {
    if (!initialize_cef()) {
        return FALSE;
    }

    src->page_loaded = FALSE;

    CefRefPtr<CefRenderHandlerImpl> render_handler =
        new CefRenderHandlerImpl(src, width, height);

    CefRefPtr<CefLoadHandlerImpl> load_handler =
        new CefLoadHandlerImpl(src);

    CefRefPtr<CefClientImpl> client =
        new CefClientImpl(render_handler, load_handler);

    CefWindowInfo window_info;
    window_info.SetAsWindowless(0);

    CefBrowserSettings browser_settings;
    browser_settings.windowless_frame_rate = src->fps_num;

    CefString cef_url(url);

    CefRefPtr<CefBrowser> browser = CefBrowserHost::CreateBrowserSync(
        window_info, client, cef_url, browser_settings, nullptr, nullptr);

    if (!browser) {
        g_warning("Failed to create CEF browser");
        return FALSE;
    }

    src->cef_browser = static_cast<gpointer>(browser.get());
    browser->AddRef();

    src->cef_client = static_cast<gpointer>(client.get());
    client->AddRef();

    src->cef_thread = g_thread_new("cef-message-loop",
        cef_message_loop_thread, src);

    if (!src->cef_thread) {
        g_warning("Failed to create CEF message loop thread");
        return FALSE;
    }

    return TRUE;
}

void cef_browser_stop(GstChromiumSrc *src) {
    if (!src) {
        return;
    }

    if (src->cef_thread) {
        g_thread_join(src->cef_thread);
        src->cef_thread = NULL;
    }

    if (src->cef_browser) {
        CefBrowser *browser = static_cast<CefBrowser *>(src->cef_browser);
        browser->GetHost()->CloseBrowser(TRUE);
        browser->Release();
        src->cef_browser = NULL;
    }

    if (src->cef_client) {
        CefClient *client = static_cast<CefClient *>(src->cef_client);
        client->Release();
        src->cef_client = NULL;
    }
}

}
