/***************************************************************************
  qgsvectortilelabeling.h
  --------------------------------------
  Date                 : April 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORTILELABELING_H
#define QGSVECTORTILELABELING_H

#include "qgis_core.h"

#include "qgsvectorlayerlabelprovider.h"

class QgsVectorTileLayer;
class QgsVectorTileRendererData;

#ifndef SIP_RUN

/**
 * \ingroup core
 * Internal base class for implementation of label providers for vector tile labeling.
 * \since QGIS 3.14
 */
class QgsVectorTileLabelProvider : public QgsVectorLayerLabelProvider
{
  public:
    //! Constructs base label provider class for the given vector tile layer
    explicit QgsVectorTileLabelProvider( QgsVectorTileLayer *layer );

    //! Returns field names for each sub-layer that are required for labeling
    virtual QMap<QString, QSet<QString> > usedAttributes( const QgsRenderContext &context, int tileZoom ) const = 0;

    //! Sets fields for each sub-layer
    virtual void setFields( const QMap<QString, QgsFields> &perLayerFields ) = 0;

    //! Registers label features for given tile to the labeling engine
    virtual void registerTileFeatures( const QgsVectorTileRendererData &tile, QgsRenderContext &context ) = 0;
};

#endif

/**
 * \ingroup core
 * Base class for labeling configuration classes for vector tile layers.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsVectorTileLabeling
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE

    const QString type = sipCpp->type();

    if ( type == QStringLiteral( "basic" ) )
      sipType = sipType_QgsVectorTileBasicLabeling;
    else
      sipType = 0;
    SIP_END
#endif

  public:
    virtual ~QgsVectorTileLabeling() = default;

    //! Unique type string of the labeling configuration implementation
    virtual QString type() const = 0;

    //! Returns a new copy of the object
    virtual QgsVectorTileLabeling *clone() const = 0 SIP_FACTORY;

    /**
     * Factory for label provider implementation
     * \note not available in Python bindings
     */
    virtual QgsVectorTileLabelProvider *provider( QgsVectorTileLayer *layer ) const SIP_SKIP { Q_UNUSED( layer ) return nullptr; }

    //! Writes labeling properties to given XML element
    virtual void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const = 0;
    //! Reads labeling properties from given XML element
    virtual void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) = 0;
    //! Resolves references to other objects - second phase of loading - after readXml()
    virtual void resolveReferences( const QgsProject &project ) { Q_UNUSED( project ) }

};

#endif // QGSVECTORTILELABELING_H
