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
 * QgsMapLayerTemporalProperties exposes user-configurable settings for controlling
 * how an individual QgsMapLayer behaves in a temporal context, e.g. while animating a map object.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsMapLayerTemporalProperties : public QgsTemporalProperty
{
    Q_OBJECT

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
     * Source of the temporal range of these properties.
     */
    enum TemporalSource
    {
      Layer = 0, //! Defined from layer .
      Project = 1//! Defined from project time settings;
    };

    /**
     * Returns the temporal properties temporal range source, can be layer or project.
     *
     *\see setTemporalSource()
    **/
    TemporalSource temporalSource() const;

    /**
     * Sets the temporal properties temporal range \a source.
     *
     *\see temporalSource()
    **/
    void setTemporalSource( TemporalSource source );

  private:

    //! Source of the properties temporal range
    TemporalSource mSource = Layer;

};

#endif // QGSMAPLAYERTEMPORALPROPERTIES_H
