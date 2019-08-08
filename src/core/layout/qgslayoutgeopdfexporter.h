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
#include <QTemporaryDir>

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
     * Returns a dict of rendered features, with layer IDs as dict keys for the specified \a map item.
     */
    QMap< QString, QVector< QgsLayoutGeoPdfExporter::RenderedFeature > > renderedFeatures( QgsLayoutItemMap *map ) const;

    /**
     * To be called after the rendering operation is complete.
     *
     * Returns TRUE if the operation was successful, or FALSE if an error occurred. If an error occurred, it
     * can be retrieved by calling errorMessage().
     */
    bool finalize();

    /**
     * Returns the last error message encountered during the export.
     */
    QString errorMessage() { return mErrorMessage; }

  private:

    QgsLayout *mLayout = nullptr;
    QHash< QgsLayoutItemMap *, QgsGeoPdfRenderedFeatureHandler * > mMapHandlers;

    QMap< QString, QgsFeatureList > mCollatedFeatures;
    QMap< QString, QString> mTemporaryFilePaths;

    QString mErrorMessage;
    QTemporaryDir mTemporaryDir;

    bool saveTemporaryLayers();

    QString createCompositionXml();

    friend class TestQgsLayoutGeoPdfExport;
};

#endif //QGSLAYOUTGEOPDFEXPORTER_H



