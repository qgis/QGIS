/***************************************************************************
                         qgsmeshlayertemporalproperties.h
                         -----------------------
    begin                : March 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
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
#include "qgsmeshdataprovidertemporalcapabilities.h"


/**
 * \class QgsMeshLayerTemporalProperties
 * \ingroup core
 * \brief Implementation of map layer temporal properties for mesh layers.
 *
 *
 * The time in a mesh layer is defined by :
 *
 * - a reference time provided by the data, the project or the user
 * - each dataset is associated with a relative times
 * - time extent is defined by the first time and the last time of all dataset
 *
 * \code{.unparsed}
 * Reference time :          AT
 * Dataset 1 time            o-----RT------RT-----RT-----------RT
 * Dataset 2 time            o---------RT------RT--------RT
 * Dataset 3 time            o------------------------------RT-------RT----------RT
 * Time extent of layer      o-----<--------------------------------------------->
 *
 * AT : absolute time (QDateTime)
 * RT : relative time (qint64)
 *  \endcode
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
    QgsDateTimeRange calculateTemporalExtent( QgsMapLayer *layer ) const override SIP_SKIP;

    /**
     * Returns the time extent
     */
    QgsDateTimeRange timeExtent() const;

    /**
     * Returns the reference time
     */
    QDateTime referenceTime() const;

    /**
     * Sets the reference time and update the time extent from the temporal capabilities,
     * if the temporal capabilities is null, set a void time extent (reference time to reference time)
     *
     * \param referenceTime the reference time
     * \param capabilities the temporal capabilities of the data provider
     */
    void setReferenceTime( const QDateTime &referenceTime, const QgsDataProviderTemporalCapabilities *capabilities );

    /**
     * Returns the method used to match dataset from temporal capabilities
     */
    QgsMeshDataProviderTemporalCapabilities::MatchingTemporalDatasetMethod matchingMethod() const;

    /**
     * Sets the method used to match dataset from temporal capabilities
     *
     * \param matchingMethod the matching method
     */
    void setMatchingMethod( const QgsMeshDataProviderTemporalCapabilities::MatchingTemporalDatasetMethod &matchingMethod );

    /**
     * Returns whether the instance is valid
     *
     * \since QGIS 3.22
     */
    bool isValid() const;

    /**
     * Sets whether the instance is valid
     *
     * \param isValid whether the instance is valid
     *
     * \since QGIS 3.22
     */
    void setIsValid( bool isValid );

    /**
     * Returns whether the time proporties are automatically reloaded from provider when project is opened or layer is reloaded
     *
     * \since QGIS 3.28
     */
    bool alwaysLoadReferenceTimeFromSource() const;

    /**
     * Sets whether the time proporties are automatically reloaded from provider when project is opened or layer is reloaded
     *
     * \param autoReloadFromProvider whether the time proporties is automatically reloaded
     *
     * \since QGIS 3.28
     */
    void setAlwaysLoadReferenceTimeFromSource( bool autoReloadFromProvider );

  private:
    QDateTime mReferenceTime;
    QgsDateTimeRange mTimeExtent;
    QgsMeshDataProviderTemporalCapabilities::MatchingTemporalDatasetMethod mMatchingMethod =
      QgsMeshDataProviderTemporalCapabilities::FindClosestDatasetBeforeStartRangeTime;
    bool mIsValid = false;
    bool mAlwaysLoadReferenceTimeFromSource = false;
};

#endif // QGSMESHLAYERTEMPORALPROPERTIES_H
