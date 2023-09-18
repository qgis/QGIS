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

#include "qgslayoutatlas.h"
#include "qgsserversettings.h"
#include "qgswmsparameters.h"
#include "qgswmsrendercontext.h"
#include "qgsfeaturefilter.h"
#include "qgslayertreemodellegendnode.h"
#include "qgseditformconfig.h"
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
class QgsServerInterface;
class QgsAttributeEditorElement;
class QgsEditFormConfig;

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
       * Constructor for QgsRenderer.
       * \param context The rendering context.
       * \since QGIS 3.8
       */
      QgsRenderer( const QgsWmsRenderContext &context );

      /**
       * Destructor for QgsRenderer.
       */
      ~QgsRenderer();

      /**
       * Returns the map legend as an image (or NULLPTR in case of error). The
       * caller takes ownership of the image object.
       * \param model The layer tree model to use for building the legend
       * \returns the legend as an image
       * \since QGIS 3.8
       */
      QImage *getLegendGraphics( QgsLayerTreeModel &model );

      /**
       * Returns the map legend as an image (or NULLPTR in case of error). The
       * caller takes ownership of the image object.
       * \param nodeModel The node model to use for building the legend
       * \returns the legend as an image
       * \since QGIS 3.8
       */
      QImage *getLegendGraphics( QgsLayerTreeModelLegendNode &nodeModel );

      /**
       * Returns the map legend as a JSON object. The caller takes the ownership
       * of the JSON object.
       * \param model The layer tree model to use for building the legend
       * \returns the legend as a JSON object
       * \since QGIS 3.12
       */
      QJsonObject getLegendGraphicsAsJson( QgsLayerTreeModel &model );

      typedef QSet<QString> SymbolSet;
      typedef QHash<QgsVectorLayer *, SymbolSet> HitTest;

      /**
       * Returns the hit test according to the current context.
       * \since QGIS 3.8
       */
      HitTest symbols();

      /**
       * Returns the map as an image (or NULLPTR in case of error). The caller
       * takes ownership of the image object).
       * \since QGIS 3.8
       */
      QImage *getMap();

      /**
       * Returns the map as DXF data
       * \returns the map as DXF data
       * \since QGIS 3.0
      */
      std::unique_ptr<QgsDxfExport> getDxf();

      /**
       * Returns printed page as binary
       * \returns printed page as binary or 0 in case of error
      */
      QByteArray getPrint();

      /**
       * Creates an xml document that describes the result of the getFeatureInfo request.
       * May throw an exception
       */
      QByteArray getFeatureInfo( const QString &version = "1.3.0" );

      /**
       * Configures \a layers for rendering optionally considering the map \a settings
       */
      void configureLayers( QList<QgsMapLayer *> &layers, QgsMapSettings *settings = nullptr );

    private:
      QgsLegendSettings legendSettings();

      // Build and returns highlight layers
      QList<QgsMapLayer *> highlightLayers( QList<QgsWmsParametersHighlightLayer> params );

      // Rendering step for layers
      QPainter *layersRendering( const QgsMapSettings &mapSettings, QImage &image ) const;

      // Rendering step for annotations
      void annotationsRendering( QPainter *painter, const QgsMapSettings &mapSettings ) const;

      // Set layer opacity
      void setLayerOpacity( QgsMapLayer *layer, int opacity ) const;

      // Set layer filter and dimension
      void setLayerFilter( QgsMapLayer *layer, const QList<QgsWmsParametersFilter> &filters );

      QStringList dimensionFilter( QgsVectorLayer *layer ) const;

      // Set layer python filter
      void setLayerAccessControlFilter( QgsMapLayer *layer ) const;

      // Set layer selection
      void setLayerSelection( QgsMapLayer *layer, const QStringList &fids ) const;

      // Combine map extent with layer extent
      void updateExtent( const QgsMapLayer *layer, QgsMapSettings &mapSettings ) const;

      // Scale image with WIDTH/HEIGHT if necessary
      QImage *scaleImage( const QImage *image ) const;

      /**
       * Creates a QImage.
       * \param size image size
       * \returns a non null pointer
       * may throw an exception
       */
      QImage *createImage( const QSize &size ) const;

      /**
       * Configures map settings according to WMS parameters.
       * \param paintDevice The device that is used for painting (for dpi)
       * \param mapSettings Map settings to use for rendering
       * \param mandatoryCrsParam does the CRS parameter has to be considered mandatory
       * may throw an exception
       */
      void configureMapSettings( const QPaintDevice *paintDevice, QgsMapSettings &mapSettings, bool mandatoryCrsParam = true );

      /**
       * Configures QgsRenderContext according to the WMS parameters and default settings as well as the passed painter.
       * Used, for example, when no mapSettings are available.
       * \param painter to create the context from
       * \returns the renderer context with default parameters and settings of the passed painter
       */
      QgsRenderContext configureDefaultRenderContext( QPainter *painter = nullptr );

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

      /**
       * Recursively called to write tab layout groups to XML
       * \param group the tab layout group
       * \param layer The vector layer
       * \param fields attribute fields
       * \param featureAttributes the feature attributes
       * \param doc Feature info XML document
       * \param featureElem the feature XML element
       * \param renderContext Context to use for feature rendering
       * \param attributes attributes for access control
       */
      void writeAttributesTabGroup( const QgsAttributeEditorElement *group, QgsVectorLayer *layer, const QgsFields &fields, QgsAttributes &featureAttributes, QDomDocument &doc, QDomElement &featureElem, QgsRenderContext &renderContext, QStringList *attributes = nullptr ) const;

      /**
       * Writes attributes to XML document using the group/attribute layout defined in the tab layout
       * \param config editor config object
       * \param layer The vector layer
       * \param fields attribute fields
       * \param featureAttributes the feature attributes
       * \param doc Feature info XML document
       * \param featureElem the feature XML element
       * \param renderContext Context to use for feature rendering
       * \param attributes attributes for access control
       */
      void writeAttributesTabLayout( QgsEditFormConfig &config, QgsVectorLayer *layer, const QgsFields &fields, QgsAttributes &featureAttributes, QDomDocument &doc, QDomElement &featureElem, QgsRenderContext &renderContext, QStringList *attributes = nullptr ) const;

      /**
       * Writes a vectorlayer attribute into the XML document
       * \param attributeIndex of attribute to be written
       * \param layer The vector layer
       * \param fields attribute fields
       * \param featureAttributes the feature attributes
       * \param doc Feature info XML document
       * \param featureElem the feature XML element
       * \param renderContext Context to use for feature rendering
       * \param attributes attributes for access control
       */
      void writeVectorLayerAttribute( int attributeIndex, QgsVectorLayer *layer, const QgsFields &fields, QgsAttributes &featureAttributes, QDomDocument &doc, QDomElement &featureElem, QgsRenderContext &renderContext, QStringList *attributes = nullptr ) const;

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
       * \returns true in case of success, false if string seems unsafe
      */
      bool testFilterStringSafety( const QString &filter ) const;
      //! Helper function for filter safety test. Groups stringlist to merge entries starting/ending with quotes
      static void groupStringList( QStringList &list, const QString &groupString );

      //! Converts a feature info xml document to SIA2045 norm
      void convertFeatureInfoToSia2045( QDomDocument &doc ) const;

      //! Converts a feature info xml document to HTML
      QByteArray convertFeatureInfoToHtml( const QDomDocument &doc ) const;

      //! Converts a feature info xml document to Text
      QByteArray convertFeatureInfoToText( const QDomDocument &doc ) const;

      //! Converts a feature info xml document to json
      QByteArray convertFeatureInfoToJson( const QList<QgsMapLayer *> &layers, const QDomDocument &doc ) const;

      QDomElement createFeatureGML(
        const QgsFeature *feat,
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

      /**
       * Configures the print layout for the GetPrint request
       *\param c the print layout
       *\param mapSettings the map settings
       *\param atlas atlas used for printing, maybe NULL
       *\returns true in case of success
       */
      bool configurePrintLayout( QgsPrintLayout *c, const QgsMapSettings &mapSettings, QgsLayoutAtlas *atlas );

      void removeTemporaryLayers();

      void handlePrintErrors( const QgsLayout *layout ) const;

      void setLayerStyle( QgsMapLayer *layer, const QString &style ) const;

      void setLayerSld( QgsMapLayer *layer, const QDomElement &sld ) const;

      QgsWmsParameters mWmsParameters;

      QgsFeatureFilter mFeatureFilter;

      const QgsProject *mProject = nullptr;
      QList<QgsMapLayer *> mTemporaryLayers;
      const QgsWmsRenderContext &mContext;

      //! True when temporal capabilities are activated and TIME was parsed successfully
      bool mIsTemporal = false;
  };

} // namespace QgsWms

#endif
