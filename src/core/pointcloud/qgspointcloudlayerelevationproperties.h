/***************************************************************************
                         qgspointcloudlayerelevationproperties.h
                         ---------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSPOINTCLOUDLAYERELEVATIONPROPERTIES_H
#define QGSPOINTCLOUDLAYERELEVATIONPROPERTIES_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmaplayerelevationproperties.h"

/**
 * \class QgsPointCloudLayerElevationProperties
 * \ingroup core
 * Point cloud layer specific subclass of QgsMapLayerElevationProperties.
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudLayerElevationProperties : public QgsMapLayerElevationProperties
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsPointCloudLayerElevationProperties, with the specified \a parent object.
     */
    QgsPointCloudLayerElevationProperties( QObject *parent SIP_TRANSFERTHIS );

    bool hasElevation() const override;
    QDomElement writeXml( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context ) override;
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    bool isVisibleInZRange( const QgsDoubleRange &range ) const override;
    QgsDoubleRange calculateZRange( QgsMapLayer *layer ) const override;
};

#endif // QGSPOINTCLOUDLAYERELEVATIONPROPERTIES_H
