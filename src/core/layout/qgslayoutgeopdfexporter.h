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
     * Contains details of a particular input component to be used during PDF composition.
     * \ingroup core
     * \since QGIS 3.10
     */
    struct CORE_EXPORT ComponentLayerDetail
    {

      //! User-friendly name for the generated PDF layer
      QString name;

      //! Associated map layer ID, or an empty string if this component layer is not associated with a map layer
      QString mapLayerId;

      //! File path to the (already created) PDF to use as the source for this component layer
      QString sourcePdfPath;

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
    bool finalize( const QList< QgsLayoutGeoPdfExporter::ComponentLayerDetail > &components );

    /**
     * Returns the last error message encountered during the export.
     */
    QString errorMessage() { return mErrorMessage; }

    /**
     * Returns a file path to use for temporary files required for GeoPDF creation.
     */
    QString generateTemporaryFilepath( const QString &filename ) const;

  private:

    QgsLayout *mLayout = nullptr;
    QHash< QgsLayoutItemMap *, QgsGeoPdfRenderedFeatureHandler * > mMapHandlers;

    QMap< QString, QgsFeatureList > mCollatedFeatures;

    struct VectorComponentDetail
    {
      //! User-friendly name for the generated PDF layer
      QString name;

      //! Associated map layer ID
      QString mapLayerId;

      //! Field name for display
      QString displayAttribute;

      //! File path to the (already created) vector dataset to use as the source for this component layer
      QString sourceVectorPath;

      //! Layer name in vector dataset to use as the source
      QString sourceVectorLayer;

    };

    QList< VectorComponentDetail > mVectorComponents;

    QString mErrorMessage;
    QTemporaryDir mTemporaryDir;

    bool saveTemporaryLayers();

    QString createCompositionXml( const QList< QgsLayoutGeoPdfExporter::ComponentLayerDetail > &components );

    friend class TestQgsLayoutGeoPdfExport;
};

#endif //QGSLAYOUTGEOPDFEXPORTER_H



