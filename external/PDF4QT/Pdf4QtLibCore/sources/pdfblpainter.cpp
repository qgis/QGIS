// MIT License
//
// Copyright (c) 2018-2025 Jakub Melka and Contributors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "pdfblpainter.h"
#include "pdffont.h"

#include <QThread>
#include <QRawFont>
#include <QPainterPath>
#include <QPaintEngine>
#include <QPainterPathStroker>

#include <utility>

#if __has_include(<blend2d/blend2d.h>)
  #include <blend2d/blend2d.h>
#else
  #include <blend2d.h>
#endif

// Compatibility layer between blend2d's snake_case API (>= 0.20) and the
// older camelCase API (< 0.20). Each wrapper below detects, at compile time,
// which spelling the installed blend2d headers actually provide and forwards
// to it, so the call sites in this file can always use the snake_case name
// regardless of which blend2d version is installed.
namespace blcompat
{

template <typename T, typename... Args>
decltype(auto) assign_data(T& obj, Args&&... args)
{
    if constexpr (requires { obj.assign_data(args...); })
    {
        return obj.assign_data(std::forward<Args>(args)...);
    }
    else
    {
        return obj.assignData(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) assign_deep(T& obj, Args&&... args)
{
    if constexpr (requires { obj.assign_deep(args...); })
    {
        return obj.assign_deep(std::forward<Args>(args)...);
    }
    else
    {
        return obj.assignDeep(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) assign_stops(T& obj, Args&&... args)
{
    if constexpr (requires { obj.assign_stops(args...); })
    {
        return obj.assign_stops(std::forward<Args>(args)...);
    }
    else
    {
        return obj.assignStops(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) blit_image(T& obj, Args&&... args)
{
    if constexpr (requires { obj.blit_image(args...); })
    {
        return obj.blit_image(std::forward<Args>(args)...);
    }
    else
    {
        return obj.blitImage(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) clear_all(T& obj, Args&&... args)
{
    if constexpr (requires { obj.clear_all(args...); })
    {
        return obj.clear_all(std::forward<Args>(args)...);
    }
    else
    {
        return obj.clearAll(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) clip_to_rect(T& obj, Args&&... args)
{
    if constexpr (requires { obj.clip_to_rect(args...); })
    {
        return obj.clip_to_rect(std::forward<Args>(args)...);
    }
    else
    {
        return obj.clipToRect(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) create_from_data(T& obj, Args&&... args)
{
    if constexpr (requires { obj.create_from_data(args...); })
    {
        return obj.create_from_data(std::forward<Args>(args)...);
    }
    else
    {
        return obj.createFromData(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) create_from_face(T& obj, Args&&... args)
{
    if constexpr (requires { obj.create_from_face(args...); })
    {
        return obj.create_from_face(std::forward<Args>(args)...);
    }
    else
    {
        return obj.createFromFace(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) cubic_to(T& obj, Args&&... args)
{
    if constexpr (requires { obj.cubic_to(args...); })
    {
        return obj.cubic_to(std::forward<Args>(args)...);
    }
    else
    {
        return obj.cubicTo(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) fill_ellipse(T& obj, Args&&... args)
{
    if constexpr (requires { obj.fill_ellipse(args...); })
    {
        return obj.fill_ellipse(std::forward<Args>(args)...);
    }
    else
    {
        return obj.fillEllipse(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) fill_rect_array(T& obj, Args&&... args)
{
    if constexpr (requires { obj.fill_rect_array(args...); })
    {
        return obj.fill_rect_array(std::forward<Args>(args)...);
    }
    else
    {
        return obj.fillRectArray(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) fill_path(T& obj, Args&&... args)
{
    if constexpr (requires { obj.fill_path(args...); })
    {
        return obj.fill_path(std::forward<Args>(args)...);
    }
    else
    {
        return obj.fillPath(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) line_to(T& obj, Args&&... args)
{
    if constexpr (requires { obj.line_to(args...); })
    {
        return obj.line_to(std::forward<Args>(args)...);
    }
    else
    {
        return obj.lineTo(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) move_to(T& obj, Args&&... args)
{
    if constexpr (requires { obj.move_to(args...); })
    {
        return obj.move_to(std::forward<Args>(args)...);
    }
    else
    {
        return obj.moveTo(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) reset_transform(T& obj, Args&&... args)
{
    if constexpr (requires { obj.reset_transform(args...); })
    {
        return obj.reset_transform(std::forward<Args>(args)...);
    }
    else
    {
        return obj.resetTransform(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) restore_clipping(T& obj, Args&&... args)
{
    if constexpr (requires { obj.restore_clipping(args...); })
    {
        return obj.restore_clipping(std::forward<Args>(args)...);
    }
    else
    {
        return obj.restoreClipping(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) set_comp_op(T& obj, Args&&... args)
{
    if constexpr (requires { obj.set_comp_op(args...); })
    {
        return obj.set_comp_op(std::forward<Args>(args)...);
    }
    else
    {
        return obj.setCompOp(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) set_fill_rule(T& obj, Args&&... args)
{
    if constexpr (requires { obj.set_fill_rule(args...); })
    {
        return obj.set_fill_rule(std::forward<Args>(args)...);
    }
    else
    {
        return obj.setFillRule(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) set_fill_style(T& obj, Args&&... args)
{
    if constexpr (requires { obj.set_fill_style(args...); })
    {
        return obj.set_fill_style(std::forward<Args>(args)...);
    }
    else
    {
        return obj.setFillStyle(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) set_global_alpha(T& obj, Args&&... args)
{
    if constexpr (requires { obj.set_global_alpha(args...); })
    {
        return obj.set_global_alpha(std::forward<Args>(args)...);
    }
    else
    {
        return obj.setGlobalAlpha(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) set_hint(T& obj, Args&&... args)
{
    if constexpr (requires { obj.set_hint(args...); })
    {
        return obj.set_hint(std::forward<Args>(args)...);
    }
    else
    {
        return obj.setHint(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) set_stroke_alpha(T& obj, Args&&... args)
{
    if constexpr (requires { obj.set_stroke_alpha(args...); })
    {
        return obj.set_stroke_alpha(std::forward<Args>(args)...);
    }
    else
    {
        return obj.setStrokeAlpha(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) set_stroke_caps(T& obj, Args&&... args)
{
    if constexpr (requires { obj.set_stroke_caps(args...); })
    {
        return obj.set_stroke_caps(std::forward<Args>(args)...);
    }
    else
    {
        return obj.setStrokeCaps(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) set_stroke_dash_array(T& obj, Args&&... args)
{
    if constexpr (requires { obj.set_stroke_dash_array(args...); })
    {
        return obj.set_stroke_dash_array(std::forward<Args>(args)...);
    }
    else
    {
        return obj.setStrokeDashArray(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) set_stroke_dash_offset(T& obj, Args&&... args)
{
    if constexpr (requires { obj.set_stroke_dash_offset(args...); })
    {
        return obj.set_stroke_dash_offset(std::forward<Args>(args)...);
    }
    else
    {
        return obj.setStrokeDashOffset(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) set_stroke_join(T& obj, Args&&... args)
{
    if constexpr (requires { obj.set_stroke_join(args...); })
    {
        return obj.set_stroke_join(std::forward<Args>(args)...);
    }
    else
    {
        return obj.setStrokeJoin(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) set_stroke_miter_limit(T& obj, Args&&... args)
{
    if constexpr (requires { obj.set_stroke_miter_limit(args...); })
    {
        return obj.set_stroke_miter_limit(std::forward<Args>(args)...);
    }
    else
    {
        return obj.setStrokeMiterLimit(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) set_stroke_options(T& obj, Args&&... args)
{
    if constexpr (requires { obj.set_stroke_options(args...); })
    {
        return obj.set_stroke_options(std::forward<Args>(args)...);
    }
    else
    {
        return obj.setStrokeOptions(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) set_stroke_style(T& obj, Args&&... args)
{
    if constexpr (requires { obj.set_stroke_style(args...); })
    {
        return obj.set_stroke_style(std::forward<Args>(args)...);
    }
    else
    {
        return obj.setStrokeStyle(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) set_stroke_width(T& obj, Args&&... args)
{
    if constexpr (requires { obj.set_stroke_width(args...); })
    {
        return obj.set_stroke_width(std::forward<Args>(args)...);
    }
    else
    {
        return obj.setStrokeWidth(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) set_transform(T& obj, Args&&... args)
{
    if constexpr (requires { obj.set_transform(args...); })
    {
        return obj.set_transform(std::forward<Args>(args)...);
    }
    else
    {
        return obj.setTransform(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) stroke_line(T& obj, Args&&... args)
{
    if constexpr (requires { obj.stroke_line(args...); })
    {
        return obj.stroke_line(std::forward<Args>(args)...);
    }
    else
    {
        return obj.strokeLine(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) stroke_options(T& obj, Args&&... args)
{
    if constexpr (requires { obj.stroke_options(args...); })
    {
        return obj.stroke_options(std::forward<Args>(args)...);
    }
    else
    {
        return obj.strokeOptions(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) stroke_path(T& obj, Args&&... args)
{
    if constexpr (requires { obj.stroke_path(args...); })
    {
        return obj.stroke_path(std::forward<Args>(args)...);
    }
    else
    {
        return obj.strokePath(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) stroke_rect_array(T& obj, Args&&... args)
{
    if constexpr (requires { obj.stroke_rect_array(args...); })
    {
        return obj.stroke_rect_array(std::forward<Args>(args)...);
    }
    else
    {
        return obj.strokeRectArray(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) user_to_meta(T& obj, Args&&... args)
{
    if constexpr (requires { obj.user_to_meta(args...); })
    {
        return obj.user_to_meta(std::forward<Args>(args)...);
    }
    else
    {
        return obj.userToMeta(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
decltype(auto) user_transform(T& obj, Args&&... args)
{
    if constexpr (requires { obj.user_transform(args...); })
    {
        return obj.user_transform(std::forward<Args>(args)...);
    }
    else
    {
        return obj.userTransform(std::forward<Args>(args)...);
    }
}

template <typename T>
decltype(auto) dash_array(T& obj)
{
    if constexpr (requires { obj.dash_array; })
    {
        return (obj.dash_array);
    }
    else
    {
        return (obj.dashArray);
    }
}

template <typename T>
decltype(auto) dash_offset(T& obj)
{
    if constexpr (requires { obj.dash_offset; })
    {
        return (obj.dash_offset);
    }
    else
    {
        return (obj.dashOffset);
    }
}

template <typename T>
decltype(auto) thread_count(T& obj)
{
    if constexpr (requires { obj.thread_count; })
    {
        return (obj.thread_count);
    }
    else
    {
        return (obj.threadCount);
    }
}

}   // namespace blcompat

namespace pdf
{

class PDFBLPaintEngine : public QPaintEngine
{
public:
    explicit PDFBLPaintEngine(QImage& qtOffscreenBuffer, bool isMultithreaded);

    virtual bool begin(QPaintDevice*) override;
    virtual bool end() override;
    virtual void updateState(const QPaintEngineState& updatedState) override;
    virtual void drawRects(const QRect* rects, int rectCount) override;
    virtual void drawRects(const QRectF* rects, int rectCount) override;
    virtual void drawLines(const QLine* lines, int lineCount) override;
    virtual void drawLines(const QLineF* lines, int lineCount) override;
    virtual void drawEllipse(const QRectF& r) override;
    virtual void drawEllipse(const QRect& r) override;
    virtual void drawPath(const QPainterPath& path) override;
    virtual void drawPoints(const QPointF* points, int pointCount) override;
    virtual void drawPoints(const QPoint* points, int pointCount) override;
    virtual void drawPolygon(const QPointF* points, int pointCount, PolygonDrawMode mode) override;
    virtual void drawPolygon(const QPoint* points, int pointCount, PolygonDrawMode mode) override;
    virtual void drawPixmap(const QRectF& r, const QPixmap& pm, const QRectF& sr) override;
    virtual void drawTextItem(const QPointF& p, const QTextItem& textItem) override;
    virtual void drawTiledPixmap(const QRectF& r, const QPixmap& pixmap, const QPointF& s) override;
    virtual void drawImage(const QRectF& r, const QImage& pm, const QRectF& sr, Qt::ImageConversionFlags flags) override;
    virtual Type type() const override;

    static PaintEngineFeatures getStaticFeatures();

private:

    /// Get BL matrix from transformation
    static BLMatrix2D getBLMatrix(QTransform transform);

    static BLPoint getBLPoint(const QPoint& point);
    static BLPoint getBLPoint(const QPointF& point);

    /// Get BL rect from regular rect
    static BLRectI getBLRect(QRect rect);

    /// Get BL rect from regular rect
    static BLRect getBLRect(QRectF rect);

    /// Get BL path from path
    static BLPath getBLPath(const QPainterPath& path);

    /// Set pen to the context
    static void setBLPen(BLContext& context, const QPen& pen);

    /// Set brush to the context
    static void setBLBrush(BLContext& context, const QBrush& brush);

    /// Load font
    static bool loadBLFont(BLFont& font, QString fontName, PDFReal fontSize);

    /// Returns composition operator
    static BLCompOp getBLCompOp(QPainter::CompositionMode mode);

    void drawPathImpl(const QPainterPath& path, bool enableStroke, bool enableFill, bool forceFill = false);

    void setFillRule(Qt::FillRule fillRule);
    void updateFont(QFont newFont);
    void setPathFillMode(PolygonDrawMode mode, QPainterPath& path);
    void updateClipping(std::optional<QRegion> clipRegion,
                        std::optional<QPainterPath> clipPath,
                        Qt::ClipOperation clipOperation);

    bool isStrokeActive() const { return m_currentPen.style() != Qt::NoPen; }
    bool isFillActive() const { return m_currentBrush.style() != Qt::NoBrush; }

    enum class ClipMode
    {
        NoClip,
        NotVisible,
        NeedsResolve
    };

    ClipMode resolveClipping(const QLineF& line) const;
    ClipMode resolveClipping(const QPointF& point) const;
    ClipMode resolveClipping(const QRectF& rect) const;
    ClipMode resolveClipping(const QPainterPath& path) const;

    QImage& m_qtOffscreenBuffer;
    std::optional<BLContext> m_blContext;
    std::optional<BLImage> m_blOffscreenBuffer;
    bool m_isMultithreaded;

    QPen m_currentPen;
    QBrush m_currentBrush;
    QFont m_currentFont;
    QRawFont m_currentRawFont;
    QTransform m_currentTransform;

    bool m_currentIsClipEnabled = false;
    bool m_clipSingleRect = false;
    std::optional<QPainterPath> m_finalClipPath;
    QRectF m_finalClipPathBoundingBox;
};

PDFBLPaintDevice::PDFBLPaintDevice(QImage& offscreenBuffer, bool isMultithreaded) :
    m_offscreenBuffer(offscreenBuffer),
    m_paintEngine(new PDFBLPaintEngine(offscreenBuffer, isMultithreaded))
{

}

PDFBLPaintDevice::~PDFBLPaintDevice()
{
    delete m_paintEngine;
    m_paintEngine = nullptr;
}

int PDFBLPaintDevice::devType() const
{
    return QInternal::CustomRaster;
}

QPaintEngine* PDFBLPaintDevice::paintEngine() const
{
    return m_paintEngine;
}

uint32_t PDFBLPaintDevice::getVersion()
{
    return BL_VERSION;
}

int PDFBLPaintDevice::metric(PaintDeviceMetric metric) const
{
    switch (metric)
    {
    case QPaintDevice::PdmWidth:
        return m_offscreenBuffer.width();
    case QPaintDevice::PdmHeight:
        return m_offscreenBuffer.height();
    case QPaintDevice::PdmWidthMM:
        return m_offscreenBuffer.widthMM();
    case QPaintDevice::PdmHeightMM:
        return m_offscreenBuffer.heightMM();
    case QPaintDevice::PdmNumColors:
        return m_offscreenBuffer.colorCount();
    case QPaintDevice::PdmDepth:
        return m_offscreenBuffer.depth();
    case QPaintDevice::PdmDpiX:
        return m_offscreenBuffer.logicalDpiX();
    case QPaintDevice::PdmDpiY:
        return m_offscreenBuffer.logicalDpiY();
    case QPaintDevice::PdmPhysicalDpiX:
        return m_offscreenBuffer.physicalDpiX();
    case QPaintDevice::PdmPhysicalDpiY:
        return m_offscreenBuffer.physicalDpiY();
    case QPaintDevice::PdmDevicePixelRatio:
        return m_offscreenBuffer.devicePixelRatio();
    case QPaintDevice::PdmDevicePixelRatioScaled:
        return m_offscreenBuffer.devicePixelRatioFScale();
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    case QPaintDevice::PdmDevicePixelRatioF_EncodedA:
    case QPaintDevice::PdmDevicePixelRatioF_EncodedB:
        return QPaintDevice::encodeMetricF(metric, m_offscreenBuffer.devicePixelRatio());
#endif
    default:
        Q_ASSERT(false);
        break;
    }

    return 0;
}

PDFBLPaintEngine::PDFBLPaintEngine(QImage& qtOffscreenBuffer, bool isMultithreaded) :
    QPaintEngine(getStaticFeatures()),
    m_qtOffscreenBuffer(qtOffscreenBuffer),
    m_isMultithreaded(isMultithreaded)
{

}

bool PDFBLPaintEngine::begin(QPaintDevice*)
{
    if (isActive())
    {
        return false;
    }

    m_blContext.emplace();
    m_blOffscreenBuffer.emplace();

    BLContextCreateInfo info{};

    if (m_isMultithreaded)
    {
        info.flags = BL_CONTEXT_CREATE_FLAG_FALLBACK_TO_SYNC;
        blcompat::thread_count(info) = QThread::idealThreadCount();
    }

    blcompat::set_hint(*m_blContext, BL_CONTEXT_HINT_RENDERING_QUALITY, BL_RENDERING_QUALITY_MAX_VALUE);

    blcompat::create_from_data(*m_blOffscreenBuffer, m_qtOffscreenBuffer.width(), m_qtOffscreenBuffer.height(), BL_FORMAT_PRGB32, m_qtOffscreenBuffer.bits(), m_qtOffscreenBuffer.bytesPerLine());
    if (m_blContext->begin(m_blOffscreenBuffer.value(), info) == BL_SUCCESS)
    {
        blcompat::clear_all(*m_blContext);

        qreal devicePixelRatio = m_qtOffscreenBuffer.devicePixelRatioF();
        m_blContext->scale(devicePixelRatio);
        blcompat::user_to_meta(*m_blContext);

        setBLPen(m_blContext.value(), m_currentPen);
        setBLBrush(m_blContext.value(), m_currentBrush);
        updateFont(QFont());
        return true;
    }
    else
    {
        m_blContext.reset();
        m_blOffscreenBuffer.reset();
    }

    return false;
}

bool PDFBLPaintEngine::end()
{
    if (!isActive())
    {
        return false;
    }

    m_blContext->end();
    m_blContext.reset();
    m_blOffscreenBuffer.reset();
    return true;
}

void PDFBLPaintEngine::updateFont(QFont newFont)
{
    m_currentFont = newFont;
    m_currentFont.setHintingPreference(QFont::PreferNoHinting);

    PDFReal pixelSize = m_currentFont.pixelSize();
    if (pixelSize == -1)
    {
        PDFReal pointSizeF = m_currentFont.pointSizeF();
        PDFReal dpi = m_qtOffscreenBuffer.logicalDpiY();
        pixelSize = pointSizeF / 72 * dpi;
        m_currentFont.setPixelSize(pixelSize);
    }

    m_currentRawFont = QRawFont::fromFont(m_currentFont);
}

void PDFBLPaintEngine::updateState(const QPaintEngineState& updatedState)
{
    if (updatedState.state().testFlag(QPaintEngine::DirtyPen))
    {
        m_currentPen = updatedState.pen();
        setBLPen(m_blContext.value(), updatedState.pen());
    }

    if (updatedState.state().testFlag(QPaintEngine::DirtyBrush))
    {
        m_currentBrush = updatedState.brush();
        setBLBrush(m_blContext.value(), updatedState.brush());
    }

    if (updatedState.state().testFlag(QPaintEngine::DirtyCompositionMode))
    {
        blcompat::set_comp_op(*m_blContext, getBLCompOp(updatedState.compositionMode()));
    }

    if (updatedState.state().testFlag(QPaintEngine::DirtyOpacity))
    {
        blcompat::set_global_alpha(*m_blContext, updatedState.opacity());
    }

    if (updatedState.state().testFlag(QPaintEngine::DirtyTransform))
    {
        m_currentTransform = updatedState.transform();
        blcompat::set_transform(*m_blContext, getBLMatrix(updatedState.transform()));
    }

    if (updatedState.state().testFlag(QPaintEngine::DirtyFont))
    {
        updateFont(updatedState.font());
    }

    if (updatedState.state().testFlag(QPaintEngine::DirtyClipEnabled))
    {
        m_currentIsClipEnabled = updatedState.isClipEnabled();
    }

    if (updatedState.state().testFlag(QPaintEngine::DirtyHints))
    {
        // Do nothing
    }

    if (updatedState.state().testAnyFlags(QPaintEngine::DirtyClipPath | QPaintEngine::DirtyClipRegion))
    {
        std::optional<QRegion> clipRegion;
        std::optional<QPainterPath> clipPath;

        if (updatedState.state().testFlag(QPaintEngine::DirtyClipRegion))
        {
            clipRegion = updatedState.clipRegion();
        }

        if (updatedState.state().testFlag(QPaintEngine::DirtyClipPath))
        {
            clipPath = updatedState.clipPath();
        }

        updateClipping(std::move(clipRegion), std::move(clipPath), updatedState.clipOperation());
    }
}

void PDFBLPaintEngine::drawRects(const QRect* rects, int rectCount)
{
    QRect boundingRect;

    BLArray<BLRectI> blRects;
    blRects.reserve(rectCount);

    for (int i = 0; i < rectCount; ++i)
    {
        boundingRect = boundingRect.united(rects[i]);
        blRects.append(getBLRect(rects[i]));
    }

    QRect mappedBoundingRect = m_currentTransform.mapRect(boundingRect);
    ClipMode clipMode = resolveClipping(mappedBoundingRect);
    switch (clipMode)
    {
        case ClipMode::NoClip:
            break;

        case ClipMode::NotVisible:
            return;

        case ClipMode::NeedsResolve:
        {
            for (int i = 0; i < rectCount; ++i)
            {
                QPainterPath path;
                path.addRect(rects[i]);
                drawPathImpl(path, true, true);
            }
            return;
        }
    }

    if (isFillActive())
    {
        blcompat::fill_rect_array(*m_blContext, blRects.view());
    }

    if (isStrokeActive())
    {
        blcompat::stroke_rect_array(*m_blContext, blRects.view());
    }
}

void PDFBLPaintEngine::drawRects(const QRectF* rects, int rectCount)
{
    QRectF boundingRect;

    BLArray<BLRect> blRects;
    blRects.reserve(rectCount);

    for (int i = 0; i < rectCount; ++i)
    {
        boundingRect = boundingRect.united(rects[i]);
        blRects.append(getBLRect(rects[i]));
    }

    QRectF mappedBoundingRect = m_currentTransform.mapRect(boundingRect);
    ClipMode clipMode = resolveClipping(mappedBoundingRect);
    switch (clipMode)
    {
        case ClipMode::NoClip:
            break;

        case ClipMode::NotVisible:
            return;

        case ClipMode::NeedsResolve:
        {
            for (int i = 0; i < rectCount; ++i)
            {
                QPainterPath path;
                path.addRect(rects[i]);
                drawPathImpl(path, true, true);
            }
            return;
        }
    }

    if (isFillActive())
    {
        blcompat::fill_rect_array(*m_blContext, blRects.view());
    }

    if (isStrokeActive())
    {
        blcompat::stroke_rect_array(*m_blContext, blRects.view());
    }
}

void PDFBLPaintEngine::drawLines(const QLine* lines, int lineCount)
{
    if (!isStrokeActive())
    {
        return;
    }

    for (int i = 0; i < lineCount; ++i)
    {
        const QLine& line = lines[i];
        ClipMode clipMode = resolveClipping(m_currentTransform.map(line));

        switch (clipMode)
        {
            case ClipMode::NoClip:
                // Do as normal
                break;

            case ClipMode::NotVisible:
                // Graphics is not visible
                return;

            case ClipMode::NeedsResolve:
            {
                QLineF lineF = line.toLineF();
                if (m_finalClipPath->isEmpty() || qFuzzyIsNull(lineF.length()))
                {
                    return;
                }

                QLineF normalVectorLine = lineF.normalVector().unitVector();
                QPointF normalVector = normalVectorLine.p2() - normalVectorLine.p1();
                qreal widthF = m_currentPen.widthF() * 0.5;

                QPainterPath path;
                path.moveTo(lineF.p1() + normalVector * widthF);
                path.lineTo(lineF.p2() + normalVector * widthF);
                path.lineTo(lineF.p2() - normalVector * widthF);
                path.lineTo(lineF.p1() - normalVector * widthF);
                path.closeSubpath();

                drawPathImpl(path, true, false);
                return;
            }
        }

        blcompat::stroke_line(*m_blContext, line.x1(), line.y1(), line.x2(), line.y2());
    }
}

void PDFBLPaintEngine::drawLines(const QLineF* lines, int lineCount)
{
    if (!isStrokeActive())
    {
        return;
    }

    for (int i = 0; i < lineCount; ++i)
    {
        const QLineF& line = lines[i];

        ClipMode clipMode = resolveClipping(m_currentTransform.map(line));
        switch (clipMode)
        {
            case ClipMode::NoClip:
                // Do as normal
                break;

            case ClipMode::NotVisible:
                // Graphics is not visible
                return;

            case ClipMode::NeedsResolve:
            {
                if (m_finalClipPath->isEmpty() || qFuzzyIsNull(line.length()))
                {
                    return;
                }

                QLineF normalVectorLine = line.normalVector().unitVector();
                QPointF normalVector = normalVectorLine.p2() - normalVectorLine.p1();
                qreal widthF = m_currentPen.widthF() * 0.5;

                QPainterPath path;
                path.moveTo(line.p1() + normalVector * widthF);
                path.lineTo(line.p2() + normalVector * widthF);
                path.lineTo(line.p2() - normalVector * widthF);
                path.lineTo(line.p1() - normalVector * widthF);
                path.closeSubpath();

                drawPathImpl(path, true, false);
                return;
            }
        }

        blcompat::stroke_line(*m_blContext, line.x1(), line.y1(), line.x2(), line.y2());
    }
}

void PDFBLPaintEngine::drawEllipse(const QRectF& r)
{
    QPainterPath path;
    path.addEllipse(r);
    drawPathImpl(path, true, true);
}

void PDFBLPaintEngine::drawEllipse(const QRect& r)
{
    QPainterPath path;
    path.addEllipse(r);
    drawPathImpl(path, true, true);
}

void PDFBLPaintEngine::drawPath(const QPainterPath& path)
{
    drawPathImpl(path, true, true);
}

void PDFBLPaintEngine::drawPathImpl(const QPainterPath& path, bool enableStroke, bool enableFill, bool forceFill)
{
    QPainterPath transformedPath = m_currentTransform.map(path);
    ClipMode clipMode = resolveClipping(transformedPath);

    setFillRule(path.fillRule());

    switch (clipMode)
    {
        case ClipMode::NoClip:
            // Do as normal
            break;

        case ClipMode::NotVisible:
            // Graphics is not visible
            return;

        case ClipMode::NeedsResolve:
        {
            if (m_finalClipPath->isEmpty())
            {
                return;
            }

            if ((isFillActive() && enableFill) || forceFill)
            {
                QPainterPath fillPath = transformedPath.intersected(m_finalClipPath.value());

                if (!fillPath.isEmpty())
                {
                    m_blContext->save();
                    blcompat::reset_transform(*m_blContext);
                    blcompat::fill_path(*m_blContext, getBLPath(fillPath));
                    m_blContext->restore();
                }
            }

            if (isStrokeActive() && enableStroke)
            {
                QPainterPathStroker stroker(m_currentPen);
                QPainterPath strokedPath = stroker.createStroke(path);
                QPainterPath transformedStrokedPath = m_currentTransform.map(strokedPath);
                QPainterPath finalTransformedStrokedPath = transformedStrokedPath.intersected(m_finalClipPath.value());

                if (!finalTransformedStrokedPath.isEmpty())
                {
                    m_blContext->save();
                    blcompat::reset_transform(*m_blContext);
                    setBLBrush(m_blContext.value(), m_currentPen.brush());
                    blcompat::fill_path(*m_blContext, getBLPath(finalTransformedStrokedPath));
                    m_blContext->restore();
                }
            }

            return;
        }
    }

    BLPath blPath = getBLPath(path);

    if ((isFillActive() && enableFill) || forceFill)
    {
        blcompat::fill_path(*m_blContext, blPath);
    }

    if (isStrokeActive() && enableStroke)
    {
        blcompat::stroke_path(*m_blContext, blPath);
    }
}

void PDFBLPaintEngine::drawPoints(const QPointF* points, int pointCount)
{
    m_blContext->save();
    blcompat::set_fill_style(*m_blContext, BLRgba32(m_currentPen.color().rgba()));

    for (int i = 0; i < pointCount; ++i)
    {
        const QPointF& c = points[i];

        if (resolveClipping(m_currentTransform.map(c)) == ClipMode::NotVisible)
        {
            continue;
        }

        BLEllipse blEllipse(c.x(), c.y(), m_currentPen.widthF() * 0.5, m_currentPen.widthF() * 0.5);
        blcompat::fill_ellipse(*m_blContext, blEllipse);
    }

    m_blContext->restore();
}

void PDFBLPaintEngine::drawPoints(const QPoint* points, int pointCount)
{
    m_blContext->save();
    blcompat::set_fill_style(*m_blContext, BLRgba32(m_currentPen.color().rgba()));

    for (int i = 0; i < pointCount; ++i)
    {
        const QPointF& c = points[i];

        if (resolveClipping(m_currentTransform.map(c)) == ClipMode::NotVisible)
        {
            continue;
        }

        BLEllipse blEllipse(c.x(), c.y(), m_currentPen.widthF() * 0.5, m_currentPen.widthF() * 0.5);
        blcompat::fill_ellipse(*m_blContext, blEllipse);
    }

    m_blContext->restore();
}

void PDFBLPaintEngine::drawPolygon(const QPointF* points, int pointCount, PolygonDrawMode mode)
{
    QPainterPath path;
    QPolygonF polygon;
    polygon.assign(points, points + pointCount);
    path.addPolygon(polygon);

    setPathFillMode(mode, path);

    drawPathImpl(path, true, mode != QPaintEngine::PolylineMode);
}

void PDFBLPaintEngine::drawPolygon(const QPoint* points, int pointCount, PolygonDrawMode mode)
{
    QPainterPath path;
    QPolygonF polygon;
    polygon.assign(points, points + pointCount);
    path.addPolygon(polygon);

    setPathFillMode(mode, path);

    drawPathImpl(path, true, mode != QPaintEngine::PolylineMode);
}

void PDFBLPaintEngine::drawPixmap(const QRectF& r, const QPixmap& pm, const QRectF& sr)
{
    drawImage(r, pm.toImage(), sr, Qt::ImageConversionFlags());
}

void PDFBLPaintEngine::drawTextItem(const QPointF& p, const QTextItem& textItem)
{
    // We will call the base implementation, which will
    // create paths from text and draw these paths.
    QPaintEngine::drawTextItem(p, textItem);
}

void PDFBLPaintEngine::drawTiledPixmap(const QRectF& r, const QPixmap& pixmap, const QPointF& s)
{
    QImage image = pixmap.toImage();

    if (image.format() != QImage::Format_ARGB32_Premultiplied)
    {
        image.convertTo(QImage::Format_ARGB32_Premultiplied);
    }

    int tilesX = qCeil(r.width() / pixmap.width());
    int tilesY = qCeil(r.height() / pixmap.height());

    BLImage blImage;
    blcompat::create_from_data(blImage, image.width(), image.height(), BL_FORMAT_PRGB32, image.bits(), image.bytesPerLine());

    BLImage blDrawImage;
    blcompat::assign_deep(blDrawImage, blImage);

    for (int x = 0; x < tilesX; ++x)
    {
        for (int y = 0; y < tilesY; ++y)
        {
            QPointF tilePos = QPointF(r.left() + x * pixmap.width() + s.x(),
                                      r.top() + y * pixmap.height() + s.y());

            if (tilePos.x() < r.right() && tilePos.y() < r.bottom())
            {
                blcompat::blit_image(*m_blContext, getBLPoint(tilePos), blDrawImage);
            }
        }
    }
}

void PDFBLPaintEngine::drawImage(const QRectF& r, const QImage& pm, const QRectF& sr, Qt::ImageConversionFlags flags)
{
    Q_UNUSED(flags);

    QRectF transformedRect = m_currentTransform.mapRect(r);
    ClipMode clipMode = resolveClipping(transformedRect);

    if (clipMode == ClipMode::NotVisible)
    {
        return;
    }

    QImage image = pm;

    if (image.format() != QImage::Format_ARGB32_Premultiplied)
    {
        image.convertTo(QImage::Format_ARGB32_Premultiplied);
    }

    if (clipMode == ClipMode::NeedsResolve)
    {
        QImage mask(image.size(), QImage::Format_ARGB32);
        mask.fill(Qt::transparent);

        QPainter maskPainter(&mask);
        maskPainter.setCompositionMode(QPainter::CompositionMode_Source);

        QPainterPath path = m_finalClipPath.value();
        path = m_currentTransform.inverted().map(path);

        maskPainter.fillPath(path, Qt::white);
        maskPainter.end();

        QPainter imagePainter(&image);
        imagePainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        imagePainter.drawImage(0, 0, mask);
        imagePainter.end();
    }

    BLImage blImage;
    blcompat::create_from_data(blImage, image.width(), image.height(), BL_FORMAT_PRGB32, image.bits(), image.bytesPerLine());

    BLImage blDrawImage;
    blcompat::assign_deep(blDrawImage, blImage);

    blcompat::blit_image(*m_blContext, BLRect(r.x(), r.y(), r.width(), r.height()),
                         blDrawImage,
                         BLRectI(sr.x(), sr.y(), sr.width(), sr.height()));
}

QPaintEngine::Type PDFBLPaintEngine::type() const
{
    return User;
}

QPaintEngine::PaintEngineFeatures PDFBLPaintEngine::getStaticFeatures()
{
    return PrimitiveTransform | PixmapTransform | LinearGradientFill |
           RadialGradientFill | ConicalGradientFill | AlphaBlend |
           PorterDuff | PainterPaths | Antialiasing | ConstantOpacity |
           BlendModes | PaintOutsidePaintEvent;
}


BLMatrix2D PDFBLPaintEngine::getBLMatrix(QTransform transform)
{
    BLMatrix2D matrix;
    matrix.reset(transform.m11(), transform.m12(), transform.m21(), transform.m22(), transform.dx(), transform.dy());
    return matrix;
}

BLPoint PDFBLPaintEngine::getBLPoint(const QPoint& point)
{
    return BLPoint(point.x(), point.y());
}

BLPoint PDFBLPaintEngine::getBLPoint(const QPointF& point)
{
    return BLPoint(point.x(), point.y());
}

BLRectI PDFBLPaintEngine::getBLRect(QRect rect)
{
    return BLRectI(rect.x(), rect.y(), rect.width(), rect.height());
}

BLRect PDFBLPaintEngine::getBLRect(QRectF rect)
{
    return BLRect(rect.x(), rect.y(), rect.width(), rect.height());
}

BLPath PDFBLPaintEngine::getBLPath(const QPainterPath& path)
{
    BLPath blPath;

    int elementCount = path.elementCount();
    for (int i = 0; i < elementCount; ++i) {
        const QPainterPath::Element& element = path.elementAt(i);

        switch (element.type)
        {
        case QPainterPath::MoveToElement:
            blcompat::move_to(blPath, element.x, element.y);
            break;

        case QPainterPath::LineToElement:
            blcompat::line_to(blPath, element.x, element.y);
            break;

        case QPainterPath::CurveToElement:
            if (i + 2 < elementCount)
            {
                const QPainterPath::Element& ctrlPoint1 = path.elementAt(i++);
                const QPainterPath::Element& ctrlPoint2 = path.elementAt(i++);
                const QPainterPath::Element& endPoint = path.elementAt(i);
                blcompat::cubic_to(blPath, ctrlPoint1.x, ctrlPoint1.y, ctrlPoint2.x, ctrlPoint2.y, endPoint.x, endPoint.y);
            }
            break;

        case QPainterPath::CurveToDataElement:
            Q_ASSERT(false);
            break;
        }
    }

    return blPath;
}

void PDFBLPaintEngine::setBLPen(BLContext& context, const QPen& pen)
{
    const Qt::PenCapStyle capStyle = pen.capStyle();
    const Qt::PenJoinStyle joinStyle = pen.joinStyle();
    const QColor color = pen.color();
    const qreal width = pen.widthF();
    const qreal miterLimit = pen.miterLimit();
    const qreal dashOffset = pen.dashOffset();
    const QList<qreal> customDashPattern = pen.dashPattern();
    const Qt::PenStyle penStyle = pen.style();

    blcompat::set_stroke_alpha(context, pen.color().alphaF());
    blcompat::set_stroke_width(context, width);
    blcompat::set_stroke_miter_limit(context, miterLimit);

    switch (capStyle)
    {
    case Qt::FlatCap:
        blcompat::set_stroke_caps(context, BL_STROKE_CAP_BUTT);
        break;
    case Qt::SquareCap:
        blcompat::set_stroke_caps(context, BL_STROKE_CAP_SQUARE);
        break;
    case Qt::RoundCap:
        blcompat::set_stroke_caps(context, BL_STROKE_CAP_ROUND);
        break;
    default:
        Q_ASSERT(false);
        break;
    }

    BLArray<double> dashArray;

    for (double value : customDashPattern)
    {
        dashArray.append(value);
    }

    blcompat::set_stroke_dash_offset(context, dashOffset);
    blcompat::set_stroke_dash_array(context, dashArray);

    switch (joinStyle)
    {
    case Qt::MiterJoin:
        blcompat::set_stroke_join(context, BL_STROKE_JOIN_MITER_CLIP);
        break;
    case Qt::BevelJoin:
        blcompat::set_stroke_join(context, BL_STROKE_JOIN_BEVEL);
        break;
    case Qt::RoundJoin:
        blcompat::set_stroke_join(context, BL_STROKE_JOIN_ROUND);
        break;
    case Qt::SvgMiterJoin:
        blcompat::set_stroke_join(context, BL_STROKE_JOIN_MITER_CLIP);
        break;
    default:
        Q_ASSERT(false);
        break;
    }

    blcompat::set_stroke_style(context, BLRgba32(color.rgba()));

    BLStrokeOptions strokeOptions = blcompat::stroke_options(context);

    switch (penStyle)
    {
    case Qt::SolidLine:
        blcompat::dash_array(strokeOptions).clear();
        blcompat::dash_offset(strokeOptions) = 0.0;
        break;

    case Qt::DashLine:
    {
        constexpr double dashPattern[] = {4, 4};
        blcompat::assign_data(blcompat::dash_array(strokeOptions), dashPattern, std::size(dashPattern));
        break;
    }

    case Qt::DotLine:
    {
        constexpr double dashPattern[] = {1, 3};
        blcompat::assign_data(blcompat::dash_array(strokeOptions), dashPattern, std::size(dashPattern));
        break;
    }

    case Qt::DashDotLine:
    {
        constexpr double dashPattern[] = {4, 2, 1, 2};
        blcompat::assign_data(blcompat::dash_array(strokeOptions), dashPattern, std::size(dashPattern));
        break;
    }

    case Qt::DashDotDotLine:
    {
        constexpr double dashPattern[] = {4, 2, 1, 2, 1, 2};
        blcompat::assign_data(blcompat::dash_array(strokeOptions), dashPattern, std::size(dashPattern));
        break;
    }

    case Qt::CustomDashLine:
    {
        auto dashPattern = pen.dashPattern();
        blcompat::assign_data(blcompat::dash_array(strokeOptions), dashPattern.data(), dashPattern.size());
        blcompat::dash_offset(strokeOptions) = pen.dashOffset();
        break;
    }

    default:
        break;
    }

    blcompat::set_stroke_options(context, strokeOptions);
}

void PDFBLPaintEngine::setBLBrush(BLContext& context, const QBrush& brush)
{
    auto setGradientStops = [](BLGradient& blGradient, const auto& qGradient)
    {
        QVector<BLGradientStop> stops;
        for (const auto& stop : qGradient.stops())
        {
            stops.append(BLGradientStop(stop.first, BLRgba32(stop.second.red(), stop.second.green(), stop.second.blue(), stop.second.alpha())));
        }
        blcompat::assign_stops(blGradient, stops.constData(), stops.size());
    };

    switch (brush.style())
    {
        default:
        case Qt::SolidPattern:
        {
            QColor color = brush.color();
            BLRgba32 blColor = BLRgba32(color.red(), color.green(), color.blue(), color.alpha());
            blcompat::set_fill_style(context, blColor);
            break;
        }
        case Qt::LinearGradientPattern:
        {
            const QGradient* gradient = brush.gradient();
            if (gradient && gradient->type() == QGradient::LinearGradient)
            {
                const QLinearGradient* linearGradient = static_cast<const QLinearGradient*>(gradient);
                BLLinearGradientValues blLinearGradient;
                blLinearGradient.x0 = linearGradient->start().x();
                blLinearGradient.y0 = linearGradient->start().y();
                blLinearGradient.x1 = linearGradient->finalStop().x();
                blLinearGradient.y1 = linearGradient->finalStop().y();
                BLGradient blGradient(blLinearGradient);
                setGradientStops(blGradient, *gradient);
                blcompat::set_fill_style(context, blGradient);
            }
            break;
        }
        case Qt::RadialGradientPattern:
        {
            const QGradient* gradient = brush.gradient();
            if (gradient && gradient->type() == QGradient::RadialGradient)
            {
                const QRadialGradient* radialGradient = static_cast<const QRadialGradient*>(gradient);
                BLRadialGradientValues blRadialGradientValues;
                blRadialGradientValues.x0 = radialGradient->center().x();
                blRadialGradientValues.y0 = radialGradient->center().y();
                blRadialGradientValues.x1 = radialGradient->focalPoint().x();
                blRadialGradientValues.y1 = radialGradient->focalPoint().y();
                blRadialGradientValues.r0 = radialGradient->radius();
                BLGradient blGradient(blRadialGradientValues);
                setGradientStops(blGradient, *gradient);
                blcompat::set_fill_style(context, blGradient);
            }
            break;
        }
    }
}

bool PDFBLPaintEngine::loadBLFont(BLFont& font, QString fontName, PDFReal fontSize)
{
    QByteArray data = PDFSystemFont::getFontData(fontName.toLatin1());

    BLFontData blFontData;
    if (blcompat::create_from_data(blFontData, data.data(), data.size()) == BL_SUCCESS)
    {
        BLFontFace fontFace;
        if (blcompat::create_from_data(fontFace, blFontData, 0) == BL_SUCCESS)
        {
            if (blcompat::create_from_face(font, fontFace, fontSize) == BL_SUCCESS)
            {
                return true;
            }
        }
    }

    return false;
}

BLCompOp PDFBLPaintEngine::getBLCompOp(QPainter::CompositionMode mode)
{
    switch (mode)
    {
    case QPainter::CompositionMode_SourceOver:
        return BL_COMP_OP_SRC_OVER;
    case QPainter::CompositionMode_DestinationOver:
        return BL_COMP_OP_DST_OVER;
    case QPainter::CompositionMode_Clear:
        return BL_COMP_OP_CLEAR;
    case QPainter::CompositionMode_Source:
        return BL_COMP_OP_SRC_COPY;
    case QPainter::CompositionMode_Destination:
        return BL_COMP_OP_DST_COPY;
    case QPainter::CompositionMode_SourceIn:
        return BL_COMP_OP_SRC_IN;
    case QPainter::CompositionMode_DestinationIn:
        return BL_COMP_OP_DST_IN;
    case QPainter::CompositionMode_SourceOut:
        return BL_COMP_OP_SRC_OUT;
    case QPainter::CompositionMode_DestinationOut:
        return BL_COMP_OP_DST_OUT;
    case QPainter::CompositionMode_SourceAtop:
        return BL_COMP_OP_SRC_ATOP;
    case QPainter::CompositionMode_DestinationAtop:
        return BL_COMP_OP_DST_ATOP;
    case QPainter::CompositionMode_Xor:
        return BL_COMP_OP_XOR;
    case QPainter::CompositionMode_Plus:
        return BL_COMP_OP_PLUS;
    case QPainter::CompositionMode_Multiply:
        return BL_COMP_OP_MULTIPLY;
    case QPainter::CompositionMode_Screen:
        return BL_COMP_OP_SCREEN;
    case QPainter::CompositionMode_Overlay:
        return BL_COMP_OP_OVERLAY;
    case QPainter::CompositionMode_Darken:
        return BL_COMP_OP_DARKEN;
    case QPainter::CompositionMode_Lighten:
        return BL_COMP_OP_LIGHTEN;
    case QPainter::CompositionMode_ColorDodge:
        return BL_COMP_OP_COLOR_DODGE;
    case QPainter::CompositionMode_ColorBurn:
        return BL_COMP_OP_COLOR_BURN;
    case QPainter::CompositionMode_HardLight:
        return BL_COMP_OP_HARD_LIGHT;
    case QPainter::CompositionMode_SoftLight:
        return BL_COMP_OP_SOFT_LIGHT;
    case QPainter::CompositionMode_Difference:
        return BL_COMP_OP_DIFFERENCE;
    case QPainter::CompositionMode_Exclusion:
        return BL_COMP_OP_EXCLUSION;
    default:
        break;
    }

    return BL_COMP_OP_SRC_OVER;
}

void PDFBLPaintEngine::setPathFillMode(PolygonDrawMode mode, QPainterPath& path)
{
    switch (mode)
    {
    case QPaintEngine::OddEvenMode:
        path.setFillRule(Qt::OddEvenFill);
        break;
    case QPaintEngine::WindingMode:
        path.setFillRule(Qt::WindingFill);
        break;
    case QPaintEngine::ConvexMode:
        path.setFillRule(Qt::OddEvenFill);
        break;
    case QPaintEngine::PolylineMode:
        path.setFillRule(Qt::OddEvenFill);
        break;
    }
}

void PDFBLPaintEngine::updateClipping(std::optional<QRegion> clipRegion,
                                      std::optional<QPainterPath> clipPath,
                                      Qt::ClipOperation clipOperation)
{
    QPainterPath finalClipPath;

    if (clipRegion.has_value())
    {
        finalClipPath.addRegion(clipRegion.value());
    }

    if (clipPath.has_value())
    {
        finalClipPath.addPath(clipPath.value());
    }

    finalClipPath = m_currentTransform.map(finalClipPath);

    switch (clipOperation)
    {
        case Qt::NoClip:
            m_finalClipPath.reset();
            m_finalClipPathBoundingBox = QRectF();
            m_clipSingleRect = false;
            blcompat::restore_clipping(*m_blContext);
            return;

        case Qt::ReplaceClip:
        {
            m_finalClipPath = std::move(finalClipPath);
            break;
        }

        case Qt::IntersectClip:
        {
            if (m_finalClipPath.has_value())
            {
                m_finalClipPath = m_finalClipPath->intersected(finalClipPath);
            }
            else
            {
                m_finalClipPath = std::move(finalClipPath);
            }
            break;
        }
    }

    m_clipSingleRect = false;
    m_finalClipPathBoundingBox = m_finalClipPath->controlPointRect();

    if (m_finalClipPath->elementCount() == 5)
    {
        QRectF testRect = m_finalClipPathBoundingBox.adjusted(1.0, 1.0, -2.0, -2.0);
        m_clipSingleRect = m_finalClipPath->contains(testRect);
    }

    if (m_clipSingleRect)
    {
        BLMatrix2D matrix = blcompat::user_transform(*m_blContext);
        blcompat::reset_transform(*m_blContext);
        blcompat::clip_to_rect(*m_blContext, getBLRect(m_finalClipPath->boundingRect()));
        blcompat::set_transform(*m_blContext, matrix);
    }
    else
    {
        blcompat::restore_clipping(*m_blContext);
    }
}

PDFBLPaintEngine::ClipMode PDFBLPaintEngine::resolveClipping(const QLineF& line) const
{
    if (!m_currentIsClipEnabled || m_clipSingleRect || !m_finalClipPath.has_value())
    {
        return ClipMode::NoClip;
    }

    if (m_finalClipPath->isEmpty())
    {
        return ClipMode::NotVisible;
    }

    QRectF clipRect = m_finalClipPathBoundingBox;

    qreal minX = qMin(line.x1(), line.x2());
    qreal maxX = qMax(line.x1(), line.x2());
    qreal minY = qMin(line.y1(), line.y2());
    qreal maxY = qMax(line.y1(), line.y2());

    QRectF rect(minX, minY, maxX - minX + 1.0, maxY - minY + 1.0);

    if (!rect.intersects(clipRect))
    {
        return ClipMode::NotVisible;
    }

    return ClipMode::NeedsResolve;
}

PDFBLPaintEngine::ClipMode PDFBLPaintEngine::resolveClipping(const QPointF& point) const
{
    if (!m_currentIsClipEnabled || m_clipSingleRect || !m_finalClipPath.has_value())
    {
        return ClipMode::NoClip;
    }

    if (m_finalClipPath->isEmpty())
    {
        return ClipMode::NotVisible;
    }

    if (!m_finalClipPath->contains(point))
    {
        return ClipMode::NotVisible;
    }

    return ClipMode::NoClip;
}

PDFBLPaintEngine::ClipMode PDFBLPaintEngine::resolveClipping(const QRectF& rect) const
{
    if (!m_currentIsClipEnabled || m_clipSingleRect || !m_finalClipPath.has_value())
    {
        return ClipMode::NoClip;
    }

    if (m_finalClipPath->isEmpty())
    {
        return ClipMode::NotVisible;
    }

    QRectF clipRect = m_finalClipPathBoundingBox;

    if (!rect.intersects(clipRect))
    {
        return ClipMode::NotVisible;
    }

    return ClipMode::NeedsResolve;
}

PDFBLPaintEngine::ClipMode PDFBLPaintEngine::resolveClipping(const QPainterPath& path) const
{
    if (!m_currentIsClipEnabled || m_clipSingleRect || !m_finalClipPath.has_value())
    {
        return ClipMode::NoClip;
    }

    if (m_finalClipPath->isEmpty())
    {
        return ClipMode::NotVisible;
    }

    QRectF clipRect = m_finalClipPathBoundingBox;
    QRectF pathRect = path.controlPointRect();

    if (!pathRect.intersects(clipRect))
    {
        return ClipMode::NotVisible;
    }

    return ClipMode::NeedsResolve;
}

void PDFBLPaintEngine::setFillRule(Qt::FillRule fillRule)
{
    BLFillRule blFillRule{};

    switch (fillRule)
    {
    case Qt::OddEvenFill:
        blFillRule = BL_FILL_RULE_EVEN_ODD;
        break;

    case Qt::WindingFill:
        blFillRule = BL_FILL_RULE_NON_ZERO;
        break;

    default:
        Q_ASSERT(false);
        break;
    }

    blcompat::set_fill_rule(*m_blContext, blFillRule);
}

}   // namespace pdf
