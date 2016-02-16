/***************************************************************************
                              qgswcsprojectparser.sip
                              ------------------------
  begin                : May 2, 2015
  copyright            : (C) 2015 by Alessandro Pasotti
  email                : a dot pasotti at itopen dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


class QgsWMSProjectParser : public QgsWMSConfigParser
{
%TypeHeaderCode
#include "qgswmsprojectparser.h"

%End
  public:
    QgsWMSProjectParser( const QString& filePath, const QgsAccessControl* ac );
    virtual ~QgsWMSProjectParser();

    /** Adds layer and style specific capabilities elements to the parent node. This includes the individual layers and styles, their description, native CRS, bounding boxes, etc.
        @param fullProjectInformation If true: add extended project information (does not validate against WMS schema)*/
    void layersAndStylesCapabilities( QDomElement& parentElement, QDomDocument& doc, const QString& version, bool fullProjectSettings = false ) const  /*override*/ ;

    QList<QgsMapLayer*> mapLayerFromStyle( const QString& lName, const QString& styleName, bool useCache = true ) const  /*override*/ ;

    QString serviceUrl() const  /*override*/ ;

    QStringList wfsLayerNames() const  /*override*/ ;

    void owsGeneralAndResourceList( QDomElement& parentElement, QDomDocument& doc, const QString& strHref ) const  /*override*/ ;

    //legend
    double legendBoxSpace() const  /*override*/ ;
    double legendLayerSpace() const  /*override*/ ;
    double legendLayerTitleSpace() const  /*override*/ ;
    double legendSymbolSpace() const  /*override*/ ;
    double legendIconLabelSpace() const  /*override*/ ;
    double legendSymbolWidth() const  /*override*/ ;
    double legendSymbolHeight() const  /*override*/ ;
    const QFont& legendLayerFont() const  /*override*/ ;
    const QFont& legendItemFont() const  /*override*/ ;

    double maxWidth() const  /*override*/ ;
    double maxHeight() const  /*override*/ ;
    double imageQuality() const  /*override*/ ;
    int WMSPrecision() const  /*override*/ ;

    //printing
    //QgsComposition* initComposition( const QString& composerTemplate, QgsMapRenderer* mapRenderer, QList< QgsComposerMap* >& mapList, //QList< QgsComposerLegend* >& legendList, QList< QgsComposerLabel* >& labelList, QList<const QgsComposerHtml *>& htmlFrameList ) const  /*override*/ ;

    void printCapabilities( QDomElement& parentElement, QDomDocument& doc ) const  /*override*/ ;

    //todo: fixme
    void setScaleDenominator( double )  /*override*/;
    void addExternalGMLData( const QString&, QDomDocument* )  /*override*/ ;

    QList< QPair< QString, QgsLayerCoordinateTransform > > layerCoordinateTransforms() const  /*override*/ ;

    /** Fills a layer and a style list. The two list have the same number of entries and the style and the layer at a position belong together (similar to the HTTP parameters 'Layers' and 'Styles'. Returns 0 in case of success*/
    int layersAndStyles( QStringList& layers, QStringList& styles ) const  /*override*/ ;

    /** Returns the xml fragment of a style*/
    QDomDocument getStyle( const QString& styleName, const QString& layerName ) const  /*override*/ ;

    /** Returns the xml fragment of layers styles*/
    QDomDocument getStyles( QStringList& layerList ) const  /*override*/ ;

    /** Returns the xml fragment of layers styles description*/
    QDomDocument describeLayer( QStringList& layerList, const QString& hrefString ) const  /*override*/ ;

    /** Returns if output are MM or PIXEL*/
    QgsMapRenderer::OutputUnits outputUnits() const  /*override*/ ;

    /** True if the feature info response should contain the wkt geometry for vector features*/
    bool featureInfoWithWktGeometry() const  /*override*/ ;

    /** Returns map with layer aliases for GetFeatureInfo (or 0 pointer if not supported). Key: layer name, Value: layer alias*/
    QHash<QString, QString> featureInfoLayerAliasMap() const  /*override*/ ;

    QString featureInfoDocumentElement( const QString& defaultValue ) const  /*override*/ ;

    QString featureInfoDocumentElementNS() const  /*override*/ ;

    QString featureInfoSchema() const  /*override*/ ;

    /** Return feature info in format SIA2045?*/
    bool featureInfoFormatSIA2045() const  /*override*/ ;

    /** Draw text annotation items from the QGIS projectfile*/
    void drawOverlays( QPainter* p, int dpi, int width, int height ) const  /*override*/ ;

    /** Load PAL engine settings from projectfile*/
    void loadLabelSettings( QgsLabelingEngineInterface* lbl ) const  /*override*/ ;

    int nLayers() const  /*override*/ ;

    void serviceCapabilities( QDomElement& parentElement, QDomDocument& doc ) const  /*override*/ ;

    bool useLayerIDs() const  /*override*/ ;

  private:

    /** Returns an ID-list of layers which are not queryable (comes from <properties> -> <Identify> -> <disabledLayers in the project file*/
    virtual QStringList identifyDisabledLayers() const  /*override*/ ;

    /** Reads layer drawing order from the legend section of the project file and appends it to the parent elemen (usually the <Capability> element)*/
    void addDrawingOrder( QDomElement& parentElem, QDomDocument& doc, const QHash<QString, QString> &idNameMap, const QStringList &layerIDList ) const;

    void addLayerStyles( QgsMapLayer* currentLayer, QDomDocument& doc, QDomElement& layerElem, const QString& version ) const;

    void addLayers( QDomDocument &doc,
                    QDomElement &parentLayer,
                    const QDomElement &legendElem,
                    const QMap<QString, QgsMapLayer *> &layerMap,
                    const QStringList &nonIdentifiableLayers,
                    QString version, //1.1.1 or 1.3.0
                    bool fullProjectSettings,
                    QHash<QString, QString> &idNameMap,
                    QStringList &layerIDList ) const;

    void addOWSLayerStyles( QgsMapLayer* currentLayer, QDomDocument& doc, QDomElement& layerElem ) const;

    void addOWSLayers( QDomDocument &doc, QDomElement &parentElem, const QDomElement &legendElem,
                       const QMap<QString, QgsMapLayer *> &layerMap, const QStringList &nonIdentifiableLayers,
                       const QString& strHref, QgsRectangle& combinedBBox, const QString& strGroup ) const;

    /** Adds layers from a legend group to list (could be embedded or a normal group)*/
    //void addLayersFromGroup( const QDomElement& legendGroupElem, QMap< int, QgsMapLayer*>& layers, bool useCache = true ) const;

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

