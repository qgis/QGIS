/***************************************************************************
                         qgsdataproviderelevationproperties.h
                         ---------------
    begin                : May 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSDATAPROVIDERELEVATIONPROPERTIES_H
#define QGSDATAPROVIDERELEVATIONPROPERTIES_H

#include "qgis_core.h"
#include "qgis_sip.h"

/**
 * \class QgsDataProviderElevationProperties
 * \ingroup core
 * \brief Base class for handling elevation related properties for a data provider.
 *
 * \since QGIS 3.32
 */
class CORE_EXPORT QgsDataProviderElevationProperties
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast<QgsRasterDataProviderElevationProperties *>( sipCpp ) )
    {
      sipType = sipType_QgsRasterDataProviderElevationProperties;
    }
    else
    {
      sipType = 0;
    }
    SIP_END
#endif

  public:

    /**
     * Constructor for QgsDataProviderElevationProperties.
     */
    QgsDataProviderElevationProperties();

    /**
     * Returns TRUE if the data provider definitely contains elevation related data.
     *
     * \note Even if this method returns FALSE, the data may still relate to elevation values. TRUE will only
     * be returned in situations where elevation data is definitively present.
     *
     * \see setContainsElevationData()
     * \since QGIS 3.36
     */
    virtual bool containsElevationData() const;

    /**
     * Sets whether the data provider definitely contains elevation related data.
     *
     * \see containsElevationData()
     * \since QGIS 3.36
     */
    virtual void setContainsElevationData( bool contains );

    virtual ~QgsDataProviderElevationProperties();

  protected:
    bool mContainsElevationData = false;

};

#endif // QGSDATAPROVIDERELEVATIONPROPERTIES_H
