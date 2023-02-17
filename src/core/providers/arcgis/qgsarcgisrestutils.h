/***************************************************************************
    qgsarcgisrestutils.h
    --------------------
    begin                : Nov 25, 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : manisandro@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSARCGISRESTUTILS_H
#define QGSARCGISRESTUTILS_H

#include "qgswkbtypes.h"
#include "qgsrectangle.h"
#include "qgsmarkersymbollayer.h"
#include "qgscoordinatereferencesystem.h"

#include "qgis_sip.h"

#include <QStringList>
#include <QVariant>
#include <QTimeZone>

#include <functional>
#include <memory>

class QNetworkReply;
class QgsNetworkAccessManager;
class QgsFields;
class QgsAbstractGeometry;
class QgsAbstractVectorLayerLabeling;
class QgsCoordinateReferenceSystem;
class QgsFeedback;
class QgsSymbol;
class QgsLineSymbol;
class QgsFillSymbol;
class QgsMarkerSymbol;
class QgsFeatureRenderer;
class QgsPoint;
class QgsAbstractGeometry;
class QgsCircularString;
class QgsCompoundCurve;
class QgsMultiPoint;
class QgsMultiSurface;
class QgsMultiLineString;
class QgsPolygon;
class QgsMultiCurve;
class QgsMultiPolygon;
class QgsCurvePolygon;

/**
 * \ingroup core
 * \brief Contains the context of a ArcGIS REST service operation.
 *
 * \see QgsArcGisRestUtils
 *
 * \since QGIS 3.28
 */
class CORE_EXPORT QgsArcGisRestContext
{
  public:

    /**
     * Sets the time \a zone for datetime values.
     *
     * \see timeZone()
     */
    void setTimeZone( const QTimeZone &zone ) { mTimeZone = zone; }

    /**
     * Returns the time zone for datetime values.
     *
     * \see setTimeZone()
     */
    QTimeZone timeZone() const { return mTimeZone; }

    /**
     * Sets the name of the objectId field.
     *
     * \see objectIdFieldName()
     */
    void setObjectIdFieldName( const QString &name ) { mObjectIdFieldName = name; }

    /**
     * Returns the name of the objectId field.
     *
     * \see setObjectIdFieldName()
     */
    QString objectIdFieldName() const { return mObjectIdFieldName; }

  private:

    QTimeZone mTimeZone;

    QString mObjectIdFieldName;

};

/**
 * \ingroup core
 * \brief Utility functions for working with ArcGIS REST services.
 *
 * \see QgsArcGisPortalUtils
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsArcGisRestUtils
{
    Q_GADGET

  public:

    /**
     * Converts an ESRI REST field \a type to a QVariant type.
     */
    static QVariant::Type convertFieldType( const QString &type );

    /**
     * Converts an ESRI REST geometry \a type to a WKB type.
     */
    static QgsWkbTypes::Type convertGeometryType( const QString &type );

    /**
     * Converts an ESRI REST \a geometry JSON definition to a QgsAbstractGeometry.
     *
     * Caller takes ownership of the returned object.
     *
     * \param geometry JSON geometry definition
     * \param esriGeometryType ESRI geometry type string
     * \param hasM set to TRUE to if geometry includes M values
     * \param hasZ set to TRUE to if geometry includes Z values
     * \param crs if specified will be set to the parsed geometry CRS
     *
     * \returns converted geometry
     */
    static QgsAbstractGeometry *convertGeometry( const QVariantMap &geometry, const QString &esriGeometryType, bool hasM, bool hasZ, QgsCoordinateReferenceSystem *crs SIP_OUT = nullptr ) SIP_FACTORY;

    /**
     * Converts a spatial reference JSON definition to a QgsCoordinateReferenceSystem value.
     */
    static QgsCoordinateReferenceSystem convertSpatialReference( const QVariantMap &spatialReferenceMap );

    /**
     * Converts a symbol JSON \a definition to a QgsSymbol.
     *
     * Caller takes ownership of the returned symbol.
     */
    static QgsSymbol *convertSymbol( const QVariantMap &definition ) SIP_FACTORY;

    /**
     * Converts renderer JSON \a data to an equivalent QgsFeatureRenderer.
     *
     * Caller takes ownership of the returned renderer.
     */
    static QgsFeatureRenderer *convertRenderer( const QVariantMap &rendererData ) SIP_FACTORY;

    /**
     * Converts labeling JSON \a data to an equivalent QGIS vector labeling.
     *
     * Caller takes ownership of the returned object.
     */
    static QgsAbstractVectorLayerLabeling *convertLabeling( const QVariantList &data ) SIP_FACTORY;

    /**
     * Converts an ESRI labeling expression to a QGIS expression string.
     */
    static QString convertLabelingExpression( const QString &string );

    /**
     * Converts ESRI JSON color data to a QColor object.
     */
    static QColor convertColor( const QVariant &data );

    /**
     * Converts an ESRI line \a style to a Qt pen style.
     */
    static Qt::PenStyle convertLineStyle( const QString &style );

    /**
     * Converts an ESRI fill \a style to a Qt brush style.
     */
    static Qt::BrushStyle convertFillStyle( const QString &style );

    /**
     * Converts a date time \a value to a QDateTime.
     */
    static QDateTime convertDateTime( const QVariant &value );

    /**
     * Converts a \a geometry to an ArcGIS REST JSON representation.
     *
     * Returns an empty map if the geometry cannot be converted.
     *
     * \since QGIS 3.28
     */
    static QVariantMap geometryToJson( const QgsGeometry &geometry, const QgsArcGisRestContext &context, const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() );

    /**
     * Converts a \a crs to an ArcGIS REST JSON representation.
     *
     * Returns an empty map if the \a crs is not valid.
     *
     * \since QGIS 3.28
     */
    static QVariantMap crsToJson( const QgsCoordinateReferenceSystem &crs );

    /**
     * Flags which control the behavior of converting features to JSON.
     *
     * \since QGIS 3.28
     */
    enum class FeatureToJsonFlag : int
    {
      IncludeGeometry = 1 << 0, //!< Whether to include the geometry definition
      IncludeNonObjectIdAttributes = 1 << 1, //!< Whether to include any non-objectId attributes
    };
    Q_ENUM( FeatureToJsonFlag );

    /**
     * Flags which control the behavior of converting features to JSON.
     *
     * \since QGIS 3.28
     */
    Q_DECLARE_FLAGS( FeatureToJsonFlags, FeatureToJsonFlag )
    Q_FLAG( FeatureToJsonFlags )

    /**
     * Converts a \a feature to an ArcGIS REST JSON representation.
     *
     * \since QGIS 3.28
     */
    static QVariantMap featureToJson( const QgsFeature &feature,
                                      const QgsArcGisRestContext &context,
                                      const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem(),
                                      QgsArcGisRestUtils::FeatureToJsonFlags flags = QgsArcGisRestUtils::FeatureToJsonFlags( static_cast< int >( QgsArcGisRestUtils::FeatureToJsonFlag::IncludeGeometry ) | static_cast< int >( QgsArcGisRestUtils::FeatureToJsonFlag::IncludeNonObjectIdAttributes ) ) );

    /**
     * Converts a variant to a REST attribute value.
     *
     * \since QGIS 3.28
     */
    static QVariant variantToAttributeValue( const QVariant &variant, QVariant::Type expectedType, const QgsArcGisRestContext &context );

    /**
     * Converts a \a field's definition to an ArcGIS REST JSON representation.
     *
     * \since QGIS 3.28
     */
    static QVariantMap fieldDefinitionToJson( const QgsField &field );

    /**
     * Converts a string value to a REST service type.
     *
     * \since QGIS 3.28
     */
    static Qgis::ArcGisRestServiceType serviceTypeFromString( const QString &type );

  private:

    /**
     * Converts a JSON \a list to a point geometry of the specified wkb \a type.
     */
    static std::unique_ptr< QgsPoint > convertPoint( const QVariantList &list, QgsWkbTypes::Type type );

    /**
     * Converts circular string JSON \a data to a geometry object of the specified \a type.
     *
     * The \a startPoint argument must specify the starting point of the circular string.
     */
    static std::unique_ptr< QgsCircularString > convertCircularString( const QVariantMap &data, QgsWkbTypes::Type type, const QgsPoint &startPoint );

    /**
     * Converts a compound curve JSON \a list to a geometry object of the specified \a type.
     */
    static std::unique_ptr< QgsCompoundCurve > convertCompoundCurve( const QVariantList &list, QgsWkbTypes::Type type );

    /**
     * Converts point \a data to a point object of the specified \a type.
     */
    static std::unique_ptr< QgsPoint > convertGeometryPoint( const QVariantMap &data, QgsWkbTypes::Type pointType );

    /**
     * Converts multipoint \a data to a multipoint object of the specified \a type.
     */
    static std::unique_ptr< QgsMultiPoint > convertMultiPoint( const QVariantMap &geometryData, QgsWkbTypes::Type pointType );

    /**
     * Converts polyline \a data to a curve object of the specified \a type.
     */
    static std::unique_ptr< QgsMultiCurve > convertGeometryPolyline( const QVariantMap &data, QgsWkbTypes::Type pointType );

    /**
     * Converts polygon \a data to a polygon object of the specified \a type.
     */
    static std::unique_ptr< QgsMultiSurface > convertGeometryPolygon( const QVariantMap &data, QgsWkbTypes::Type pointType );

    /**
     * Converts envelope \a data to a polygon object.
     */
    static std::unique_ptr< QgsPolygon > convertEnvelope( const QVariantMap &data );

    static std::unique_ptr< QgsLineSymbol > parseEsriLineSymbolJson( const QVariantMap &symbolData );
    static std::unique_ptr< QgsFillSymbol > parseEsriFillSymbolJson( const QVariantMap &symbolData );
    static std::unique_ptr< QgsFillSymbol > parseEsriPictureFillSymbolJson( const QVariantMap &symbolData );
    static std::unique_ptr< QgsMarkerSymbol > parseEsriMarkerSymbolJson( const QVariantMap &symbolData );
    static std::unique_ptr< QgsMarkerSymbol > parseEsriPictureMarkerSymbolJson( const QVariantMap &symbolData );

    static Qgis::MarkerShape parseEsriMarkerShape( const QString &style );

    static QVariantMap pointToJson( const QgsPoint *point );
    static QVariantMap multiPointToJson( const QgsMultiPoint *multiPoint );
    static QVariantList lineStringToJsonPath( const QgsLineString *line );
    static QVariantList curveToJsonCurve( const QgsCurve *curve, bool includeStart );
    static QVariantMap lineStringToJson( const QgsLineString *line );
    static QVariantMap curveToJson( const QgsCurve *curve );
    static QVariantMap multiLineStringToJson( const QgsMultiLineString *multiLine );
    static QVariantMap multiCurveToJson( const QgsMultiCurve *multiCurve );
    static QVariantList polygonToJsonRings( const QgsPolygon *polygon );
    static QVariantList curvePolygonToJsonRings( const QgsCurvePolygon *polygon );
    static QVariantMap polygonToJson( const QgsPolygon *polygon );
    static QVariantMap curvePolygonToJson( const QgsCurvePolygon *polygon );
    static QVariantMap multiPolygonToJson( const QgsMultiPolygon *polygon );
    static QVariantMap multiSurfaceToJson( const QgsMultiSurface *multiSurface );

    friend class TestQgsArcGisRestUtils;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsArcGisRestUtils::FeatureToJsonFlags )

#endif // QGSARCGISRESTUTILS_H
