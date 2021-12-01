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
#include "qgstextformat.h"

#include <QPicture>

class QgsTextDocument;
class QgsRenderContext;

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

    //! Draw mode to calculate width and height
    enum DrawMode
    {
      Rect = 0, //!< Text within rectangle draw mode
      Point, //!< Text at point of origin draw mode
      Label, //!< Label-specific draw mode
    };

    //! Components of text
    enum TextPart
    {
      Text = 0, //!< Text component
      Buffer, //!< Buffer component
      Background, //!< Background shape
      Shadow, //!< Drop shadow
    };

    //! Horizontal alignment
    enum HAlignment
    {
      AlignLeft = 0, //!< Left align
      AlignCenter, //!< Center align
      AlignRight, //!< Right align
      AlignJustify, //!< Justify align
    };

    /**
     * Converts a Qt horizontal \a alignment flag to a QgsTextRenderer::HAlignment value.
     *
     * \see convertQtVAlignment()
     * \since QGIS 3.16
     */
    static HAlignment convertQtHAlignment( Qt::Alignment alignment );

    /**
     * Vertical alignment
     * \since QGIS 3.16
     */
    enum VAlignment
    {
      AlignTop = 0, //!< Align to top
      AlignVCenter, //!< Center align
      AlignBottom, //!< Align to bottom
    };

    /**
     * Converts a Qt vertical \a alignment flag to a QgsTextRenderer::VAlignment value.
     *
     * \see convertQtHAlignment()
     * \since QGIS 3.16
     */
    static VAlignment convertQtVAlignment( Qt::Alignment alignment );

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
     */
    static void drawText( const QRectF &rect, double rotation, HAlignment alignment, const QStringList &textLines,
                          QgsRenderContext &context, const QgsTextFormat &format,
                          bool drawAsOutlines = true, VAlignment vAlignment = AlignTop,
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
    static void drawText( QPointF point, double rotation, HAlignment alignment, const QStringList &textLines,
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
    Q_DECL_DEPRECATED static void drawPart( const QRectF &rect, double rotation, HAlignment alignment, const QStringList &textLines,
                                            QgsRenderContext &context, const QgsTextFormat &format,
                                            TextPart part, bool drawAsOutlines = true ) SIP_DEPRECATED;

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
    Q_DECL_DEPRECATED static void drawPart( QPointF origin, double rotation, HAlignment alignment, const QStringList &textLines,
                                            QgsRenderContext &context, const QgsTextFormat &format,
                                            TextPart part, bool drawAsOutlines = true ) SIP_DEPRECATED;

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
    static double textHeight( const QgsRenderContext &context, const QgsTextFormat &format, const QStringList &textLines, DrawMode mode = Point,
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
      HAlignment hAlign = AlignLeft;

      //! Any additional word spacing to apply while rendering component
      double extraWordSpacing = 0;
      //! Any additional letter spacing to apply while rendering component
      double extraLetterSpacing = 0;
    };

    static double textWidth( const QgsRenderContext &context, const QgsTextFormat &format, const QgsTextDocument &document );
    static double textHeight( const QgsRenderContext &context, const QgsTextFormat &format, const QgsTextDocument &document, DrawMode mode = Point );

    /**
     * Draws a single component of rendered text using the specified settings.
     * \param rect destination rectangle for text
     * \param rotation text rotation
     * \param alignment horizontal alignment
     * \param vAlignment vertical alignment
     * \param document text document to draw
     * \param context render context
     * \param format text format
     * \param part component of text to draw. Note that Shadow parts cannot be drawn
     * individually and instead are drawn with their associated part (e.g., drawn together
     * with the text or background parts)
     * \note Not available in Python bindings
     * \since QGIS 3.14
     */
    static void drawPart( const QRectF &rect, double rotation, HAlignment alignment, VAlignment vAlignment, const QgsTextDocument &document,
                          QgsRenderContext &context, const QgsTextFormat &format,
                          TextPart part );

    /**
     * Draws a single component of rendered text using the specified settings.
     * \param origin origin for start of text. Y coordinate will be used as baseline.
     * \param rotation text rotation
     * \param alignment horizontal alignment
     * \param document document to draw
     * \param context render context
     * \param format text format
     * \param part component of text to draw. Note that Shadow parts cannot be drawn
     * individually and instead are drawn with their associated part (e.g., drawn together
     * with the text or background parts)
     * \note Not available in Python bindings
     * \since QGIS 3.14
     */
    static void drawPart( QPointF origin, double rotation, HAlignment alignment, const QgsTextDocument &document,
                          QgsRenderContext &context, const QgsTextFormat &format,
                          TextPart part );

    static double drawBuffer( QgsRenderContext &context,
                              const Component &component,
                              const QgsTextFormat &format,
                              DrawMode mode );

    static void drawBackground( QgsRenderContext &context,
                                Component component,
                                const QgsTextFormat &format,
                                const QgsTextDocument &document,
                                DrawMode mode = Rect );

    static void drawShadow( QgsRenderContext &context,
                            const Component &component,
                            const QgsTextFormat &format );

    static void drawMask( QgsRenderContext &context,
                          const Component &component,
                          const QgsTextFormat &format,
                          DrawMode mode );

    static void drawText( QgsRenderContext &context,
                          const Component &component,
                          const QgsTextFormat &format );

    static void drawTextInternal( TextPart drawType,
                                  QgsRenderContext &context,
                                  const QgsTextFormat &format,
                                  const Component &component,
                                  const QgsTextDocument &document,
                                  const QFontMetricsF *fontMetrics,
                                  HAlignment alignment,
                                  VAlignment vAlignment,
                                  DrawMode mode = Rect );

    static QgsTextFormat::TextOrientation calculateRotationAndOrientationForComponent( const QgsTextFormat &format, const Component &component, double &rotation );

    static void calculateExtraSpacingForLineJustification( double spaceToDistribute, const QgsTextBlock &block, double &extraWordSpace, double &extraLetterSpace );
    static void applyExtraSpacingForLineJustification( QFont &font, double extraWordSpace, double extraLetterSpace );

    static void drawTextInternalHorizontal( QgsRenderContext &context,
                                            const QgsTextFormat &format,
                                            TextPart drawType,
                                            DrawMode mode,
                                            const Component &component,
                                            const QgsTextDocument &document,
                                            double fontScale,
                                            const QFontMetricsF *fontMetrics,
                                            HAlignment hAlignment,
                                            VAlignment vAlignment,
                                            double rotation );

    static void drawTextInternalVertical( QgsRenderContext &context,
                                          const QgsTextFormat &format,
                                          TextPart drawType,
                                          DrawMode mode,
                                          const Component &component,
                                          const QgsTextDocument &document,
                                          double fontScale,
                                          const QFontMetricsF *fontMetrics,
                                          HAlignment hAlignment,
                                          VAlignment vAlignment,
                                          double rotation );

    static double calculateScaleFactorForFormat( const QgsRenderContext &context, const QgsTextFormat &format );

    friend class QgsVectorLayerLabelProvider;
    friend class QgsLabelPreview;

    static QgsTextFormat updateShadowPosition( const QgsTextFormat &format );

};

#endif // QGSTEXTRENDERER_H
