/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgssymbollayerutils.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





typedef QMap<QString, QString> QgsStringMap;
typedef QMap<QString, QgsSymbol * > QgsSymbolMap;


class QgsSymbolLayerUtils
{

%TypeHeaderCode
#include "qgssymbollayerutils.h"
%End
  public:

    static QString encodeColor( const QColor &color );
    static QColor decodeColor( const QString &str );

    static QString encodeSldAlpha( int alpha );
    static int decodeSldAlpha( const QString &str );

    static QString encodeSldFontStyle( QFont::Style style );
    static QFont::Style decodeSldFontStyle( const QString &str );

    static QString encodeSldFontWeight( int weight );
    static int decodeSldFontWeight( const QString &str );

    static QString encodePenStyle( Qt::PenStyle style );
    static Qt::PenStyle decodePenStyle( const QString &str );

    static QString encodePenJoinStyle( Qt::PenJoinStyle style );
    static Qt::PenJoinStyle decodePenJoinStyle( const QString &str );

    static QString encodePenCapStyle( Qt::PenCapStyle style );
    static Qt::PenCapStyle decodePenCapStyle( const QString &str );

    static QString encodeSldLineJoinStyle( Qt::PenJoinStyle style );
    static Qt::PenJoinStyle decodeSldLineJoinStyle( const QString &str );

    static QString encodeSldLineCapStyle( Qt::PenCapStyle style );
    static Qt::PenCapStyle decodeSldLineCapStyle( const QString &str );

    static QString encodeBrushStyle( Qt::BrushStyle style );
    static Qt::BrushStyle decodeBrushStyle( const QString &str );

    static QString encodeSldBrushStyle( Qt::BrushStyle style );
    static Qt::BrushStyle decodeSldBrushStyle( const QString &str );

    static QString encodePoint( QPointF point );
%Docstring
Encodes a QPointF to a string.

.. seealso:: :py:func:`decodePoint`

.. seealso:: :py:func:`encodeSize`
%End

    static QPointF decodePoint( const QString &string );
%Docstring
Decodes a QSizeF from a string.

.. seealso:: :py:func:`encodePoint`

.. seealso:: :py:func:`decodeSize`
%End

    static QString encodeSize( QSizeF size );
%Docstring
Encodes a QSizeF to a string.

.. seealso:: :py:func:`decodeSize`

.. seealso:: :py:func:`encodePoint`

.. versionadded:: 3.0
%End

    static QSizeF decodeSize( const QString &string );
%Docstring
Decodes a QSizeF from a string.

.. seealso:: :py:func:`encodeSize`

.. seealso:: :py:func:`decodePoint`

.. versionadded:: 3.0
%End

    static QString encodeMapUnitScale( const QgsMapUnitScale &mapUnitScale );
    static QgsMapUnitScale decodeMapUnitScale( const QString &str );

    static QString encodeRealVector( const QVector<qreal> &v );
    static QVector<qreal> decodeRealVector( const QString &s );

    static QString encodeSldRealVector( const QVector<qreal> &v );
    static QVector<qreal> decodeSldRealVector( const QString &s );

    static QString encodeSldUom( QgsUnitTypes::RenderUnit unit, double *scaleFactor );
%Docstring
Encodes a render unit into an SLD unit of measure string.

:param unit: unit to encode
:param scaleFactor: if specified, will be set to scale factor for unit of measure

:return: encoded string

.. seealso:: :py:func:`decodeSldUom`
%End

    static QgsUnitTypes::RenderUnit decodeSldUom( const QString &str, double *scaleFactor );
%Docstring
Decodes a SLD unit of measure string to a render unit.

:param str: string to decode
:param scaleFactor: if specified, will be set to scale factor for unit of measure

:return: matching render unit

.. seealso:: :py:func:`encodeSldUom`
%End

    static double sizeInPixelsFromSldUom( const QString &uom, double size );
%Docstring
Returns the size scaled in pixels according to the uom attribute.

:param uom: The uom attribute from SLD 1.1 version
:param size: The original size

:return: the size in pixels

.. versionadded:: 3.0
%End

    static QString encodeScaleMethod( QgsSymbol::ScaleMethod scaleMethod );
    static QgsSymbol::ScaleMethod decodeScaleMethod( const QString &str );

    static QPainter::CompositionMode decodeBlendMode( const QString &s );

    static QIcon symbolPreviewIcon( QgsSymbol *symbol, QSize size, int padding = 0 );
%Docstring
Returns an icon preview for a color ramp.

:param symbol: symbol
:param size: target pixmap size
:param padding: space between icon edge and symbol

.. seealso:: :py:func:`symbolPreviewPixmap`
%End

    static QPixmap symbolPreviewPixmap( QgsSymbol *symbol, QSize size, int padding = 0, QgsRenderContext *customContext = 0 );
%Docstring
Returns a pixmap preview for a color ramp.

:param symbol: symbol
:param size: target pixmap size
:param padding: space between icon edge and symbol
:param customContext: render context to use when rendering symbol

.. note::

   Parameter customContext added in QGIS 2.6

.. seealso:: :py:func:`symbolPreviewIcon`
%End

    static QPicture symbolLayerPreviewPicture( QgsSymbolLayer *layer, QgsUnitTypes::RenderUnit units, QSize size, const QgsMapUnitScale &scale = QgsMapUnitScale() );
%Docstring
Draws a symbol layer preview to a QPicture

:param layer: symbol layer to draw
:param units: size units
:param size: target size of preview picture
:param scale: map unit scale for preview

:return: QPicture containing symbol layer preview

.. versionadded:: 2.9

.. seealso:: :py:func:`symbolLayerPreviewIcon`
%End

    static QIcon symbolLayerPreviewIcon( QgsSymbolLayer *layer, QgsUnitTypes::RenderUnit u, QSize size, const QgsMapUnitScale &scale = QgsMapUnitScale() );
%Docstring
Draws a symbol layer preview to an icon.

:param layer: symbol layer to draw
:param u: size units
:param size: target size of preview icon
:param scale: map unit scale for preview

:return: icon containing symbol layer preview

.. seealso:: :py:func:`symbolLayerPreviewPicture`
%End

    static QIcon colorRampPreviewIcon( QgsColorRamp *ramp, QSize size, int padding = 0 );
%Docstring
Returns an icon preview for a color ramp.

:param ramp: color ramp
:param size: target icon size
:param padding: space between icon edge and color ramp

.. seealso:: :py:func:`colorRampPreviewPixmap`
%End

    static QPixmap colorRampPreviewPixmap( QgsColorRamp *ramp, QSize size, int padding = 0 );
%Docstring
Returns a pixmap preview for a color ramp.

:param ramp: color ramp
:param size: target pixmap size
:param padding: space between icon edge and color ramp

.. seealso:: :py:func:`colorRampPreviewIcon`
%End

    static void drawStippledBackground( QPainter *painter, QRect rect );

    static double estimateMaxSymbolBleed( QgsSymbol *symbol, const QgsRenderContext &context );
%Docstring
Returns the maximum estimated bleed for the symbol
%End

    static QgsSymbol *loadSymbol( const QDomElement &element, const QgsReadWriteContext &context ) /Factory/;
%Docstring
Attempts to load a symbol from a DOM element

:param element: DOM element representing symbol
:param context: object to transform relative to absolute paths

:return: decoded symbol, if possible
%End


    static QgsSymbolLayer *loadSymbolLayer( QDomElement &element, const QgsReadWriteContext &context ) /Factory/;
%Docstring
Reads and returns symbol layer from XML. Caller is responsible for deleting the returned object
%End
    static QDomElement saveSymbol( const QString &symbolName, QgsSymbol *symbol, QDomDocument &doc, const QgsReadWriteContext &context );
%Docstring
Writes a symbol definition to XML
%End

    static QString symbolProperties( QgsSymbol *symbol );
%Docstring
Returns a string representing the symbol. Can be used to test for equality
between symbols.

.. versionadded:: 2.12
%End

    static bool createSymbolLayerListFromSld( QDomElement &element, QgsWkbTypes::GeometryType geomType, QgsSymbolLayerList &layers );

    static QgsSymbolLayer *createFillLayerFromSld( QDomElement &element ) /Factory/;
    static QgsSymbolLayer *createLineLayerFromSld( QDomElement &element ) /Factory/;
    static QgsSymbolLayer *createMarkerLayerFromSld( QDomElement &element ) /Factory/;

    static bool convertPolygonSymbolizerToPointMarker( QDomElement &element, QgsSymbolLayerList &layerList );
    static bool hasExternalGraphic( QDomElement &element );
    static bool hasWellKnownMark( QDomElement &element );

    static bool needFontMarker( QDomElement &element );
    static bool needSvgMarker( QDomElement &element );
    static bool needEllipseMarker( QDomElement &element );
    static bool needMarkerLine( QDomElement &element );
    static bool needLinePatternFill( QDomElement &element );
    static bool needPointPatternFill( QDomElement &element );
    static bool needSvgFill( QDomElement &element );

    static void fillToSld( QDomDocument &doc, QDomElement &element,
                           Qt::BrushStyle brushStyle, const QColor &color = QColor() );
    static bool fillFromSld( QDomElement &element,
                             Qt::BrushStyle &brushStyle, QColor &color );

    static bool lineFromSld( QDomElement &element,
                             Qt::PenStyle &penStyle, QColor &color, double &width,
                             Qt::PenJoinStyle *penJoinStyle = 0, Qt::PenCapStyle *penCapStyle = 0,
                             QVector<qreal> *customDashPattern = 0, double *dashOffset = 0 );

    static void externalGraphicToSld( QDomDocument &doc, QDomElement &element,
                                      const QString &path, const QString &mime,
                                      const QColor &color, double size = -1 );
    static bool externalGraphicFromSld( QDomElement &element,
                                        QString &path, QString &mime,
                                        QColor &color, double &size );

    static void wellKnownMarkerToSld( QDomDocument &doc, QDomElement &element,
                                      const QString &name, const QColor &color, const QColor &strokeColor, Qt::PenStyle strokeStyle,
                                      double strokeWidth = -1, double size = -1 );

    static bool wellKnownMarkerFromSld( QDomElement &element,
                                        QString &name, QColor &color, QColor &strokeColor, Qt::PenStyle &strokeStyle,
                                        double &strokeWidth, double &size ) /PyName=wellKnownMarkerFromSld2/;
%Docstring

.. note::

   available in Python as wellKnownMarkerFromSld2
%End

    static void externalMarkerToSld( QDomDocument &doc, QDomElement &element,
                                     const QString &path, const QString &format, int *markIndex = 0,
                                     const QColor &color = QColor(), double size = -1 );
    static bool externalMarkerFromSld( QDomElement &element,
                                       QString &path, QString &format, int &markIndex,
                                       QColor &color, double &size );


    static void labelTextToSld( QDomDocument &doc, QDomElement &element, const QString &label,
                                const QFont &font, const QColor &color = QColor(), double size = -1 );

    static QString ogrFeatureStylePen( double width, double mmScaleFactor, double mapUnitsScaleFactor, const QColor &c,
                                       Qt::PenJoinStyle joinStyle = Qt::MiterJoin,
                                       Qt::PenCapStyle capStyle = Qt::FlatCap,
                                       double offset = 0.0,
                                       const QVector<qreal> *dashPattern = 0 );
%Docstring
Create ogr feature style string for pen
%End

    static QString ogrFeatureStyleBrush( const QColor &fillColr );
%Docstring
Create ogr feature style string for brush

:param fillColr: fill color*
%End

    static void createRotationElement( QDomDocument &doc, QDomElement &element, const QString &rotationFunc );
    static bool rotationFromSldElement( QDomElement &element, QString &rotationFunc );

    static void createOpacityElement( QDomDocument &doc, QDomElement &element, const QString &alphaFunc );
    static bool opacityFromSldElement( QDomElement &element, QString &alphaFunc );

    static void createDisplacementElement( QDomDocument &doc, QDomElement &element, QPointF offset );
    static bool displacementFromSldElement( QDomElement &element, QPointF &offset );

    static void createAnchorPointElement( QDomDocument &doc, QDomElement &element, QPointF anchor );
%Docstring
 Creates a SE 1.1 anchor point element as a child of the specified element

:param doc: The document
:param element: The parent element
:param anchor: An anchor specification, with values between 0 and 1
%End

    static void createOnlineResourceElement( QDomDocument &doc, QDomElement &element, const QString &path, const QString &format );
    static bool onlineResourceFromSldElement( QDomElement &element, QString &path, QString &format );

    static void createGeometryElement( QDomDocument &doc, QDomElement &element, const QString &geomFunc );
    static bool geometryFromSldElement( QDomElement &element, QString &geomFunc );

    static bool createExpressionElement( QDomDocument &doc, QDomElement &element, const QString &function );
%Docstring
Creates a OGC Expression element based on the provided function expression

:param doc: The document owning the element
:param element: The element parent
:param function: The expression to be encoded

:return:
%End
    static bool createFunctionElement( QDomDocument &doc, QDomElement &element, const QString &function );
    static bool functionFromSldElement( QDomElement &element, QString &function );

    static QDomElement createSvgParameterElement( QDomDocument &doc, const QString &name, const QString &value );
    static QgsStringMap getSvgParameterList( QDomElement &element );

    static QDomElement createVendorOptionElement( QDomDocument &doc, const QString &name, const QString &value );
    static QgsStringMap getVendorOptionList( QDomElement &element );

    static QgsStringMap parseProperties( QDomElement &element );
    static void saveProperties( QgsStringMap props, QDomDocument &doc, QDomElement &element );

    static QgsSymbolMap loadSymbols( QDomElement &element, const QgsReadWriteContext &context ) /Factory/;
%Docstring
Reads a collection of symbols from XML and returns them in a map. Caller is responsible for deleting returned symbols.
%End
    static QDomElement saveSymbols( QgsSymbolMap &symbols, const QString &tagName, QDomDocument &doc, const QgsReadWriteContext &context );
%Docstring
Writes a collection of symbols to XML with specified tagName for the top-level element
%End

    static void clearSymbolMap( QgsSymbolMap &symbols );

    static QMimeData *symbolToMimeData( QgsSymbol *symbol ) /Factory/;
%Docstring
Creates new mime data from a ``symbol``.
This also sets the mime color data to match the symbol's color, so that copied symbols
can be paste in places where a color is expected.

.. seealso:: :py:func:`symbolFromMimeData`

.. versionadded:: 3.0
%End

    static QgsSymbol *symbolFromMimeData( const QMimeData *data ) /Factory/;
%Docstring
Attempts to parse ``mime`` data as a symbol. A new symbol instance will be returned
if the data was successfully converted to a symbol.

.. seealso:: :py:func:`symbolToMimeData`

.. versionadded:: 3.0
%End

    static QgsColorRamp *loadColorRamp( QDomElement &element ) /Factory/;
%Docstring
Creates a color ramp from the settings encoded in an XML element

:param element: DOM element

:return: new color ramp. Caller takes responsibility for deleting the returned value.

.. seealso:: :py:func:`saveColorRamp`
%End

    static QDomElement saveColorRamp( const QString &name, QgsColorRamp *ramp, QDomDocument &doc );
%Docstring
Encodes a color ramp's settings to an XML element

:param name: name of ramp
:param ramp: color ramp to save
:param doc: XML document

:return: DOM element representing state of color ramp

.. seealso:: :py:func:`loadColorRamp`
%End

    static QVariant colorRampToVariant( const QString &name, QgsColorRamp *ramp );
%Docstring
Saves a color ramp to a QVariantMap, wrapped in a QVariant.
You can use QgsXmlUtils.writeVariant to save it to an XML document.

.. seealso:: :py:func:`loadColorRamp`
%End

    static QgsColorRamp *loadColorRamp( const QVariant &value ) /Factory/;
%Docstring
Load a color ramp from a QVariantMap, wrapped in a QVariant.
You can use QgsXmlUtils.readVariant to load it from an XML document.

.. seealso:: :py:func:`colorRampToVariant`
%End

    static QString colorToName( const QColor &color );
%Docstring
Returns a friendly display name for a color

:param color: source color

:return: display name for color

.. versionadded:: 2.5
%End

    static QList< QColor > parseColorList( const QString &colorStr );
%Docstring
Attempts to parse a string as a list of colors using a variety of common formats, including hex
codes, rgb and rgba strings.

:param colorStr: string representing the color list

:return: list of parsed colors

.. versionadded:: 2.5
%End

    static QMimeData *colorToMimeData( const QColor &color ) /Factory/;
%Docstring
Creates mime data from a color. Sets both the mime data's color data, and the
mime data's text with the color's hex code.

:param color: color to encode as mime data

.. seealso:: :py:func:`colorFromMimeData`

.. versionadded:: 2.5
%End

    static QColor colorFromMimeData( const QMimeData *data, bool &hasAlpha );
%Docstring
Attempts to parse mime data as a color

:param data: mime data to parse
:param hasAlpha: will be set to true if mime data was interpreted as a color containing
an explicit alpha value

:return: valid color if mimedata could be interpreted as a color, otherwise an
invalid color

.. versionadded:: 2.5
%End

    static QgsNamedColorList colorListFromMimeData( const QMimeData *data );
%Docstring
Attempts to parse mime data as a list of named colors

:param data: mime data to parse

:return: list of parsed colors

.. versionadded:: 2.5
%End

    static QMimeData *colorListToMimeData( const QgsNamedColorList &colorList, const bool allFormats = true ) /Factory/;
%Docstring
Creates mime data from a list of named colors

:param colorList: list of named colors
:param allFormats: set to true to include additional mime formats, include text/plain and application/x-color

:return: mime data containing encoded colors

.. versionadded:: 2.5
%End

    static bool saveColorsToGpl( QFile &file, const QString &paletteName, const QgsNamedColorList &colors );
%Docstring
Exports colors to a gpl GIMP palette file

:param file: destination file
:param paletteName: name of palette, which is stored in gpl file
:param colors: colors to export

:return: true if export was successful

.. seealso:: :py:func:`importColorsFromGpl`
%End

    static QgsNamedColorList importColorsFromGpl( QFile &file, bool &ok, QString &name );
%Docstring
Imports colors from a gpl GIMP palette file

:param file: source gpl file
:param ok: will be true if file was successfully read
:param name: will be set to palette name from gpl file, if present

:return: list of imported colors

.. seealso:: :py:func:`saveColorsToGpl`
%End

    static QColor parseColor( const QString &colorStr, bool strictEval = false );
%Docstring
Attempts to parse a string as a color using a variety of common formats, including hex
codes, rgb and rgba strings.

:param colorStr: string representing the color
:param strictEval: set to true for stricter color parsing rules

:return: parsed color

.. versionadded:: 2.3
%End

    static QColor parseColorWithAlpha( const QString &colorStr, bool &containsAlpha, bool strictEval = false );
%Docstring
Attempts to parse a string as a color using a variety of common formats, including hex
codes, rgb and rgba strings.

:param colorStr: string representing the color
:param containsAlpha: if colorStr contains an explicit alpha value then containsAlpha will be set to true
:param strictEval: set to true for stricter color parsing rules

:return: parsed color

.. versionadded:: 2.3
%End

    static void multiplyImageOpacity( QImage *image, qreal opacity );
%Docstring
Multiplies opacity of image pixel values with a (global) transparency value.
%End

    static void blurImageInPlace( QImage &image, QRect rect, int radius, bool alphaOnly );
%Docstring
Blurs an image in place, e.g. creating Qt-independent drop shadows
%End

    static void premultiplyColor( QColor &rgb, int alpha );
%Docstring
Converts a QColor into a premultiplied ARGB QColor value using a specified alpha value

.. versionadded:: 2.3
%End

    static void sortVariantList( QList<QVariant> &list, Qt::SortOrder order );
%Docstring
Sorts the passed list in requested order
%End
    static QPointF pointOnLineWithDistance( QPointF startPoint, QPointF directionPoint, double distance );
%Docstring
Returns a point on the line from startPoint to directionPoint that is a certain distance away from the starting point
%End

    static QStringList listSvgFiles();
%Docstring
Return a list of all available svg files
%End

    static QStringList listSvgFilesAt( const QString &directory );
%Docstring
Return a list of svg files at the specified directory
%End

    static QString svgSymbolNameToPath( QString name, const QgsPathResolver &pathResolver );
%Docstring
Get SVG symbol's path from its name.
If the name is not absolute path the file is searched in SVG paths specified
in settings svg/searchPathsForSVG.
%End

    static QString svgSymbolPathToName( QString path, const QgsPathResolver &pathResolver );
%Docstring
Get SVG symbols's name from its path
%End

    static QPointF polygonCentroid( const QPolygonF &points );
%Docstring
Calculate the centroid point of a QPolygonF
%End

    static QPointF polygonPointOnSurface( const QPolygonF &points );
%Docstring
Calculate a point within of a QPolygonF
%End

    static bool pointInPolygon( const QPolygonF &points, QPointF point );
%Docstring
Calculate whether a point is within of a QPolygonF
%End

    static QgsExpression *fieldOrExpressionToExpression( const QString &fieldOrExpression ) /Factory/;
%Docstring
Return a new valid expression instance for given field or expression string.
If the input is not a valid expression, it is assumed that it is a field name and gets properly quoted.
If the string is empty, returns null pointer.
This is useful when accepting input which could be either a non-quoted field name or expression.

.. versionadded:: 2.2
%End

    static QString fieldOrExpressionFromExpression( QgsExpression *expression );
%Docstring
Return a field name if the whole expression is just a name of the field .
Returns full expression string if the expression is more complex than just one field.
Using just expression->expression() method may return quoted field name, but that is not
wanted for saving (due to backward compatibility) or display in GUI.

.. versionadded:: 2.2
%End

    static QList<double> prettyBreaks( double minimum, double maximum, int classes );
%Docstring
Computes a sequence of about 'classes' equally spaced round values
which cover the range of values from 'minimum' to 'maximum'.
The values are chosen so that they are 1, 2 or 5 times a power of 10.

.. versionadded:: 2.10
%End

    static double rescaleUom( double size, QgsUnitTypes::RenderUnit unit, const QgsStringMap &props );
%Docstring
Rescales the given size based on the uomScale found in the props, if any is found, otherwise
returns the value un-modified

.. versionadded:: 3.0
%End

    static QPointF rescaleUom( QPointF point, QgsUnitTypes::RenderUnit unit, const QgsStringMap &props ) /PyName=rescalePointUom/;
%Docstring
Rescales the given point based on the uomScale found in the props, if any is found, otherwise
returns a copy of the original point

.. versionadded:: 3.0
%End

    static QVector<qreal> rescaleUom( const QVector<qreal> &array, QgsUnitTypes::RenderUnit unit, const QgsStringMap &props ) /PyName=rescaleArrayUom/;
%Docstring
Rescales the given array based on the uomScale found in the props, if any is found, otherwise
returns a copy of the original point

.. versionadded:: 3.0
%End

    static void applyScaleDependency( QDomDocument &doc, QDomElement &ruleElem, QgsStringMap &props );
%Docstring
Checks if the properties contain scaleMinDenom and scaleMaxDenom, if available, they are added into the SE Rule element

.. versionadded:: 3.0
%End

    static void mergeScaleDependencies( int mScaleMinDenom, int mScaleMaxDenom, QgsStringMap &props );
%Docstring
Merges the local scale limits, if any, with the ones already in the map, if any

.. versionadded:: 3.0
%End

    static void parametricSvgToSld( QDomDocument &doc, QDomElement &graphicElem,
                                    const QString &path,
                                    const QColor &fillColor, double size, const QColor &strokeColor, double strokeWidth );
%Docstring
Encodes a reference to a parametric SVG into SLD, as a succession of parametric SVG using URL parameters,
a fallback SVG without parameters, and a final fallback as a mark with the right colors and stroke for systems
that cannot do SVG at all

.. versionadded:: 3.0
%End

    static QString getSvgParametricPath( const QString &basePath, const QColor &fillColor, const QColor &strokeColor, double strokeWidth );
%Docstring
Encodes a reference to a parametric SVG into a path with parameters according to the SVG Parameters spec

.. versionadded:: 3.0
%End

};





/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgssymbollayerutils.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
