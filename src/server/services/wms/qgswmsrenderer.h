/***************************************************************************
                              qgswmsrendrer.h
                              -------------------
  begin                : May 14, 2006
  copyright            : (C) 2006 by Marco Hugentobler
                         (C) 2017 by David Marteau
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
                         david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWMSRENDERER_H
#define QGSWMSRENDERER_H

#include "qgsserversettings.h"
#include "qgswmsparameters.h"
#include "qgsfeaturefilter.h"
#include <QDomDocument>
#include <QMap>
#include <QString>

class QgsCoordinateReferenceSystem;
class QgsPrintLayout;
class QgsFeature;
class QgsLayout;
class QgsMapLayer;
class QgsMapSettings;
class QgsPointXY;
class QgsRasterLayer;
class QgsRectangle;
class QgsRenderContext;
class QgsVectorLayer;
class QgsAccessControl;
class QgsDxfExport;
class QgsLayerTreeModel;
class QgsLayerTree;

class QImage;
class QPaintDevice;
class QPainter;
class QgsLayerTreeGroup;

namespace QgsWms
{

  /**
   * \ingroup server
   * \class QgsWms::QgsRenderer
   * \brief Map renderer for WMS requests
   * \since QGIS 3.0
   */
  class QgsRenderer
  {
    public:

      /**
       * Constructor. Does _NOT_ take ownership of
          QgsConfigParser and QgsCapabilitiesCache*/
      QgsRenderer( QgsServerInterface *serverIface,
                   const QgsProject *project,
                   const QgsWmsParameters &parameters );

      ~QgsRenderer();

      /**
       * Returns the map legend as an image (or NULLPTR in case of error). The caller takes ownership
      of the image object*/
      QImage *getLegendGraphics();

      typedef QSet<QString> SymbolSet;
      typedef QHash<QgsVectorLayer *, SymbolSet> HitTest;

      /**
       * Returns the map as an image (or NULLPTR in case of error). The caller takes ownership
      of the image object). If an instance to existing hit test structure is passed, instead of rendering
      it will fill the structure with symbols that would be used for rendering */
      QImage *getMap( HitTest *hitTest = nullptr );

      /**
       * Identical to getMap( HitTest* hitTest ) and updates the map settings actually used.
        \since QGIS 3.0 */
      QImage *getMap( QgsMapSettings &mapSettings, HitTest *hitTest = nullptr );

      /**
       * Returns the map as DXF data
       * \param options extracted from the FORMAT_OPTIONS parameter
       * \returns the map as DXF data
       * \since QGIS 3.0
      */
      QgsDxfExport getDxf( const QMap<QString, QString> &options );

      /**
       * Returns printed page as binary
        \returns printed page as binary or 0 in case of error*/
      QByteArray getPrint();

      /**
       * Creates an xml document that describes the result of the getFeatureInfo request.
       * May throw an exception
       */
      QByteArray getFeatureInfo( const QString &version = "1.3.0" );

      //! Returns the image quality to use for getMap request
      int imageQuality() const;

      //! Returns the precision to use for GetFeatureInfo request
      int wmsPrecision() const;

    private:

      // Init the restricted layers with nicknames
      void initRestrictedLayers();

      // Build and returns highlight layers
      QList<QgsMapLayer *> highlightLayers( QList<QgsWmsParametersHighlightLayer> params );

      // Build and returns external layers
      QList<QgsMapLayer *> externalLayers( const QList<QgsWmsParametersExternalLayer> &params );

      // Init a map with nickname for layers' project
      void initNicknameLayers();

      void initLayerGroupsRecursive( const QgsLayerTreeGroup *group, const QString &groupName );

      // Return the nickname of the layer (short name, id or name according to
      // the project configuration)
      QString layerNickname( const QgsMapLayer &layer ) const;

      // Return true if the layer has to be displayed according to he current
      // scale
      bool layerScaleVisibility( const QgsMapLayer &layer, double scaleDenominator ) const;

      // Remove unwanted layers (restricted, not visible, etc)
      void removeUnwantedLayers( QList<QgsMapLayer *> &layers, double scaleDenominator = -1 ) const;

      // Remove non identifiable layers (restricted, not visible, etc)
      void removeNonIdentifiableLayers( QList<QgsMapLayer *> &layers ) const;

      // Rendering step for layers
      QPainter *layersRendering( const QgsMapSettings &mapSettings, QImage &image, HitTest *hitTest = nullptr ) const;

      // Rendering step for annotations
      void annotationsRendering( QPainter *painter ) const;

      // Return a list of layers stylized with LAYERS/STYLES parameters
      QList<QgsMapLayer *> stylizedLayers( const QList<QgsWmsParametersLayer> &params );

      // Return a list of layers stylized with SLD parameter
      QList<QgsMapLayer *> sldStylizedLayers( const QString &sld ) const;

      // Set layer opacity
      void setLayerOpacity( QgsMapLayer *layer, int opacity ) const;

      // Set layer filter
      void setLayerFilter( QgsMapLayer *layer, const QList<QgsWmsParametersFilter> &filters );

      // Set layer python filter
      void setLayerAccessControlFilter( QgsMapLayer *layer ) const;

      // Set layer selection
      void setLayerSelection( QgsMapLayer *layer, const QStringList &fids ) const;

      // Combine map extent with layer extent
      void updateExtent( const QgsMapLayer *layer, QgsMapSettings &mapSettings ) const;

      // Scale image with WIDTH/HEIGHT if necessary
      QImage *scaleImage( const QImage *image ) const;

      // Check layer read permissions
      void checkLayerReadPermissions( QgsMapLayer *layer ) const;

      // Build a layer tree model for legend
      QgsLayerTreeModel *buildLegendTreeModel( const QList<QgsMapLayer *> &layers, double scaleDenominator, QgsLayerTree &rootGroup );

      // Returns default dots per mm
      qreal dotsPerMm() const;

      /**
       * Creates a QImage from the HEIGHT and WIDTH parameters
       * \param width image width (or -1 if width should be taken from WIDTH wms parameter)
       * \param height image height (or -1 if height should be taken from HEIGHT wms parameter)
       * \param useBbox flag to indicate if the BBOX has to be used to adapt aspect ratio
       * \returns a non null pointer
       * may throw an exception
       */
      QImage *createImage( int width = -1, int height = -1, bool useBbox = true ) const;

      /**
       * Configures map settings according to WMS parameters.
       * \param paintDevice The device that is used for painting (for dpi)
       * \param mapSettings Map settings to use for rendering
       * may throw an exception
       */
      void configureMapSettings( const QPaintDevice *paintDevice, QgsMapSettings &mapSettings ) const;

      QDomDocument featureInfoDocument( QList<QgsMapLayer *> &layers, const QgsMapSettings &mapSettings,
                                        const QImage *outputImage, const QString &version ) const;

      /**
       * Appends feature info xml for the layer to the layer element of the
       * feature info dom document.
       * \param layer The vector layer
       * \param infoPoint The point coordinates
       * \param nFeatures The number of features
       * \param infoDocument Feature info document
       * \param layerElement Layer XML element
       * \param mapSettings Map settings with extent, CRS, ...
       * \param renderContext Context to use for feature rendering
       * \param version WMS version
       * \param featureBBox The bounding box of the selected features in output CRS
       * \param filterGeom Geometry for filtering selected features
       * \returns TRUE in case of success
       */
      bool featureInfoFromVectorLayer( QgsVectorLayer *layer,
                                       const QgsPointXY *infoPoint,
                                       int nFeatures,
                                       QDomDocument &infoDocument,
                                       QDomElement &layerElement,
                                       const QgsMapSettings &mapSettings,
                                       QgsRenderContext &renderContext,
                                       const QString &version,
                                       QgsRectangle *featureBBox = nullptr,
                                       QgsGeometry *filterGeom = nullptr ) const;

      //! Appends feature info xml for the layer to the layer element of the dom document
      bool featureInfoFromRasterLayer( QgsRasterLayer *layer,
                                       const QgsMapSettings &mapSettings,
                                       const QgsPointXY *infoPoint,
                                       QDomDocument &infoDocument,
                                       QDomElement &layerElement,
                                       const QString &version ) const;

      //! Record which symbols would be used if the map was in the current configuration of renderer. This is useful for content-based legend
      void runHitTest( const QgsMapSettings &mapSettings, HitTest &hitTest ) const;
      //! Record which symbols within one layer would be rendered with the given renderer context
      void runHitTestLayer( QgsVectorLayer *vl, SymbolSet &usedSymbols, QgsRenderContext &context ) const;

      /**
       * Tests if a filter sql string is allowed (safe)
        \returns true in case of success, false if string seems unsafe*/
      bool testFilterStringSafety( const QString &filter ) const;
      //! Helper function for filter safety test. Groups stringlist to merge entries starting/ending with quotes
      static void groupStringList( QStringList &list, const QString &groupString );

      /**
       * Checks WIDTH/HEIGHT values against MaxWidth and MaxHeight
        \returns true if width/height values are okay*/
      bool checkMaximumWidthHeight() const;

      //! Converts a feature info xml document to SIA2045 norm
      void convertFeatureInfoToSia2045( QDomDocument &doc ) const;

      //! Converts a feature info xml document to HTML
      QByteArray convertFeatureInfoToHtml( const QDomDocument &doc ) const;

      //! Converts a feature info xml document to Text
      QByteArray convertFeatureInfoToText( const QDomDocument &doc ) const;

      //! Converts a feature info xml document to json
      QByteArray convertFeatureInfoToJson( const QList<QgsMapLayer *> &layers, const QDomDocument &doc ) const;

      QDomElement createFeatureGML(
        QgsFeature *feat,
        QgsVectorLayer *layer,
        QDomDocument &doc,
        QgsCoordinateReferenceSystem &crs,
        const QgsMapSettings &mapSettings,
        const QString &typeName,
        bool withGeom,
        int version,
        QStringList *attributes = nullptr ) const;

      //! Replaces attribute value with ValueRelation or ValueRelation if defined. Otherwise returns the original value
      static QString replaceValueMapAndRelation( QgsVectorLayer *vl, int idx, const QVariant &attributeVal );
      //! Gets layer search rectangle (depending on request parameter, layer type, map and layer crs)
      QgsRectangle featureInfoSearchRect( QgsVectorLayer *ml, const QgsMapSettings &ms, const QgsRenderContext &rct, const QgsPointXY &infoPoint ) const;

      /*
       * Configures the print layout for the GetPrint request
       *\param c the print layout
       *\param mapSettings the map settings
       *\param atlasPrint true if atlas is used for printing
       *\returns true in case of success
       * */
      bool configurePrintLayout( QgsPrintLayout *c, const QgsMapSettings &mapSettings, bool atlasPrint = false );

      void removeTemporaryLayers();

      void handlePrintErrors( const QgsLayout *layout ) const;

      const QgsWmsParameters &mWmsParameters;

#ifdef HAVE_SERVER_PYTHON_PLUGINS
      //! The access control helper
      QgsAccessControl *mAccessControl = nullptr;
#endif
      QgsFeatureFilter mFeatureFilter;

      const QgsServerSettings &mSettings;
      const QgsProject *mProject = nullptr;
      QStringList mRestrictedLayers;
      QMap<QString, QgsMapLayer *> mNicknameLayers;
      QMap<QString, QList<QgsMapLayer *> > mLayerGroups;
      QList<QgsMapLayer *> mTemporaryLayers;
  };

} // namespace QgsWms

#endif
