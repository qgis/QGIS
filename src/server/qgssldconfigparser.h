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
    void layersAndStylesCapabilities( QDomElement& parentElement, QDomDocument& doc, const QString& version, bool fullProjectSettings = false ) const override;

    /**Returns one or possibly several maplayers for a given layer name and style. If no layers/style are found, an empty list is returned*/
    QList<QgsMapLayer*> mapLayerFromStyle( const QString& lName, const QString& styleName, bool useCache = true ) const override;

    /**Fills a layer and a style list. The two list have the same number of entries and the style and the layer at a position belong together (similar to the HTTP parameters 'Layers' and 'Styles'. Returns 0 in case of success*/
    int layersAndStyles( QStringList& layers, QStringList& styles ) const override;

    /**Returns the xml fragment of a style*/
    QDomDocument getStyle( const QString& styleName, const QString& layerName ) const override;

    /**Returns the xml fragment of layers styles*/
    QDomDocument getStyles( QStringList& layerList ) const override;

    /**Returns the xml fragment of layers styles description*/
    QDomDocument describeLayer( QStringList& layerList, const QString& hrefString ) const override;

    /**Returns if output are MM or PIXEL*/
    QgsMapRenderer::OutputUnits outputUnits() const override;

    /**Returns an ID-list of layers which are not queryable (comes from <properties> -> <Identify> -> <disabledLayers in the project file*/
    QStringList identifyDisabledLayers() const override;

    /**True if the feature info response should contain the wkt geometry for vector features*/
    bool featureInfoWithWktGeometry() const override;

    /**Returns map with layer aliases for GetFeatureInfo (or 0 pointer if not supported). Key: layer name, Value: layer alias*/
    QHash<QString, QString> featureInfoLayerAliasMap() const override;

    QString featureInfoDocumentElement( const QString& defaultValue ) const override;

    QString featureInfoDocumentElementNS() const override;

    QString featureInfoSchema() const override;

    /**Return feature info in format SIA2045?*/
    bool featureInfoFormatSIA2045() const override;

    /**Draw text annotation items from the QGIS projectfile*/
    void drawOverlays( QPainter* p, int dpi, int width, int height ) const override;

    /**Load PAL engine settings from projectfile*/
    void loadLabelSettings( QgsLabelingEngineInterface* lbl ) const override;

    QString serviceUrl() const override;

    QStringList wfsLayerNames() const override;

    void owsGeneralAndResourceList( QDomElement& parentElement, QDomDocument& doc, const QString& strHref ) const override;

    //legend
    double legendBoxSpace() const override;
    double legendLayerSpace() const override;
    double legendLayerTitleSpace() const override;
    double legendSymbolSpace() const override;
    double legendIconLabelSpace() const override;
    double legendSymbolWidth() const override;
    double legendSymbolHeight() const override;
    const QFont& legendLayerFont() const override;
    const QFont& legendItemFont() const override;

    double maxWidth() const override;
    double maxHeight() const override;
    double imageQuality() const override;
    int WMSPrecision() const override;

    //printing

    /**Creates a print composition, usually for a GetPrint request. Replaces map and label parameters*/
    QgsComposition* createPrintComposition( const QString& composerTemplate, QgsMapRenderer* mapRenderer, const QMap< QString, QString >& parameterMap ) const;

    /**Creates a composition from the project file (probably delegated to the fallback parser)*/
    QgsComposition* initComposition( const QString& composerTemplate, QgsMapRenderer* mapRenderer, QList< QgsComposerMap*>& mapList, QList< QgsComposerLegend* >& legendList, QList< QgsComposerLabel* >& labelList, QList<const QgsComposerHtml *>& htmlFrameList ) const override;

    /**Adds print capabilities to xml document. ParentElem usually is the <Capabilities> element*/
    void printCapabilities( QDomElement& parentElement, QDomDocument& doc ) const override;

    void setScaleDenominator( double denom ) override;
    void addExternalGMLData( const QString& layerName, QDomDocument* gmlDoc ) override;

    QList< QPair< QString, QgsLayerCoordinateTransform > > layerCoordinateTransforms() const override;

    int nLayers() const override;

    void serviceCapabilities( QDomElement& parentElement, QDomDocument& doc ) const override;

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

    /**Returns the <NamedStyle> node of a given <NamedLayer> or a null node in case of failure*/
    QDomElement findNamedStyleElement( const QDomElement& layerElement, const QString& styleName ) const;

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

    bool useLayerIDs() const override { return false; }
};

#endif // QGSSLDCONFIGPARSER_H
