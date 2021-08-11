/***************************************************************************
                          qgsserverapiutils.h

  Class defining utilities for QGIS server APIs.
  -------------------
  begin                : 2019-04-16
  copyright            : (C) 2019 by Alessandro Pasotti
  email                : elpaso at itopen dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSSERVERAPIUTILS_H
#define QGSSERVERAPIUTILS_H

#include "qgis_server.h"
#include <QString>
#include "qgsproject.h"
#include "qgsserverprojectutils.h"
#include "qgsserverapicontext.h"
#include "qgsserverexception.h"
#include "qgsmaplayerserverproperties.h"
#include "qgsrange.h"
#include "qgsjsonutils.h"

#ifdef HAVE_SERVER_PYTHON_PLUGINS
#include "qgsaccesscontrol.h"
#include "qgsserverinterface.h"
#endif

class QgsRectangle;
class QgsCoordinateReferenceSystem;
class QgsVectorLayer;

#ifndef SIP_RUN
#include "nlohmann/json_fwd.hpp"
using namespace nlohmann;
#endif

/**
 * \ingroup server
 * \brief The QgsServerApiUtils class contains helper functions to handle common API operations.
 * \since QGIS 3.10
 */
class SERVER_EXPORT QgsServerApiUtils
{

  public:

    /**
     * Parses a comma separated \a bbox into a (possibly empty) QgsRectangle.
     *
     * \note Z values (i.e. a 6 elements bbox) are silently discarded
     */
    static QgsRectangle parseBbox( const QString &bbox );

    /**
     * Returns a list of temporal dimensions information for the given \a layer (either configured in wmsDimensions or the first date/datetime field)
     * \since QGIS 3.12
     */
    static QList< QgsServerWmsDimensionProperties::WmsDimensionInfo > temporalDimensions( const QgsVectorLayer *layer );

    /**
     * Parses a date \a interval and returns a QgsDateRange
     *
     * \throws QgsServerApiBadRequestException if interval cannot be parsed
     * \since QGIS 3.12
     */
    static QgsDateRange parseTemporalDateInterval( const QString &interval ) SIP_THROW( QgsServerApiBadRequestException );

    /**
     * Parses a datetime \a interval and returns a QgsDateTimeRange
     *
     * \throws QgsServerApiBadRequestException if interval cannot be parsed
     * \since QGIS 3.12
     */
    static QgsDateTimeRange parseTemporalDateTimeInterval( const QString &interval ) SIP_THROW( QgsServerApiBadRequestException );

///@cond PRIVATE
    // T is TemporalDateInterval|TemporalDateTimeInterval, T2 is QDate|QdateTime
    template<typename T, class T2> static T parseTemporalInterval( const QString &interval ) SIP_SKIP;
/// @endcond


    /**
     * Parses the \a interval and constructs a (possibly invalid) temporal filter expression for the given \a layer
     *
     * Interval syntax:
     *
     *   interval-closed     = date-time "/" date-time
     *   interval-open-start = [".."] "/" date-time
     *   interval-open-end   = date-time "/" [".."]
     *   interval            = interval-closed / interval-open-start / interval-open-end
     *   datetime            = date-time / interval
     * \since QGIS 3.12
     */
    static QgsExpression temporalFilterExpression( const QgsVectorLayer *layer, const QString &interval );

    /**
     * layerExtent returns json array with [xMin,yMin,xMax,yMax] CRS84 extent for the given \a layer
     */
    static json layerExtent( const QgsVectorLayer *layer ) SIP_SKIP;

    /**
     * temporalExtent returns a json array with an array of [min, max] temporal extent for the given \a layer.
     * In case multiple temporal dimensions are available in the layer, a union of all dimensions is returned.
     *
     * From specifications: http://schemas.opengis.net/ogcapi/features/part1/1.0/openapi/schemas/extent.yaml
     *
     * One or more time intervals that describe the temporal extent of the dataset.
     * The value `null` is supported and indicates an open time interval.
     *
     * In the Core only a single time interval is supported. Extensions may support
     * multiple intervals. If multiple intervals are provided, the union of the
     * intervals describes the temporal extent.
     *
     * \returns An array of intervals
     * \note not available in Python bindings
     * \since QGIS 3.12
     */
    static json temporalExtent( const QgsVectorLayer *layer ) SIP_SKIP;

    /**
     * temporalExtent returns a json array with an array of [min, max] temporal extent for the given \a layer.
     * In case multiple temporal dimensions are available in the layer, a union of all dimensions is returned.
     *
     * From specifications: http://schemas.opengis.net/ogcapi/features/part1/1.0/openapi/schemas/extent.yaml
     *
     * One or more time intervals that describe the temporal extent of the dataset.
     * The value `null` is supported and indicates an open time interval.
     *
     * In the Core only a single time interval is supported. Extensions may support
     * multiple intervals. If multiple intervals are provided, the union of the
     * intervals describes the temporal extent.
     *
     * \returns An array of intervals
     * \since QGIS 3.12
     */
    static QVariantList temporalExtentList( const QgsVectorLayer *layer ) SIP_PYNAME( temporalExtent );

    /**
     * Parses the CRS URI \a bboxCrs (example: "http://www.opengis.net/def/crs/OGC/1.3/CRS84") into a QGIS CRS object
     */
    static QgsCoordinateReferenceSystem parseCrs( const QString &bboxCrs );

    /**
     * Returns the list of layers accessible to the service for a given \a context.
     *
     * This method takes into account the ACL restrictions provided by QGIS Server Access Control plugins.
     */
    static const QVector<QgsVectorLayer *> publishedWfsLayers( const QgsServerApiContext &context );

#ifndef SIP_RUN

    /**
     * Returns the list of layers of type T accessible to the WFS service for a given \a project.
     *
     * Example:
     *
     *     QVector<const QgsVectorLayer*> vectorLayers = publishedLayers<QgsVectorLayer>();
     *
     * \note not available in Python bindings
     */
    template <typename T>
    static const QVector<T> publishedWfsLayers( const QgsServerApiContext &context )
    {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      QgsAccessControl *accessControl = context.serverInterface()->accessControls();
#endif
      const QgsProject *project = context.project();
      QVector<T> result;
      if ( project )
      {
        const QStringList wfsLayerIds = QgsServerProjectUtils::wfsLayerIds( *project );
        const auto constLayers { project->layers<T>() };
        for ( const auto &layer : constLayers )
        {
          if ( ! wfsLayerIds.contains( layer->id() ) )
          {
            continue;
          }
#ifdef HAVE_SERVER_PYTHON_PLUGINS
          if ( accessControl && !accessControl->layerReadPermission( layer ) )
          {
            continue;
          }
#endif
          result.push_back( layer );
        }
      }
      return result;
    }

#endif


    /**
     * Sanitizes the input \a value by removing URL encoding.
     * \note the returned value is meant to become part of a QgsExpression filter
     */
    static QString sanitizedFieldValue( const QString &value );

    /**
     * Returns the list of CRSs (format: http://www.opengis.net/def/crs/OGC/1.3/CRS84) available for this \a project.
     * Information is read from project WMS configuration.
     */
    static QStringList publishedCrsList( const QgsProject *project );

    /**
     * Returns a \a crs as OGC URI (format: http://www.opengis.net/def/crs/OGC/1.3/CRS84)
     * Returns an empty string on failure.
     */
    static QString crsToOgcUri( const QgsCoordinateReferenceSystem &crs );

    /**
     * Appends MAP query string parameter from current \a requestUrl to the given \a path
     */
    static QString appendMapParameter( const QString &path, const QUrl &requestUrl );

};
#endif // QGSSERVERAPIUTILS_H
