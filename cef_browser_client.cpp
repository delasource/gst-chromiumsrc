#include "cef_browser_client.h"
#include "debug_utils.h"

#include <include/cef_app.h>
#include <include/cef_browser.h>
#include <include/cef_client.h>
#include <include/wrapper/cef_helpers.h>

// ============================================================================
// BrowserInstance - Internal browser tracking structure
// ============================================================================

struct _BrowserInstance
{
    CefRefPtr<CefBrowser> cef_browser;
    BrowserCallbacks callbacks;
    gint width;
    gint height;
    gint fps;
    gboolean page_loaded;
    gboolean running;
};

// Forward declaration for handler callbacks
class CefBrowserClientImpl;

// ============================================================================
// CEF Handler implementations
// ============================================================================

/**
 * CefBrowserRenderHandler - Handles offscreen rendering for a browser
 */
class CefBrowserRenderHandler : public CefRenderHandler
{
public:
    CefBrowserRenderHandler(BrowserInstance* browser, int width, int height)
        : browser_(browser), width_(width), height_(height)
    {
    }

    void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override
    {
        rect.Set(0, 0, width_, height_);
    }

    void OnPaint(
        CefRefPtr<CefBrowser> browser,
        PaintElementType type,
        const RectList& dirtyRects,
        const void* buffer,
        int width,
        int height) override
    {
        if (!browser_ || !browser_->running)
        {
            return;
        }

        if (width != width_ || height != height_)
        {
            DEBUG_LOG("OnPaint - Size mismatch: got %dx%d, expected %dx%d",
                      width, height, width_, height_);
            return;
        }

        // Forward to callback
        if (browser_->callbacks.on_paint)
        {
            browser_->callbacks.on_paint(browser_->callbacks.user_data, buffer, width, height);
        }
    }

private:
    BrowserInstance* browser_;
    int width_;
    int height_;
    IMPLEMENT_REFCOUNTING(CefBrowserRenderHandler);
};

/**
 * CefBrowserLoadHandler - Handles page load events
 */
class CefBrowserLoadHandler : public CefLoadHandler
{
public:
    CefBrowserLoadHandler(BrowserInstance* browser)
        : browser_(browser)
    {
    }

    void OnLoadEnd(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        int httpStatusCode) override
    {
        if (frame->IsMain())
        {
            DEBUG_LOG_CEF("Page loaded (HTTP %d)", httpStatusCode);
            browser_->page_loaded = TRUE;

            if (browser_->callbacks.on_load_end)
            {
                browser_->callbacks.on_load_end(browser_->callbacks.user_data, httpStatusCode);
            }
        }
    }

    void OnLoadError(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        ErrorCode errorCode,
        const CefString& errorText,
        const CefString& failedUrl) override
    {
        if (frame->IsMain())
        {
            DEBUG_LOG_CEF("Load error: %s (%d) - %s",
                          errorText.ToString().c_str(), errorCode, failedUrl.ToString().c_str());
        }
    }

private:
    BrowserInstance* browser_;
    IMPLEMENT_REFCOUNTING(CefBrowserLoadHandler);
};

/**
 * CefBrowserLifeSpanHandler - Handles browser lifecycle events
 */
class CefBrowserLifeSpanHandler : public CefLifeSpanHandler
{
public:
    CefBrowserLifeSpanHandler(BrowserInstance* browser)
        : browser_(browser)
    {
    }

    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override
    {
        CEF_REQUIRE_UI_THREAD();
        DEBUG_LOG_CEF("OnAfterCreated - browser=%p", browser.get());
        browser_->cef_browser = browser;
    }

private:
    BrowserInstance* browser_;
    IMPLEMENT_REFCOUNTING(CefBrowserLifeSpanHandler);
};

/**
 * CefBrowserClientImpl - Main CEF client for a browser instance
 */
class CefBrowserClientImpl : public CefClient
{
public:
    CefBrowserClientImpl(
        CefRefPtr<CefRenderHandler> render_handler,
        CefRefPtr<CefLoadHandler> load_handler,
        CefRefPtr<CefLifeSpanHandler> lifespan_handler)
        : render_handler_(render_handler),
          load_handler_(load_handler),
          lifespan_handler_(lifespan_handler)
    {
    }

    CefRefPtr<CefRenderHandler> GetRenderHandler() override { return render_handler_; }
    CefRefPtr<CefLoadHandler> GetLoadHandler() override { return load_handler_; }
    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return lifespan_handler_; }

private:
    CefRefPtr<CefRenderHandler> render_handler_;
    CefRefPtr<CefLoadHandler> load_handler_;
    CefRefPtr<CefLifeSpanHandler> lifespan_handler_;
    IMPLEMENT_REFCOUNTING(CefBrowserClientImpl);
};

// ============================================================================
// Browser client tracking - keeps clients alive during browser lifetime
// ============================================================================

// Global tracking to keep client references alive
// This is managed by the create/destroy functions
static GMutex g_clients_mutex;
static std::map<BrowserInstance*, CefRefPtr<CefClient>> g_browser_clients;

// ============================================================================
// C API Implementation
// ============================================================================

extern "C" {

BrowserInstance* cef_browser_create(
    const gchar* url,
    gint width,
    gint height,
    gint fps,
    BrowserCallbacks* callbacks)
{
    if (!url || !callbacks)
    {
        return nullptr;
    }

    // Create browser instance
    const auto browser = new BrowserInstance();
    browser->width = width;
    browser->height = height;
    browser->fps = fps;
    browser->page_loaded = FALSE;
    browser->running = TRUE;
    browser->callbacks = *callbacks;

    // Create handlers
    const auto render_handler = new CefBrowserRenderHandler(browser, width, height);
    const auto load_handler = new CefBrowserLoadHandler(browser);
    const auto lifespan_handler = new CefBrowserLifeSpanHandler(browser);

    // Create client
    const auto client = new CefBrowserClientImpl(render_handler, load_handler, lifespan_handler);

    // Track browser client (keeps reference alive)
    g_mutex_lock(&g_clients_mutex);
    g_browser_clients[browser] = client;
    g_mutex_unlock(&g_clients_mutex);

    // Configure windowless rendering
    CefWindowInfo window_info;
    window_info.SetAsWindowless(0);

    // Configure browser settings
    CefBrowserSettings browser_settings;
    browser_settings.windowless_frame_rate = fps;

    CefString cef_url(url);

    DEBUG_LOG_CEF("CreateBrowser - url=%s, width=%d, height=%d, fps=%d", url, width, height, fps);

    // Create browser asynchronously
    if (!CefBrowserHost::CreateBrowser(
        window_info, client, cef_url, browser_settings, nullptr, nullptr))
    {
        DEBUG_LOG_CEF("CreateBrowser FAILED");
        g_mutex_lock(&g_clients_mutex);
        g_browser_clients.erase(browser);
        g_mutex_unlock(&g_clients_mutex);
        delete browser;
        return nullptr;
    }

    DEBUG_LOG_CEF("Browser creation initiated");
    return browser;
}

void cef_browser_destroy(BrowserInstance* browser)
{
    if (!browser) return;

    DEBUG_LOG_CEF("Destroying browser...");

    browser->running = FALSE;

    // Remove from tracking
    g_mutex_lock(&g_clients_mutex);
    g_browser_clients.erase(browser);
    g_mutex_unlock(&g_clients_mutex);

    // Close CEF browser
    if (browser->cef_browser)
    {
        browser->cef_browser->GetHost()->CloseBrowser(TRUE);
        browser->cef_browser = nullptr;
    }

    delete browser;
    DEBUG_LOG_CEF("Browser destroyed");
}

gpointer cef_browser_get_handle(BrowserInstance* browser)
{
    return browser ? browser->cef_browser.get() : nullptr;
}

gboolean cef_browser_is_page_loaded(BrowserInstance* browser)
{
    return browser ? browser->page_loaded : FALSE;
}

} // extern "C"
