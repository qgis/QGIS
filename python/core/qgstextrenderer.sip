/** \class QgsTextBufferSettings
  * \ingroup core
  * Container for settings relating to a text buffer.
  * \note QgsTextBufferSettings objects are implicitly shared.
  * \note added in QGIS 3.0
 */

class QgsTextBufferSettings
{
%TypeHeaderCode
#include <qgstextrenderer.h>
%End
  public:

    QgsTextBufferSettings();

    QgsTextBufferSettings( const QgsTextBufferSettings& other );

    ~QgsTextBufferSettings();

    /** Returns whether the buffer is enabled.
     * @see setEnabled()
     */
    bool enabled() const;

    /** Sets whether the text buffer will be drawn.
     * @param enabled set to true to draw buffer
     * @see enabled()
     */
    void setEnabled( bool enabled );

    /** Returns the size of the buffer.
     * @see sizeUnit()
     * @see setSize()
     */
    double size() const;

    /** Sets the size of the buffer. The size units are specified using setSizeUnit().
     * @param size buffer size
     * @see size()
     * @see setSizeUnit()
     */
    void setSize( double size );

    /** Returns the units for the buffer size.
     * @see size()
     * @see setSizeUnit()
     */
    QgsUnitTypes::RenderUnit sizeUnit() const;

    /** Sets the units used for the buffer size.
     * @param unit size unit
     * @see setSize()
     * @see sizeUnit()
     */
    void setSizeUnit( QgsUnitTypes::RenderUnit unit );

    /** Returns the map unit scale object for the buffer size. This is only used if the
     * buffer size is set to QgsUnitTypes::RenderMapUnit.
     * @see setSizeMapUnitScale()
     * @see sizeUnit()
     */
    QgsMapUnitScale sizeMapUnitScale() const;

    /** Sets the map unit scale object for the buffer size. This is only used if the
     * buffer size is set to QgsUnitTypes::RenderMapUnit.
     * @param scale scale for buffer size
     * @see sizeMapUnitScale()
     * @see setSizeUnit()
     */
    void setSizeMapUnitScale( const QgsMapUnitScale& scale );

    /** Returns the color of the buffer.
     * @see setColor()
     */
    QColor color() const;

    /** Sets the color for the buffer.
     * @param color buffer color
     * @see color()
     */
    void setColor( const QColor& color );

    /** Returns whether the interior of the buffer will be filled in. If false, only the outline
     * of the text will be drawn as the buffer. The effect of this setting is only visible for
     * semi-transparent text.
     * @see setFillBufferInterior()
     */
    bool fillBufferInterior() const;

    /** Sets whether the interior of the buffer will be filled in.
     * @param fill set to false to drawn only the outline of the text as the buffer, or true to also
     * shade the area inside the text. The effect of this setting is only visible for semi-transparent text.
     * @see fillBufferInterior()
     */
    void setFillBufferInterior( bool fill );

    /** Returns the buffer opacity. The opacity is a double value between 0 (fully transparent) and 1 (totally
     * opaque).
     * @see setOpacity()
     */
    double opacity() const;

    /** Sets the buffer opacity.
     * @param opacity opacity as a double value between 0 (fully transparent) and 1 (totally
     * opaque)
     * @see opacity()
     */
    void setOpacity( double opacity );

    /** Returns the buffer join style.
     * @see setJoinStyle
     */
    Qt::PenJoinStyle joinStyle() const;

    /** Sets the join style used for drawing the buffer.
     * @param style join style
     * @see joinStyle()
     */
    void setJoinStyle( Qt::PenJoinStyle style );

    /** Returns the blending mode used for drawing the buffer.
     * @see setBlendMode()
     */
    QPainter::CompositionMode blendMode() const;

    /** Sets the blending mode used for drawing the buffer.
     * @param mode blending mode
     * @see blendMode()
     */
    void setBlendMode( QPainter::CompositionMode mode );

        /** Reads settings from a layer's custom properties.
     * @param layer source vector layer
     * @see writeToLayer()
     */
    void readFromLayer( QgsVectorLayer* layer );

    /** Writes settings to a layer's custom properties.
     * @param layer target vector layer
     * @see readFromLayer()
     */
    void writeToLayer( QgsVectorLayer* layer ) const;

    /** Read settings from a DOM element.
     * @see writeXml()
     */
    void readXml( const QDomElement& elem );

    /** Write settings into a DOM element.
     * @see readXml()
     */
    QDomElement writeXml( QDomDocument& doc ) const;

};


/** \class QgsTextBackgroundSettings
  * \ingroup core
  * Container for settings relating to a text background object.
  * \note QgsTextBackgroundSettings objects are implicitly shared.
  * \note added in QGIS 3.0
 */

class QgsTextBackgroundSettings
{
%TypeHeaderCode
#include <qgstextrenderer.h>
%End
  public:

    /** Background shape types.
     */
    enum ShapeType
    {
      ShapeRectangle, /*!< rectangle */
      ShapeSquare, /*!< square */
      ShapeEllipse, /*!< ellipse */
      ShapeCircle, /*!< circle */
      ShapeSVG /*!< SVG file */
    };

    /** Methods for determining the background shape size.
     */
    enum SizeType
    {
      SizeBuffer, /*!< shape size is determined by adding a buffer margin around text */
      SizeFixed, /*!< fixed size */
      SizePercent /*!< shape size is determined by percent of text size */
    };

    /** Methods for determining the rotation of the background shape.
     */
    enum RotationType
    {
      RotationSync, /*!< shape rotation is synced with text rotation */
      RotationOffset, /*!< shape rotation is offset from text rotation */
      RotationFixed /*!< shape rotation is a fixed angle */
    };

    QgsTextBackgroundSettings();

    QgsTextBackgroundSettings( const QgsTextBackgroundSettings& other );

    ~QgsTextBackgroundSettings();

    /** Returns whether the background is enabled.
     * @see setEnabled()
     */
    bool enabled() const;

    /** Sets whether the text background will be drawn.
     * @param enabled set to true to draw background
     * @see enabled()
     */
    void setEnabled( bool enabled );

    /** Returns the type of background shape (e.g., square, ellipse, SVG).
     * @see setType()
     */
    ShapeType type() const;

    /** Sets the type of background shape to draw (e.g., square, ellipse, SVG).
     * @param type shape type
     * @see type()
     */
    void setType( ShapeType type );

    /** Returns the path to the background SVG file, if set.
     * @see setSvgFile()
     */
    QString svgFile() const;

    /** Sets the path to the background SVG file. This is only used if type() is set to
     * QgsTextBackgroundSettings::ShapeSVG.
     * @param file SVG file path
     * @see svgFile()
     */
    void setSvgFile( const QString& file );

    /** Returns the method used to determine the size of the background shape (e.g., fixed size or buffer
     * around text).
     * @see setSizeType()
     * @see size()
     */
    SizeType sizeType() const;

    /** Sets the method used to determine the size of the background shape (e.g., fixed size or buffer
     * around text).
     * @param type size method
     * @see sizeType()
     * @see setSize()
     */
    void setSizeType( SizeType type );

    /** Returns the size of the background shape. The meaning of the size depends on the current sizeType(),
     * e.g., for size types of QgsTextBackgroundSettings::SizeFixed the size will represent the actual width and
     * height of the shape, for QgsTextBackgroundSettings::SizeBuffer the size will represent the horizontal
     * and vertical margins to add to the text when calculating the size of the shape.
     * @see setSize()
     * @see sizeType()
     */
    QSizeF size() const;

    /** Sets the size of the background shape. The meaning of the size depends on the current sizeType(),
     * e.g., for size types of QgsTextBackgroundSettings::SizeFixed the size will represent the actual width and
     * height of the shape, for QgsTextBackgroundSettings::SizeBuffer the size will represent the horizontal
     * and vertical margins to add to the text when calculating the size of the shape.
     * @param size QSizeF representing horizontal and vertical size components for shape
     * @see size()
     * @see setSizeType()
     */
    void setSize( const QSizeF& size );

    /** Returns the units used for the shape's size. This value has no meaning if the sizeType() is set to
     * QgsTextBackgroundSettings::SizePercent.
     * @see setSizeUnit()
     * @see sizeType()
     * @see size()
     */
    QgsUnitTypes::RenderUnit sizeUnit() const;

    /** Sets the units used for the shape's size. This value has no meaning if the sizeType() is set to
     * QgsTextBackgroundSettings::SizePercent.
     * @param unit size units
     * @see sizeUnit()
     * @see setSizeType()
     * @see setSize()
     */
    void setSizeUnit( QgsUnitTypes::RenderUnit unit );

    /** Returns the map unit scale object for the shape size. This is only used if the
     * sizeUnit() is set to QgsUnitTypes::RenderMapUnit.
     * @see setSizeMapUnitScale()
     * @see sizeUnit()
     */
    QgsMapUnitScale sizeMapUnitScale() const;

    /** Sets the map unit scale object for the shape size. This is only used if the
     * sizeUnit() is set to QgsUnitTypes::RenderMapUnit.
     * @param scale scale for shape size
     * @see sizeMapUnitScale()
     * @see setSizeUnit()
     */
    void setSizeMapUnitScale( const QgsMapUnitScale& scale );

    /** Returns the method used for rotating the background shape.
     * @see setRotationType()
     * @see rotation()
     */
    RotationType rotationType() const;

    /** Sets the method used for rotating the background shape.
     * @param type rotation method
     * @see rotationType()
     * @see setRotation()
     */
    void setRotationType( RotationType type );

    /** Returns the rotation for the background shape.
     * @see rotationType()
     * @see setRotation()
     */
    double rotation() const;

    /** Sets the rotation for the background shape.
     * @param rotation angle in degrees to rotate
     * @see rotation()
     * @see setRotationType()
     */
    void setRotation( double rotation );

    /** Returns the offset used for drawing the background shape. Units are determined
     * via offsetUnit().
     * @see setOffset()
     * @see offsetUnit()
     */
    QPointF offset() const;

    /** Sets the offset used for drawing the background shape. Units are specified using
     * setOffsetUnit().
     * @param offset offset for shape
     * @see offset()
     * @see setOffsetUnit()
     */
    void setOffset( const QPointF& offset );

    /** Returns the units used for the shape's offset.
     * @see setOffsetUnit()
     * @see offset()
     */
    QgsUnitTypes::RenderUnit offsetUnit() const;

    /** Sets the units used for the shape's offset.
     * @param unit offset units
     * @see offsetUnit()
     * @see setOffset()
     */
    void setOffsetUnit( QgsUnitTypes::RenderUnit units );

    /** Returns the map unit scale object for the shape offset. This is only used if the
     * offsetUnit() is set to QgsUnitTypes::RenderMapUnit.
     * @see setOffsetMapUnitScale()
     * @see offsetUnit()
     */
    QgsMapUnitScale offsetMapUnitScale() const;

    /** Sets the map unit scale object for the shape offset. This is only used if the
     * offsetUnit() is set to QgsUnitTypes::RenderMapUnit.
     * @param scale scale for shape offset
     * @see offsetMapUnitScale()
     * @see setOffsetUnit()
     */
    void setOffsetMapUnitScale( const QgsMapUnitScale& scale );

    /** Returns the radii used for rounding the corners of shapes. Units are retrieved
     * through radiiUnit().
     * @see setRadii()
     * @see radiiUnit()
     */
    QSizeF radii() const;

    /** Sets the radii used for rounding the corners of shapes. This is only used if
     * type() is set to QgsTextBackgroundSettings::ShapeRectangle or QgsTextBackgroundSettings::ShapeSquare.
     * @param radii QSizeF representing horizontal and vertical radii for rounded corners. Units are
     * specified through setRadiiUnit()
     * @see radii()
     * @see setRadiiUnit()
     */
    void setRadii( const QSizeF& radii );

    /** Returns the units used for the shape's radii.
     * @see setRadiiUnit()
     * @see radii()
     */
    QgsUnitTypes::RenderUnit radiiUnit() const;

    /** Sets the units used for the shape's radii.
     * @param unit radii units
     * @see radiiUnit()
     * @see setRadii()
     */
    void setRadiiUnit( QgsUnitTypes::RenderUnit units );

    /** Returns the map unit scale object for the shape radii. This is only used if the
     * radiiUnit() is set to QgsUnitTypes::RenderMapUnit.
     * @see setRadiiMapUnitScale()
     * @see radiiUnit()
     */
    QgsMapUnitScale radiiMapUnitScale() const;

    /** Sets the map unit scale object for the shape radii. This is only used if the
     * radiiUnit() is set to QgsUnitTypes::RenderMapUnit.
     * @param scale scale for shape radii
     * @see radiiMapUnitScale()
     * @see setRadiiUnit()
     */
    void setRadiiMapUnitScale( const QgsMapUnitScale& scale );

    /** Returns the background shape's opacity. The opacity is a double value between 0 (fully transparent) and 1 (totally
     * opaque).
     * @see setOpacity()
     */
    double opacity() const;

    /** Sets the background shape's opacity.
     * @param opacity opacity as a double value between 0 (fully transparent) and 1 (totally
     * opaque)
     * @see opacity()
     */
    void setOpacity( double opacity );

    /** Returns the blending mode used for drawing the background shape.
     * @see setBlendMode()
     */
    QPainter::CompositionMode blendMode() const;

    /** Sets the blending mode used for drawing the background shape.
     * @param mode blending mode
     * @see blendMode()
     */
    void setBlendMode( QPainter::CompositionMode mode );

    /** Returns the color used for filing the background shape.
     * @see setFillColor()
     * @see borderColor()
     */
    QColor fillColor() const;

    /** Sets the color used for filing the background shape.
     * @param color background color
     * @see fillColor()
     * @see setBorderColor()
     */
    void setFillColor( const QColor& color );

    /** Returns the color used for outlining the background shape.
     * @see setBorderColor()
     * @see fillColor()
     */
    QColor borderColor() const;

    /** Sets the color used for outlining the background shape.
     * @param color outline color
     * @see borderColor()
     * @see setFillColor()
     */
    void setBorderColor( const QColor& color );

    /** Returns the width of the shape's border (outline). Units are retrieved through
     * borderWidthUnit().
     * @see setBorderWidth()
     * @see borderWidthUnit()
     */
    double borderWidth() const;

    /** Sets the width of the shape's border (outline). Units are specified through
     * setBorderWidthUnit().
     * @see borderWidth()
     * @see setBorderWidthUnit()
     */
    void setBorderWidth( double width );

    /** Returns the units used for the shape's border width.
     * @see setBorderWidthUnit()
     * @see borderWidth()
     */
    QgsUnitTypes::RenderUnit borderWidthUnit() const;

    /** Sets the units used for the shape's border width.
     * @param unit border width units
     * @see borderWidthUnit()
     * @see setBorderWidth()
     */
    void setBorderWidthUnit( QgsUnitTypes::RenderUnit units );

    /** Returns the map unit scale object for the shape border width. This is only used if the
     * borderWidthUnit() is set to QgsUnitTypes::RenderMapUnit.
     * @see setBorderWidthMapUnitScale()
     * @see borderWidthUnit()
     */
    QgsMapUnitScale borderWidthMapUnitScale() const;

    /** Sets the map unit scale object for the shape border width. This is only used if the
     * borderWidthUnit() is set to QgsUnitTypes::RenderMapUnit.
     * @param scale scale for shape border width
     * @see borderWidthMapUnitScale()
     * @see setBorderWidthUnit()
     */
    void setBorderWidthMapUnitScale( const QgsMapUnitScale& scale );

    /** Returns the join style used for drawing the background shape.
     * @see setJoinStyle
     */
    Qt::PenJoinStyle joinStyle() const;

    /** Sets the join style used for drawing the background shape.
     * @param style join style
     * @see joinStyle()
     */
    void setJoinStyle( Qt::PenJoinStyle style );

    /** Reads settings from a layer's custom properties.
     * @param layer source vector layer
     * @see writeToLayer()
     */
    void readFromLayer( QgsVectorLayer* layer );

    /** Writes settings to a layer's custom properties.
     * @param layer target vector layer
     * @see readFromLayer()
     */
    void writeToLayer( QgsVectorLayer* layer ) const;

    /** Read settings from a DOM element.
     * @see writeXml()
     */
    void readXml( const QDomElement& elem );

    /** Write settings into a DOM element.
     * @see readXml()
     */
    QDomElement writeXml( QDomDocument& doc ) const;

};


/** \class QgsTextShadowSettings
  * \ingroup core
  * Container for settings relating to a text shadow.
  * \note QgsTextShadowSettings objects are implicitly shared.
  * \note added in QGIS 3.0
 */

class QgsTextShadowSettings
{

%TypeHeaderCode
#include <qgstextrenderer.h>
%End

  public:

    /** Placement positions for text shadow.
     */
    enum ShadowPlacement
    {
      ShadowLowest, /*!< draw shadow below all text components */
      ShadowText, /*!< draw shadow under text */
      ShadowBuffer, /*!< draw shadow under buffer */
      ShadowShape /*!< draw shadow under background shape */
    };

    QgsTextShadowSettings();

    QgsTextShadowSettings( const QgsTextShadowSettings& other );

    ~QgsTextShadowSettings();

    /** Returns whether the shadow is enabled.
     * @see setEnabled()
     */
    bool enabled() const;

    /** Sets whether the text shadow will be drawn.
     * @param enabled set to true to draw shadow
     * @see enabled()
     */
    void setEnabled( bool enabled );

    /** Returns the placement for the drop shadow. The placement determines
     * both the z-order stacking position for the shadow and the what shape (e.g., text,
     * background shape) is used for casting the shadow.
     * @see setShadowPlacement()
     */
    QgsTextShadowSettings::ShadowPlacement shadowPlacement() const;

    /** Sets the placement for the drop shadow. The placement determines
     * both the z-order stacking position for the shadow and the what shape (e.g., text,
     * background shape) is used for casting the shadow.
     * @param placement shadow placement
     * @see shadowPlacement()
     */
    void setShadowPlacement( QgsTextShadowSettings::ShadowPlacement placement );

    /** Returns the angle for offsetting the position of the shadow from the text.
     * @see setOffsetAngle
     * @see offsetDistance()
     */
    int offsetAngle() const;

    /** Sets the angle for offseting the position of the shadow from the text.
     * @param angle offset angle in degrees
     * @see offsetAngle()
     * @see setOffsetDistance()
     */
    void setOffsetAngle( int angle );

    /** Returns the distance for offsetting the position of the shadow from the text. Offset units
     * are retrieved via offsetUnit().
     * @see setOffsetDistance()
     * @see offsetUnit()
     */
    double offsetDistance() const;

    /** Sets the distance for offsetting the position of the shadow from the text. Offset units
     * are specified via setOffsetUnit().
     * @param offset distance
     * @see offsetDistance()
     * @see setOffsetUnit()
     */
    void setOffsetDistance( double distance );

    /** Returns the units used for the shadow's offset.
     * @see setOffsetUnit()
     * @see offsetDistance()
     */
    QgsUnitTypes::RenderUnit offsetUnit();

    /** Sets the units used for the shadow's offset.
     * @param units shadow distance units
     * @see offsetUnit()
     * @see setOffsetDistance()
     */
    void setOffsetUnit( QgsUnitTypes::RenderUnit units );

    /** Returns the map unit scale object for the shadow offset distance. This is only used if the
     * offsetUnit() is set to QgsUnitTypes::RenderMapUnit.
     * @see setOffsetMapUnitScale()
     * @see offsetUnit()
     */
    QgsMapUnitScale offsetMapUnitScale() const;

    /** Sets the map unit scale object for the shadow offset distance. This is only used if the
     * offsetUnit() is set to QgsUnitTypes::RenderMapUnit.
     * @param scale scale for shadow offset
     * @see offsetMapUnitScale()
     * @see setOffsetUnit()
     */
    void setOffsetMapUnitScale( const QgsMapUnitScale& scale );

    /** Returns true if the global shadow offset will be used.
     * @see setOffsetGlobal()
     */
    bool offsetGlobal() const;

    /** Sets whether the global shadow offset should be used.
     * @param global set to true to use global shadow offset.
     */
    void setOffsetGlobal( bool global );

    /** Returns the blur radius for the shadow. Radius units are retrieved via blurRadiusUnit().
     * @see setBlurRadius()
     * @see blurRadiusUnit()
     */
    double blurRadius() const;

    /** Sets the blur radius for the shadow. Radius units are specified via setBlurRadiusUnit().
     * @param blurRadius blur radius
     * @see blurRadius()
     * @see setBlurRadiusUnit()
     */
    void setBlurRadius( double blurRadius );

    /** Returns the units used for the shadow's blur radius.
     * @see setBlurRadiusUnit()
     * @see blurRadius()
     */
    QgsUnitTypes::RenderUnit blurRadiusUnit() const;

    /** Sets the units used for the shadow's blur radius.
     * @param units shadow blur radius units
     * @see blurRadiusUnit()
     * @see setBlurRadius()
     */
    void setBlurRadiusUnit( QgsUnitTypes::RenderUnit units );

    /** Returns the map unit scale object for the shadow blur radius. This is only used if the
     * blurRadiusUnit() is set to QgsUnitTypes::RenderMapUnit.
     * @see setBlurRadiusMapUnitScale()
     * @see blurRadiusUnit()
     */
    QgsMapUnitScale blurRadiusMapUnitScale() const;

    /** Sets the map unit scale object for the shadow blur radius. This is only used if the
     * blurRadiusUnit() is set to QgsUnitTypes::RenderMapUnit.
     * @param scale scale for shadow blur radius
     * @see blurRadiusMapUnitScale()
     * @see setBlurRadiusUnit()
     */
    void setBlurRadiusMapUnitScale( const QgsMapUnitScale& scale );

    /** Returns whether only the alpha channel for the shadow will be blurred.
     * @see setBlurAlphaOnly()
     */
    bool blurAlphaOnly() const;

    /** Sets whether only the alpha channel for the shadow should be blurred.
     * @param alphaOnly set to true to blur only the alpha channel. If false, all channels (including
     * red, green and blue channel) will be blurred.
     * @see blurAlphaOnly()
     */
    void setBlurAlphaOnly( bool alphaOnly );

    /** Returns the shadow's opacity. The opacity is a double value between 0 (fully transparent) and 1 (totally
     * opaque).
     * @see setOpacity()
     */
    double opacity() const;

    /** Sets the shadow's opacity.
     * @param opacity opacity as a double value between 0 (fully transparent) and 1 (totally
     * opaque)
     * @see opacity()
     */
    void setOpacity( double opacity );

    /** Returns the scaling used for the drop shadow (in percentage of original size).
     * @see setScale()
     */
    int scale() const;

    /** Sets the scaling used for the drop shadow (in percentage of original size).
     * @param scale scale percent for drop shadow
     * @see scale()
     */
    void setScale( int scale );

    /** Returns the color of the drop shadow.
     * @see setColor()
     */
    QColor color() const;

    /** Sets the color for the drop shadow.
     * @param color shadow color
     * @see color()
     */
    void setColor( const QColor& color );

    /** Returns the blending mode used for drawing the drop shadow.
     * @see setBlendMode()
     */
    QPainter::CompositionMode blendMode() const;

    /** Sets the blending mode used for drawing the drop shadow.
     * @param mode blending mode
     * @see blendMode()
     */
    void setBlendMode( QPainter::CompositionMode mode );

    /** Reads settings from a layer's custom properties.
     * @param layer source vector layer
     * @see writeToLayer()
     */
    void readFromLayer( QgsVectorLayer* layer );

    /** Writes settings to a layer's custom properties.
     * @param layer target vector layer
     * @see readFromLayer()
     */
    void writeToLayer( QgsVectorLayer* layer ) const;

    /** Read settings from a DOM element.
     * @see writeXml()
     */
    void readXml( const QDomElement& elem );

    /** Write settings into a DOM element.
     * @see readXml()
     */
    QDomElement writeXml( QDomDocument& doc ) const;

};

/** \class QgsTextFormat
  * \ingroup core
  * Container for all settings relating to text rendering.
  * \note QgsTextFormat objects are implicitly shared.
  * \note added in QGIS 3.0
 */

class QgsTextFormat
{

%TypeHeaderCode
#include <qgstextrenderer.h>
%End

  public:

    QgsTextFormat();

    QgsTextFormat( const QgsTextFormat& other );

    ~QgsTextFormat();


    /** Returns a reference to the text buffer settings.
     * @see setBuffer()
     */
    QgsTextBufferSettings& buffer();

    /** Sets the text's buffer settings.
     * @param bufferSettings buffer settings
     * @see buffer()
     */
    void setBuffer( const QgsTextBufferSettings& bufferSettings );

    /** Returns a reference to the text background settings.
     * @see setBackground()
     */
    QgsTextBackgroundSettings& background();

    /** Sets the text's background settings.
     * @param backgroundSettings background settings
     * @see background()
     */
    void setBackground( const QgsTextBackgroundSettings& backgroundSettings );

    /** Returns a reference to the text drop shadow settings.
     * @see setShadow()
     */
    QgsTextShadowSettings& shadow();

    /** Sets the text's drop shadow settings.
     * @param shadowSettings shadow settings
     * @see shadow()
     */
    void setShadow( const QgsTextShadowSettings& shadowSettings );

    /** Returns the font used for rendering text. Note that the size of the font
     * is not used, and size() should be called instead to determine the size
     * of rendered text.
     * @see setFont()
     * @see namedStyle()
     */
    QFont font() const;

    /** Returns a font with the size scaled to match the format's size settings (including
     * units and map unit scale) for a specified render context.
     * @param context destination render context
     * @returns font with scaled size
     * @see font()
     * @see size()
     */
    QFont scaledFont( const QgsRenderContext& context ) const;

    /** Sets the font used for rendering text. Note that the size of the font
     * is not used, and setSize() should be called instead to explicitly set the size
     * of rendered text.
     * @param font desired font
     * @see font()
     * @see setNamedStyle()
     */
    void setFont( const QFont& font );

    /** Returns the named style for the font used for rendering text (e.g., "bold").
     * @see setNamedStyle()
     * @see font()
     */
    QString namedStyle() const;

    /** Sets the named style for the font used for rendering text.
     * @param style named style, e.g., "bold"
     * @see namedStyle()
     * @see setFont()
     */
    void setNamedStyle( const QString& style );

    /** Returns the size for rendered text. Units are retrieved using sizeUnit().
     * @see setSize()
     * @see sizeUnit()
     */
    double size() const;

    /** Sets the size for rendered text.
     * @param size size of rendered text. Units are set using setSizeUnit()
     * @see size()
     * @see setSizeUnit()
     */
    void setSize( double size );

    /** Returns the units for the size of rendered text.
     * @see size()
     * @see setSizeUnit()
     * @see sizeMapUnitScale()
     */
    QgsUnitTypes::RenderUnit sizeUnit() const;

    /** Sets the units for the size of rendered text.
     * @param unit size units
     * @see setSize()
     * @see sizeUnit()
     * @see setSizeMapUnitScale()
     */
    void setSizeUnit( QgsUnitTypes::RenderUnit unit );

    /** Returns the map unit scale object for the size. This is only used if the
     * sizeUnit() is set to QgsUnitTypes::RenderMapUnit.
     * @see setSizeMapUnitScale()
     * @see sizeUnit()
     */
    QgsMapUnitScale sizeMapUnitScale() const;

    /** Sets the map unit scale object for the size. This is only used if the
     * sizeUnit() is set to QgsUnitTypes::RenderMapUnit.
     * @see sizeMapUnitScale()
     * @see setSizeUnit()
     */
    void setSizeMapUnitScale( const QgsMapUnitScale& scale );

    /** Returns the color that text will be rendered in.
     * @see setColor()
     */
    QColor color() const;

    /** Sets the color that text will be rendered in.
     * @param color text color
     * @see color()
     */
    void setColor( const QColor& color );

    /** Returns the text's opacity. The opacity is a double value between 0 (fully transparent) and 1 (totally
     * opaque).
     * @see setOpacity()
     */
    double opacity() const;

    /** Sets the text's opacity.
     * @param opacity opacity as a double value between 0 (fully transparent) and 1 (totally
     * opaque)
     * @see opacity()
     */
    void setOpacity( double opacity );

    /** Returns the blending mode used for drawing the text.
     * @see setBlendMode()
     */
    QPainter::CompositionMode blendMode() const;

    /** Sets the blending mode used for drawing the text.
     * @param mode blending mode
     * @see blendMode()
     */
    void setBlendMode( QPainter::CompositionMode mode );

    /** Returns the line height for text. This is a number between
     * 0.0 and 10.0 representing the leading between lines as a
     * multiplier of line height.
     * @see setLineHeight()
     */
    double lineHeight() const;

    /** Sets the line height for text.
     * @param height a number between
     * 0.0 and 10.0 representing the leading between lines as a
     * multiplier of line height.
     * @see lineHeight()
     */
    void setLineHeight( double height );

    /** Reads settings from a layer's custom properties.
     * @param layer source vector layer
     * @see writeToLayer()
     */
    void readFromLayer( QgsVectorLayer* layer );

    /** Writes settings to a layer's custom properties.
     * @param layer target vector layer
     * @see readFromLayer()
     */
    void writeToLayer( QgsVectorLayer* layer ) const;

    /** Read settings from a DOM element.
     * @see writeXml()
     */
    void readXml( const QDomElement& elem );

    /** Write settings into a DOM element.
     * @see readXml()
     */
    QDomElement writeXml( QDomDocument& doc ) const;

    /** Returns true if any component of the font format requires advanced effects
     * such as blend modes, which require output in raster formats to be fully respected.
     */
    bool containsAdvancedEffects() const;

    /** Returns true if the specified font was found on the system, or false
     * if the font was not found and a replacement was used instead.
     * @see resolvedFontFamily()
     */
    bool fontFound() const;

    /** Returns the family for the resolved font, ie if the specified font
     * was not found on the system this will return the name of the replacement
     * font.
     * @see fontFound()
     */
    QString resolvedFontFamily() const;

};


class QgsTextRenderer
{
%TypeHeaderCode
#include <qgstextrenderer.h>
%End

  public:

    enum TextPart
    {
      Text,
      Buffer,
      Background,
      Shadow
    };

    enum HAlignment
    {
      AlignLeft,
      AlignCenter,
      AlignRight,
    };

    /** Calculates pixel size (considering output size should be in pixel or map units, scale factors and optionally oversampling)
     * @param size size to convert
     * @param c rendercontext
     * @param unit size units
     * @param mapUnitScale a mapUnitScale clamper
     * @return font pixel size
     */
    static int sizeToPixel( double size, const QgsRenderContext& c, QgsUnitTypes::RenderUnit unit, const QgsMapUnitScale& mapUnitScale = QgsMapUnitScale() );

    /** Calculates size (considering output size should be in pixel or map units, scale factors and optionally oversampling)
     * @param size size to convert
     * @param c rendercontext
     * @param unit size units
     * @param mapUnitScale a mapUnitScale clamper
     * @return size that will render, as double
     */
    static double scaleToPixelContext( double size, const QgsRenderContext& c, QgsUnitTypes::RenderUnit unit, const QgsMapUnitScale& mapUnitScale = QgsMapUnitScale() );

    /** Draws text within a rectangle using the specified settings.
     * @param rect destination rectangle for text
     * @param rotation text rotation
     * @param textLines list of lines of text to draw
     * @param context render context
     * @param format text format
     * @param drawAsOutlines set to false to render text as text. This allows outputs to
     * formats like SVG to maintain text as text objects, but at the cost of degraded
     * rendering and may result in side effects like misaligned text buffers.
     */
    static void drawText( const QRectF& rect, double rotation, HAlignment alignment, const QStringList& textLines,
                          QgsRenderContext& context, const QgsTextFormat& format,
                          bool drawAsOutlines = true );

    /** Draws text at a point origin using the specified settings.
     * @param point origin of text
     * @param rotation text rotation
     * @param textLines list of lines of text to draw
     * @param context render context
     * @param format text format
     * @param drawAsOutlines set to false to render text as text. This allows outputs to
     * formats like SVG to maintain text as text objects, but at the cost of degraded
     * rendering and may result in side effects like misaligned text buffers.
     */
    static void drawText( const QPointF& point, double rotation, HAlignment alignment, const QStringList& textLines,
                          QgsRenderContext& context, const QgsTextFormat& format,
                          bool drawAsOutlines = true );

    /** Draws a single component of rendered text using the specified settings.
     * @param rect destination rectangle for text
     * @param rotation text rotation
     * @param textLines list of lines of text to draw
     * @param context render context
     * @param format text format
     * @param part component of text to draw
     * @param drawAsOutlines set to false to render text as text. This allows outputs to
     * formats like SVG to maintain text as text objects, but at the cost of degraded
     * rendering and may result in side effects like misaligned text buffers.
     */
    static void drawPart( const QRectF& rect, double rotation, HAlignment alignment, const QStringList& textLines,
                          QgsRenderContext& context, const QgsTextFormat& format,
                          TextPart part, bool drawAsOutlines = true );

    /** Draws a single component of rendered text using the specified settings.
     * @param origin origin for start of text. Y coordinate will be used as baseline.
     * @param rotation text rotation
     * @param textLines list of lines of text to draw
     * @param context render context
     * @param format text format
     * @param part component of text to draw. Note that Shadow parts cannot be drawn
     * individually and instead are drawn with their associated part (e.g., drawn together
     * with the text or background parts)
     * @param drawAsOutlines set to false to render text as text. This allows outputs to
     * formats like SVG to maintain text as text objects, but at the cost of degraded
     * rendering and may result in side effects like misaligned text buffers.
     */
    static void drawPart( const QPointF& origin, double rotation, HAlignment alignment, const QStringList& textLines,
                          QgsRenderContext& context, const QgsTextFormat& format,
                          TextPart part, bool drawAsOutlines = true );
};
