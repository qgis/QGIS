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

#include <QDomElement>

/**
 * \class QgsMapLayerTemporalProperties
 * \ingroup core
 * Base class for storage of map layer temporal properties.
 *
 * QgsMapLayerTemporalProperties expose user-configurable settings for controlling
 * how an individual QgsMapLayer behaves in a temporal context, e.g. while animating a map object.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsMapLayerTemporalProperties : public QgsTemporalProperty
{
  public:

    /**
     * Constructor for QgsMapLayerTemporalProperties.
     *
     * The \a enabled argument specifies whether the temporal properties are initially enabled or not (see isActive()).
     */
    QgsMapLayerTemporalProperties( bool enabled = false );

    virtual ~QgsMapLayerTemporalProperties() = default;

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

};

#endif // QGSMAPLAYERTEMPORALPROPERTIES_H
