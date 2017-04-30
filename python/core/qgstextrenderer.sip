/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgstextrenderer.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsTextBufferSettings
{
%Docstring
 Container for settings relating to a text buffer.
.. note::

   QgsTextBufferSettings objects are implicitly shared.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgstextrenderer.h"
%End
  public:

    QgsTextBufferSettings();

    QgsTextBufferSettings( const QgsTextBufferSettings &other );
%Docstring
 Copy constructor.
 \param other source settings
%End


    ~QgsTextBufferSettings();
%Docstring
 Copy constructor.
 \param other source QgsTextBufferSettings
%End

    bool enabled() const;
%Docstring
 Returns whether the buffer is enabled.
 \see setEnabled()
 :rtype: bool
%End

    void setEnabled( bool enabled );
%Docstring
 Sets whether the text buffer will be drawn.
 \param enabled set to true to draw buffer
 \see enabled()
%End

    double size() const;
%Docstring
 Returns the size of the buffer.
 \see sizeUnit()
 \see setSize()
 :rtype: float
%End

    void setSize( double size );
%Docstring
 Sets the size of the buffer. The size units are specified using setSizeUnit().
 \param size buffer size
 \see size()
 \see setSizeUnit()
%End

    QgsUnitTypes::RenderUnit sizeUnit() const;
%Docstring
 Returns the units for the buffer size.
 \see size()
 \see setSizeUnit()
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setSizeUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
 Sets the units used for the buffer size.
 \param unit size unit
 \see setSize()
 \see sizeUnit()
%End

    QgsMapUnitScale sizeMapUnitScale() const;
%Docstring
 Returns the map unit scale object for the buffer size. This is only used if the
 buffer size is set to QgsUnitTypes.RenderMapUnit.
 \see setSizeMapUnitScale()
 \see sizeUnit()
 :rtype: QgsMapUnitScale
%End

    void setSizeMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
 Sets the map unit scale object for the buffer size. This is only used if the
 buffer size is set to QgsUnitTypes.RenderMapUnit.
 \param scale scale for buffer size
 \see sizeMapUnitScale()
 \see setSizeUnit()
%End

    QColor color() const;
%Docstring
 Returns the color of the buffer.
 \see setColor()
 :rtype: QColor
%End

    void setColor( const QColor &color );
%Docstring
 Sets the color for the buffer.
 \param color buffer color
 \see color()
%End

    bool fillBufferInterior() const;
%Docstring
 Returns whether the interior of the buffer will be filled in. If false, only the stroke
 of the text will be drawn as the buffer. The effect of this setting is only visible for
 semi-transparent text.
 \see setFillBufferInterior()
 :rtype: bool
%End

    void setFillBufferInterior( bool fill );
%Docstring
 Sets whether the interior of the buffer will be filled in.
 \param fill set to false to drawn only the stroke of the text as the buffer, or true to also
 shade the area inside the text. The effect of this setting is only visible for semi-transparent text.
 \see fillBufferInterior()
%End

    double opacity() const;
%Docstring
 Returns the buffer opacity. The opacity is a double value between 0 (fully transparent) and 1 (totally
 opaque).
 \see setOpacity()
 :rtype: float
%End

    void setOpacity( double opacity );
%Docstring
 Sets the buffer opacity.
 \param opacity opacity as a double value between 0 (fully transparent) and 1 (totally
 opaque)
 \see opacity()
%End

    Qt::PenJoinStyle joinStyle() const;
%Docstring
 Returns the buffer join style.
 \see setJoinStyle
 :rtype: Qt.PenJoinStyle
%End

    void setJoinStyle( Qt::PenJoinStyle style );
%Docstring
 Sets the join style used for drawing the buffer.
 \param style join style
 \see joinStyle()
%End

    QPainter::CompositionMode blendMode() const;
%Docstring
 Returns the blending mode used for drawing the buffer.
 \see setBlendMode()
 :rtype: QPainter.CompositionMode
%End

    void setBlendMode( QPainter::CompositionMode mode );
%Docstring
 Sets the blending mode used for drawing the buffer.
 \param mode blending mode
 \see blendMode()
%End

    void readFromLayer( QgsVectorLayer *layer );
%Docstring
 Reads settings from a layer's custom properties.
 \param layer source vector layer
 \see writeToLayer()
%End

    void writeToLayer( QgsVectorLayer *layer ) const;
%Docstring
 Writes settings to a layer's custom properties.
 \param layer target vector layer
 \see readFromLayer()
%End

    void readXml( const QDomElement &elem );
%Docstring
 Read settings from a DOM element.
 \see writeXml()
%End

    QDomElement writeXml( QDomDocument &doc ) const;
%Docstring
 Write settings into a DOM element.
 \see readXml()
 :rtype: QDomElement
%End

    QgsPaintEffect *paintEffect() const;
%Docstring
 Returns the current paint effect for the buffer.
 :return: paint effect
 \see setPaintEffect()
 :rtype: QgsPaintEffect
%End

    void setPaintEffect( QgsPaintEffect *effect /Transfer/ );
%Docstring
 Sets the current paint ``effect`` for the buffer.
 \param effect paint effect. Ownership is transferred to the buffer settings.
 \see paintEffect()
%End

};


class QgsTextBackgroundSettings
{
%Docstring
 Container for settings relating to a text background object.
.. note::

   QgsTextBackgroundSettings objects are implicitly shared.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgstextrenderer.h"
%End
  public:

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

    QgsTextBackgroundSettings();

    QgsTextBackgroundSettings( const QgsTextBackgroundSettings &other );
%Docstring
 Copy constructor.
 \param other source QgsTextBackgroundSettings
%End


    ~QgsTextBackgroundSettings();

    bool enabled() const;
%Docstring
 Returns whether the background is enabled.
 \see setEnabled()
 :rtype: bool
%End

    void setEnabled( bool enabled );
%Docstring
 Sets whether the text background will be drawn.
 \param enabled set to true to draw background
 \see enabled()
%End

    ShapeType type() const;
%Docstring
 Returns the type of background shape (e.g., square, ellipse, SVG).
 \see setType()
 :rtype: ShapeType
%End

    void setType( ShapeType type );
%Docstring
 Sets the type of background shape to draw (e.g., square, ellipse, SVG).
 \param type shape type
 \see type()
%End

    QString svgFile() const;
%Docstring
 Returns the path to the background SVG file, if set.
 \see setSvgFile()
 :rtype: str
%End

    void setSvgFile( const QString &file );
%Docstring
 Sets the path to the background SVG file. This is only used if type() is set to
 QgsTextBackgroundSettings.ShapeSVG.
 \param file SVG file path
 \see svgFile()
%End

    SizeType sizeType() const;
%Docstring
 Returns the method used to determine the size of the background shape (e.g., fixed size or buffer
 around text).
 \see setSizeType()
 \see size()
 :rtype: SizeType
%End

    void setSizeType( SizeType type );
%Docstring
 Sets the method used to determine the size of the background shape (e.g., fixed size or buffer
 around text).
 \param type size method
 \see sizeType()
 \see setSize()
%End

    QSizeF size() const;
%Docstring
 Returns the size of the background shape. The meaning of the size depends on the current sizeType(),
 e.g., for size types of QgsTextBackgroundSettings.SizeFixed the size will represent the actual width and
 height of the shape, for QgsTextBackgroundSettings.SizeBuffer the size will represent the horizontal
 and vertical margins to add to the text when calculating the size of the shape.
 \see setSize()
 \see sizeType()
 :rtype: QSizeF
%End

    void setSize( const QSizeF &size );
%Docstring
 Sets the size of the background shape. The meaning of the size depends on the current sizeType(),
 e.g., for size types of QgsTextBackgroundSettings.SizeFixed the size will represent the actual width and
 height of the shape, for QgsTextBackgroundSettings.SizeBuffer the size will represent the horizontal
 and vertical margins to add to the text when calculating the size of the shape.
 \param size QSizeF representing horizontal and vertical size components for shape
 \see size()
 \see setSizeType()
%End

    QgsUnitTypes::RenderUnit sizeUnit() const;
%Docstring
 Returns the units used for the shape's size. This value has no meaning if the sizeType() is set to
 QgsTextBackgroundSettings.SizePercent.
 \see setSizeUnit()
 \see sizeType()
 \see size()
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setSizeUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
 Sets the units used for the shape's size. This value has no meaning if the sizeType() is set to
 QgsTextBackgroundSettings.SizePercent.
 \param unit size units
 \see sizeUnit()
 \see setSizeType()
 \see setSize()
%End

    QgsMapUnitScale sizeMapUnitScale() const;
%Docstring
 Returns the map unit scale object for the shape size. This is only used if the
 sizeUnit() is set to QgsUnitTypes.RenderMapUnit.
 \see setSizeMapUnitScale()
 \see sizeUnit()
 :rtype: QgsMapUnitScale
%End

    void setSizeMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
 Sets the map unit scale object for the shape size. This is only used if the
 sizeUnit() is set to QgsUnitTypes.RenderMapUnit.
 \param scale scale for shape size
 \see sizeMapUnitScale()
 \see setSizeUnit()
%End

    RotationType rotationType() const;
%Docstring
 Returns the method used for rotating the background shape.
 \see setRotationType()
 \see rotation()
 :rtype: RotationType
%End

    void setRotationType( RotationType type );
%Docstring
 Sets the method used for rotating the background shape.
 \param type rotation method
 \see rotationType()
 \see setRotation()
%End

    double rotation() const;
%Docstring
 Returns the rotation for the background shape.
 \see rotationType()
 \see setRotation()
 :rtype: float
%End

    void setRotation( double rotation );
%Docstring
 Sets the rotation for the background shape.
 \param rotation angle in degrees to rotate
 \see rotation()
 \see setRotationType()
%End

    QPointF offset() const;
%Docstring
 Returns the offset used for drawing the background shape. Units are determined
 via offsetUnit().
 \see setOffset()
 \see offsetUnit()
 :rtype: QPointF
%End

    void setOffset( const QPointF &offset );
%Docstring
 Sets the offset used for drawing the background shape. Units are specified using
 setOffsetUnit().
 \param offset offset for shape
 \see offset()
 \see setOffsetUnit()
%End

    QgsUnitTypes::RenderUnit offsetUnit() const;
%Docstring
 Returns the units used for the shape's offset.
 \see setOffsetUnit()
 \see offset()
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setOffsetUnit( QgsUnitTypes::RenderUnit units );
%Docstring
 Sets the units used for the shape's offset.
 \param units offset units
 \see offsetUnit()
 \see setOffset()
%End

    QgsMapUnitScale offsetMapUnitScale() const;
%Docstring
 Returns the map unit scale object for the shape offset. This is only used if the
 offsetUnit() is set to QgsUnitTypes.RenderMapUnit.
 \see setOffsetMapUnitScale()
 \see offsetUnit()
 :rtype: QgsMapUnitScale
%End

    void setOffsetMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
 Sets the map unit scale object for the shape offset. This is only used if the
 offsetUnit() is set to QgsUnitTypes.RenderMapUnit.
 \param scale scale for shape offset
 \see offsetMapUnitScale()
 \see setOffsetUnit()
%End

    QSizeF radii() const;
%Docstring
 Returns the radii used for rounding the corners of shapes. Units are retrieved
 through radiiUnit().
 \see setRadii()
 \see radiiUnit()
 :rtype: QSizeF
%End

    void setRadii( const QSizeF &radii );
%Docstring
 Sets the radii used for rounding the corners of shapes. This is only used if
 type() is set to QgsTextBackgroundSettings.ShapeRectangle or QgsTextBackgroundSettings.ShapeSquare.
 \param radii QSizeF representing horizontal and vertical radii for rounded corners. Units are
 specified through setRadiiUnit()
 \see radii()
 \see setRadiiUnit()
%End

    QgsUnitTypes::RenderUnit radiiUnit() const;
%Docstring
 Returns the units used for the shape's radii.
 \see setRadiiUnit()
 \see radii()
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setRadiiUnit( QgsUnitTypes::RenderUnit units );
%Docstring
 Sets the units used for the shape's radii.
 \param units radii units
 \see radiiUnit()
 \see setRadii()
%End

    QgsMapUnitScale radiiMapUnitScale() const;
%Docstring
 Returns the map unit scale object for the shape radii. This is only used if the
 radiiUnit() is set to QgsUnitTypes.RenderMapUnit.
 \see setRadiiMapUnitScale()
 \see radiiUnit()
 :rtype: QgsMapUnitScale
%End

    void setRadiiMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
 Sets the map unit scale object for the shape radii. This is only used if the
 radiiUnit() is set to QgsUnitTypes.RenderMapUnit.
 \param scale scale for shape radii
 \see radiiMapUnitScale()
 \see setRadiiUnit()
%End

    double opacity() const;
%Docstring
 Returns the background shape's opacity. The opacity is a double value between 0 (fully transparent) and 1 (totally
 opaque).
 \see setOpacity()
 :rtype: float
%End

    void setOpacity( double opacity );
%Docstring
 Sets the background shape's opacity.
 \param opacity opacity as a double value between 0 (fully transparent) and 1 (totally
 opaque)
 \see opacity()
%End

    QPainter::CompositionMode blendMode() const;
%Docstring
 Returns the blending mode used for drawing the background shape.
 \see setBlendMode()
 :rtype: QPainter.CompositionMode
%End

    void setBlendMode( QPainter::CompositionMode mode );
%Docstring
 Sets the blending mode used for drawing the background shape.
 \param mode blending mode
 \see blendMode()
%End

    QColor fillColor() const;
%Docstring
 Returns the color used for filing the background shape.
 \see setFillColor()
 \see strokeColor()
 :rtype: QColor
%End

    void setFillColor( const QColor &color );
%Docstring
 Sets the color used for filing the background shape.
 \param color background color
 \see fillColor()
 \see setStrokeColor()
%End

    QColor strokeColor() const;
%Docstring
 Returns the color used for outlining the background shape.
 \see setStrokeColor()
 \see fillColor()
 :rtype: QColor
%End

    void setStrokeColor( const QColor &color );
%Docstring
 Sets the color used for outlining the background shape.
 \param color stroke color
 \see strokeColor()
 \see setFillColor()
%End

    double strokeWidth() const;
%Docstring
 Returns the width of the shape's stroke (stroke). Units are retrieved through
 strokeWidthUnit().
 \see setStrokeWidth()
 \see strokeWidthUnit()
 :rtype: float
%End

    void setStrokeWidth( double width );
%Docstring
 Sets the width of the shape's stroke (stroke). Units are specified through
 setStrokeWidthUnit().
 \see strokeWidth()
 \see setStrokeWidthUnit()
%End

    QgsUnitTypes::RenderUnit strokeWidthUnit() const;
%Docstring
 Returns the units used for the shape's stroke width.
 \see setStrokeWidthUnit()
 \see strokeWidth()
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setStrokeWidthUnit( QgsUnitTypes::RenderUnit units );
%Docstring
 Sets the units used for the shape's stroke width.
 \param units stroke width units
 \see strokeWidthUnit()
 \see setStrokeWidth()
%End

    QgsMapUnitScale strokeWidthMapUnitScale() const;
%Docstring
 Returns the map unit scale object for the shape stroke width. This is only used if the
 strokeWidthUnit() is set to QgsUnitTypes.RenderMapUnit.
 \see setStrokeWidthMapUnitScale()
 \see strokeWidthUnit()
 :rtype: QgsMapUnitScale
%End

    void setStrokeWidthMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
 Sets the map unit scale object for the shape stroke width. This is only used if the
 strokeWidthUnit() is set to QgsUnitTypes.RenderMapUnit.
 \param scale scale for shape stroke width
 \see strokeWidthMapUnitScale()
 \see setStrokeWidthUnit()
%End

    Qt::PenJoinStyle joinStyle() const;
%Docstring
 Returns the join style used for drawing the background shape.
 \see setJoinStyle
 :rtype: Qt.PenJoinStyle
%End

    void setJoinStyle( Qt::PenJoinStyle style );
%Docstring
 Sets the join style used for drawing the background shape.
 \param style join style
 \see joinStyle()
%End

    QgsPaintEffect *paintEffect() const;
%Docstring
 Returns the current paint effect for the background shape.
 :return: paint effect
 \see setPaintEffect()
 :rtype: QgsPaintEffect
%End

    void setPaintEffect( QgsPaintEffect *effect /Transfer/ );
%Docstring
 Sets the current paint ``effect`` for the background shape.
 \param effect paint effect. Ownership is transferred to the background settings.
 \see paintEffect()
%End

    void readFromLayer( QgsVectorLayer *layer );
%Docstring
 Reads settings from a layer's custom properties.
 \param layer source vector layer
 \see writeToLayer()
%End

    void writeToLayer( QgsVectorLayer *layer ) const;
%Docstring
 Writes settings to a layer's custom properties.
 \param layer target vector layer
 \see readFromLayer()
%End

    void readXml( const QDomElement &elem );
%Docstring
 Read settings from a DOM element.
 \see writeXml()
%End

    QDomElement writeXml( QDomDocument &doc ) const;
%Docstring
 Write settings into a DOM element.
 \see readXml()
 :rtype: QDomElement
%End

};


class QgsTextShadowSettings
{
%Docstring
 Container for settings relating to a text shadow.
.. note::

   QgsTextShadowSettings objects are implicitly shared.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgstextrenderer.h"
%End
  public:

    enum ShadowPlacement
    {
      ShadowLowest,
      ShadowText,
      ShadowBuffer,
      ShadowShape
    };

    QgsTextShadowSettings();

    QgsTextShadowSettings( const QgsTextShadowSettings &other );
%Docstring
 Copy constructor.
 \param other source QgsTextShadowSettings
%End


    ~QgsTextShadowSettings();

    bool enabled() const;
%Docstring
 Returns whether the shadow is enabled.
 \see setEnabled()
 :rtype: bool
%End

    void setEnabled( bool enabled );
%Docstring
 Sets whether the text shadow will be drawn.
 \param enabled set to true to draw shadow
 \see enabled()
%End

    QgsTextShadowSettings::ShadowPlacement shadowPlacement() const;
%Docstring
 Returns the placement for the drop shadow. The placement determines
 both the z-order stacking position for the shadow and the what shape (e.g., text,
 background shape) is used for casting the shadow.
 \see setShadowPlacement()
 :rtype: QgsTextShadowSettings.ShadowPlacement
%End

    void setShadowPlacement( QgsTextShadowSettings::ShadowPlacement placement );
%Docstring
 Sets the placement for the drop shadow. The placement determines
 both the z-order stacking position for the shadow and the what shape (e.g., text,
 background shape) is used for casting the shadow.
 \param placement shadow placement
 \see shadowPlacement()
%End

    int offsetAngle() const;
%Docstring
 Returns the angle for offsetting the position of the shadow from the text.
 \see setOffsetAngle
 \see offsetDistance()
 :rtype: int
%End

    void setOffsetAngle( int angle );
%Docstring
 Sets the angle for offsetting the position of the shadow from the text.
 \param angle offset angle in degrees
 \see offsetAngle()
 \see setOffsetDistance()
%End

    double offsetDistance() const;
%Docstring
 Returns the distance for offsetting the position of the shadow from the text. Offset units
 are retrieved via offsetUnit().
 \see setOffsetDistance()
 \see offsetUnit()
 :rtype: float
%End

    void setOffsetDistance( double distance );
%Docstring
 Sets the distance for offsetting the position of the shadow from the text. Offset units
 are specified via setOffsetUnit().
 \param distance offset distance
 \see offsetDistance()
 \see setOffsetUnit()
%End

    QgsUnitTypes::RenderUnit offsetUnit() const;
%Docstring
 Returns the units used for the shadow's offset.
 \see setOffsetUnit()
 \see offsetDistance()
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setOffsetUnit( QgsUnitTypes::RenderUnit units );
%Docstring
 Sets the units used for the shadow's offset.
 \param units shadow distance units
 \see offsetUnit()
 \see setOffsetDistance()
%End

    QgsMapUnitScale offsetMapUnitScale() const;
%Docstring
 Returns the map unit scale object for the shadow offset distance. This is only used if the
 offsetUnit() is set to QgsUnitTypes.RenderMapUnit.
 \see setOffsetMapUnitScale()
 \see offsetUnit()
 :rtype: QgsMapUnitScale
%End

    void setOffsetMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
 Sets the map unit scale object for the shadow offset distance. This is only used if the
 offsetUnit() is set to QgsUnitTypes.RenderMapUnit.
 \param scale scale for shadow offset
 \see offsetMapUnitScale()
 \see setOffsetUnit()
%End

    bool offsetGlobal() const;
%Docstring
 Returns true if the global shadow offset will be used.
 \see setOffsetGlobal()
 :rtype: bool
%End

    void setOffsetGlobal( bool global );
%Docstring
 Sets whether the global shadow offset should be used.
 \param global set to true to use global shadow offset.
%End

    double blurRadius() const;
%Docstring
 Returns the blur radius for the shadow. Radius units are retrieved via blurRadiusUnits().
 \see setBlurRadius()
 \see blurRadiusUnit()
 :rtype: float
%End

    void setBlurRadius( double blurRadius );
%Docstring
 Sets the blur radius for the shadow. Radius units are specified via setBlurRadiusUnits().
 \param blurRadius blur radius
 \see blurRadius()
 \see setBlurRadiusUnit()
%End

    QgsUnitTypes::RenderUnit blurRadiusUnit() const;
%Docstring
 Returns the units used for the shadow's blur radius.
 \see setBlurRadiusUnit()
 \see blurRadius()
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setBlurRadiusUnit( QgsUnitTypes::RenderUnit units );
%Docstring
 Sets the units used for the shadow's blur radius.
 \param units shadow blur radius units
 \see blurRadiusUnit()
 \see setBlurRadius()
%End

    QgsMapUnitScale blurRadiusMapUnitScale() const;
%Docstring
 Returns the map unit scale object for the shadow blur radius. This is only used if the
 blurRadiusUnit() is set to QgsUnitTypes.RenderMapUnit.
 \see setBlurRadiusMapUnitScale()
 \see blurRadiusUnit()
 :rtype: QgsMapUnitScale
%End

    void setBlurRadiusMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
 Sets the map unit scale object for the shadow blur radius. This is only used if the
 blurRadiusUnit() is set to QgsUnitTypes.RenderMapUnit.
 \param scale scale for shadow blur radius
 \see blurRadiusMapUnitScale()
 \see setBlurRadiusUnit()
%End

    bool blurAlphaOnly() const;
%Docstring
 Returns whether only the alpha channel for the shadow will be blurred.
 \see setBlurAlphaOnly()
 :rtype: bool
%End

    void setBlurAlphaOnly( bool alphaOnly );
%Docstring
 Sets whether only the alpha channel for the shadow should be blurred.
 \param alphaOnly set to true to blur only the alpha channel. If false, all channels (including
 red, green and blue channel) will be blurred.
 \see blurAlphaOnly()
%End

    double opacity() const;
%Docstring
 Returns the shadow's opacity. The opacity is a double value between 0 (fully transparent) and 1 (totally
 opaque).
 \see setOpacity()
 :rtype: float
%End

    void setOpacity( double opacity );
%Docstring
 Sets the shadow's opacity.
 \param opacity opacity as a double value between 0 (fully transparent) and 1 (totally
 opaque)
 \see opacity()
%End

    int scale() const;
%Docstring
 Returns the scaling used for the drop shadow (in percentage of original size).
 \see setScale()
 :rtype: int
%End

    void setScale( int scale );
%Docstring
 Sets the scaling used for the drop shadow (in percentage of original size).
 \param scale scale percent for drop shadow
 \see scale()
%End

    QColor color() const;
%Docstring
 Returns the color of the drop shadow.
 \see setColor()
 :rtype: QColor
%End

    void setColor( const QColor &color );
%Docstring
 Sets the color for the drop shadow.
 \param color shadow color
 \see color()
%End

    QPainter::CompositionMode blendMode() const;
%Docstring
 Returns the blending mode used for drawing the drop shadow.
 \see setBlendMode()
 :rtype: QPainter.CompositionMode
%End

    void setBlendMode( QPainter::CompositionMode mode );
%Docstring
 Sets the blending mode used for drawing the drop shadow.
 \param mode blending mode
 \see blendMode()
%End

    void readFromLayer( QgsVectorLayer *layer );
%Docstring
 Reads settings from a layer's custom properties.
 \param layer source vector layer
 \see writeToLayer()
%End

    void writeToLayer( QgsVectorLayer *layer ) const;
%Docstring
 Writes settings to a layer's custom properties.
 \param layer target vector layer
 \see readFromLayer()
%End

    void readXml( const QDomElement &elem );
%Docstring
 Read settings from a DOM element.
 \see writeXml()
%End

    QDomElement writeXml( QDomDocument &doc ) const;
%Docstring
 Write settings into a DOM element.
 \see readXml()
 :rtype: QDomElement
%End

};



class QgsTextFormat
{
%Docstring
 Container for all settings relating to text rendering.
.. note::

   QgsTextFormat objects are implicitly shared.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgstextrenderer.h"
%End
  public:

    QgsTextFormat();

    QgsTextFormat( const QgsTextFormat &other );
%Docstring
 Copy constructor.
 \param other source QgsTextFormat
%End


    ~QgsTextFormat();

    QgsTextBufferSettings &buffer();
%Docstring
 Returns a reference to the text buffer settings.
 \see setBuffer()
 :rtype: QgsTextBufferSettings
%End


    void setBuffer( const QgsTextBufferSettings &bufferSettings );
%Docstring
 Sets the text's buffer settings.
 \param bufferSettings buffer settings
 \see buffer()
%End

    QgsTextBackgroundSettings &background();
%Docstring
 Returns a reference to the text background settings.
 \see setBackground()
 :rtype: QgsTextBackgroundSettings
%End


    void setBackground( const QgsTextBackgroundSettings &backgroundSettings );
%Docstring
 Sets the text's background settings.q
 \param backgroundSettings background settings
 \see background()
%End

    QgsTextShadowSettings &shadow();
%Docstring
 Returns a reference to the text drop shadow settings.
 \see setShadow()
 :rtype: QgsTextShadowSettings
%End


    void setShadow( const QgsTextShadowSettings &shadowSettings );
%Docstring
 Sets the text's drop shadow settings.
 \param shadowSettings shadow settings
 \see shadow()
%End

    QFont font() const;
%Docstring
 Returns the font used for rendering text. Note that the size of the font
 is not used, and size() should be called instead to determine the size
 of rendered text.
 \see scaledFont()
 \see setFont()
 \see namedStyle()
 :rtype: QFont
%End

    QFont scaledFont( const QgsRenderContext &context ) const;
%Docstring
 Returns a font with the size scaled to match the format's size settings (including
 units and map unit scale) for a specified render context.
 \param context destination render context
 :return: font with scaled size
 \see font()
 \see size()
 :rtype: QFont
%End

    void setFont( const QFont &font );
%Docstring
 Sets the font used for rendering text. Note that the size of the font
 is not used, and setSize() should be called instead to explicitly set the size
 of rendered text.
 \param font desired font
 \see font()
 \see setNamedStyle()
%End

    QString namedStyle() const;
%Docstring
 Returns the named style for the font used for rendering text (e.g., "bold").
 \see setNamedStyle()
 \see font()
 :rtype: str
%End

    void setNamedStyle( const QString &style );
%Docstring
 Sets the named style for the font used for rendering text.
 \param style named style, e.g., "bold"
 \see namedStyle()
 \see setFont()
%End

    double size() const;
%Docstring
 Returns the size for rendered text. Units are retrieved using sizeUnit().
 \see setSize()
 \see sizeUnit()
 :rtype: float
%End

    void setSize( double size );
%Docstring
 Sets the size for rendered text.
 \param size size of rendered text. Units are set using setSizeUnit()
 \see size()
 \see setSizeUnit()
%End

    QgsUnitTypes::RenderUnit sizeUnit() const;
%Docstring
 Returns the units for the size of rendered text.
 \see size()
 \see setSizeUnit()
 \see sizeMapUnitScale()
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setSizeUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
 Sets the units for the size of rendered text.
 \param unit size units
 \see setSize()
 \see sizeUnit()
 \see setSizeMapUnitScale()
%End

    QgsMapUnitScale sizeMapUnitScale() const;
%Docstring
 Returns the map unit scale object for the size. This is only used if the
 sizeUnit() is set to QgsUnitTypes.RenderMapUnit.
 \see setSizeMapUnitScale()
 \see sizeUnit()
 :rtype: QgsMapUnitScale
%End

    void setSizeMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
 Sets the map unit scale object for the size. This is only used if the
 sizeUnit() is set to QgsUnitTypes.RenderMapUnit.
 \see sizeMapUnitScale()
 \see setSizeUnit()
%End

    QColor color() const;
%Docstring
 Returns the color that text will be rendered in.
 \see setColor()
 :rtype: QColor
%End

    void setColor( const QColor &color );
%Docstring
 Sets the color that text will be rendered in.
 \param color text color
 \see color()
%End

    double opacity() const;
%Docstring
 Returns the text's opacity. The opacity is a double value between 0 (fully transparent) and 1 (totally
 opaque).
 \see setOpacity()
 :rtype: float
%End

    void setOpacity( double opacity );
%Docstring
 Sets the text's opacity.
 \param opacity opacity as a double value between 0 (fully transparent) and 1 (totally
 opaque)
 \see opacity()
%End

    QPainter::CompositionMode blendMode() const;
%Docstring
 Returns the blending mode used for drawing the text.
 \see setBlendMode()
 :rtype: QPainter.CompositionMode
%End

    void setBlendMode( QPainter::CompositionMode mode );
%Docstring
 Sets the blending mode used for drawing the text.
 \param mode blending mode
 \see blendMode()
%End

    double lineHeight() const;
%Docstring
 Returns the line height for text. This is a number between
 0.0 and 10.0 representing the leading between lines as a
 multiplier of line height.
 \see setLineHeight()
 :rtype: float
%End

    void setLineHeight( double height );
%Docstring
 Sets the line height for text.
 \param height a number between
 0.0 and 10.0 representing the leading between lines as a
 multiplier of line height.
 \see lineHeight()
%End

    void readFromLayer( QgsVectorLayer *layer );
%Docstring
 Reads settings from a layer's custom properties.
 \param layer source vector layer
 \see writeToLayer()
%End

    void writeToLayer( QgsVectorLayer *layer ) const;
%Docstring
 Writes settings to a layer's custom properties.
 \param layer target vector layer
 \see readFromLayer()
%End

    void readXml( const QDomElement &elem );
%Docstring
 Read settings from a DOM element.
 \see writeXml()
%End

    QDomElement writeXml( QDomDocument &doc ) const;
%Docstring
 Write settings into a DOM element.
 \see readXml()
 :rtype: QDomElement
%End

    bool containsAdvancedEffects() const;
%Docstring
 Returns true if any component of the font format requires advanced effects
 such as blend modes, which require output in raster formats to be fully respected.
 :rtype: bool
%End

    bool fontFound() const;
%Docstring
 Returns true if the specified font was found on the system, or false
 if the font was not found and a replacement was used instead.
 \see resolvedFontFamily()
 :rtype: bool
%End

    QString resolvedFontFamily() const;
%Docstring
 Returns the family for the resolved font, ie if the specified font
 was not found on the system this will return the name of the replacement
 font.
 \see fontFound()
 :rtype: str
%End

};


class QgsTextRenderer
{
%Docstring
 Handles rendering text using rich formatting options, including drop shadows, buffers
 and background shapes.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgstextrenderer.h"
%End
  public:

    enum TextPart
    {
      Text,
      Buffer,
      Background,
      Shadow,
    };

    enum HAlignment
    {
      AlignLeft,
      AlignCenter,
      AlignRight,
    };

    static int sizeToPixel( double size, const QgsRenderContext &c, QgsUnitTypes::RenderUnit unit, const QgsMapUnitScale &mapUnitScale = QgsMapUnitScale() );
%Docstring
 Calculates pixel size (considering output size should be in pixel or map units, scale factors and optionally oversampling)
 \param size size to convert
 \param c rendercontext
 \param unit size units
 \param mapUnitScale a mapUnitScale clamper
 :return: font pixel size
 :rtype: int
%End

    static void drawText( const QRectF &rect, double rotation, HAlignment alignment, const QStringList &textLines,
                          QgsRenderContext &context, const QgsTextFormat &format,
                          bool drawAsOutlines = true );
%Docstring
 Draws text within a rectangle using the specified settings.
 \param rect destination rectangle for text
 \param rotation text rotation
 \param alignment horizontal alignment
 \param textLines list of lines of text to draw
 \param context render context
 \param format text format
 \param drawAsOutlines set to false to render text as text. This allows outputs to
 formats like SVG to maintain text as text objects, but at the cost of degraded
 rendering and may result in side effects like misaligned text buffers.
%End

    static void drawText( const QPointF &point, double rotation, HAlignment alignment, const QStringList &textLines,
                          QgsRenderContext &context, const QgsTextFormat &format,
                          bool drawAsOutlines = true );
%Docstring
 Draws text at a point origin using the specified settings.
 \param point origin of text
 \param rotation text rotation
 \param alignment horizontal alignment
 \param textLines list of lines of text to draw
 \param context render context
 \param format text format
 \param drawAsOutlines set to false to render text as text. This allows outputs to
 formats like SVG to maintain text as text objects, but at the cost of degraded
 rendering and may result in side effects like misaligned text buffers.
%End

    static void drawPart( const QRectF &rect, double rotation, HAlignment alignment, const QStringList &textLines,
                          QgsRenderContext &context, const QgsTextFormat &format,
                          TextPart part, bool drawAsOutlines = true );
%Docstring
 Draws a single component of rendered text using the specified settings.
 \param rect destination rectangle for text
 \param rotation text rotation
 \param alignment horizontal alignment
 \param textLines list of lines of text to draw
 \param context render context
 \param format text format
 \param part component of text to draw. Note that Shadow parts cannot be drawn
 individually and instead are drawn with their associated part (e.g., drawn together
 with the text or background parts)
 \param drawAsOutlines set to false to render text as text. This allows outputs to
 formats like SVG to maintain text as text objects, but at the cost of degraded
 rendering and may result in side effects like misaligned text buffers.
%End

    static void drawPart( const QPointF &origin, double rotation, HAlignment alignment, const QStringList &textLines,
                          QgsRenderContext &context, const QgsTextFormat &format,
                          TextPart part, bool drawAsOutlines = true );
%Docstring
 Draws a single component of rendered text using the specified settings.
 \param origin origin for start of text. Y coordinate will be used as baseline.
 \param rotation text rotation
 \param alignment horizontal alignment
 \param textLines list of lines of text to draw
 \param context render context
 \param format text format
 \param part component of text to draw. Note that Shadow parts cannot be drawn
 individually and instead are drawn with their associated part (e.g., drawn together
 with the text or background parts)
 \param drawAsOutlines set to false to render text as text. This allows outputs to
 formats like SVG to maintain text as text objects, but at the cost of degraded
 rendering and may result in side effects like misaligned text buffers.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgstextrenderer.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
