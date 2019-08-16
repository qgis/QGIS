/***************************************************************************
                              qgsabstractgeopdfexporter.h
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
#ifndef QGSABSTRACTGEOPDFEXPORTER_H
#define QGSABSTRACTGEOPDFEXPORTER_H

#include "qgis_core.h"
#include <QList>
#include <QTemporaryDir>
#include <QMutex>
#include "qgsfeature.h"

#define SIP_NO_FILE


class QgsGeoPdfRenderedFeatureHandler;

/**
 * \class QgsAbstractGeoPdfExporter
 * \ingroup core
 *
 * Abstract base class for GeoPDF exporters.
 *
 * The base class handles generic GeoPDF export setup, cleanup and processing steps.
 *
 * This class is a low level implementation detail only. Generally, you should use the high level interface exposed by
 * classes like QgsLayoutExporter instead.
 *
 * \warning QgsAbstractGeoPdfExporter is designed to be a short lived object. It should be created for a
 * single export operation only, and then immediately destroyed. Failure to correctly
 * destroy the object after exporting a PDF will leave the application in an inconsistent, unstable state.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsAbstractGeoPdfExporter
{
  public:

    /**
     * Constructor for QgsAbstractGeoPdfExporter.
     */
    QgsAbstractGeoPdfExporter() = default;


    virtual ~QgsAbstractGeoPdfExporter() = default;

    /**
     * Contains information about a feature rendered inside the PDF.
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
       * Bounds, in PDF units, of rendered feature.
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
     * Called multiple times during the rendering operation, whenever a \a feature associated with the specified
     * \a layerId is rendered.
     */
    void pushRenderedFeature( const QString &layerId, const QgsAbstractGeoPdfExporter::RenderedFeature &feature );

    /**
     * To be called after the rendering operation is complete.
     *
     * Will export the list of PDF layer \a components to a new PDF file at \a destinationFile. Any features
     * previously collected by calls to pushRenderedFeature() will be included automatically in the GeoPDF
     * export.
     *
     * Returns TRUE if the operation was successful, or FALSE if an error occurred. If an error occurred, it
     * can be retrieved by calling errorMessage().
     */
    bool finalize( const QList< QgsAbstractGeoPdfExporter::ComponentLayerDetail > &components, const QString &destinationFile );

    /**
     * Returns the last error message encountered during the export.
     */
    QString errorMessage() { return mErrorMessage; }

    /**
     * Returns a file path to use for temporary files required for GeoPDF creation.
     */
    QString generateTemporaryFilepath( const QString &filename ) const;

  protected:

    /**
     * Contains information relating to a single PDF layer in the GeoPDF export.
     */
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

  private:

    QMutex mMutex;
    QMap< QString, QgsFeatureList > mCollatedFeatures;



    virtual VectorComponentDetail componentDetailForLayerId( const QString &layerId ) = 0;

    QList< VectorComponentDetail > mVectorComponents;

    QString mErrorMessage;
    QTemporaryDir mTemporaryDir;

    bool saveTemporaryLayers();

    QString createCompositionXml( const QList< QgsAbstractGeoPdfExporter::ComponentLayerDetail > &components );

    friend class TestQgsLayoutGeoPdfExport;
};

#endif //QGSABSTRACTGEOPDFEXPORTER_H



