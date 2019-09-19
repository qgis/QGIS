/***************************************************************************
  qgsrenderedfeaturehandlerinterface.h
  --------------------------------------
  Date                 : August 2019
  Copyright            : (C) 2019 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRENDEREDFEATUREHANDLERINTERFACE_H
#define QGSRENDEREDFEATUREHANDLERINTERFACE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QSet>
#include <QString>

class QgsFeature;
class QgsGeometry;
class QgsRenderContext;
class QgsVectorLayer;

/**
 * \ingroup core
 *
 * An interface for classes which provider custom handlers for features rendered
 * as part of a map render job.
 *
 * QgsRenderedFeatureHandlerInterface objects are registered in the QgsMapSettings
 * objects used to construct map render jobs. During the rendering operation,
 * the handleRenderedFeature() method will be called once for every rendered feature,
 * allowing the handler to perform some custom task based on the provided information.
 *
 * They can be used for custom tasks which operate on a set of rendered features,
 * such as creating spatial indexes of the location and rendered symbology bounding box
 * of all features rendered on a map.
 *
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsRenderedFeatureHandlerInterface
{
  public:
    virtual ~QgsRenderedFeatureHandlerInterface() = default;

    struct CORE_EXPORT RenderedFeatureContext
    {

      /**
       * Constructor for RenderedFeatureContext.
       * \param renderContext The render context which was used while rendering feature.
       */
      RenderedFeatureContext( const QgsRenderContext &renderContext )
        : renderContext( renderContext )
      {}

      /**
       * The render context which was used while rendering feature.
       */
      const QgsRenderContext &renderContext;
    };

    /**
     * Called whenever a \a feature is rendered during a map render job.
     *
     * The \a renderedBounds argument specifies the (approximate) bounds of the rendered feature's
     * symbology. E.g. for point geometry features, this will be the bounding box of the marker symbol
     * used to symbolize the point. \a renderedBounds geometries are specified in painter units (not
     * map units).
     *
     * \warning This method may be called from many different threads (for multi-threaded map render operations),
     * and accordingly care must be taken to ensure that handleRenderedFeature() implementations are
     * appropriately thread safe.
     *
     * The \a context argument is used to provide additional context relating to the rendering of a feature.
     */
    virtual void handleRenderedFeature( const QgsFeature &feature, const QgsGeometry &renderedBounds, const QgsRenderedFeatureHandlerInterface::RenderedFeatureContext &context ) = 0;

    /**
     * Returns a list of attributes required by this handler, for the specified \a layer. Attributes not listed in here may
     * not be requested from the provider at rendering time.
     */
    virtual QSet<QString> usedAttributes( QgsVectorLayer *layer, const QgsRenderContext &context ) const { Q_UNUSED( layer ); Q_UNUSED( context ); return QSet< QString >(); }
};

#endif // QGSRENDEREDFEATUREHANDLERINTERFACE_H
