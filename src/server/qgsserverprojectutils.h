/***************************************************************************
                              qgsserverprojectutils.h
                              -----------------------
  begin                : December 19, 2016
  copyright            : (C) 2016 by Paul Blottiere
  email                : paul dot blottiere at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSERVERPROJECTUTILS_H
#define QGSSERVERPROJECTUTILS_H

#include <QString>
#include <QHash>
#include <cmath>

#include "qgis_server.h"
#include "qgis_sip.h"
#include "qgsserverrequest.h"
#include "qgsserversettings.h"

class QgsProject;
class QgsRectangle;

/**
 * \ingroup server
 * \brief Contains utility functions to retrieve specific entries from a QgsProject.
 * \since QGIS 3.0
 */
class SERVER_EXPORT QgsServerProjectUtils
{
  public:
    /**
   * Returns a double greater than \a number to the specified number of \a places.
   *
   * \since QGIS 3.10.1
   */
    static double ceilWithPrecision( double number, int places ) SIP_SKIP;

    /**
   * Returns a double less than \a number to the specified number of \a places.
   *
   * \since QGIS 3.10.1
   */
    static double floorWithPrecision( double number, int places ) SIP_SKIP;

    /**
   * Returns if owsService capabilities are enabled.
   * \param project the QGIS project
   * \returns if owsService capabilities are enabled.
   */
    static bool owsServiceCapabilities( const QgsProject &project );

    /**
   * Returns the owsService title defined in project.
   * \param project the QGIS project
   * \returns the owsService title if defined in project with project title as fallback, "Untitled" otherwise.
   */
    static QString owsServiceTitle( const QgsProject &project );

    /**
   * Returns the owsService abstract defined in project.
   * \param project the QGIS project
   * \returns the owsService abstract if defined in project.
   */
    static QString owsServiceAbstract( const QgsProject &project );

    /**
   * Returns the owsService keywords defined in project.
   * \param project the QGIS project
   * \returns the owsService keywords if defined in project.
   */
    static QStringList owsServiceKeywords( const QgsProject &project );

    /**
   * Returns the owsService online resource defined in project.
   * \param project the QGIS project
   * \returns the owsService online resource if defined in project.
   */
    static QString owsServiceOnlineResource( const QgsProject &project );

    /**
   * Returns the owsService contact organization defined in project.
   * \param project the QGIS project
   * \returns the owsService contact organization if defined in project.
   */
    static QString owsServiceContactOrganization( const QgsProject &project );

    /**
   * Returns the owsService contact position defined in project.
   * \param project the QGIS project
   * \returns the owsService contact position if defined in project.
   */
    static QString owsServiceContactPosition( const QgsProject &project );

    /**
   * Returns the owsService contact person defined in project.
   * \param project the QGIS project
   * \returns the owsService contact person if defined in project.
   */
    static QString owsServiceContactPerson( const QgsProject &project );

    /**
   * Returns the owsService contact mail defined in project.
   * \param project the QGIS project
   * \returns the owsService contact mail if defined in project.
   */
    static QString owsServiceContactMail( const QgsProject &project );

    /**
   * Returns the owsService contact phone defined in project.
   * \param project the QGIS project
   * \returns the owsService contact phone if defined in project.
   */
    static QString owsServiceContactPhone( const QgsProject &project );

    /**
   * Returns the owsService fees defined in project.
   * \param project the QGIS project
   * \returns the owsService fees if defined in project.
   */
    static QString owsServiceFees( const QgsProject &project );

    /**
   * Returns the owsService access constraints defined in project.
   * \param project the QGIS project
   * \returns the owsService access constraints if defined in project.
   */
    static QString owsServiceAccessConstraints( const QgsProject &project );

    /**
   * Returns the maximum width for WMS images defined in a QGIS project.
   * \param project the QGIS project
   * \returns width if defined in project, -1 otherwise.
   */
    static int wmsMaxWidth( const QgsProject &project );

    /**
   * Returns the maximum height for WMS images defined in a QGIS project.
   * \param project the QGIS project
   * \returns height if defined in project, -1 otherwise.
   */
    static int wmsMaxHeight( const QgsProject &project );

    /**
   * Returns the quality for WMS images defined in a QGIS project.
   * \param project the QGIS project
   * \returns quality if defined in project, -1 otherwise.
   */
    static int wmsImageQuality( const QgsProject &project );

    /**
   * Returns the tile buffer in pixels for WMS images defined in a QGIS project.
   * \param project the QGIS project
   * \returns tile buffer if defined in project, 0 otherwise.
   * \since QGIS 3.10
   */
    static int wmsTileBuffer( const QgsProject &project );

    /**
   * Returns TRUE if WMS requests should use the QgsMapSettings::RenderMapTile flag,
   * so that no visible artifacts are visible between adjacent tiles.
   *
   * This flag can slow down rendering considerably, so it is only used if the corresponding
   * setting is enabled in the project.
   *
   * \param project the QGIS project
   * \returns TRUE if the flag should be used, or FALSE if not.
   *
   * \since QGIS 3.18
   */
    static bool wmsRenderMapTiles( const QgsProject &project );

    /**
   * Returns the maximum number of atlas features which can be printed in a request
   * \param project the QGIS project
   * \return the number of atlas features
   */
    static int wmsMaxAtlasFeatures( const QgsProject &project );

    /**
   * Returns the default number of map units per millimeters in case of the scale is not given
   * \param project the QGIS project
   * \returns the default number of map units per millimeter
   * \since QGIS 3.4
   */
    static double wmsDefaultMapUnitsPerMm( const QgsProject &project );

    /**
   * Returns if layer ids are used as name in WMS.
   * \param project the QGIS project
   * \returns if layer ids are used as name.
   */
    static bool wmsUseLayerIds( const QgsProject &project );

    /**
   * Returns if the info format is SIA20145.
   * \param project the QGIS project
   * \returns if the info format is SIA20145.
   */
    static bool wmsInfoFormatSia2045( const QgsProject &project );

    /**
   * Returns if the geometry is displayed as Well Known Text in GetFeatureInfo request.
   * \param project the QGIS project
   * \returns if the geometry is displayed as Well Known Text in GetFeatureInfo request.
   */
    static bool wmsFeatureInfoAddWktGeometry( const QgsProject &project );

    /**
    * Returns if feature form settings should be considered for the format of the feature info response
    * \param project the QGIS project
    * \returns true if the feature form settings shall be considered for the feature info response
   */
    static bool wmsFeatureInfoUseAttributeFormSettings( const QgsProject &project );

    /**
   * Returns if the geometry has to be segmentize in GetFeatureInfo request.
   * \param project the QGIS project
   * \returns if the geometry has to be segmentize in GetFeatureInfo request.
   */
    static bool wmsFeatureInfoSegmentizeWktGeometry( const QgsProject &project );

    /**
   * Returns if legend groups should be in the legend graphic response if GetLegendGraphic is called on a layer group.
   * \param project the QGIS project
   * \returns if the GetLegendGraphic response has to contain legend groups
   */
    static bool wmsAddLegendGroupsLegendGraphic( const QgsProject &project );

    /**
   * Returns if name attribute should be skipped for groups in WMS capabilities document.
   * \param project the QGIS project
   * \returns if name attribute should be skipped for groups in capabilities
   * \since QGIS 3.36
   */
    static bool wmsSkipNameForGroup( const QgsProject &project );

    /**
   * Returns the geometry precision for GetFeatureInfo request.
   * \param project the QGIS project
   * \returns the geometry precision for GetFeatureInfo request.
   */
    static int wmsFeatureInfoPrecision( const QgsProject &project );

    /**
   * Returns the document element name for XML GetFeatureInfo request.
   * \param project the QGIS project
   * \returns the document element name for XML GetFeatureInfo request.
   */
    static QString wmsFeatureInfoDocumentElement( const QgsProject &project );

    /**
   * Returns the document element namespace for XML GetFeatureInfo request.
   * \param project the QGIS project
   * \returns the document element namespace for XML GetFeatureInfo request.
   */
    static QString wmsFeatureInfoDocumentElementNs( const QgsProject &project );

    /**
   * Returns the schema URL for XML GetFeatureInfo request.
   * \param project the QGIS project
   * \returns the schema URL for XML GetFeatureInfo request.
   */
    static QString wmsFeatureInfoSchema( const QgsProject &project );

    /**
   * Returns the mapping between layer name and wms layer name for GetFeatureInfo request.
   * \param project the QGIS project
   * \returns the mapping between layer name and wms layer name for GetFeatureInfo request.
   */
    static QHash<QString, QString> wmsFeatureInfoLayerAliasMap( const QgsProject &project );

    /**
   * Returns if Inspire is activated.
   * \param project the QGIS project
   * \returns if Inspire is activated.
   */
    static bool wmsInspireActivate( const QgsProject &project );

    /**
   * Returns the Inspire language.
   * \param project the QGIS project
   * \returns the Inspire language if defined in project.
   */
    static QString wmsInspireLanguage( const QgsProject &project );

    /**
   * Returns the Inspire metadata URL.
   * \param project the QGIS project
   * \returns the Inspire metadata URL if defined in project.
   */
    static QString wmsInspireMetadataUrl( const QgsProject &project );

    /**
   * Returns the Inspire metadata URL type.
   * \param project the QGIS project
   * \returns the Inspire metadata URL type if defined in project.
   */
    static QString wmsInspireMetadataUrlType( const QgsProject &project );

    /**
   * Returns the Inspire temporal reference.
   * \param project the QGIS project
   * \returns the Inspire temporal reference if defined in project.
   */
    static QString wmsInspireTemporalReference( const QgsProject &project );

    /**
   * Returns the Inspire metadata date.
   * \param project the QGIS project
   * \returns the Inspire metadata date if defined in project.
   */
    static QString wmsInspireMetadataDate( const QgsProject &project );

    /**
   * Returns the restricted composer list.
   * \param project the QGIS project
   * \returns the restricted composer list if defined in project.
   */
    static QStringList wmsRestrictedComposers( const QgsProject &project );

    /**
   * Returns the WMS service url.
   * The URL defined in the project or if not defined the URL from serviceUrl.
   *
   * \param project the QGIS project
   * \param request the request
   * \param settings the server settings
   * \returns url to use for this service
   */
    static QString wmsServiceUrl( const QgsProject &project, const QgsServerRequest &request = QgsServerRequest(), const QgsServerSettings &settings = QgsServerSettings() );

    /**
   * Returns the WMS root layer name defined in a QGIS project.
   * \param project the QGIS project
   * \returns root layer name to use for this service
   */
    static QString wmsRootName( const QgsProject &project );

    /**
   * Returns the restricted layer name list.
   * \param project the QGIS project
   * \returns the restricted layer name list if defined in project.
   */
    static QStringList wmsRestrictedLayers( const QgsProject &project );

    /**
   * Returns the WMS output CRS list.
   * \param project the QGIS project
   * \returns the WMS output CRS list.
   */
    static QStringList wmsOutputCrsList( const QgsProject &project );

    /**
   * Returns the WMS Extent restriction.
   * \param project the QGIS project
   * \returns the WMS Extent restriction.
   */
    static QgsRectangle wmsExtent( const QgsProject &project );

    /**
   * Returns the WFS service url.
   * The URL defined in the project or if not defined the URL from serviceUrl.
   *
   * \param project the QGIS project
   * \param request the request
   * \param settings the server settings
   * \returns url to use for this service
   */
    static QString wfsServiceUrl( const QgsProject &project, const QgsServerRequest &request = QgsServerRequest(), const QgsServerSettings &settings = QgsServerSettings() );

    /**
   * Returns the Layer ids list defined in a QGIS project as published in WFS.
   * \param project the QGIS project
   * \return the Layer ids list.
   */
    static QStringList wfsLayerIds( const QgsProject &project );

    /**
   * Returns the Layer precision defined in a QGIS project for the WFS GetFeature.
   * \param project the QGIS project
   * \param layerId the layer id in the project
   * \return the layer precision for WFS GetFeature.
   */

    static int wfsLayerPrecision( const QgsProject &project, const QString &layerId );

    /**
   * Returns the Layer ids list defined in a QGIS project as published as WFS-T with update capabilities.
   * \param project the QGIS project
   * \return the Layer ids list.
   */
    static QStringList wfstUpdateLayerIds( const QgsProject &project );

    /**
   * Returns the Layer ids list defined in a QGIS project as published as WFS-T with insert capabilities.
   * \param project the QGIS project
   * \return the Layer ids list.
   */
    static QStringList wfstInsertLayerIds( const QgsProject &project );

    /**
   * Returns the Layer ids list defined in a QGIS project as published as WFS-T with delete capabilities.
   * \param project the QGIS project
   * \return the Layer ids list.
   */
    static QStringList wfstDeleteLayerIds( const QgsProject &project );

    /**
   * Returns the WCS service url.
   * The URL defined in the project or if not defined the URL from serviceUrl.
   *
   * \param project the QGIS project
   * \param request the request
   * \param settings the server settings
   * \returns url to use for this service
   */
    static QString wcsServiceUrl( const QgsProject &project, const QgsServerRequest &request = QgsServerRequest(), const QgsServerSettings &settings = QgsServerSettings() );

    /**
   * Returns the Layer ids list defined in a QGIS project as published in WCS.
   * \param project the QGIS project
   * \returns the Layer ids list.
   */
    static QStringList wcsLayerIds( const QgsProject &project );

    /**
   * Returns the WMTS service url.
   * The URL defined in the project or if not defined the URL from serviceUrl.
   *
   * \param project the QGIS project
   * \param request the request
   * \param settings the server settings
   * \returns url to use for this service
   * \since QGIS 3.4
   */
    static QString wmtsServiceUrl( const QgsProject &project, const QgsServerRequest &request = QgsServerRequest(), const QgsServerSettings &settings = QgsServerSettings() );

    /**
   * Returns the service url defined in the environment variable or with HTTP header.
   * This is calculated from (in order of precedence):
   *
   * - Value defined in the project per service
   * - The ``QGIS_SERVER_<service>_SERVICE_URL`` environment variable
   * - The ``QGIS_SERVER_SERVICE_URL`` environment variable
   * - The custom ``X-Qgis-<service>-Service-Url`` header
   * - The custom ``X-Qgis-Service-Url`` header
   * - Build from the standard ``Forwarded`` header
   * - Build from the pseudo standard ``X-Forwarded-Host`` and ``X-Forwarded-Proto`` headers
   * - Build from the standard ``Host`` header and the server protocol
   * - Build from the server name and the server protocol.
   *
   * \param request the request
   * \param service the used service
   * \param settings the server settings
   * \returns url to use for this service
   * \since QGIS 3.20
   */
    static QString serviceUrl( const QString &service, const QgsServerRequest &request, const QgsServerSettings &settings );
};

#endif
