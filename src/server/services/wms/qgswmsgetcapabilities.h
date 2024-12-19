/***************************************************************************
                              qgswmsgetcapabilities.h
                              -------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  (original code)
                         (C) 2014 by Alessandro Pasotti (original code)
                         (C) 2016 by David Marteau
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
                         a dot pasotti at itopen dot it
                         david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSWMSGETCAPABILITIES_H
#define QGSWMSGETCAPABILITIES_H

#include "qgslayertreenode.h"
#include "qgslayertreegroup.h"
#include "qgslayertreelayer.h"
#include "qgslayertree.h"

#include "qgswmsrequest.h"
#include "qgswmslayerinfos.h"

namespace QgsWms
{

  /**
   * Create element for get capabilities document
   */
  QDomElement getLayersAndStylesCapabilitiesElement( QDomDocument &doc,
      QgsServerInterface *serverIface,
      const QgsProject *project,
      const QgsWmsRequest &request,
      bool projectSettings );

  /**
   * Create WFSLayers element for get capabilities document
   */
  QDomElement getWFSLayersElement( QDomDocument &doc, const QgsProject *project );

  /**
   * Create ComposerTemplates element for get capabilities document
   */
  QDomElement getComposerTemplatesElement( QDomDocument &doc, const QgsProject *project );

  /**
   * Create InspireCapabilities element for get capabilities document
   */
  QDomElement getInspireCapabilitiesElement( QDomDocument &doc, const QgsProject *project );

  /**
   * Create Capability element for get capabilities document
   */
  QDomElement getCapabilityElement( QDomDocument &doc, const QgsProject *project,
                                    const QgsWmsRequest &request, bool projectSettings,
                                    QgsServerInterface *serverIface );

  /**
   * Create Service element for get capabilities document
   */
  QDomElement getServiceElement( QDomDocument &doc, const QgsProject *project,
                                 const QgsWmsRequest &request, const QgsServerSettings *serverSettings );

  /**
   * Output GetCapabilities response
   */
  void writeGetCapabilities( QgsServerInterface *serverIface,
                             const QgsProject *project,
                             const QgsWmsRequest &request,
                             QgsServerResponse &response,
                             bool projectSettings = false );

  /**
   * Creates the WMS GetCapabilities XML document.
   * \param serverIface Interface for plugins
   * \param project Project
   * \param request WMS request
   * \param projectSettings If TRUE, adds extended project information (does not validate against WMS schema)
   * \returns GetCapabilities XML document
   */
  QDomDocument getCapabilities( QgsServerInterface *serverIface, const QgsProject *project,
                                const QgsWmsRequest &request,
                                bool projectSettings );

  /**
   * Returns true if at least one layer from the layers ids is queryable
   * \param layerIds  list of layer ids
   * \param wmsLayerInfos WMS layers definition to build WMS capabilities
   * \returns True if at least one layer form the layers ids is queryable
   * \since QGIS 3.28.0
   */
  bool hasQueryableLayers( const QStringList &layerIds, const QMap< QString, QgsWmsLayerInfos > &wmsLayerInfos );

  /**
   * Returns the combination of the WGS84 bounding rectangle of the layers from the list of layers ids
   * \param layerIds  list of layer ids
   * \param wmsLayerInfos WMS layers definition to build WMS capabilities
   * \returns the extent combination of the WGS84 bounding rectangle of the layers from the list of layers ids
   * \since QGIS 3.28.0
   */
  QgsRectangle combineWgs84BoundingRect( const QStringList &layerIds, const QMap< QString, QgsWmsLayerInfos > &wmsLayerInfos );

  /**
   * Returns the combinations of the extent CRSes of the layers from the list of layers ids
   * \param layerIds  list of layer ids
   * \param wmsLayerInfos WMS layers definition to build WMS capabilities
   * \returns the extent combination of the WGS84 bounding rectangle of the layers from the list of layers ids
   * \since QGIS 3.28.0
   */
  QMap<QString, QgsRectangle> combineCrsExtents( const QStringList &layerIds, const QMap< QString, QgsWmsLayerInfos > &wmsLayerInfos );

} // namespace QgsWms

#endif
