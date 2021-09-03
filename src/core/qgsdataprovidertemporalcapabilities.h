/***************************************************************************
                         qgsdataprovidertemporalcapabilities.h
                         ---------------
    begin                : February 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSDATAPROVIDERTEMPORALCAPABILITIES_H
#define QGSDATAPROVIDERTEMPORALCAPABILITIES_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgstemporalproperty.h"

/**
 * \class QgsDataProviderTemporalCapabilities
 * \ingroup core
 * \brief Base class for handling properties relating to a data provider's temporal capabilities.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsDataProviderTemporalCapabilities
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast<QgsRasterDataProviderTemporalCapabilities *>( sipCpp ) )
    {
      sipType = sipType_QgsRasterDataProviderTemporalCapabilities;
    }
    else if ( dynamic_cast<QgsVectorDataProviderTemporalCapabilities *>( sipCpp ) )
    {
      sipType = sipType_QgsVectorDataProviderTemporalCapabilities;
    }
    else if ( dynamic_cast < QgsMeshDataProviderTemporalCapabilities * >( sipCpp ) )
    {
      sipType = sipType_QgsMeshDataProviderTemporalCapabilities;
    }
    else
    {
      sipType = 0;
    }
    SIP_END
#endif

  public:

    /**
     * Constructor for QgsDataProviderTemporalCapabilities.
     *
    * The \a available argument specifies whether the data provider has temporal capabilities. Set to
    * TRUE to indicate that the provider has temporal capabilities available for use.
     */
    QgsDataProviderTemporalCapabilities( bool available = false );

    virtual ~QgsDataProviderTemporalCapabilities() = default;

    /**
     * Returns TRUE if the provider has temporal capabilities available.
     *
     * \see setHasTemporalCapabilities()
     */
    bool hasTemporalCapabilities() const { return mHasTemporalCapabilities; }

    /**
     * Sets whether the provider has temporal capabilities \a available.
     *
     * Set \a available to TRUE to indicate that the provider has temporal capabilities available for use.
    *
     * \see hasTemporalCapabilities()
     */
    void setHasTemporalCapabilities( bool available ) { mHasTemporalCapabilities = available; }

  private:

    bool mHasTemporalCapabilities = false;
};

#endif // QGSDATAPROVIDERTEMPORALCAPABILITIES_H
