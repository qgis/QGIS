/***************************************************************************
  qgstextrenderer.h
  -----------------
   begin                : September 2015
   copyright            : (C) Nyall Dawson
   email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTEXTRENDERER_H
#define QGSTEXTRENDERER_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgstextblock.h"
#include "qgsunittypes.h"
#include "qgsmapunitscale.h"
#include "qgis.h"

#include <QPicture>

class QgsTextDocument;
class QgsTextDocumentMetrics;
class QgsTextFormat;
class QgsRenderContext;

class QFontMetricsF;

/**
 * \class QgsTextRenderer
  * \ingroup core
  * \brief Handles rendering text using rich formatting options, including drop shadows, buffers
  * and background shapes.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsTextRenderer
{
  public:

    /**
     * Converts a Qt horizontal \a alignment flag to a Qgis::TextHorizontalAlignment value.
     *
     * \see convertQtVAlignment()
     * \since QGIS 3.16
     */
    static Qgis::TextHorizontalAlignment convertQtHAlignment( Qt::Alignment alignment );

    /**
     * Converts a Qt vertical \a alignment flag to a Qgis::TextVerticalAlignment value.
     *
     * \see convertQtHAlignment()
     * \since QGIS 3.16
     */
    static Qgis::TextVerticalAlignment convertQtVAlignment( Qt::Alignment alignment );

    /**
     * Calculates pixel size (considering output size should be in pixel or map units, scale factors and optionally oversampling)
     * \param size size to convert
     * \param c rendercontext
     * \param unit size units
     * \param mapUnitScale a mapUnitScale clamper
     * \returns font pixel size
     */
    static int sizeToPixel( double size, const QgsRenderContext &c, QgsUnitTypes::RenderUnit unit, const QgsMapUnitScale &mapUnitScale = QgsMapUnitScale() );

    // TODO QGIS 4.0 -- remove drawAsOutlines from below methods!

    /**
     * Draws text within a rectangle using the specified settings.
     * \param rect destination rectangle for text
     * \param rotation text rotation
     * \param alignment horizontal alignment
     * \param textLines list of lines of text to draw
     * \param context render context
     * \param format text format
     * \param drawAsOutlines set to FALSE to render text as text. This allows outputs to
     * formats like SVG to maintain text as text objects, but at the cost of degraded
     * rendering and may result in side effects like misaligned text buffers. This setting is deprecated and has no effect
     * as of QGIS 3.4.3 and the text format should be set using QgsRenderContext::setTextRenderFormat() instead.
     * \param vAlignment vertical alignment (since QGIS 3.16)
     * \param flags text rendering flags (since QGIS 3.24)
     * \param mode text layout mode. Only Qgis::TextLayoutMode::Rectangle, Qgis::TextLayoutMode::RectangleCapHeightBased and Qgis::TextLayoutMode::RectangleAscentBased are accepted (since QGIS 3.30)
     *
     * \see drawDocument(), which is more efficient if the text document and metrics have already been calculated.
     */
    static void drawText( const QRectF &rect, double rotation, Qgis::TextHorizontalAlignment alignment, const QStringList &textLines,
                          QgsRenderContext &context, const QgsTextFormat &format,
                          bool drawAsOutlines = true, Qgis::TextVerticalAlignment vAlignment = Qgis::TextVerticalAlignment::Top,
                          Qgis::TextRendererFlags flags = Qgis::TextRendererFlags(),
                          Qgis::TextLayoutMode mode = Qgis::TextLayoutMode::Rectangle );

    /**
     * Draws a text document within a rectangle using the specified settings.
     *
     * Calling this method is more efficient than calling drawText() if the text document and metrics have already
     * been calculated.
     *
     * \warning Unlike drawText(), this method does not automatically update data defined properties in the text \a format. This
     * is the caller's responsibility to do, and must be done prior to generating the text \a document and \a metrics.
     *
     * \param rect destination rectangle for text
     * \param format base text format
     * \param document text document to draw
     * \param metrics precalculated text metrics
     * \param context destination render context
     * \param horizontalAlignment horizontal alignment
     * \param verticalAlignment vertical alignment
     * \param rotation text rotation
     * \param mode text layout mode. Only Qgis::TextLayoutMode::Rectangle, Qgis::TextLayoutMode::RectangleCapHeightBased and Qgis::TextLayoutMode::RectangleAscentBased are accepted.
     * \param flags text rendering flags
     *
     * \since QGIS 3.30
     */
    static void drawDocument( const QRectF &rect,
                              const QgsTextFormat &format,
                              const QgsTextDocument &document,
                              const QgsTextDocumentMetrics &metrics,
                              QgsRenderContext &context,
                              Qgis::TextHorizontalAlignment horizontalAlignment = Qgis::TextHorizontalAlignment::Left,
                              Qgis::TextVerticalAlignment verticalAlignment = Qgis::TextVerticalAlignment::Top,
                              double rotation = 0,
                              Qgis::TextLayoutMode mode = Qgis::TextLayoutMode::Rectangle,
                              Qgis::TextRendererFlags flags = Qgis::TextRendererFlags() );

    /**
     * Draws text at a point origin using the specified settings.
     * \param point origin of text
     * \param rotation text rotation
     * \param alignment horizontal alignment
     * \param textLines list of lines of text to draw
     * \param context render context
     * \param format text format
     * \param drawAsOutlines set to FALSE to render text as text. This allows outputs to
     * formats like SVG to maintain text as text objects, but at the cost of degraded
     * rendering and may result in side effects like misaligned text buffers. This setting is deprecated and has no effect
     * as of QGIS 3.4.3 and the text format should be set using QgsRenderContext::setTextRenderFormat() instead.
     */
    static void drawText( QPointF point, double rotation, Qgis::TextHorizontalAlignment alignment, const QStringList &textLines,
                          QgsRenderContext &context, const QgsTextFormat &format,
                          bool drawAsOutlines = true );

    /**
     * Draws a single component of rendered text using the specified settings.
     * \param rect destination rectangle for text
     * \param rotation text rotation
     * \param alignment horizontal alignment
     * \param textLines list of lines of text to draw
     * \param context render context
     * \param format text format
     * \param part component of text to draw. Note that Shadow parts cannot be drawn
     * individually and instead are drawn with their associated part (e.g., drawn together
     * with the text or background parts)
     * \param drawAsOutlines set to FALSE to render text as text. This allows outputs to
     * formats like SVG to maintain text as text objects, but at the cost of degraded
     * rendering and may result in side effects like misaligned text buffers. This setting is deprecated and has no effect
     * as of QGIS 3.4.3 and the text format should be set using QgsRenderContext::setTextRenderFormat() instead.
     *
     * \deprecated Private API only, will be removed in 4.0
     */
    Q_DECL_DEPRECATED static void drawPart( const QRectF &rect, double rotation, Qgis::TextHorizontalAlignment alignment, const QStringList &textLines,
                                            QgsRenderContext &context, const QgsTextFormat &format,
                                            Qgis::TextComponent part, bool drawAsOutlines = true ) SIP_DEPRECATED;

    /**
     * Draws a single component of rendered text using the specified settings.
     * \param origin origin for start of text. Y coordinate will be used as baseline.
     * \param rotation text rotation
     * \param alignment horizontal alignment
     * \param textLines list of lines of text to draw
     * \param context render context
     * \param format text format
     * \param part component of text to draw. Note that Shadow parts cannot be drawn
     * individually and instead are drawn with their associated part (e.g., drawn together
     * with the text or background parts)
     * \param drawAsOutlines set to FALSE to render text as text. This allows outputs to
     * formats like SVG to maintain text as text objects, but at the cost of degraded
     * rendering and may result in side effects like misaligned text buffers. This setting is deprecated and has no effect
     * as of QGIS 3.4.3 and the text format should be set using QgsRenderContext::setTextRenderFormat() instead.
     *
     * \deprecated Private API only, will be removed in 4.0
     */
    Q_DECL_DEPRECATED static void drawPart( QPointF origin, double rotation, Qgis::TextHorizontalAlignment alignment, const QStringList &textLines,
                                            QgsRenderContext &context, const QgsTextFormat &format,
                                            Qgis::TextComponent part, bool drawAsOutlines = true ) SIP_DEPRECATED;

    /**
     * Returns the font metrics for the given text \a format, when rendered
     * in the specified render \a context. The font metrics will take into account
     * all scaling required by the render context.
     *
     * The optional \a scaleFactor argument can specify a font size scaling factor. It is recommended to set this to
     * QgsTextRenderer::FONT_WORKAROUND_SCALE and then manually scale painter devices or calculations
     * based on the resultant font metrics. Failure to do so will result in poor quality text rendering
     * at small font sizes.
     *
     * \since QGIS 3.2
     */
    static QFontMetricsF fontMetrics( QgsRenderContext &context, const QgsTextFormat &format, double scaleFactor = 1.0 );

    /**
     * Returns the width of a text based on a given format.
     * \param context render context
     * \param format text format
     * \param textLines list of lines of text to calculate width from
     * \param fontMetrics font metrics
     */
    static double textWidth( const QgsRenderContext &context, const QgsTextFormat &format, const QStringList &textLines,
                             QFontMetricsF *fontMetrics = nullptr );

    /**
     * Returns the height of a text based on a given format.
     * \param context render context
     * \param format text format
     * \param textLines list of lines of text to calculate width from
     * \param mode draw mode
     * \param fontMetrics font metrics
     * \param flags text renderer flags (since QGIS 3.24)
     * \param maxLineWidth maximum line width, in painter units. Used when the Qgis::TextRendererFlag::WrapLines flag is used (since QGIS 3.24)
     */
    static double textHeight( const QgsRenderContext &context, const QgsTextFormat &format, const QStringList &textLines, Qgis::TextLayoutMode mode = Qgis::TextLayoutMode::Point,
                              QFontMetricsF *fontMetrics = nullptr, Qgis::TextRendererFlags flags = Qgis::TextRendererFlags(), double maxLineWidth = 0 );

    /**
     * Returns the height of a character when rendered with the specified text \a format.
     *
     * \param context render context
     * \param format text format
     * \param character character to determine height of. If \a character is invalid, then the maximum character height will be returned.
     * \param includeEffects if TRUE, then the size of formatting effects such as buffers and shadows will be considered in the
     * returned height. If FALSE, then the returned size considers the character only.
     *
     * \since QGIS 3.16
     */
    static double textHeight( const QgsRenderContext &context, const QgsTextFormat &format, QChar character, bool includeEffects = false );

    /**
     * Returns TRUE if the specified \a text requires line wrapping in order to fit within the specified \a width (in painter units).
     *
     * \see wrappedText()
     * \since QGIS 3.24
     */
    static bool textRequiresWrapping( const QgsRenderContext &context, const QString &text, double width, const QgsTextFormat &format );

    /**
     * Wraps a \a text string to multiple lines, such that each individual line will fit within the specified \a width (in painter units).
     *
     * \see textRequiresWrapping()
     * \since QGIS 3.24
     */
    static QStringList wrappedText( const QgsRenderContext &context, const QString &text, double width, const QgsTextFormat &format );

    /**
     * Scale factor for upscaling font sizes and downscaling destination painter devices.
     *
     * Using this scale factor and manually adjusting any font metric based calculations results in more stable
     * font metrics and sizes for small font sizes.
     *
     * \since QGIS 3.16
     */
    static constexpr double FONT_WORKAROUND_SCALE = 10;

  private:

    struct Component
    {
      //! Block to render
      QgsTextBlock block;

      //! Index of block
      int blockIndex = 0;

      //! Index of first fragment in block
      int firstFragmentIndex = 0;

      //! Current origin point for painting (generally current painter rotation point)
      QPointF origin;
      //! Whether to translate the painter to supplied origin
      bool useOrigin = false;
      //! Any rotation to be applied to painter (in radians)
      double rotation = 0.0;
      //! Any rotation to be applied to painter (in radians) after initial rotation
      double rotationOffset = 0.0;
      //! Current center point of label component, after rotation
      QPointF center;
      //! Width and height of label component, transformed and ready for painting
      QSizeF size;
      //! Any translation offsets to be applied before painting, transformed and ready for painting
      QPointF offset;
      //! A stored QPicture of painting for the component
      QPicture picture;

      /**
       * Buffer for component to accommodate graphic items ignored by QPicture,
       * e.g. half-width of an applied QPen, which would extend beyond boundingRect() of QPicture
       */
      double pictureBuffer = 0.0;
      //! A ratio of native painter dpi and that of rendering context's painter
      double dpiRatio = 1.0;
      //! Horizontal alignment
      Qgis::TextHorizontalAlignment hAlign = Qgis::TextHorizontalAlignment::Left;

      //! Any additional word spacing to apply while rendering component
      double extraWordSpacing = 0;
      //! Any additional letter spacing to apply while rendering component
      double extraLetterSpacing = 0;
    };

    static double textWidth( const QgsRenderContext &context, const QgsTextFormat &format, const QgsTextDocument &document );
    static double textHeight( const QgsRenderContext &context, const QgsTextFormat &format, const QgsTextDocument &document, Qgis::TextLayoutMode mode = Qgis::TextLayoutMode::Point );

    /**
     * Draws a single component of rendered text using the specified settings.
     * \param rect destination rectangle for text
     * \param rotation text rotation
     * \param alignment horizontal alignment
     * \param vAlignment vertical alignment
     * \param document text document to draw
     * \param metrics document metrics
     * \param context render context
     * \param format text format
     * \param part component of text to draw. Note that Shadow parts cannot be drawn
     * individually and instead are drawn with their associated part (e.g., drawn together
     * with the text or background parts)
     * \param mode layout mode
     * \note Not available in Python bindings
     * \since QGIS 3.14
     */
    static void drawPart( const QRectF &rect, double rotation, Qgis::TextHorizontalAlignment alignment, Qgis::TextVerticalAlignment vAlignment, const QgsTextDocument &document, const QgsTextDocumentMetrics &metrics,
                          QgsRenderContext &context, const QgsTextFormat &format,
                          Qgis::TextComponent part, Qgis::TextLayoutMode mode );

    /**
     * Draws a single component of rendered text using the specified settings.
     * \param origin origin for start of text. Y coordinate will be used as baseline.
     * \param rotation text rotation
     * \param alignment horizontal alignment
     * \param document document to draw
     * \param metrics precalculated document metrics
     * \param context render context
     * \param format text format
     * \param part component of text to draw. Note that Shadow parts cannot be drawn
     * individually and instead are drawn with their associated part (e.g., drawn together
     * with the text or background parts)
     * \param mode layout mode
     * \note Not available in Python bindings
     * \since QGIS 3.14
     */
    static void drawPart( QPointF origin, double rotation, Qgis::TextHorizontalAlignment alignment, const QgsTextDocument &document,
                          const QgsTextDocumentMetrics &metrics,
                          QgsRenderContext &context, const QgsTextFormat &format,
                          Qgis::TextComponent part,
                          Qgis::TextLayoutMode mode );

    static double drawBuffer( QgsRenderContext &context,
                              const Component &component,
                              const QgsTextFormat &format,
                              const QgsTextDocumentMetrics &metrics,
                              Qgis::TextLayoutMode mode );

    static void drawBackground( QgsRenderContext &context,
                                Component component,
                                const QgsTextFormat &format,
                                const QgsTextDocumentMetrics &metrics,
                                Qgis::TextLayoutMode mode = Qgis::TextLayoutMode::Rectangle );

    static void drawShadow( QgsRenderContext &context,
                            const Component &component,
                            const QgsTextFormat &format );

    static void drawMask( QgsRenderContext &context,
                          const Component &component,
                          const QgsTextFormat &format,
                          const QgsTextDocumentMetrics &metrics,
                          Qgis::TextLayoutMode mode );

    static void drawText( QgsRenderContext &context,
                          const Component &component,
                          const QgsTextFormat &format );

    static void drawTextInternal( Qgis::TextComponent drawType,
                                  QgsRenderContext &context,
                                  const QgsTextFormat &format,
                                  const Component &component,
                                  const QgsTextDocument &document,
                                  const QgsTextDocumentMetrics &metrics,
                                  Qgis::TextHorizontalAlignment alignment,
                                  Qgis::TextVerticalAlignment vAlignment,
                                  Qgis::TextLayoutMode mode = Qgis::TextLayoutMode::Rectangle );

    static Qgis::TextOrientation calculateRotationAndOrientationForComponent( const QgsTextFormat &format, const Component &component, double &rotation );

    static void calculateExtraSpacingForLineJustification( double spaceToDistribute, const QgsTextBlock &block, double &extraWordSpace, double &extraLetterSpace );
    static void applyExtraSpacingForLineJustification( QFont &font, double extraWordSpace, double extraLetterSpace );

    static void drawTextInternalHorizontal( QgsRenderContext &context,
                                            const QgsTextFormat &format,
                                            Qgis::TextComponent drawType,
                                            Qgis::TextLayoutMode mode,
                                            const Component &component,
                                            const QgsTextDocument &document,
                                            const QgsTextDocumentMetrics &metrics,
                                            double fontScale,
                                            Qgis::TextHorizontalAlignment hAlignment,
                                            Qgis::TextVerticalAlignment vAlignment,
                                            double rotation );

    static void drawTextInternalVertical( QgsRenderContext &context,
                                          const QgsTextFormat &format,
                                          Qgis::TextComponent drawType,
                                          Qgis::TextLayoutMode mode,
                                          const Component &component,
                                          const QgsTextDocument &document,
                                          const QgsTextDocumentMetrics &metrics,
                                          double fontScale,
                                          Qgis::TextHorizontalAlignment hAlignment,
                                          Qgis::TextVerticalAlignment vAlignment,
                                          double rotation );

    static double calculateScaleFactorForFormat( const QgsRenderContext &context, const QgsTextFormat &format );

    friend class QgsVectorLayerLabelProvider;
    friend class QgsLabelPreview;

    static QgsTextFormat updateShadowPosition( const QgsTextFormat &format );

};

#endif // QGSTEXTRENDERER_H
