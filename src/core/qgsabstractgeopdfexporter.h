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
#include <QDateTime>
#include <QPainter>

#include "qgsfeature.h"
#include "qgsabstractmetadatabase.h"
#include "qgspolygon.h"
#include "qgscoordinatereferencesystem.h"

#define SIP_NO_FILE


class QgsGeoPdfRenderedFeatureHandler;

/**
 * \class QgsAbstractGeoPdfExporter
 * \ingroup core
 *
 * \brief Abstract base class for GeoPDF exporters.
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
     * Returns TRUE if the current QGIS build is capable of GeoPDF support.
     *
     * If FALSE is returned, a user-friendly explanation why can be retrieved via
     * geoPDFAvailabilityExplanation().
     */
    static bool geoPDFCreationAvailable();

    /**
     * Returns a user-friendly, translated string explaining why GeoPDF export
     * support is not available on the current QGIS build (or an empty string if
     * GeoPDF support IS available).
     * \see geoPDFCreationAvailable()
     */
    static QString geoPDFAvailabilityExplanation();

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
       * Bounds, in PDF units, of rendered feature. (Multi)LineString or Polygon types only.
       */
      QgsGeometry renderedBounds;
    };

    /**
     * \brief Contains details of a particular input component to be used during PDF composition.
     * \ingroup core
     * \since QGIS 3.10
     */
    struct CORE_EXPORT ComponentLayerDetail
    {

      //! User-friendly name for the generated PDF layer
      QString name;

      //! Associated map layer ID, or an empty string if this component layer is not associated with a map layer
      QString mapLayerId;

      //! Optional group name, for arranging layers in top-level groups
      QString group;

      //! File path to the (already created) PDF to use as the source for this component layer
      QString sourcePdfPath;

      //! Component composition mode
      QPainter::CompositionMode compositionMode = QPainter::CompositionMode_SourceOver;

      //! Component opacity
      double opacity = 1.0;

    };

    /**
     * \brief Contains details of a control point used during georeferencing GeoPDF outputs.
     * \ingroup core
     * \since QGIS 3.10
     */
    struct ControlPoint
    {

      /**
       * Constructor for ControlPoint, at the specified \a pagePoint (in millimeters)
       * and \a geoPoint (in CRS units).
       */
      ControlPoint( const QgsPointXY &pagePoint, const QgsPointXY &geoPoint )
        : pagePoint( pagePoint )
        , geoPoint( geoPoint )
      {}

      //! Coordinate on the page of the control point, in millimeters
      QgsPointXY pagePoint;

      //! Georeferenced coordinate of the control point, in CRS units
      QgsPointXY geoPoint;
    };

    struct GeoReferencedSection
    {

      /**
       * Bounds of the georeferenced section on the page, in millimeters.
       *
       * \note if pageBoundsPolygon is specified then this setting is ignored.
       */
      QgsRectangle pageBoundsMm;

      /**
       * Bounds of the georeferenced section on the page, in millimeters, as a free-form polygon.
       *
       * If specified, this will be used instead of pageBoundsMm.
       */
      QgsPolygon pageBoundsPolygon;

      //! Coordinate reference system for georeferenced section
      QgsCoordinateReferenceSystem crs;

      //! List of control points corresponding to this georeferenced section
      QList< QgsAbstractGeoPdfExporter::ControlPoint > controlPoints;

    };

    /**
     * Called multiple times during the rendering operation, whenever a \a feature associated with the specified
     * \a layerId is rendered.
     *
     * The optional \a group argument can be used to differentiate features from the same layer exported
     * multiple times as part of different layer groups.
     */
    void pushRenderedFeature( const QString &layerId, const QgsAbstractGeoPdfExporter::RenderedFeature &feature, const QString &group = QString() );

    struct ExportDetails
    {
      //! Page size, in millimeters
      QSizeF pageSizeMm;

      //! Output DPI
      double dpi = 300;

      //! List of georeferenced sections
      QList< QgsAbstractGeoPdfExporter::GeoReferencedSection > georeferencedSections;

      //! Metadata author tag
      QString author;

      //! Metadata producer tag
      QString producer;

      //! Metadata creator tag
      QString creator;

      //! Metadata creation datetime
      QDateTime creationDateTime;

      //! Metadata subject tag
      QString subject;

      //! Metadata title tag
      QString title;

      //! Metadata keyword map
      QgsAbstractMetadataBase::KeywordMap keywords;

      /**
       * TRUE if ISO3200 extension format georeferencing should be used.
       *
       * This is a recommended setting which results in Geospatial PDF files compatible
       * with the built-in Acrobat geospatial tools.
       */
      bool useIso32000ExtensionFormatGeoreferencing = true;

      /**
       * TRUE if OGC "best practice" format georeferencing should be used.
       *
       * \warning This results in GeoPDF files compatible with the TerraGo suite of tools, but
       * can break compatibility with the built-in Acrobat geospatial tools (yes, GeoPDF
       * format is a mess!).
      */
      bool useOgcBestPracticeFormatGeoreferencing = false;

      /**
       * TRUE if feature vector information (such as attributes) should be exported.
       */
      bool includeFeatures = true;

      /**
       * Optional map of map layer ID to custom logical layer tree group in created PDF file.
       *
       * E.g. if the map contains "layer1": "Environment", "layer2": "Environment", "layer3": "Transport"
       * then the created PDF file will have entries in its layer tree for "Environment" and "Transport",
       * and toggling "Environment" will toggle BOTH layer1 and layer2.
       *
       * Layers which are not included in this group will always have their own individual layer tree entry
       * created for them automatically.
       */
      QMap< QString, QString > customLayerTreeGroups;

      /**
       * Optional map of map layer ID to custom layer tree name to show in the created PDF file.
       *
       * \since QGIS 3.14
       */
      QMap< QString, QString > layerIdToPdfLayerTreeNameMap;

      /**
       * Optional map of map layer ID to initial visibility state. If a layer ID is not present in this,
       * it will default to being initially visible when opening the PDF.
       *
       * \since QGIS 3.14
       */
      QMap< QString, bool > initialLayerVisibility;

      /**
       * Optional list of layer IDs, in the order desired to appear in the generated GeoPDF file.
       *
       * Layers appearing earlier in the list will show earlier in the GeoPDF layer tree list.
       *
       * \since QGIS 3.14
       */
      QStringList layerOrder;

    };

    /**
     * To be called after the rendering operation is complete.
     *
     * Will export the list of PDF layer \a components to a new PDF file at \a destinationFile. The \a components
     * argument gives a list of additional layers to include in the generated PDF file. These must have already
     * been created, e.g. as a result of rendering layers to separate PDF source files.
     *
     * Any features previously collected by calls to pushRenderedFeature() will be included automatically in the GeoPDF
     * export.
     *
     * Returns TRUE if the operation was successful, or FALSE if an error occurred. If an error occurred, it
     * can be retrieved by calling errorMessage().
     */
    bool finalize( const QList< QgsAbstractGeoPdfExporter::ComponentLayerDetail > &components, const QString &destinationFile, const ExportDetails &details );

    /**
     * Returns the last error message encountered during the export.
     */
    QString errorMessage() const { return mErrorMessage; }

    /**
     * Returns a file path to use for temporary files required for GeoPDF creation.
     */
    QString generateTemporaryFilepath( const QString &filename ) const;

    /**
     * Returns TRUE if the specified composition \a mode is supported for layers
     * during GeoPDF exports.
     *
     * \since QGIS 3.14
     */
    static bool compositionModeSupported( QPainter::CompositionMode mode );

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

      //! Optional layer group name
      QString group;

      //! Field name for display
      QString displayAttribute;

      //! File path to the (already created) vector dataset to use as the source for this component layer
      QString sourceVectorPath;

      //! Layer name in vector dataset to use as the source
      QString sourceVectorLayer;

    };

  private:

    QMutex mMutex;
    QMap< QString, QMap< QString, QgsFeatureList > > mCollatedFeatures;

    /**
     * Returns the PDF output component details for the layer with given \a layerId.
     */
    virtual VectorComponentDetail componentDetailForLayerId( const QString &layerId ) = 0;

    QList< VectorComponentDetail > mVectorComponents;

    QString mErrorMessage;
    QTemporaryDir mTemporaryDir;


    bool saveTemporaryLayers();

    QString createCompositionXml( const QList< QgsAbstractGeoPdfExporter::ComponentLayerDetail > &components, const ExportDetails &details );

    /**
     * Returns the GDAL string representation of the specified QPainter composition \a mode.
     */
    static QString compositionModeToString( QPainter::CompositionMode mode );

    friend class TestQgsLayoutGeoPdfExport;
    friend class TestQgsGeoPdfExport;
};

#endif //QGSABSTRACTGEOPDFEXPORTER_H



