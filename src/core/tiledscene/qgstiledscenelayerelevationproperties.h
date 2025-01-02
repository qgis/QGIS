/***************************************************************************
                         qgstiledscenelayerelevationproperties.h
                         ---------------
    begin                : August 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSTILEDSCENELAYERELEVATIONPROPERTIES_H
#define QGSTILEDSCENELAYERELEVATIONPROPERTIES_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmaplayerelevationproperties.h"

/**
 * \class QgsTiledSceneLayerElevationProperties
 * \ingroup core
 * \brief Tiled scene layer specific subclass of QgsMapLayerElevationProperties.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledSceneLayerElevationProperties : public QgsMapLayerElevationProperties
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsTiledSceneLayerElevationProperties, with the specified \a parent object.
     */
    QgsTiledSceneLayerElevationProperties( QObject *parent SIP_TRANSFERTHIS );

    bool hasElevation() const override;
    QDomElement writeXml( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context ) override;
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    QgsTiledSceneLayerElevationProperties *clone() const override SIP_FACTORY;
    QString htmlSummary() const override;
    QgsDoubleRange calculateZRange( QgsMapLayer *layer ) const override;
    QList< double > significantZValues( QgsMapLayer *layer ) const override;

};

#endif // QGSTILEDSCENELAYERELEVATIONPROPERTIES_H
