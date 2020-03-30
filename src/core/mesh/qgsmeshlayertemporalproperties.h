/***************************************************************************
                         qgsmeshlayertemporalproperties.h
                         -----------------------
    begin                : March 2020
    copyright            : (C) 2020 by Vincent
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHLAYERTEMPORALPROPERTIES_H
#define QGSMESHLAYERTEMPORALPROPERTIES_H

#include "qgsmaplayertemporalproperties.h"


/**
 * \class QgsMeshLayerTemporalProperties
 * \ingroup core
 * Implementation of map layer temporal properties for mesh layers.
 *
 *
 * The time in a mesh layer is defined by :
 * - a reference time provided by the data, the project or the user
 * - each dataset is associated with a relative times
 * - time extent is defined by the first time and the last time of all dataset
 *
 * Reference time :          AT
 * Dataset 1 time            o-----RT------RT-----RT-----------RT
 * Dataset 2 time            o---------RT------RT--------RT
 * Dataset 3 time            o------------------------------RT-------RT----------RT
 * Time extent of layer      o-----<--------------------------------------------->
 *
 * AT : absolute time (QDateTime)
 * RT : relative time (qint64)
 *
 * \since QGIS 3.14
 */

class CORE_EXPORT QgsMeshLayerTemporalProperties : public QgsMapLayerTemporalProperties
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsMeshLayerTemporalProperties
     *
     * \param parent pointer to the parent object
     * \param enabled argument specifies whether the temporal properties are initially enabled or not (see isActive()).
     */
    QgsMeshLayerTemporalProperties( QObject *parent SIP_TRANSFERTHIS = nullptr, bool enabled = true );

  public:
    QDomElement writeXml( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context ) override;
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    void setDefaultsFromDataProviderTemporalCapabilities( const QgsDataProviderTemporalCapabilities *capabilities ) override;

    /**
     * Returns the time extent
     */
    QgsDateTimeRange timeExtent() const;

    /**
     * Returns the reference time
     */
    QDateTime referenceTime() const;

    /**
     * Sets the reference time and update the time extent from the temporal capabilities
     *
     * \param timeExtent the time time extent
     * \param capabilities the temporal capabilities of the data provider
     */
    void setReferenceTime( const QDateTime &referenceTime, const QgsDataProviderTemporalCapabilities *capabilities );

  private:
    QDateTime mReferenceTime;
    QgsDateTimeRange mTimeExtent;
};

#endif // QGSMESHLAYERTEMPORALPROPERTIES_H
