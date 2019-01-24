/***************************************************************************
 qgssymbollayerutils.h
 ---------------------
 begin                : November 2009
 copyright            : (C) 2009 by Martin Dobias
 email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSSYMBOLLAYERUTILS_H
#define QGSSYMBOLLAYERUTILS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QMap>
#include <Qt>
#include <QtCore>
#include <QFont>
#include <QColor>
#include <QPainter>
#include "qgssymbol.h"
#include "qgis.h"
#include "qgsmapunitscale.h"
#include "qgscolorramp.h"
#include "qgsarrowsymbollayer.h"

class QgsExpression;
class QgsPathResolver;
class QgsReadWriteContext;
class QgsSymbolLayer;

typedef QMap<QString, QString> QgsStringMap;
typedef QMap<QString, QgsSymbol * > QgsSymbolMap;
typedef QList< QPair< QColor, QString > > QgsNamedColorList SIP_SKIP;

class QDomDocument;
class QDomElement;
class QIcon;
class QPixmap;
class QPointF;
class QSize;

/**
 * \ingroup core
 * \class QgsSymbolLayerUtils
 */
class CORE_EXPORT QgsSymbolLayerUtils
{
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

    /**
     * Decodes a \a value representing an arrow head type.
     * \since QGIS 3.2
     */
    static QgsArrowSymbolLayer::HeadType decodeArrowHeadType( const QVariant &value, bool *ok SIP_OUT = nullptr );

    /**
     * Decodes a \a value representing an arrow type.
     * \since QGIS 3.2
     */
    static QgsArrowSymbolLayer::ArrowType decodeArrowType( const QVariant &value, bool *ok SIP_OUT = nullptr );

    /**
     * Encodes a QPointF to a string.
     * \see decodePoint()
     * \see encodeSize()
     */
    static QString encodePoint( QPointF point );

    /**
     * Decodes a QSizeF from a string.
     * \see encodePoint()
     * \see decodeSize()
     */
    static QPointF decodePoint( const QString &string );

    /**
     * Encodes a QSizeF to a string.
     * \see decodeSize()
     * \see encodePoint()
     * \since QGIS 3.0
     */
    static QString encodeSize( QSizeF size );

    /**
     * Decodes a QSizeF from a string.
     * \see encodeSize()
     * \see decodePoint()
     * \since QGIS 3.0
     */
    static QSizeF decodeSize( const QString &string );

    static QString encodeMapUnitScale( const QgsMapUnitScale &mapUnitScale );
    static QgsMapUnitScale decodeMapUnitScale( const QString &str );

    static QString encodeRealVector( const QVector<qreal> &v );
    static QVector<qreal> decodeRealVector( const QString &s );

    static QString encodeSldRealVector( const QVector<qreal> &v );
    static QVector<qreal> decodeSldRealVector( const QString &s );

    /**
     * Encodes a render unit into an SLD unit of measure string.
     * \param unit unit to encode
     * \param scaleFactor if specified, will be set to scale factor for unit of measure
     * \returns encoded string
     * \see decodeSldUom()
     */
    static QString encodeSldUom( QgsUnitTypes::RenderUnit unit, double *scaleFactor );

    /**
     * Decodes a SLD unit of measure string to a render unit.
     * \param str string to decode
     * \param scaleFactor if specified, will be set to scale factor for unit of measure
     * \returns matching render unit
     * \see encodeSldUom()
     */
    static QgsUnitTypes::RenderUnit decodeSldUom( const QString &str, double *scaleFactor );

    /**
     * Returns the size scaled in pixels according to the uom attribute.
     * \param uom The uom attribute from SLD 1.1 version
     * \param size The original size
     * \returns the size in pixels
     * \since QGIS 3.0
     */
    static double sizeInPixelsFromSldUom( const QString &uom, double size );

    static QString encodeScaleMethod( QgsSymbol::ScaleMethod scaleMethod );
    static QgsSymbol::ScaleMethod decodeScaleMethod( const QString &str );

    static QPainter::CompositionMode decodeBlendMode( const QString &s );

    /**
     * Returns an icon preview for a color ramp.
     * \param symbol symbol
     * \param size target pixmap size
     * \param padding space between icon edge and symbol
     * \see symbolPreviewPixmap()
     */
    static QIcon symbolPreviewIcon( const QgsSymbol *symbol, QSize size, int padding = 0 );

    /**
     * Returns a pixmap preview for a color ramp.
     * \param symbol symbol
     * \param size target pixmap size
     * \param padding space between icon edge and symbol
     * \param customContext render context to use when rendering symbol
     * \note Parameter customContext added in QGIS 2.6
     * \see symbolPreviewIcon()
     */
    static QPixmap symbolPreviewPixmap( const QgsSymbol *symbol, QSize size, int padding = 0, QgsRenderContext *customContext = nullptr );

    /**
     * Draws a symbol layer preview to a QPicture
     * \param layer symbol layer to draw
     * \param units size units
     * \param size target size of preview picture
     * \param scale map unit scale for preview
     * \returns QPicture containing symbol layer preview
     * \see symbolLayerPreviewIcon()
     * \since QGIS 2.9
     */
    static QPicture symbolLayerPreviewPicture( const QgsSymbolLayer *layer, QgsUnitTypes::RenderUnit units, QSize size, const QgsMapUnitScale &scale = QgsMapUnitScale() );

    /**
     * Draws a symbol layer preview to an icon.
     * \param layer symbol layer to draw
     * \param u size units
     * \param size target size of preview icon
     * \param scale map unit scale for preview
     * \returns icon containing symbol layer preview
     * \see symbolLayerPreviewPicture()
     */
    static QIcon symbolLayerPreviewIcon( const QgsSymbolLayer *layer, QgsUnitTypes::RenderUnit u, QSize size, const QgsMapUnitScale &scale = QgsMapUnitScale() );

    /**
     * Returns an icon preview for a color ramp.
     * \param ramp color ramp
     * \param size target icon size
     * \param padding space between icon edge and color ramp
     * \see colorRampPreviewPixmap()
     */
    static QIcon colorRampPreviewIcon( QgsColorRamp *ramp, QSize size, int padding = 0 );

    /**
     * Returns a pixmap preview for a color ramp.
     * \param ramp color ramp
     * \param size target pixmap size
     * \param padding space between icon edge and color ramp
     * \see colorRampPreviewIcon()
     */
    static QPixmap colorRampPreviewPixmap( QgsColorRamp *ramp, QSize size, int padding = 0 );

    static void drawStippledBackground( QPainter *painter, QRect rect );

    //! Returns the maximum estimated bleed for the symbol
    static double estimateMaxSymbolBleed( QgsSymbol *symbol, const QgsRenderContext &context );

    /**
     * Attempts to load a symbol from a DOM element
     * \param element DOM element representing symbol
     * \param context object to transform relative to absolute paths
     * \returns decoded symbol, if possible
     */
    static QgsSymbol *loadSymbol( const QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    /**
     * Attempts to load a symbol from a DOM element and cast it to a particular symbol
     * type.
     * \param element DOM element representing symbol
     * \param context object to transform relative to absolute paths
     * \returns decoded symbol cast to specified type, if possible
     * \note not available in Python bindings
     */
    template <class SymbolType> static SymbolType *loadSymbol( const QDomElement &element, const QgsReadWriteContext &context ) SIP_SKIP
    {
      QgsSymbol *tmpSymbol = QgsSymbolLayerUtils::loadSymbol( element, context );
      SymbolType *symbolCastToType = dynamic_cast<SymbolType *>( tmpSymbol );

      if ( symbolCastToType )
      {
        return symbolCastToType;
      }
      else
      {
        //could not cast
        delete tmpSymbol;
        return nullptr;
      }
    }

    //! Reads and returns symbol layer from XML. Caller is responsible for deleting the returned object
    static QgsSymbolLayer *loadSymbolLayer( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;
    //! Writes a symbol definition to XML
    static QDomElement saveSymbol( const QString &symbolName, QgsSymbol *symbol, QDomDocument &doc, const QgsReadWriteContext &context );

    /**
     * Returns a string representing the symbol. Can be used to test for equality
     * between symbols.
     * \since QGIS 2.12
     */
    static QString symbolProperties( QgsSymbol *symbol );

    static bool createSymbolLayerListFromSld( QDomElement &element, QgsWkbTypes::GeometryType geomType, QgsSymbolLayerList &layers );

    static QgsSymbolLayer *createFillLayerFromSld( QDomElement &element ) SIP_FACTORY;
    static QgsSymbolLayer *createLineLayerFromSld( QDomElement &element ) SIP_FACTORY;
    static QgsSymbolLayer *createMarkerLayerFromSld( QDomElement &element ) SIP_FACTORY;

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

    //! \note not available in Python bindings
    static void lineToSld( QDomDocument &doc, QDomElement &element,
                           Qt::PenStyle penStyle, const QColor &color, double width = -1,
                           const Qt::PenJoinStyle *penJoinStyle = nullptr, const Qt::PenCapStyle *penCapStyle = nullptr,
                           const QVector<qreal> *customDashPattern = nullptr, double dashOffset = 0.0 ) SIP_SKIP;
    static bool lineFromSld( QDomElement &element,
                             Qt::PenStyle &penStyle, QColor &color, double &width,
                             Qt::PenJoinStyle *penJoinStyle = nullptr, Qt::PenCapStyle *penCapStyle = nullptr,
                             QVector<qreal> *customDashPattern = nullptr, double *dashOffset = nullptr );

    static void externalGraphicToSld( QDomDocument &doc, QDomElement &element,
                                      const QString &path, const QString &mime,
                                      const QColor &color, double size = -1 );
    static bool externalGraphicFromSld( QDomElement &element,
                                        QString &path, QString &mime,
                                        QColor &color, double &size );

    static void wellKnownMarkerToSld( QDomDocument &doc, QDomElement &element,
                                      const QString &name, const QColor &color, const QColor &strokeColor, Qt::PenStyle strokeStyle,
                                      double strokeWidth = -1, double size = -1 );

    //! \note available in Python as wellKnownMarkerFromSld2
    static bool wellKnownMarkerFromSld( QDomElement &element,
                                        QString &name, QColor &color, QColor &strokeColor, Qt::PenStyle &strokeStyle,
                                        double &strokeWidth, double &size ) SIP_PYNAME( wellKnownMarkerFromSld2 );

    static void externalMarkerToSld( QDomDocument &doc, QDomElement &element,
                                     const QString &path, const QString &format, int *markIndex = nullptr,
                                     const QColor &color = QColor(), double size = -1 );
    static bool externalMarkerFromSld( QDomElement &element,
                                       QString &path, QString &format, int &markIndex,
                                       QColor &color, double &size );


    static void labelTextToSld( QDomDocument &doc, QDomElement &element, const QString &label,
                                const QFont &font, const QColor &color = QColor(), double size = -1 );

    //! Create ogr feature style string for pen
    static QString ogrFeatureStylePen( double width, double mmScaleFactor, double mapUnitsScaleFactor, const QColor &c,
                                       Qt::PenJoinStyle joinStyle = Qt::MiterJoin,
                                       Qt::PenCapStyle capStyle = Qt::FlatCap,
                                       double offset = 0.0,
                                       const QVector<qreal> *dashPattern = nullptr );

    /**
     * Create ogr feature style string for brush
     \param fillColr fill color*/
    static QString ogrFeatureStyleBrush( const QColor &fillColr );

    static void createRotationElement( QDomDocument &doc, QDomElement &element, const QString &rotationFunc );
    static bool rotationFromSldElement( QDomElement &element, QString &rotationFunc );

    static void createOpacityElement( QDomDocument &doc, QDomElement &element, const QString &alphaFunc );
    static bool opacityFromSldElement( QDomElement &element, QString &alphaFunc );

    static void createDisplacementElement( QDomDocument &doc, QDomElement &element, QPointF offset );
    static bool displacementFromSldElement( QDomElement &element, QPointF &offset );

    /**
     * \brief Creates a SE 1.1 anchor point element as a child of the specified element
     * \param doc The document
     * \param element The parent element
     * \param anchor An anchor specification, with values between 0 and 1
     */
    static void createAnchorPointElement( QDomDocument &doc, QDomElement &element, QPointF anchor );

    static void createOnlineResourceElement( QDomDocument &doc, QDomElement &element, const QString &path, const QString &format );
    static bool onlineResourceFromSldElement( QDomElement &element, QString &path, QString &format );

    static void createGeometryElement( QDomDocument &doc, QDomElement &element, const QString &geomFunc );
    static bool geometryFromSldElement( QDomElement &element, QString &geomFunc );

    /**
     * Creates a OGC Expression element based on the provided function expression
     * \param doc The document owning the element
     * \param element The element parent
     * \param function The expression to be encoded
     */
    static bool createExpressionElement( QDomDocument &doc, QDomElement &element, const QString &function );
    static bool createFunctionElement( QDomDocument &doc, QDomElement &element, const QString &function );
    static bool functionFromSldElement( QDomElement &element, QString &function );

    static QDomElement createSvgParameterElement( QDomDocument &doc, const QString &name, const QString &value );
    static QgsStringMap getSvgParameterList( QDomElement &element );

    static QDomElement createVendorOptionElement( QDomDocument &doc, const QString &name, const QString &value );
    static QgsStringMap getVendorOptionList( QDomElement &element );

    static QgsStringMap parseProperties( QDomElement &element );
    static void saveProperties( QgsStringMap props, QDomDocument &doc, QDomElement &element );

    //! Reads a collection of symbols from XML and returns them in a map. Caller is responsible for deleting returned symbols.
    static QgsSymbolMap loadSymbols( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;
    //! Writes a collection of symbols to XML with specified tagName for the top-level element
    static QDomElement saveSymbols( QgsSymbolMap &symbols, const QString &tagName, QDomDocument &doc, const QgsReadWriteContext &context );

    static void clearSymbolMap( QgsSymbolMap &symbols );

    /**
     * Creates new mime data from a \a symbol.
     * This also sets the mime color data to match the symbol's color, so that copied symbols
     * can be paste in places where a color is expected.
     * \see symbolFromMimeData()
     * \since QGIS 3.0
     */
    static QMimeData *symbolToMimeData( QgsSymbol *symbol ) SIP_FACTORY;

    /**
     * Attempts to parse \a mime data as a symbol. A new symbol instance will be returned
     * if the data was successfully converted to a symbol.
     * \see symbolToMimeData()
     * \since QGIS 3.0
     */
    static QgsSymbol *symbolFromMimeData( const QMimeData *data ) SIP_FACTORY;

    /**
     * Creates a color ramp from the settings encoded in an XML element
     * \param element DOM element
     * \returns new color ramp. Caller takes responsibility for deleting the returned value.
     * \see saveColorRamp()
     */
    static QgsColorRamp *loadColorRamp( QDomElement &element ) SIP_FACTORY;

    /**
     * Encodes a color ramp's settings to an XML element
     * \param name name of ramp
     * \param ramp color ramp to save
     * \param doc XML document
     * \returns DOM element representing state of color ramp
     * \see loadColorRamp()
     */
    static QDomElement saveColorRamp( const QString &name, QgsColorRamp *ramp, QDomDocument &doc );

    /**
     * Saves a color ramp to a QVariantMap, wrapped in a QVariant.
     * You can use QgsXmlUtils::writeVariant to save it to an XML document.
     *
     * \see loadColorRamp( const QVariant &value )
     */
    static QVariant colorRampToVariant( const QString &name, QgsColorRamp *ramp );

    /**
     * Load a color ramp from a QVariantMap, wrapped in a QVariant.
     * You can use QgsXmlUtils::readVariant to load it from an XML document.
     *
     * \see colorRampToVariant()
     */
    static QgsColorRamp *loadColorRamp( const QVariant &value ) SIP_FACTORY;

    /**
     * Returns a friendly display name for a color
     * \param color source color
     * \returns display name for color
     * \since QGIS 2.5
     */
    static QString colorToName( const QColor &color );

    /**
     * Attempts to parse a string as a list of colors using a variety of common formats, including hex
     * codes, rgb and rgba strings.
     * \param colorStr string representing the color list
     * \returns list of parsed colors
     * \since QGIS 2.5
     */
    static QList< QColor > parseColorList( const QString &colorStr );

    /**
     * Creates mime data from a color. Sets both the mime data's color data, and the
     * mime data's text with the color's hex code.
     * \param color color to encode as mime data
     * \see colorFromMimeData
     * \since QGIS 2.5
     */
    static QMimeData *colorToMimeData( const QColor &color ) SIP_FACTORY;

    /**
     * Attempts to parse mime data as a color
     * \param data mime data to parse
     * \param hasAlpha will be set to true if mime data was interpreted as a color containing
     * an explicit alpha value
     * \returns valid color if mimedata could be interpreted as a color, otherwise an
     * invalid color
     * \since QGIS 2.5
     */
    static QColor colorFromMimeData( const QMimeData *data, bool &hasAlpha );

    /**
     * Attempts to parse mime data as a list of named colors
     * \param data mime data to parse
     * \returns list of parsed colors
     * \since QGIS 2.5
     */
    static QgsNamedColorList colorListFromMimeData( const QMimeData *data );

    /**
     * Creates mime data from a list of named colors
     * \param colorList list of named colors
     * \param allFormats set to true to include additional mime formats, include text/plain and application/x-color
     * \returns mime data containing encoded colors
     * \since QGIS 2.5
     */
    static QMimeData *colorListToMimeData( const QgsNamedColorList &colorList, bool allFormats = true ) SIP_FACTORY;

    /**
     * Exports colors to a gpl GIMP palette file
     * \param file destination file
     * \param paletteName name of palette, which is stored in gpl file
     * \param colors colors to export
     * \returns true if export was successful
     * \see importColorsFromGpl
     */
    static bool saveColorsToGpl( QFile &file, const QString &paletteName, const QgsNamedColorList &colors );

    /**
     * Imports colors from a gpl GIMP palette file
     * \param file source gpl file
     * \param ok will be true if file was successfully read
     * \param name will be set to palette name from gpl file, if present
     * \returns list of imported colors
     * \see saveColorsToGpl
     */
    static QgsNamedColorList importColorsFromGpl( QFile &file, bool &ok, QString &name );

    /**
     * Attempts to parse a string as a color using a variety of common formats, including hex
     * codes, rgb and rgba strings.
     * \param colorStr string representing the color
     * \param strictEval set to true for stricter color parsing rules
     * \returns parsed color
     * \since QGIS 2.3
     */
    static QColor parseColor( const QString &colorStr, bool strictEval = false );

    /**
     * Attempts to parse a string as a color using a variety of common formats, including hex
     * codes, rgb and rgba strings.
     * \param colorStr string representing the color
     * \param containsAlpha if colorStr contains an explicit alpha value then containsAlpha will be set to true
     * \param strictEval set to true for stricter color parsing rules
     * \returns parsed color
     * \since QGIS 2.3
     */
    static QColor parseColorWithAlpha( const QString &colorStr, bool &containsAlpha, bool strictEval = false );

    /**
     * Multiplies opacity of image pixel values with a (global) transparency value.
     */
    static void multiplyImageOpacity( QImage *image, qreal opacity );

    //! Blurs an image in place, e.g. creating Qt-independent drop shadows
    static void blurImageInPlace( QImage &image, QRect rect, int radius, bool alphaOnly );

    /**
     * Converts a QColor into a premultiplied ARGB QColor value using a specified alpha value
     * \since QGIS 2.3
     */
    static void premultiplyColor( QColor &rgb, int alpha );

    //! Sorts the passed list in requested order
    static void sortVariantList( QList<QVariant> &list, Qt::SortOrder order );
    //! Returns a point on the line from startPoint to directionPoint that is a certain distance away from the starting point
    static QPointF pointOnLineWithDistance( QPointF startPoint, QPointF directionPoint, double distance );

    //! Returns a list of all available svg files
    static QStringList listSvgFiles();

    //! Returns a list of svg files at the specified directory
    static QStringList listSvgFilesAt( const QString &directory );

    /**
     * Determines an SVG symbol's path from its \a name.
     * If \a name is not an absolute path the file is scanned for in the SVG paths specified
     * in settings svg/searchPathsForSVG.
     * \see svgSymbolPathToName()
     */
    static QString svgSymbolNameToPath( const QString &name, const QgsPathResolver &pathResolver );

    /**
     * Determines an SVG symbol's name from its \a path.
     * \see svgSymbolNameToPath()
     */
    static QString svgSymbolPathToName( const QString &path, const QgsPathResolver &pathResolver );

    //! Calculate the centroid point of a QPolygonF
    static QPointF polygonCentroid( const QPolygonF &points );

    //! Calculate a point within of a QPolygonF
    static QPointF polygonPointOnSurface( const QPolygonF &points );

    //! Calculate whether a point is within of a QPolygonF
    static bool pointInPolygon( const QPolygonF &points, QPointF point );

    /**
     * Returns a new valid expression instance for given field or expression string.
     * If the input is not a valid expression, it is assumed that it is a field name and gets properly quoted.
     * If the string is empty, returns null pointer.
     * This is useful when accepting input which could be either a non-quoted field name or expression.
     * \since QGIS 2.2
     */
    static QgsExpression *fieldOrExpressionToExpression( const QString &fieldOrExpression ) SIP_FACTORY;

    /**
     * Returns a field name if the whole expression is just a name of the field .
     *  Returns full expression string if the expression is more complex than just one field.
     *  Using just expression->expression() method may return quoted field name, but that is not
     *  wanted for saving (due to backward compatibility) or display in GUI.
     * \since QGIS 2.2
     */
    static QString fieldOrExpressionFromExpression( QgsExpression *expression );

    /**
     * Computes a sequence of about 'classes' equally spaced round values
     *  which cover the range of values from 'minimum' to 'maximum'.
     *  The values are chosen so that they are 1, 2 or 5 times a power of 10.
     * \since QGIS 2.10
     */
    static QList<double> prettyBreaks( double minimum, double maximum, int classes );

    /**
     * Rescales the given size based on the uomScale found in the props, if any is found, otherwise
     *  returns the value un-modified
     * \since QGIS 3.0
     */
    static double rescaleUom( double size, QgsUnitTypes::RenderUnit unit, const QgsStringMap &props );

    /**
     * Rescales the given point based on the uomScale found in the props, if any is found, otherwise
     *  returns a copy of the original point
     * \since QGIS 3.0
     */
    static QPointF rescaleUom( QPointF point, QgsUnitTypes::RenderUnit unit, const QgsStringMap &props ) SIP_PYNAME( rescalePointUom );

    /**
     * Rescales the given array based on the uomScale found in the props, if any is found, otherwise
     *  returns a copy of the original point
     * \since QGIS 3.0
     */
    static QVector<qreal> rescaleUom( const QVector<qreal> &array, QgsUnitTypes::RenderUnit unit, const QgsStringMap &props ) SIP_PYNAME( rescaleArrayUom );

    /**
     * Checks if the properties contain scaleMinDenom and scaleMaxDenom, if available, they are added into the SE Rule element
     * \since QGIS 3.0
     */
    static void applyScaleDependency( QDomDocument &doc, QDomElement &ruleElem, QgsStringMap &props );

    /**
      * Merges the local scale limits, if any, with the ones already in the map, if any
      * \since QGIS 3.0
      */
    static void mergeScaleDependencies( double mScaleMinDenom, double mScaleMaxDenom, QgsStringMap &props );

    /**
     * Encodes a reference to a parametric SVG into SLD, as a succession of parametric SVG using URL parameters,
     * a fallback SVG without parameters, and a final fallback as a mark with the right colors and stroke for systems
     * that cannot do SVG at all
     * \since QGIS 3.0
     */
    static void parametricSvgToSld( QDomDocument &doc, QDomElement &graphicElem,
                                    const QString &path,
                                    const QColor &fillColor, double size, const QColor &strokeColor, double strokeWidth );

    /**
     * Encodes a reference to a parametric SVG into a path with parameters according to the SVG Parameters spec
     * \since QGIS 3.0
     */
    static QString getSvgParametricPath( const QString &basePath, const QColor &fillColor, const QColor &strokeColor, double strokeWidth );

};

class QPolygonF;

//! calculate geometry shifted by a specified distance
QList<QPolygonF> offsetLine( QPolygonF polyline, double dist, QgsWkbTypes::GeometryType geometryType ) SIP_SKIP;

#endif


