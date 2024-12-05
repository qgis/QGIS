/***************************************************************************
                         qgsmaplayerselectionproperties.h
                         ---------------
    begin                : July 2023
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


#ifndef QGSMAPLAYERSELECTIONPROPERTIES_H
#define QGSMAPLAYERSELECTIONPROPERTIES_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsreadwritecontext.h"
#include <QObject>

/**
 * \class QgsMapLayerSelectionProperties
 * \ingroup core
 * \brief Base class for storage of map layer selection properties.
 *
 * QgsMapLayerSelectionProperties exposes settings for controlling
 * how an individual QgsMapLayer behaves with respect to feature selection.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsMapLayerSelectionProperties : public QObject
{
    //SIP_TYPEHEADER_INCLUDE( "qgsvectorlayerselectionproperties.h" );

    Q_OBJECT

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsVectorLayerSelectionProperties *>( sipCpp ) )
    {
      sipType = sipType_QgsVectorLayerSelectionProperties;
    }
    else
    {
      sipType = 0;
    }
    SIP_END
#endif

  public:

    /**
     * Constructor for QgsMapLayerSelectionProperties, with the specified \a parent object.
     */
    QgsMapLayerSelectionProperties( QObject *parent SIP_TRANSFERTHIS );

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
     * Creates a clone of the properties.
     */
    virtual QgsMapLayerSelectionProperties *clone() const = 0 SIP_FACTORY;
};

#endif // QGSMAPLAYERSELECTIONPROPERTIES_H
