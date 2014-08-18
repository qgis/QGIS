class QgsTextRendererSettings
{
%TypeHeaderCode
#include <qgstextrenderer.h>
%End
  public:

    QgsTextRendererSettings();
    QgsTextRendererSettings( const QgsTextRendererSettings& s );
    ~QgsTextRendererSettings();

    enum MultiLineAlign
    {
      MultiLeft,
      MultiCenter,
      MultiRight
    };

    enum ShapeType
    {
      ShapeRectangle,
      ShapeSquare,
      ShapeEllipse,
      ShapeCircle,
      ShapeSVG
    };

    enum SizeType
    {
      SizeBuffer,
      SizeFixed,
      SizePercent
    };

    enum RotationType
    {
      RotationSync,
      RotationOffset,
      RotationFixed
    };

    /** Units used for option sizes, before being converted to rendered sizes */
    enum SizeUnit
    {
      Points,
      MM,
      MapUnits,
      Percent
    };

    enum ShadowType
    {
      ShadowLowest,
      ShadowText,
      ShadowBuffer,
      ShadowShape
    };

    //-- text style
    QFont textFont;
    QString textNamedStyle;
    bool fontSizeInMapUnits; //true if font size is in map units (otherwise in points)
    QgsMapUnitScale fontSizeMapUnitScale; // scale range for map units for font size
    QColor textColor;
    int textTransp;
    QPainter::CompositionMode blendMode;
    QColor previewBkgrdColor;

    //-- text formatting

    QString wrapChar;
    double multilineHeight; //0.0 to 10.0, leading between lines as multiplyer of line height
    MultiLineAlign multilineAlign; // horizontal alignment of multi-line labels

    //-- text buffer

    bool bufferDraw;
    double bufferSize; // buffer size
    bool bufferSizeInMapUnits; //true if buffer is in map units (otherwise in mm)
    QgsMapUnitScale bufferSizeMapUnitScale; // scale range for map units for buffer size
    QColor bufferColor;
    bool bufferNoFill; //set interior of buffer to 100% transparent
    int bufferTransp;
    Qt::PenJoinStyle bufferJoinStyle;
    QPainter::CompositionMode bufferBlendMode;

    //-- shape background

    bool shapeDraw;
    ShapeType shapeType;
    QString shapeSVGFile;
    SizeType shapeSizeType;
    QPointF shapeSize;
    SizeUnit shapeSizeUnits;
    QgsMapUnitScale shapeSizeMapUnitScale;
    RotationType shapeRotationType;
    double shapeRotation;
    QPointF shapeOffset;
    SizeUnit shapeOffsetUnits;
    QgsMapUnitScale shapeOffsetMapUnitScale;
    QPointF shapeRadii;
    SizeUnit shapeRadiiUnits;
    QgsMapUnitScale shapeRadiiMapUnitScale;
    int shapeTransparency;
    QPainter::CompositionMode shapeBlendMode;
    QColor shapeFillColor;
    QColor shapeBorderColor;
    double shapeBorderWidth;
    SizeUnit shapeBorderWidthUnits;
    QgsMapUnitScale shapeBorderWidthMapUnitScale;
    Qt::PenJoinStyle shapeJoinStyle;

    //-- drop shadow

    bool shadowDraw;
    ShadowType shadowUnder;
    int shadowOffsetAngle;
    double shadowOffsetDist;
    SizeUnit shadowOffsetUnits;
    QgsMapUnitScale shadowOffsetMapUnitScale;
    bool shadowOffsetGlobal;
    double shadowRadius;
    SizeUnit shadowRadiusUnits;
    QgsMapUnitScale shadowRadiusMapUnitScale;
    bool shadowRadiusAlphaOnly;
    int shadowTransparency;
    int shadowScale;
    QColor shadowColor;
    QPainter::CompositionMode shadowBlendMode;

    //-- scale factors
    double vectorScaleFactor; //scale factor painter units->pixels
    double rasterCompressFactor; //pixel resolution scale factor

    /** Calculates size (considering output size should be in pixel or map units, scale factors and optionally oversampling)
     * @param size size to convert
     * @param c rendercontext
     * @param unit SizeUnit enum value of size
     * @param rasterfactor whether to consider oversampling
     * @param mapUnitScale a mapUnitScale clamper
     * @return size that will render, as double
     * @note added in 1.9, as a better precision replacement for sizeToPixel
     */
    double scaleToPixelContext( double size, const QgsRenderContext& c, SizeUnit unit, bool rasterfactor = false, const QgsMapUnitScale& mapUnitScale = QgsMapUnitScale() ) const;

};

/** \ingroup core
  * Maintains current state of more grainular and temporal values when creating/painting
  * component parts of an individual label (e.g. buffer, background, shadow, etc.).
  */
class QgsLabelComponent
{
%TypeHeaderCode
#include <qgstextrenderer.h>
%End
  public:
    QgsLabelComponent();

    // methods

    const QString& text();
    void setText( const QString& text );

    const QgsPoint& origin();
    void setOrigin( QgsPoint point );

    bool useOrigin() const;
    void setUseOrigin( bool use );

    double rotation() const;
    void setRotation( double rotation );

    double rotationOffset() const;
    void setRotationOffset( double rotation );

    bool useRotation() const;
    void setUseRotation( bool use );

    const QgsPoint& center();
    void setCenter( QgsPoint point );

    bool useCenter() const;
    void setUseCenter( bool use );

    const QgsPoint& size();
    void setSize( QgsPoint point );

    const QgsPoint& offset();
    void setOffset( QgsPoint point );

    const QPicture* picture();
    void setPicture( QPicture* picture );

    double pictureBuffer() const;
    void setPictureBuffer( double buffer );

    double dpiRatio() const;
    void setDpiRatio( double ratio );

};

class QgsTextRenderer
{
%TypeHeaderCode
#include <qgstextrenderer.h>
%End
  public:

    enum TextComponentPart
    {
      TextPart = 0,
      LabelPart,
      ShapePart,
      SVGPart,
      ShadowPart
    };

    /**Draws text using the specified settings
     * @param rect destination rectangle for text
     * @param rotation text rotation
     * @param textLines list of lines of text to draw
     * @param context render context
     * @param layerSettings pal layer settings
     * @param dpiRatio scaling for dpi
     * @param drawAsOutlines set to true to draw text as outlines, rather than
     * text objects. Drawing as outlines is required for correct rendering of buffered text
     * @note added in QGIS 2.5
     */
    static void drawText( const QRectF rect, const double rotation, const QStringList textLines,
                          QgsRenderContext &context, QgsTextRendererSettings &textSettings,
                          const double dpiRatio = 1.0, const bool drawAsOutlines = true );

    /**Draws a part of a label using the specified settings
     * @param rect destination rectangle for text
     * @param rotation text rotation
     * @param textLines list of lines of text to draw
     * @param context render context
     * @param layerSettings Pal layer settings
     * @param drawType part of label to draw
     * @param dpiRatio scaling for dpi
     * @note added in QGIS 2.5
     */
    static void drawPart( const QRectF rect, const double rotation, const QStringList textLines,
                           QgsRenderContext& context, QgsTextRendererSettings& textSettings,
                           const TextComponentPart component, const double dpiRatio, const bool drawAsOutlines );

    static void drawBackgroundPart( QgsRenderContext& context,
                                     QgsLabelComponent component,
                                     const QgsTextRendererSettings& textSettings );

    static void drawShadowPart( QgsRenderContext& context,
                                 QgsLabelComponent component,
                                 const QgsTextRendererSettings& textSettings );

    static void drawBufferPart( QgsRenderContext& context,
                                 QgsLabelComponent component,
                                 const QgsTextRendererSettings& textSettings );

    static void drawTextPart( const QgsPoint point, const QSizeF size, const bool drawFromTop, const QStringList textLines,
                               const TextComponentPart drawType, QgsLabelComponent component,
                               const QgsTextRendererSettings& settings, const QFontMetricsF* fontMetrics,
                               QgsRenderContext& context, const bool drawAsOutlines );

};
