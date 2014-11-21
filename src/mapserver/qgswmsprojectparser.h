/***************************************************************************
                              qgswmsprojectparser.h
                              ---------------------
  begin                : March 25, 2014
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

#ifndef QGSWMSPROJECTPARSER_H
#define QGSWMSPROJECTPARSER_H

#include "qgswmsconfigparser.h"
#include "qgsserverprojectparser.h"
#include "qgslayertreegroup.h"

class QTextDocument;
class QSvgRenderer;

class QgsWMSProjectParser : public QgsWMSConfigParser
{
  public:
    QgsWMSProjectParser( const QString& filePath );
    virtual ~QgsWMSProjectParser();

    /**Adds layer and style specific capabilities elements to the parent node. This includes the individual layers and styles, their description, native CRS, bounding boxes, etc.
        @param fullProjectInformation If true: add extended project information (does not validate against WMS schema)*/
    void layersAndStylesCapabilities( QDomElement& parentElement, QDomDocument& doc, const QString& version, bool fullProjectSettings = false ) const;

    QList<QgsMapLayer*> mapLayerFromStyle( const QString& lName, const QString& styleName, bool useCache = true ) const;

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
    QgsComposition* initComposition( const QString& composerTemplate, QgsMapRenderer* mapRenderer, QList< QgsComposerMap* >& mapList, QList< QgsComposerLegend* >& legendList, QList< QgsComposerLabel* >& labelList, QList<const QgsComposerHtml *>& htmlFrameList ) const;

    void printCapabilities( QDomElement& parentElement, QDomDocument& doc ) const;

    //todo: fixme
    void setScaleDenominator( double ) {}
    void addExternalGMLData( const QString&, QDomDocument* ) {}

    QList< QPair< QString, QgsLayerCoordinateTransform > > layerCoordinateTransforms() const;

    /**Fills a layer and a style list. The two list have the same number of entries and the style and the layer at a position belong together (similar to the HTTP parameters 'Layers' and 'Styles'. Returns 0 in case of success*/
    int layersAndStyles( QStringList& layers, QStringList& styles ) const;

    /**Returns the xml fragment of a style*/
    QDomDocument getStyle( const QString& styleName, const QString& layerName ) const;

    /**Returns the xml fragment of layers styles*/
    QDomDocument getStyles( QStringList& layerList ) const;

    /**Returns if output are MM or PIXEL*/
    QgsMapRenderer::OutputUnits outputUnits() const;

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

    int nLayers() const;

    void serviceCapabilities( QDomElement& parentElement, QDomDocument& doc ) const;

    bool useLayerIDs() const { return mProjectParser->useLayerIDs(); }

  private:
    QgsServerProjectParser* mProjectParser;

    mutable QFont mLegendLayerFont;
    mutable QFont mLegendItemFont;

    /**Watermark text items*/
    QList< QPair< QTextDocument*, QDomElement > > mTextAnnotationItems;
    /**Watermark items (content cached in QgsSVGCache)*/
    QList< QPair< QSvgRenderer*, QDomElement > > mSvgAnnotationElems;

    /**Returns an ID-list of layers which are not queryable (comes from <properties> -> <Identify> -> <disabledLayers in the project file*/
    virtual QStringList identifyDisabledLayers() const;

    /**Reads layer drawing order from the legend section of the project file and appends it to the parent elemen (usually the <Capability> element)*/
    void addDrawingOrder( QDomElement& parentElem, QDomDocument& doc ) const;

    /**Adds drawing order info from layer element or group element (recursive)*/
    void addDrawingOrder( QDomElement groupElem, bool useDrawingOrder, QMap<int, QString>& orderedLayerList ) const;

    /**Adds drawing order info from embedded layer element or embedded group element (recursive)*/
    void addDrawingOrderEmbeddedGroup( QDomElement groupElem, bool useDrawingOrder, QMap<int, QString>& orderedLayerList ) const;

    void addLayers( QDomDocument &doc,
                    QDomElement &parentLayer,
                    const QDomElement &legendElem,
                    const QMap<QString, QgsMapLayer *> &layerMap,
                    const QStringList &nonIdentifiableLayers,
                    QString version, //1.1.1 or 1.3.0
                    bool fullProjectSettings = false ) const;

    void addOWSLayers( QDomDocument &doc, QDomElement &parentElem, const QDomElement &legendElem,
                       const QMap<QString, QgsMapLayer *> &layerMap, const QStringList &nonIdentifiableLayers,
                       const QString& strHref, QgsRectangle& combinedBBox, QString strGroup ) const;

    /**Adds layers from a legend group to list (could be embedded or a normal group)*/
    void addLayersFromGroup( const QDomElement& legendGroupElem, QMap< int, QgsMapLayer*>& layers, bool useCache = true ) const;

    QDomElement composerByName( const QString& composerName ) const;
    QgsLayerTreeGroup* projectLayerTreeGroup() const;

    static bool annotationPosition( const QDomElement& elem, double scaleFactor, double& xPos, double& yPos );
    static void drawAnnotationRectangle( QPainter* p, const QDomElement& elem, double scaleFactor, double xPos, double yPos, int itemWidth, int itemHeight );
    void createTextAnnotationItems();
    void createSvgAnnotationItems();
    void cleanupSvgAnnotationItems();
    void cleanupTextAnnotationItems();

    QString getCapaServiceUrl( QDomDocument& doc ) const;
};

#endif // QGSWMSPROJECTPARSER_H
