/***************************************************************************
                              qgssldconfigparser.h
                              --------------------
  begin                : March 28, 2014
  copyright            : (C) 2014 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSLDCONFIGPARSER_H
#define QGSSLDCONFIGPARSER_H

#include "qgswmsconfigparser.h"

class QgsFeatureRendererV2;
class QgsVectorLayer;
class QgsRasterLayer;
class QTemporaryFile;

class QgsSLDConfigParser : public QgsWMSConfigParser
{
  public:
    /**Constructor takes a dom document as argument. The class takes ownership of the document and deletes it in the destructor
    @param doc SLD document
    @param parameterMap map containing the wms request parameters*/
    QgsSLDConfigParser( QDomDocument* doc, const QMap<QString, QString>& parameters );
    virtual ~QgsSLDConfigParser();

    void setFallbackParser( QgsWMSConfigParser* p ) { mFallbackParser = p; }

    /**Adds layer and style specific capabilities elements to the parent node. This includes the individual layers and styles, their description, native CRS, bounding boxes, etc.
        @param fullProjectInformation If true: add extended project information (does not validate against WMS schema)*/
    void layersAndStylesCapabilities( QDomElement& parentElement, QDomDocument& doc, const QString& version, bool fullProjectSettings = false ) const;

    /**Returns one or possibly several maplayers for a given layer name and style. If no layers/style are found, an empty list is returned*/
    QList<QgsMapLayer*> mapLayerFromStyle( const QString& lName, const QString& styleName, bool useCache = true ) const;

    /**Fills a layer and a style list. The two list have the same number of entries and the style and the layer at a position belong together (similar to the HTTP parameters 'Layers' and 'Styles'. Returns 0 in case of success*/
    int layersAndStyles( QStringList& layers, QStringList& styles ) const;

    /**Returns the xml fragment of a style*/
    QDomDocument getStyle( const QString& styleName, const QString& layerName ) const;

    /**Returns the xml fragment of layers styles*/
    QDomDocument getStyles( QStringList& layerList ) const;

    /**Returns if output are MM or PIXEL*/
    QgsMapRenderer::OutputUnits outputUnits() const;

    /**Returns an ID-list of layers which are not queryable (comes from <properties> -> <Identify> -> <disabledLayers in the project file*/
    QStringList identifyDisabledLayers() const;

    /**True if the feature info response should contain the wkt geometry for vector features*/
    bool featureInfoWithWktGeometry() const;

    /**Returns map with layer aliases for GetFeatureInfo (or 0 pointer if not supported). Key: layer name, Value: layer alias*/
    QHash<QString, QString> featureInfoLayerAliasMap() const;

    QString featureInfoDocumentElement( const QString& defaultValue ) const;

    QString featureInfoDocumentElementNS() const;

    QString featureInfoSchema() const;

    /**Return feature info in format SIA2045?*/
    bool featureInfoFormatSIA2045() const;

    /**Draw text annotation items from the QGIS projectfile*/
    void drawOverlays( QPainter* p, int dpi, int width, int height ) const;

    /**Load PAL engine settings from projectfile*/
    void loadLabelSettings( QgsLabelingEngineInterface* lbl ) const;

    QString serviceUrl() const;

    QStringList wfsLayerNames() const;

    void owsGeneralAndResourceList( QDomElement& parentElement, QDomDocument& doc, const QString& strHref ) const;

    //legend
    double legendBoxSpace() const;
    double legendLayerSpace() const;
    double legendLayerTitleSpace() const;
    double legendSymbolSpace() const;
    double legendIconLabelSpace() const;
    double legendSymbolWidth() const;
    double legendSymbolHeight() const;
    const QFont& legendLayerFont() const;
    const QFont& legendItemFont() const;

    double maxWidth() const;
    double maxHeight() const;
    double imageQuality() const;
    int WMSPrecision() const;

    //printing

    /**Creates a print composition, usually for a GetPrint request. Replaces map and label parameters*/
    QgsComposition* createPrintComposition( const QString& composerTemplate, QgsMapRenderer* mapRenderer, const QMap< QString, QString >& parameterMap ) const;

    /**Creates a composition from the project file (probably delegated to the fallback parser)*/
    QgsComposition* initComposition( const QString& composerTemplate, QgsMapRenderer* mapRenderer, QList< QgsComposerMap*>& mapList, QList< QgsComposerLabel* >& labelList, QList<const QgsComposerHtml *>& htmlFrameList ) const;

    /**Adds print capabilities to xml document. ParentElem usually is the <Capabilities> element*/
    void printCapabilities( QDomElement& parentElement, QDomDocument& doc ) const;

    void setScaleDenominator( double denom );
    void addExternalGMLData( const QString& layerName, QDomDocument* gmlDoc );

    QList< QPair< QString, QgsLayerCoordinateTransform > > layerCoordinateTransforms() const;

    int nLayers() const;

    void serviceCapabilities( QDomElement& parentElement, QDomDocument& doc ) const;

  private:

    /**SLD as dom document*/
    QDomDocument* mXMLDoc;

    /**Map containing the WMS parameters of the request*/
    QMap<QString, QString> mParameterMap;

    QString mSLDNamespace;

    /**Output units (pixel or mm)*/
    QgsMapRenderer::OutputUnits mOutputUnits;

    QgsWMSConfigParser* mFallbackParser;

    QFont mLegendLayerFont;

    QFont mLegendItemFont;

    /**Stores pointers to layers that have to be removed after the request*/
    mutable QList<QgsMapLayer*> mLayersToRemove;

    /**Stores the temporary file objects. The class takes ownership of the objects and deletes them in the destructor*/
    mutable QList<QTemporaryFile*> mFilesToRemove;

    /**Stores paths of files that need to be removed after each request (necessary because of contours shapefiles that
      cannot be handles with QTemporaryFile*/
    mutable QList<QString> mFilePathsToRemove;

    //default constructor forbidden
    QgsSLDConfigParser();

    /**Returns a list of all <NamedLayer> element that match the layer name. Returns an empty list if no such layer*/
    QList<QDomElement> findNamedLayerElements( const QString& layerName ) const;

    /**Returns the <UserStyle> node of a given <UserLayer> or a null node in case of failure*/
    QDomElement findUserStyleElement( const QDomElement& userLayerElement, const QString& styleName ) const;

    /**Creates a Renderer from a UserStyle SLD node. Returns 0 in case of error*/
    QgsFeatureRendererV2* rendererFromUserStyle( const QDomElement& userStyleElement, QgsVectorLayer* vec ) const;

    /**Searches for a <TextSymbolizer> element and applies the settings to the vector layer
     @return true if settings have been applied, false in case of <TextSymbolizer> element not present or error*/
    bool labelSettingsFromUserStyle( const QDomElement& userStyleElement, QgsVectorLayer* vec ) const;

    /**Searches for a <RasterSymbolizer> element and applies the settings to the raster layer
     @return true if settings have been applied, false in case of error*/
    bool rasterSymbologyFromUserStyle( const QDomElement& userStyleElement, QgsRasterLayer* r ) const;

    /**Creates a line layer (including renderer) from contour symboliser
     @return the layer or 0 if no layer could be created*/
    QgsVectorLayer* contourLayerFromRaster( const QDomElement& userStyleElem, QgsRasterLayer* rasterLayer ) const;

    /**Returns the <UserLayer> dom node or a null node in case of failure*/
    QDomElement findUserLayerElement( const QString& layerName ) const;

    /**Creates a vector layer from a <UserLayer> tag.
       @param layerName the WMS layer name. This is only necessary for the fallback SLD parser
       @return 0 in case of error.
       Delegates the work to specific methods for <SendedVDS>, <HostedVDS> or <RemoteOWS>*/
    QgsMapLayer* mapLayerFromUserLayer( const QDomElement& userLayerElem, const QString& layerName, bool allowCaching = true ) const;

    /**Reads attributes "epsg" or "proj" from layer element and sets specified CRS if present*/
    void setCrsForLayer( const QDomElement& layerElem, QgsMapLayer* ml ) const;

    bool useLayerIDs() const { return false; }
};

#endif // QGSSLDCONFIGPARSER_H
