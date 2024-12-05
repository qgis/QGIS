/***************************************************************************
                         qgsmaplayertemporalproperties.h
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


#ifndef QGSMAPLAYERTEMPORALPROPERTIES_H
#define QGSMAPLAYERTEMPORALPROPERTIES_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgstemporalproperty.h"
#include "qgsreadwritecontext.h"
#include "qgsrange.h"

#include <QDomElement>

class QgsMapLayer;
class QgsDataProviderTemporalCapabilities;

/**
 * \class QgsMapLayerTemporalProperties
 * \ingroup core
 * \brief Base class for storage of map layer temporal properties.
 *
 * QgsMapLayerTemporalProperties exposes user-configurable settings for controlling
 * how an individual QgsMapLayer behaves in a temporal context, e.g. while animating a map object.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsMapLayerTemporalProperties : public QgsTemporalProperty
{
    //SIP_TYPEHEADER_INCLUDE( "qgsrasterlayertemporalproperties.h" );
    //SIP_TYPEHEADER_INCLUDE( "qgsmeshlayertemporalproperties.h" );
    //SIP_TYPEHEADER_INCLUDE( "qgsvectorlayertemporalproperties.h" );

    Q_OBJECT

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsRasterLayerTemporalProperties *>( sipCpp ) )
    {
      sipType = sipType_QgsRasterLayerTemporalProperties;
    }
    else if ( qobject_cast<QgsMeshLayerTemporalProperties *>( sipCpp ) )
    {
      sipType = sipType_QgsMeshLayerTemporalProperties;
    }
    else if ( qobject_cast<QgsVectorLayerTemporalProperties *>( sipCpp ) )
    {
      sipType = sipType_QgsVectorLayerTemporalProperties;
    }
    else
    {
      sipType = 0;
    }
    SIP_END
#endif

  public:

    /**
     * Constructor for QgsMapLayerTemporalProperties, with the specified \a parent object.
     *
     * The \a enabled argument specifies whether the temporal properties are initially enabled or not (see isActive()).
     */
    QgsMapLayerTemporalProperties( QObject *parent SIP_TRANSFERTHIS, bool enabled = false );

    /**
     * Writes the properties to a DOM \a element, to be used later with readXml().
     *
     * \see readXml()
     */
    virtual QDomElement writeXml( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context ) = 0;

    /**
     * Reads temporal properties from a DOM \a element previously written by writeXml().
     *
     * \see writeXml()
     */
    virtual bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) = 0;

    /**
     * Returns TRUE if the layer should be visible and rendered for the specified time \a range.
     */
    virtual bool isVisibleInTemporalRange( const QgsDateTimeRange &range ) const;

    /**
     * Sets the layers temporal settings to appropriate defaults based on
     * a provider's temporal \a capabilities.
     */
    virtual void setDefaultsFromDataProviderTemporalCapabilities( const QgsDataProviderTemporalCapabilities *capabilities ) = 0;

#ifndef SIP_RUN
// sip gets confused with this, refuses to compile

    /**
     * Attempts to calculate the overall temporal extent for the specified \a layer, using
     * the settings defined by the temporal properties object.
     *
     * May return an infinite range if the extent could not be calculated.
     *
     * \note Not available in Python bindings
     */
    virtual QgsDateTimeRange calculateTemporalExtent( QgsMapLayer *layer ) const;
#endif

    /**
     * Attempts to calculate the overall list of all temporal extents which are contained in the specified \a layer, using
     * the settings defined by the temporal properties object.
     *
     * May return an empty list if the ranges could not be calculated.
     *
     * \since QGIS 3.20
     */
    virtual QList< QgsDateTimeRange > allTemporalRanges( QgsMapLayer *layer ) const;

};

#endif // QGSMAPLAYERTEMPORALPROPERTIES_H
