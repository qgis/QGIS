/***************************************************************************
                              qgssldparser.h
                 Creates a set of maplayers from an sld
                              -------------------
  begin                : May 07, 2006
  copyright            : (C) 2006 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSLDPARSER
#define QGSSLDPARSER

class QBrush;
class QDomDocument;
class QDomElement;
class QDomNode;
class QPen;
class QgsMapLayer;
class QgsVectorLayer;
class QgsRasterLayer;
class QgsRenderer;

#include "qgsconfigparser.h"
#include "qgsmaprenderer.h"
#include <map>
#include <QList>
#include <QString>

#ifdef DIAGRAMSERVER
#include "qgsdiagramcategory.h"
#include "qgsdiagramrenderer.h"
class QgsVectorOverlay;
class QgsVectorDataProvider;
#endif //DIAGRAMSERVER


/**A class that creates QGIS maplayers and layer specific capabilities output from Styled layer descriptor (SLD). A QgsSLDParser object may have a pointer to a fallback object to model the situation where a user defined SLD refers to layers in the administrator sld*/
class QgsSLDParser: public QgsConfigParser
{
  public:
    /**Constructor takes a dom document as argument. The class takes ownership of the document and deletes it in the destructor
     @param doc SLD document
    @param parameterMap map containing the wms request parameters*/
    QgsSLDParser( QDomDocument* doc );
    virtual ~QgsSLDParser();

    /**Adds layer and style specific capabilities elements to the parent node. This includes the individual layers and styles, their description, native CRS, bounding boxes, etc.*/
    void layersAndStylesCapabilities( QDomElement& parentElement, QDomDocument& doc ) const;

    /**Returns number of layers in configuration*/
    int numberOfLayers() const;

    /**Returns one or possibly several maplayers for a given layer name and style. If no layers/style are found, an empty list is returned*/
    QList<QgsMapLayer*> mapLayerFromStyle( const QString& layerName, const QString& styleName, bool useCache = true ) const;

    /**Fills a layer and a style list. The two list have the same number of entries and the style and the layer at a position belong together (similar to the HTTP parameters 'Layers' and 'Styles'. Returns 0 in case of success*/
    int layersAndStyles( QStringList& layers, QStringList& styles ) const;

    /**Returns the xml fragment of a style*/
    QDomDocument getStyle( const QString& styleName, const QString& layerName ) const;

    virtual void setParameterMap( const QMap<QString, QString>& parameterMap ) { mParameterMap = parameterMap; }

    /**Creates a composition from the project file (delegated to the fallback parser)*/
    QgsComposition* initComposition( const QString& composerTemplate, QgsMapRenderer* mapRenderer, QList< QgsComposerMap*>& mapList, QList< QgsComposerLabel* >& labelList ) const;

    /**Adds print capabilities to xml document. Delegated to fallback parser*/
    void printCapabilities( QDomElement& parentElement, QDomDocument& doc ) const;

    /**True if the feature info response should contain the wkt geometry for vector features*/
    virtual bool featureInfoWithWktGeometry() const;

  private:
    /**Don't use the default constructor*/
    QgsSLDParser();
    /**Creates a Renderer from a UserStyle SLD node. Returns 0 in case of error*/
    QgsRenderer* rendererFromUserStyle( const QDomElement& userStyleElement, QgsVectorLayer* vec ) const;
    /**Searches for a <TextSymbolizer> element and applies the settings to the vector layer
     @return true if settings have been applied, false in case of <TextSymbolizer> element not present or error*/
    bool labelSettingsFromUserStyle( const QDomElement& userStyleElement, QgsVectorLayer* vec ) const;
    /**Searches for a <RasterSymbolizer> element and applies the settings to the raster layer
     @return true if settings have been applied, false in case of error*/
    bool rasterSymbologyFromUserStyle( const QDomElement& userStyleElement, QgsRasterLayer* r ) const;

    /**Returns the <UserLayer> dom node or a null node in case of failure*/
    QDomElement findUserLayerElement( const QString& layerName ) const;
    /**Returns the <UserStyle> node of a given <UserLayer> or a null node in case of failure*/
    QDomElement findUserStyleElement( const QDomElement& userLayerElement, const QString& styleName ) const;
    /**Returns a list of all <NamedLayer> element that match the layer name. Returns an empty list if no such layer*/
    QList<QDomElement> findNamedLayerElements( const QString& layerName ) const;

    /**Creates a vector layer from a <UserLayer> tag.
       @param layerName the WMS layer name. This is only necessary for the fallback SLD parser
       @return 0 in case of error.
       Delegates the work to specific methods for <SendedVDS>, <HostedVDS> or <RemoteOWS>*/
    QgsMapLayer* mapLayerFromUserLayer( const QDomElement& userLayerElem, const QString& layerName, bool allowCaching = true ) const;
    /**Writes a temporary file and creates a vector layer. The file is removed at destruction time*/
    QgsVectorLayer* vectorLayerFromGML( const QDomElement gmlRootElement ) const;
    /**Creates a line layer (including renderer) from contour symboliser
     @return the layer or 0 if no layer could be created*/
    QgsVectorLayer* contourLayerFromRaster( const QDomElement& userStyleElem, QgsRasterLayer* rasterLayer ) const;
    /**Creates a suitable layer name from a URL. */
    QString layerNameFromUri( const QString& uri ) const;
    /**Sets the opacity on layer level if the <Opacity> tag is present*/
    void setOpacityForLayer( const QDomElement& layerElem, QgsMapLayer* layer ) const;
    /**Resets the former symbology of a raster layer. This is important for single band layers (e.g. dems)
     coming from the cash*/
    void clearRasterSymbology( QgsRasterLayer* rl ) const;
    /**Reads attributes "epsg" or "proj" from layer element and sets specified CRS if present*/
    void setCrsForLayer( const QDomElement& layerElem, QgsMapLayer* ml ) const;


#ifdef DIAGRAMSERVER
    /**Parses the user style and adds the vector overlays contained in it(most likely diagrams)
       to the vector layer*/
    int overlaysFromUserStyle( const QDomElement& userStyleElement, QgsVectorLayer* vec ) const;
    /**Creates a diagram overlay from a diagram symbolizer element*/
    QgsVectorOverlay* vectorOverlayFromDiagramSymbolizer( const QDomElement& symbolizerElem, QgsVectorLayer* vec ) const;
    /**Returns a list of diagram items from the contents of a <Categorize> element
       @param attribute name of the scaling attribute
     @return 0 in case of success*/
    int diagramItemsFromCategorize( const QDomElement& categorizeElement, QList<QgsDiagramItem>& items, QString& attribute ) const;
    /**Returns a list of diagram items from the contents of a <Interpolate> element
       @param attribute name of the scaling attribute
    @return 0 in case of success*/
    int diagramItemsFromInterpolate( const QDomElement& interpolateElement, QList<QgsDiagramItem>& items, QString& attribute ) const;
    /**Returns diagram categories from <Category> tags
       @param diagramElement <Diagram> xml element containing the categories
       @param p provider for the vectorlayer (needed to convert attribute names to indices)
       @param categories list that will contain the created elements
       @return 0 in case of success*/
    int symbologyFromCategoryTags( const QDomElement& diagramElement, const QgsVectorDataProvider* p, QList<QgsDiagramCategory>& categories ) const;
    /**Returns the scale multiplication factor from the <Scale> Element*/
    double scaleFactorFromScaleTag( const QDomElement& scaleElem ) const;
#endif //DIAGRAMSERVER

    /**SLD as dom document*/
    QDomDocument* mXMLDoc;

    /**Map containing the WMS parameters of the request*/
    QMap<QString, QString> mParameterMap;

    QString mSLDNamespace;
};

#endif
