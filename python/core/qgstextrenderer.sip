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

:param other: source settings
%End


    ~QgsTextBufferSettings();

    bool enabled() const;
%Docstring
Returns whether the buffer is enabled.

.. seealso:: :py:func:`setEnabled`
%End

    void setEnabled( bool enabled );
%Docstring
Sets whether the text buffer will be drawn.

:param enabled: set to true to draw buffer

.. seealso:: :py:func:`enabled`
%End

    double size() const;
%Docstring
Returns the size of the buffer.

.. seealso:: :py:func:`sizeUnit`

.. seealso:: :py:func:`setSize`
%End

    void setSize( double size );
%Docstring
Sets the size of the buffer. The size units are specified using setSizeUnit().

:param size: buffer size

.. seealso:: :py:func:`size`

.. seealso:: :py:func:`setSizeUnit`
%End

    QgsUnitTypes::RenderUnit sizeUnit() const;
%Docstring
Returns the units for the buffer size.

.. seealso:: :py:func:`size`

.. seealso:: :py:func:`setSizeUnit`
%End

    void setSizeUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units used for the buffer size.

:param unit: size unit

.. seealso:: :py:func:`setSize`

.. seealso:: :py:func:`sizeUnit`
%End

    QgsMapUnitScale sizeMapUnitScale() const;
%Docstring
Returns the map unit scale object for the buffer size. This is only used if the
buffer size is set to QgsUnitTypes.RenderMapUnit.

.. seealso:: :py:func:`setSizeMapUnitScale`

.. seealso:: :py:func:`sizeUnit`
%End

    void setSizeMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
Sets the map unit scale object for the buffer size. This is only used if the
buffer size is set to QgsUnitTypes.RenderMapUnit.

:param scale: scale for buffer size

.. seealso:: :py:func:`sizeMapUnitScale`

.. seealso:: :py:func:`setSizeUnit`
%End

    QColor color() const;
%Docstring
Returns the color of the buffer.

.. seealso:: :py:func:`setColor`
%End

    void setColor( const QColor &color );
%Docstring
Sets the color for the buffer.

:param color: buffer color

.. seealso:: :py:func:`color`
%End

    bool fillBufferInterior() const;
%Docstring
Returns whether the interior of the buffer will be filled in. If false, only the stroke
of the text will be drawn as the buffer. The effect of this setting is only visible for
semi-transparent text.

.. seealso:: :py:func:`setFillBufferInterior`
%End

    void setFillBufferInterior( bool fill );
%Docstring
Sets whether the interior of the buffer will be filled in.

:param fill: set to false to drawn only the stroke of the text as the buffer, or true to also
shade the area inside the text. The effect of this setting is only visible for semi-transparent text.

.. seealso:: :py:func:`fillBufferInterior`
%End

    double opacity() const;
%Docstring
Returns the buffer opacity. The opacity is a double value between 0 (fully transparent) and 1 (totally
opaque).

.. seealso:: :py:func:`setOpacity`
%End

    void setOpacity( double opacity );
%Docstring
Sets the buffer opacity.

:param opacity: opacity as a double value between 0 (fully transparent) and 1 (totally
opaque)

.. seealso:: :py:func:`opacity`
%End

    Qt::PenJoinStyle joinStyle() const;
%Docstring
Returns the buffer join style.

.. seealso:: :py:func:`setJoinStyle`
%End

    void setJoinStyle( Qt::PenJoinStyle style );
%Docstring
Sets the join style used for drawing the buffer.

:param style: join style

.. seealso:: :py:func:`joinStyle`
%End

    QPainter::CompositionMode blendMode() const;
%Docstring
Returns the blending mode used for drawing the buffer.

.. seealso:: :py:func:`setBlendMode`
%End

    void setBlendMode( QPainter::CompositionMode mode );
%Docstring
Sets the blending mode used for drawing the buffer.

:param mode: blending mode

.. seealso:: :py:func:`blendMode`
%End

    void readFromLayer( QgsVectorLayer *layer );
%Docstring
Reads settings from a layer's custom properties (for QGIS 2.x projects).

:param layer: source vector layer
%End

    void readXml( const QDomElement &elem );
%Docstring
Read settings from a DOM element.

.. seealso:: :py:func:`writeXml`
%End

    QDomElement writeXml( QDomDocument &doc ) const;
%Docstring
Write settings into a DOM element.

.. seealso:: :py:func:`readXml`
%End

    QgsPaintEffect *paintEffect() const;
%Docstring
Returns the current paint effect for the buffer.

:return: paint effect

.. seealso:: :py:func:`setPaintEffect`
%End

    void setPaintEffect( QgsPaintEffect *effect /Transfer/ );
%Docstring
Sets the current paint ``effect`` for the buffer.

:param effect: paint effect. Ownership is transferred to the buffer settings.

.. seealso:: :py:func:`paintEffect`
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

:param other: source QgsTextBackgroundSettings
%End


    ~QgsTextBackgroundSettings();

    bool enabled() const;
%Docstring
Returns whether the background is enabled.

.. seealso:: :py:func:`setEnabled`
%End

    void setEnabled( bool enabled );
%Docstring
Sets whether the text background will be drawn.

:param enabled: set to true to draw background

.. seealso:: :py:func:`enabled`
%End

    ShapeType type() const;
%Docstring
Returns the type of background shape (e.g., square, ellipse, SVG).

.. seealso:: :py:func:`setType`
%End

    void setType( ShapeType type );
%Docstring
Sets the type of background shape to draw (e.g., square, ellipse, SVG).

:param type: shape type

.. seealso:: :py:func:`type`
%End

    QString svgFile() const;
%Docstring
Returns the absolute path to the background SVG file, if set.

.. seealso:: :py:func:`setSvgFile`
%End

    void setSvgFile( const QString &file );
%Docstring
Sets the path to the background SVG file. This is only used if type() is set to
QgsTextBackgroundSettings.ShapeSVG. The path must be absolute.

:param file: Absolute SVG file path

.. seealso:: :py:func:`svgFile`
%End

    SizeType sizeType() const;
%Docstring
Returns the method used to determine the size of the background shape (e.g., fixed size or buffer
around text).

.. seealso:: :py:func:`setSizeType`

.. seealso:: :py:func:`size`
%End

    void setSizeType( SizeType type );
%Docstring
Sets the method used to determine the size of the background shape (e.g., fixed size or buffer
around text).

:param type: size method

.. seealso:: :py:func:`sizeType`

.. seealso:: :py:func:`setSize`
%End

    QSizeF size() const;
%Docstring
Returns the size of the background shape. The meaning of the size depends on the current sizeType(),
e.g., for size types of QgsTextBackgroundSettings.SizeFixed the size will represent the actual width and
height of the shape, for QgsTextBackgroundSettings.SizeBuffer the size will represent the horizontal
and vertical margins to add to the text when calculating the size of the shape.

.. seealso:: :py:func:`setSize`

.. seealso:: :py:func:`sizeType`
%End

    void setSize( const QSizeF &size );
%Docstring
Sets the size of the background shape. The meaning of the size depends on the current sizeType(),
e.g., for size types of QgsTextBackgroundSettings.SizeFixed the size will represent the actual width and
height of the shape, for QgsTextBackgroundSettings.SizeBuffer the size will represent the horizontal
and vertical margins to add to the text when calculating the size of the shape.

:param size: QSizeF representing horizontal and vertical size components for shape

.. seealso:: :py:func:`size`

.. seealso:: :py:func:`setSizeType`
%End

    QgsUnitTypes::RenderUnit sizeUnit() const;
%Docstring
Returns the units used for the shape's size. This value has no meaning if the sizeType() is set to
QgsTextBackgroundSettings.SizePercent.

.. seealso:: :py:func:`setSizeUnit`

.. seealso:: :py:func:`sizeType`

.. seealso:: :py:func:`size`
%End

    void setSizeUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units used for the shape's size. This value has no meaning if the sizeType() is set to
QgsTextBackgroundSettings.SizePercent.

:param unit: size units

.. seealso:: :py:func:`sizeUnit`

.. seealso:: :py:func:`setSizeType`

.. seealso:: :py:func:`setSize`
%End

    QgsMapUnitScale sizeMapUnitScale() const;
%Docstring
Returns the map unit scale object for the shape size. This is only used if the
sizeUnit() is set to QgsUnitTypes.RenderMapUnit.

.. seealso:: :py:func:`setSizeMapUnitScale`

.. seealso:: :py:func:`sizeUnit`
%End

    void setSizeMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
Sets the map unit scale object for the shape size. This is only used if the
sizeUnit() is set to QgsUnitTypes.RenderMapUnit.

:param scale: scale for shape size

.. seealso:: :py:func:`sizeMapUnitScale`

.. seealso:: :py:func:`setSizeUnit`
%End

    RotationType rotationType() const;
%Docstring
Returns the method used for rotating the background shape.

.. seealso:: :py:func:`setRotationType`

.. seealso:: :py:func:`rotation`
%End

    void setRotationType( RotationType type );
%Docstring
Sets the method used for rotating the background shape.

:param type: rotation method

.. seealso:: :py:func:`rotationType`

.. seealso:: :py:func:`setRotation`
%End

    double rotation() const;
%Docstring
Returns the rotation for the background shape, in degrees clockwise.

.. seealso:: :py:func:`rotationType`

.. seealso:: :py:func:`setRotation`
%End

    void setRotation( double rotation );
%Docstring
Sets the ``rotation`` for the background shape, in degrees clockwise.

.. seealso:: :py:func:`rotation`

.. seealso:: :py:func:`setRotationType`
%End

    QPointF offset() const;
%Docstring
Returns the offset used for drawing the background shape. Units are determined
via offsetUnit().

.. seealso:: :py:func:`setOffset`

.. seealso:: :py:func:`offsetUnit`
%End

    void setOffset( const QPointF &offset );
%Docstring
Sets the offset used for drawing the background shape. Units are specified using
setOffsetUnit().

:param offset: offset for shape

.. seealso:: :py:func:`offset`

.. seealso:: :py:func:`setOffsetUnit`
%End

    QgsUnitTypes::RenderUnit offsetUnit() const;
%Docstring
Returns the units used for the shape's offset.

.. seealso:: :py:func:`setOffsetUnit`

.. seealso:: :py:func:`offset`
%End

    void setOffsetUnit( QgsUnitTypes::RenderUnit units );
%Docstring
Sets the units used for the shape's offset.

:param units: offset units

.. seealso:: :py:func:`offsetUnit`

.. seealso:: :py:func:`setOffset`
%End

    QgsMapUnitScale offsetMapUnitScale() const;
%Docstring
Returns the map unit scale object for the shape offset. This is only used if the
offsetUnit() is set to QgsUnitTypes.RenderMapUnit.

.. seealso:: :py:func:`setOffsetMapUnitScale`

.. seealso:: :py:func:`offsetUnit`
%End

    void setOffsetMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
Sets the map unit scale object for the shape offset. This is only used if the
offsetUnit() is set to QgsUnitTypes.RenderMapUnit.

:param scale: scale for shape offset

.. seealso:: :py:func:`offsetMapUnitScale`

.. seealso:: :py:func:`setOffsetUnit`
%End

    QSizeF radii() const;
%Docstring
Returns the radii used for rounding the corners of shapes. Units are retrieved
through radiiUnit().

.. seealso:: :py:func:`setRadii`

.. seealso:: :py:func:`radiiUnit`
%End

    void setRadii( const QSizeF &radii );
%Docstring
Sets the radii used for rounding the corners of shapes. This is only used if
type() is set to QgsTextBackgroundSettings.ShapeRectangle or QgsTextBackgroundSettings.ShapeSquare.

:param radii: QSizeF representing horizontal and vertical radii for rounded corners. Units are
specified through setRadiiUnit()

.. seealso:: :py:func:`radii`

.. seealso:: :py:func:`setRadiiUnit`
%End

    QgsUnitTypes::RenderUnit radiiUnit() const;
%Docstring
Returns the units used for the shape's radii.

.. seealso:: :py:func:`setRadiiUnit`

.. seealso:: :py:func:`radii`
%End

    void setRadiiUnit( QgsUnitTypes::RenderUnit units );
%Docstring
Sets the units used for the shape's radii.

:param units: radii units

.. seealso:: :py:func:`radiiUnit`

.. seealso:: :py:func:`setRadii`
%End

    QgsMapUnitScale radiiMapUnitScale() const;
%Docstring
Returns the map unit scale object for the shape radii. This is only used if the
radiiUnit() is set to QgsUnitTypes.RenderMapUnit.

.. seealso:: :py:func:`setRadiiMapUnitScale`

.. seealso:: :py:func:`radiiUnit`
%End

    void setRadiiMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
Sets the map unit scale object for the shape radii. This is only used if the
radiiUnit() is set to QgsUnitTypes.RenderMapUnit.

:param scale: scale for shape radii

.. seealso:: :py:func:`radiiMapUnitScale`

.. seealso:: :py:func:`setRadiiUnit`
%End

    double opacity() const;
%Docstring
Returns the background shape's opacity. The opacity is a double value between 0 (fully transparent) and 1 (totally
opaque).

.. seealso:: :py:func:`setOpacity`
%End

    void setOpacity( double opacity );
%Docstring
Sets the background shape's opacity.

:param opacity: opacity as a double value between 0 (fully transparent) and 1 (totally
opaque)

.. seealso:: :py:func:`opacity`
%End

    QPainter::CompositionMode blendMode() const;
%Docstring
Returns the blending mode used for drawing the background shape.

.. seealso:: :py:func:`setBlendMode`
%End

    void setBlendMode( QPainter::CompositionMode mode );
%Docstring
Sets the blending mode used for drawing the background shape.

:param mode: blending mode

.. seealso:: :py:func:`blendMode`
%End

    QColor fillColor() const;
%Docstring
Returns the color used for filing the background shape.

.. seealso:: :py:func:`setFillColor`

.. seealso:: :py:func:`strokeColor`
%End

    void setFillColor( const QColor &color );
%Docstring
Sets the color used for filing the background shape.

:param color: background color

.. seealso:: :py:func:`fillColor`

.. seealso:: :py:func:`setStrokeColor`
%End

    QColor strokeColor() const;
%Docstring
Returns the color used for outlining the background shape.

.. seealso:: :py:func:`setStrokeColor`

.. seealso:: :py:func:`fillColor`
%End

    void setStrokeColor( const QColor &color );
%Docstring
Sets the color used for outlining the background shape.

:param color: stroke color

.. seealso:: :py:func:`strokeColor`

.. seealso:: :py:func:`setFillColor`
%End

    double strokeWidth() const;
%Docstring
Returns the width of the shape's stroke (stroke). Units are retrieved through
strokeWidthUnit().

.. seealso:: :py:func:`setStrokeWidth`

.. seealso:: :py:func:`strokeWidthUnit`
%End

    void setStrokeWidth( double width );
%Docstring
Sets the width of the shape's stroke (stroke). Units are specified through
setStrokeWidthUnit().

.. seealso:: :py:func:`strokeWidth`

.. seealso:: :py:func:`setStrokeWidthUnit`
%End

    QgsUnitTypes::RenderUnit strokeWidthUnit() const;
%Docstring
Returns the units used for the shape's stroke width.

.. seealso:: :py:func:`setStrokeWidthUnit`

.. seealso:: :py:func:`strokeWidth`
%End

    void setStrokeWidthUnit( QgsUnitTypes::RenderUnit units );
%Docstring
Sets the units used for the shape's stroke width.

:param units: stroke width units

.. seealso:: :py:func:`strokeWidthUnit`

.. seealso:: :py:func:`setStrokeWidth`
%End

    QgsMapUnitScale strokeWidthMapUnitScale() const;
%Docstring
Returns the map unit scale object for the shape stroke width. This is only used if the
strokeWidthUnit() is set to QgsUnitTypes.RenderMapUnit.

.. seealso:: :py:func:`setStrokeWidthMapUnitScale`

.. seealso:: :py:func:`strokeWidthUnit`
%End

    void setStrokeWidthMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
Sets the map unit scale object for the shape stroke width. This is only used if the
strokeWidthUnit() is set to QgsUnitTypes.RenderMapUnit.

:param scale: scale for shape stroke width

.. seealso:: :py:func:`strokeWidthMapUnitScale`

.. seealso:: :py:func:`setStrokeWidthUnit`
%End

    Qt::PenJoinStyle joinStyle() const;
%Docstring
Returns the join style used for drawing the background shape.

.. seealso:: :py:func:`setJoinStyle`
%End

    void setJoinStyle( Qt::PenJoinStyle style );
%Docstring
Sets the join style used for drawing the background shape.

:param style: join style

.. seealso:: :py:func:`joinStyle`
%End

    QgsPaintEffect *paintEffect() const;
%Docstring
Returns the current paint effect for the background shape.

:return: paint effect

.. seealso:: :py:func:`setPaintEffect`
%End

    void setPaintEffect( QgsPaintEffect *effect /Transfer/ );
%Docstring
Sets the current paint ``effect`` for the background shape.

:param effect: paint effect. Ownership is transferred to the background settings.

.. seealso:: :py:func:`paintEffect`
%End

    void readFromLayer( QgsVectorLayer *layer );
%Docstring
Reads settings from a layer's custom properties (for QGIS 2.x projects).

:param layer: source vector layer
%End

    void readXml( const QDomElement &elem, const QgsReadWriteContext &context );
%Docstring
Read settings from a DOM element.

.. seealso:: :py:func:`writeXml`
%End

    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const;
%Docstring
Write settings into a DOM element.

.. seealso:: :py:func:`readXml`
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

:param other: source QgsTextShadowSettings
%End


    ~QgsTextShadowSettings();

    bool enabled() const;
%Docstring
Returns whether the shadow is enabled.

.. seealso:: :py:func:`setEnabled`
%End

    void setEnabled( bool enabled );
%Docstring
Sets whether the text shadow will be drawn.

:param enabled: set to true to draw shadow

.. seealso:: :py:func:`enabled`
%End

    QgsTextShadowSettings::ShadowPlacement shadowPlacement() const;
%Docstring
Returns the placement for the drop shadow. The placement determines
both the z-order stacking position for the shadow and the what shape (e.g., text,
background shape) is used for casting the shadow.

.. seealso:: :py:func:`setShadowPlacement`
%End

    void setShadowPlacement( QgsTextShadowSettings::ShadowPlacement placement );
%Docstring
Sets the placement for the drop shadow. The placement determines
both the z-order stacking position for the shadow and the what shape (e.g., text,
background shape) is used for casting the shadow.

:param placement: shadow placement

.. seealso:: :py:func:`shadowPlacement`
%End

    int offsetAngle() const;
%Docstring
Returns the angle for offsetting the position of the shadow from the text.

.. seealso:: :py:func:`setOffsetAngle`

.. seealso:: :py:func:`offsetDistance`
%End

    void setOffsetAngle( int angle );
%Docstring
Sets the angle for offsetting the position of the shadow from the text.

:param angle: offset angle in degrees

.. seealso:: :py:func:`offsetAngle`

.. seealso:: :py:func:`setOffsetDistance`
%End

    double offsetDistance() const;
%Docstring
Returns the distance for offsetting the position of the shadow from the text. Offset units
are retrieved via offsetUnit().

.. seealso:: :py:func:`setOffsetDistance`

.. seealso:: :py:func:`offsetUnit`
%End

    void setOffsetDistance( double distance );
%Docstring
Sets the distance for offsetting the position of the shadow from the text. Offset units
are specified via setOffsetUnit().

:param distance: offset distance

.. seealso:: :py:func:`offsetDistance`

.. seealso:: :py:func:`setOffsetUnit`
%End

    QgsUnitTypes::RenderUnit offsetUnit() const;
%Docstring
Returns the units used for the shadow's offset.

.. seealso:: :py:func:`setOffsetUnit`

.. seealso:: :py:func:`offsetDistance`
%End

    void setOffsetUnit( QgsUnitTypes::RenderUnit units );
%Docstring
Sets the units used for the shadow's offset.

:param units: shadow distance units

.. seealso:: :py:func:`offsetUnit`

.. seealso:: :py:func:`setOffsetDistance`
%End

    QgsMapUnitScale offsetMapUnitScale() const;
%Docstring
Returns the map unit scale object for the shadow offset distance. This is only used if the
offsetUnit() is set to QgsUnitTypes.RenderMapUnit.

.. seealso:: :py:func:`setOffsetMapUnitScale`

.. seealso:: :py:func:`offsetUnit`
%End

    void setOffsetMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
Sets the map unit scale object for the shadow offset distance. This is only used if the
offsetUnit() is set to QgsUnitTypes.RenderMapUnit.

:param scale: scale for shadow offset

.. seealso:: :py:func:`offsetMapUnitScale`

.. seealso:: :py:func:`setOffsetUnit`
%End

    bool offsetGlobal() const;
%Docstring
Returns true if the global shadow offset will be used.

.. seealso:: :py:func:`setOffsetGlobal`
%End

    void setOffsetGlobal( bool global );
%Docstring
Sets whether the global shadow offset should be used.

:param global: set to true to use global shadow offset.
%End

    double blurRadius() const;
%Docstring
Returns the blur radius for the shadow. Radius units are retrieved via blurRadiusUnits().

.. seealso:: :py:func:`setBlurRadius`

.. seealso:: :py:func:`blurRadiusUnit`
%End

    void setBlurRadius( double blurRadius );
%Docstring
Sets the blur radius for the shadow. Radius units are specified via setBlurRadiusUnits().

:param blurRadius: blur radius

.. seealso:: :py:func:`blurRadius`

.. seealso:: :py:func:`setBlurRadiusUnit`
%End

    QgsUnitTypes::RenderUnit blurRadiusUnit() const;
%Docstring
Returns the units used for the shadow's blur radius.

.. seealso:: :py:func:`setBlurRadiusUnit`

.. seealso:: :py:func:`blurRadius`
%End

    void setBlurRadiusUnit( QgsUnitTypes::RenderUnit units );
%Docstring
Sets the units used for the shadow's blur radius.

:param units: shadow blur radius units

.. seealso:: :py:func:`blurRadiusUnit`

.. seealso:: :py:func:`setBlurRadius`
%End

    QgsMapUnitScale blurRadiusMapUnitScale() const;
%Docstring
Returns the map unit scale object for the shadow blur radius. This is only used if the
blurRadiusUnit() is set to QgsUnitTypes.RenderMapUnit.

.. seealso:: :py:func:`setBlurRadiusMapUnitScale`

.. seealso:: :py:func:`blurRadiusUnit`
%End

    void setBlurRadiusMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
Sets the map unit scale object for the shadow blur radius. This is only used if the
blurRadiusUnit() is set to QgsUnitTypes.RenderMapUnit.

:param scale: scale for shadow blur radius

.. seealso:: :py:func:`blurRadiusMapUnitScale`

.. seealso:: :py:func:`setBlurRadiusUnit`
%End

    bool blurAlphaOnly() const;
%Docstring
Returns whether only the alpha channel for the shadow will be blurred.

.. seealso:: :py:func:`setBlurAlphaOnly`
%End

    void setBlurAlphaOnly( bool alphaOnly );
%Docstring
Sets whether only the alpha channel for the shadow should be blurred.

:param alphaOnly: set to true to blur only the alpha channel. If false, all channels (including
red, green and blue channel) will be blurred.

.. seealso:: :py:func:`blurAlphaOnly`
%End

    double opacity() const;
%Docstring
Returns the shadow's opacity. The opacity is a double value between 0 (fully transparent) and 1 (totally
opaque).

.. seealso:: :py:func:`setOpacity`
%End

    void setOpacity( double opacity );
%Docstring
Sets the shadow's opacity.

:param opacity: opacity as a double value between 0 (fully transparent) and 1 (totally
opaque)

.. seealso:: :py:func:`opacity`
%End

    int scale() const;
%Docstring
Returns the scaling used for the drop shadow (in percentage of original size).

.. seealso:: :py:func:`setScale`
%End

    void setScale( int scale );
%Docstring
Sets the scaling used for the drop shadow (in percentage of original size).

:param scale: scale percent for drop shadow

.. seealso:: :py:func:`scale`
%End

    QColor color() const;
%Docstring
Returns the color of the drop shadow.

.. seealso:: :py:func:`setColor`
%End

    void setColor( const QColor &color );
%Docstring
Sets the color for the drop shadow.

:param color: shadow color

.. seealso:: :py:func:`color`
%End

    QPainter::CompositionMode blendMode() const;
%Docstring
Returns the blending mode used for drawing the drop shadow.

.. seealso:: :py:func:`setBlendMode`
%End

    void setBlendMode( QPainter::CompositionMode mode );
%Docstring
Sets the blending mode used for drawing the drop shadow.

:param mode: blending mode

.. seealso:: :py:func:`blendMode`
%End

    void readFromLayer( QgsVectorLayer *layer );
%Docstring
Reads settings from a layer's custom properties (for QGIS 2.x projects).

:param layer: source vector layer
%End

    void readXml( const QDomElement &elem );
%Docstring
Read settings from a DOM element.

.. seealso:: :py:func:`writeXml`
%End

    QDomElement writeXml( QDomDocument &doc ) const;
%Docstring
Write settings into a DOM element.

.. seealso:: :py:func:`readXml`
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

:param other: source QgsTextFormat
%End


    ~QgsTextFormat();

    QgsTextBufferSettings &buffer();
%Docstring
Returns a reference to the text buffer settings.

.. seealso:: :py:func:`setBuffer`
%End


    void setBuffer( const QgsTextBufferSettings &bufferSettings );
%Docstring
Sets the text's buffer settings.

:param bufferSettings: buffer settings

.. seealso:: :py:func:`buffer`
%End

    QgsTextBackgroundSettings &background();
%Docstring
Returns a reference to the text background settings.

.. seealso:: :py:func:`setBackground`
%End


    void setBackground( const QgsTextBackgroundSettings &backgroundSettings );
%Docstring
Sets the text's background settings.q

:param backgroundSettings: background settings

.. seealso:: :py:func:`background`
%End

    QgsTextShadowSettings &shadow();
%Docstring
Returns a reference to the text drop shadow settings.

.. seealso:: :py:func:`setShadow`
%End


    void setShadow( const QgsTextShadowSettings &shadowSettings );
%Docstring
Sets the text's drop shadow settings.

:param shadowSettings: shadow settings

.. seealso:: :py:func:`shadow`
%End

    QFont font() const;
%Docstring
Returns the font used for rendering text. Note that the size of the font
is not used, and size() should be called instead to determine the size
of rendered text.

.. seealso:: :py:func:`scaledFont`

.. seealso:: :py:func:`setFont`

.. seealso:: :py:func:`namedStyle`
%End

    QFont scaledFont( const QgsRenderContext &context ) const;
%Docstring
Returns a font with the size scaled to match the format's size settings (including
units and map unit scale) for a specified render context.

:param context: destination render context

:return: font with scaled size

.. seealso:: :py:func:`font`

.. seealso:: :py:func:`size`
%End

    void setFont( const QFont &font );
%Docstring
Sets the font used for rendering text. Note that the size of the font
is not used, and setSize() should be called instead to explicitly set the size
of rendered text.

:param font: desired font

.. seealso:: :py:func:`font`

.. seealso:: :py:func:`setNamedStyle`
%End

    QString namedStyle() const;
%Docstring
Returns the named style for the font used for rendering text (e.g., "bold").

.. seealso:: :py:func:`setNamedStyle`

.. seealso:: :py:func:`font`
%End

    void setNamedStyle( const QString &style );
%Docstring
Sets the named style for the font used for rendering text.

:param style: named style, e.g., "bold"

.. seealso:: :py:func:`namedStyle`

.. seealso:: :py:func:`setFont`
%End

    double size() const;
%Docstring
Returns the size for rendered text. Units are retrieved using sizeUnit().

.. seealso:: :py:func:`setSize`

.. seealso:: :py:func:`sizeUnit`
%End

    void setSize( double size );
%Docstring
Sets the size for rendered text.

:param size: size of rendered text. Units are set using setSizeUnit()

.. seealso:: :py:func:`size`

.. seealso:: :py:func:`setSizeUnit`
%End

    QgsUnitTypes::RenderUnit sizeUnit() const;
%Docstring
Returns the units for the size of rendered text.

.. seealso:: :py:func:`size`

.. seealso:: :py:func:`setSizeUnit`

.. seealso:: :py:func:`sizeMapUnitScale`
%End

    void setSizeUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units for the size of rendered text.

:param unit: size units

.. seealso:: :py:func:`setSize`

.. seealso:: :py:func:`sizeUnit`

.. seealso:: :py:func:`setSizeMapUnitScale`
%End

    QgsMapUnitScale sizeMapUnitScale() const;
%Docstring
Returns the map unit scale object for the size. This is only used if the
sizeUnit() is set to QgsUnitTypes.RenderMapUnit.

.. seealso:: :py:func:`setSizeMapUnitScale`

.. seealso:: :py:func:`sizeUnit`
%End

    void setSizeMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
Sets the map unit scale object for the size. This is only used if the
sizeUnit() is set to QgsUnitTypes.RenderMapUnit.

.. seealso:: :py:func:`sizeMapUnitScale`

.. seealso:: :py:func:`setSizeUnit`
%End

    QColor color() const;
%Docstring
Returns the color that text will be rendered in.

.. seealso:: :py:func:`setColor`
%End

    void setColor( const QColor &color );
%Docstring
Sets the color that text will be rendered in.

:param color: text color

.. seealso:: :py:func:`color`
%End

    double opacity() const;
%Docstring
Returns the text's opacity. The opacity is a double value between 0 (fully transparent) and 1 (totally
opaque).

.. seealso:: :py:func:`setOpacity`
%End

    void setOpacity( double opacity );
%Docstring
Sets the text's opacity.

:param opacity: opacity as a double value between 0 (fully transparent) and 1 (totally
opaque)

.. seealso:: :py:func:`opacity`
%End

    QPainter::CompositionMode blendMode() const;
%Docstring
Returns the blending mode used for drawing the text.

.. seealso:: :py:func:`setBlendMode`
%End

    void setBlendMode( QPainter::CompositionMode mode );
%Docstring
Sets the blending mode used for drawing the text.

:param mode: blending mode

.. seealso:: :py:func:`blendMode`
%End

    double lineHeight() const;
%Docstring
Returns the line height for text. This is a number between
0.0 and 10.0 representing the leading between lines as a
multiplier of line height.

.. seealso:: :py:func:`setLineHeight`
%End

    void setLineHeight( double height );
%Docstring
Sets the line height for text.

:param height: a number between
0.0 and 10.0 representing the leading between lines as a
multiplier of line height.

.. seealso:: :py:func:`lineHeight`
%End

    void readFromLayer( QgsVectorLayer *layer );
%Docstring
Reads settings from a layer's custom properties (for QGIS 2.x projects).

:param layer: source vector layer
%End

    void readXml( const QDomElement &elem, const QgsReadWriteContext &context );
%Docstring
Read settings from a DOM element.

.. seealso:: :py:func:`writeXml`
%End

    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const;
%Docstring
Write settings into a DOM element.

.. seealso:: :py:func:`readXml`
%End

    QMimeData *toMimeData() const /Factory/;
%Docstring
Returns new mime data representing the text format settings.
Caller takes responsibility for deleting the returned object.

.. seealso:: :py:func:`fromMimeData`
%End

    static QgsTextFormat fromMimeData( const QMimeData *data, bool *ok /Out/ = 0 );
%Docstring
Attempts to parse the provided mime ``data`` as a QgsTextFormat.
If data can be parsed as a text format, ``ok`` will be set to true.

.. seealso:: :py:func:`toMimeData`
%End

    bool containsAdvancedEffects() const;
%Docstring
Returns true if any component of the font format requires advanced effects
such as blend modes, which require output in raster formats to be fully respected.
%End

    bool fontFound() const;
%Docstring
Returns true if the specified font was found on the system, or false
if the font was not found and a replacement was used instead.

.. seealso:: :py:func:`resolvedFontFamily`
%End

    QString resolvedFontFamily() const;
%Docstring
Returns the family for the resolved font, ie if the specified font
was not found on the system this will return the name of the replacement
font.

.. seealso:: :py:func:`fontFound`
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

:param size: size to convert
:param c: rendercontext
:param unit: size units
:param mapUnitScale: a mapUnitScale clamper

:return: font pixel size
%End

    static void drawText( const QRectF &rect, double rotation, HAlignment alignment, const QStringList &textLines,
                          QgsRenderContext &context, const QgsTextFormat &format,
                          bool drawAsOutlines = true );
%Docstring
Draws text within a rectangle using the specified settings.

:param rect: destination rectangle for text
:param rotation: text rotation
:param alignment: horizontal alignment
:param textLines: list of lines of text to draw
:param context: render context
:param format: text format
:param drawAsOutlines: set to false to render text as text. This allows outputs to
formats like SVG to maintain text as text objects, but at the cost of degraded
rendering and may result in side effects like misaligned text buffers.
%End

    static void drawText( const QPointF &point, double rotation, HAlignment alignment, const QStringList &textLines,
                          QgsRenderContext &context, const QgsTextFormat &format,
                          bool drawAsOutlines = true );
%Docstring
Draws text at a point origin using the specified settings.

:param point: origin of text
:param rotation: text rotation
:param alignment: horizontal alignment
:param textLines: list of lines of text to draw
:param context: render context
:param format: text format
:param drawAsOutlines: set to false to render text as text. This allows outputs to
formats like SVG to maintain text as text objects, but at the cost of degraded
rendering and may result in side effects like misaligned text buffers.
%End

    static void drawPart( const QRectF &rect, double rotation, HAlignment alignment, const QStringList &textLines,
                          QgsRenderContext &context, const QgsTextFormat &format,
                          TextPart part, bool drawAsOutlines = true );
%Docstring
Draws a single component of rendered text using the specified settings.

:param rect: destination rectangle for text
:param rotation: text rotation
:param alignment: horizontal alignment
:param textLines: list of lines of text to draw
:param context: render context
:param format: text format
:param part: component of text to draw. Note that Shadow parts cannot be drawn
individually and instead are drawn with their associated part (e.g., drawn together
with the text or background parts)
:param drawAsOutlines: set to false to render text as text. This allows outputs to
formats like SVG to maintain text as text objects, but at the cost of degraded
rendering and may result in side effects like misaligned text buffers.
%End

    static void drawPart( const QPointF &origin, double rotation, HAlignment alignment, const QStringList &textLines,
                          QgsRenderContext &context, const QgsTextFormat &format,
                          TextPart part, bool drawAsOutlines = true );
%Docstring
Draws a single component of rendered text using the specified settings.

:param origin: origin for start of text. Y coordinate will be used as baseline.
:param rotation: text rotation
:param alignment: horizontal alignment
:param textLines: list of lines of text to draw
:param context: render context
:param format: text format
:param part: component of text to draw. Note that Shadow parts cannot be drawn
individually and instead are drawn with their associated part (e.g., drawn together
with the text or background parts)
:param drawAsOutlines: set to false to render text as text. This allows outputs to
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
