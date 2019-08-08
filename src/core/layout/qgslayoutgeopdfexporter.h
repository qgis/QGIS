/***************************************************************************
                              qgslayoutgeopdfexporter.h
                             --------------------------
    begin                : August 2019
    copyright            : (C) 2019 by Nyall Dawson
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
#ifndef QGSLAYOUTGEOPDFEXPORTER_H
#define QGSLAYOUTGEOPDFEXPORTER_H

#include "qgis_core.h"
#include "qgslayoutitemmap.h"
#include <QList>

#define SIP_NO_FILE

class QgsLayout;
class QgsGeoPdfRenderedFeatureHandler;

/**
 * \class QgsLayoutGeoPdfExporter
 * \ingroup core
 *
 * Handles GeoPDF export specific setup, cleanup and processing steps.
 *
 * This class is a low level implementation detail only. Generally, you should use the high level interface exposed by
 * QgsLayoutExporter instead.
 *
 * \warning QgsLayoutGeoPdfExporter is designed to be a short lived object. It should be created for a
 * single layout export operation only, and then immediately destroyed. Failure to correctly
 * destroy the object after exporting a layout will leave the layout in an inconsistent, unstable state.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsLayoutGeoPdfExporter
{
  public:

    QgsLayoutGeoPdfExporter( QgsLayout *layout );


    ~QgsLayoutGeoPdfExporter();

    /**
     * Contains information about a feature rendered inside the layout.
     */
    struct RenderedFeature
    {

      /**
       * Constructor for RenderedFeature.
       */
      RenderedFeature() = default;

      /**
       * Constructor for RenderedFeature.
       */
      RenderedFeature( const QgsFeature &feature, const QgsGeometry &renderedBounds )
        : feature( feature )
        , renderedBounds( renderedBounds )
      {}

      /**
       * Rendered feature.
       */
      QgsFeature feature;

      /**
       * Bounds, in layout units, of rendered feature.
       */
      QgsGeometry renderedBounds;
    };

    /**
     * Returns a map of rendered features, with layer IDs as map keys.
     */
    QMap< QString, QVector< QgsLayoutGeoPdfExporter::RenderedFeature > > renderedFeatures() const;

  private:

    QgsLayout *mLayout = nullptr;
    QList< QgsLayoutItemMap * > mMaps;
    std::unique_ptr< QgsGeoPdfRenderedFeatureHandler > mHandler;

};

#endif //QGSLAYOUTGEOPDFEXPORTER_H



