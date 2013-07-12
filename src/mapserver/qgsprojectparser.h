/***************************************************************************
                              qgsprojectparser.h
                              ------------------
  begin                : May 27, 2010
  copyright            : (C) 2010 by Marco Hugentobler
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

#ifndef QGSPROJECTPARSER_H
#define QGSPROJECTPARSER_H

#include "qgsconfigparser.h"
#include "qgsvectorlayer.h"
#include <QList>
#include <QPair>

class QSvgRenderer;
class QTextDocument;

//Information about relationship between groups and layers
//key: group name (or null strings for single layers without groups)
//value: containter with layer ids contained in the group
typedef QPair< QString, QList<QString> > GroupLayerInfo;

/**QGIS mapserver configuration parser for project files (.qgs)*/
class QgsProjectParser: public QgsConfigParser
{
  public:
    /**Constructor. Takes ownership of xml document*/
    QgsProjectParser( QDomDocument* xmlDoc, const QString& filePath );
    virtual ~QgsProjectParser();

    /**Adds layer and style specific capabilities elements to the parent node. This includes the individual layers and styles, their description, native CRS, bounding boxes, etc.
        @param fullProjectInformation If true: add extended project information (does not validate against WMS schema)*/
    virtual void layersAndStylesCapabilities( QDomElement& parentElement, QDomDocument& doc, const QString& version, bool fullProjectSettings = false ) const;

    virtual void featureTypeList( QDomElement& parentElement, QDomDocument& doc ) const;

    virtual void owsGeneralAndResourceList( QDomElement& parentElement, QDomDocument& doc, const QString& strHref ) const;

    virtual void describeFeatureType( const QString& aTypeName, QDomElement& parentElement, QDomDocument& doc ) const;
    /**Returns one or possibly several maplayers for a given type name. If no layers/style are found, an empty list is returned*/
    virtual QList<QgsMapLayer*> mapLayerFromTypeName( const QString& tName, bool useCache = true ) const;

    int numberOfLayers() const;

    /**Returns one or possibly several maplayers for a given layer name and style. If no layers/style are found, an empty list is returned*/
    virtual QList<QgsMapLayer*> mapLayerFromStyle( const QString& lName, const QString& styleName, bool useCache = true ) const;

    /**Returns maplayers for a layer Id.*/
    virtual QgsMapLayer* mapLayerFromLayerId( const QString& lId ) const;

    /**Fills a layer and a style list. The two list have the same number of entries and the style and the layer at a position belong together (similar to the HTTP parameters 'Layers' and 'Styles'. Returns 0 in case of success*/
    virtual int layersAndStyles( QStringList& layers, QStringList& styles ) const;

    /**Returns the xml fragment of a style*/
    virtual QDomDocument getStyle( const QString& styleName, const QString& layerName ) const;

    /**Returns if output are MM or PIXEL*/
    virtual QgsMapRenderer::OutputUnits outputUnits() const;

    /**Adds an external GML dataset. The class takes ownership and deletes all the documents in the destructor*/
    void addExternalGMLData( const QString& layerName, QDomDocument* gmlDoc );

    /**Returns an ID-list of layers which are not queryable (comes from <properties> -> <Identify> -> <disabledLayers in the project file*/
    virtual QStringList identifyDisabledLayers() const;

    /**Returns an ID-list of layers queryable for WFS service (comes from <properties> -> <WFSLayers> in the project file*/
    virtual QStringList wfsLayers() const;
    virtual QStringList wfstUpdateLayers() const;
    virtual QStringList wfstInsertLayers() const;
    virtual QStringList wfstDeleteLayers() const;

    /**Returns a set of supported epsg codes for the capabilities document. The list comes from the property <WMSEpsgList> in the project file.
       An empty set means that all possible CRS should be advertised (which could result in very long capabilities documents)
       Example:
       <properties>
          ....
          <WMSEpsgList type="QStringList">
            <value>21781</value>
            <value>4326</value>
          </WMSEpsgList>
      </properties>
    */
    virtual QStringList supportedOutputCrsList() const;

    /**True if the feature info response should contain the wkt geometry for vector features*/
    virtual bool featureInfoWithWktGeometry() const;

    /**Returns map rectangle for the project file*/
    QgsRectangle mapRectangle() const;

    /**Returns epsg number of the project crs (or Null in case of error)*/
    QString mapAuthid() const;

    /**Return project title*/
    QString projectTitle() const;

    const QDomDocument* xmlDoc() const { return mXMLDoc; }

    /**Creates a composition from the project file (probably delegated to the fallback parser)*/
    QgsComposition* initComposition( const QString& composerTemplate, QgsMapRenderer* mapRenderer, QList< QgsComposerMap*>& mapList, QList< QgsComposerLabel* >& labelList, QList<const QgsComposerHtml *>& htmlList ) const;

    /**Adds print capabilities to xml document. ParentElem usually is the <Capabilities> element*/
    void printCapabilities( QDomElement& parentElement, QDomDocument& doc ) const;

    /**Reads service metadata from projectfile or falls back to parent class method if not there*/
    void serviceCapabilities( QDomElement& parentElement, QDomDocument& doc ) const;

    QString serviceUrl() const;

    QString wfsServiceUrl() const;

    /**Returns the names of the published wfs layers (not the ids as in wfsLayers() )*/
    QStringList wfsLayerNames() const;

    /**Returns map with layer aliases for GetFeatureInfo (or 0 pointer if not supported). Key: layer name, Value: layer alias*/
    virtual QHash<QString, QString> featureInfoLayerAliasMap() const;

    virtual QString featureInfoDocumentElement( const QString& defaultValue ) const;

    virtual QString featureInfoDocumentElementNS() const;

    virtual QString featureInfoSchema() const;

    /**Return feature info in format SIA2045?*/
    bool featureInfoFormatSIA2045() const;

    /**Draw text annotation items from the QGIS projectfile*/
    void drawOverlays( QPainter* p, int dpi, int width, int height ) const;

  private:

    //forbidden
    QgsProjectParser();

    /**Content of project file*/
    QDomDocument* mXMLDoc;

    /**Absolute project file path (including file name)*/
    QString mProjectPath;

    /**List of project layer (ordered same as in the project file)*/
    QList<QDomElement> mProjectLayerElements;
    /**List of all legend group elements*/
    QList<QDomElement> mLegendGroupElements;
    /**Project layer elements, accessible by layer id*/
    QHash< QString, QDomElement > mProjectLayerElementsById;
    /**Project layer elements, accessible by layer name*/
    QHash< QString, QDomElement > mProjectLayerElementsByName;
    /**Names of layers and groups which should not be published*/
    QSet<QString> mRestrictedLayers;
    /**Watermark text items*/
    QList< QPair< QTextDocument*, QDomElement > > mTextAnnotationItems;
    /**Watermark items (content cached in QgsSVGCache)*/
    QList< QPair< QSvgRenderer*, QDomElement > > mSvgAnnotationElems;

    /**Creates a maplayer object from <maplayer> element. The layer cash owns the maplayer, so don't delete it
    @return the maplayer or 0 in case of error*/
    QgsMapLayer* createLayerFromElement( const QDomElement& elem, bool useCache = true ) const;
    /**Adds layers from a legend group to list (could be embedded or a normal group)*/
    void addLayersFromGroup( const QDomElement& legendGroupElem, QList<QgsMapLayer*>& layerList, bool useCache = true ) const;
    void addLayerFromLegendLayer( const QDomElement& legendLayerElem, QList<QgsMapLayer*>& layerList, bool useCache = true ) const;
    /**Returns the text of the <id> element for a layer element
    @return id or a null string in case of error*/
    QString layerId( const QDomElement& layerElem ) const;
    /**Returns the text of the <layername> element for a layer element
    @return id or a null string in case of error*/
    QString layerName( const QDomElement& layerElem ) const;
    /**Sets legend parameters according to the first <ComposerLegend> element in the project file*/
    void setLegendParametersFromProject();
    /**Returns the groups / toplevel layers and the maplayer ids contained in it*/
    QList< GroupLayerInfo > groupLayerRelationshipFromProject() const;
    /**Returns the layer id under a <legendlayer> tag in the QGIS projectfile*/
    QString layerIdFromLegendLayer( const QDomElement& legendLayer ) const;

    void addLayers( QDomDocument &doc,
                    QDomElement &parentLayer,
                    const QDomElement &legendElem,
                    const QMap<QString, QgsMapLayer *> &layerMap,
                    const QStringList &nonIdentifiableLayers,
                    QString version, //1.1.1 or 1.3.0
                    bool fullProjectSettings = false ) const;

    void addOWSLayers( QDomDocument &doc,
                       QDomElement &parentElem,
                       const QDomElement &legendElem,
                       const QMap<QString, QgsMapLayer *> &layerMap,
                       const QStringList &nonIdentifiableLayers,
                       const QString& strHref,
                       QgsRectangle& combinedBBox,
                       QString strGroup ) const;

    static void addLayerProjectSettings( QDomElement& layerElem, QDomDocument& doc, QgsMapLayer* currentLayer );

    /**@param considerMapExtent Take user-defined map extent instead of data-calculated extent if present in project file*/
    void combineExtentAndCrsOfGroupChildren( QDomElement& groupElement, QDomDocument& doc, bool considerMapExtent = false ) const;

    /**Returns dom element of composer (identified by composer title) or a null element in case of error*/
    QDomElement composerByName( const QString& composerName ) const;

    /**Returns the composer elements published by this WMS. It is possible to hide composers from the WMS*/
    QList<QDomElement> publishedComposerElements() const;

    /**Converts a (possibly relative) path to absolute*/
    QString convertToAbsolutePath( const QString& file ) const;

    /**Returns mapcanvas output CRS from project file*/
    const QgsCoordinateReferenceSystem& projectCRS() const;

    /**Returns bbox of layer in project CRS (or empty rectangle in case of error)*/
    QgsRectangle layerBoundingBoxInProjectCRS( const QDomElement& layerElem, const QDomDocument& doc ) const;

    /**Reads selection color from project and sets it to QgsConfigParser::mSelectionColor*/
    void setSelectionColor();
    /**Reads maxWidth / maxHeight from project and sets it to QgsConfigParser::mMaxWidth / mMaxHeight*/
    void setMaxWidthHeight();
    /**Reads layer drawing order from the legend section of the project file and appends it to the parent elemen (usually the <Capability> element)*/
    void addDrawingOrder( QDomElement& parentElem, QDomDocument& doc ) const;
    /**Adds drawing order info from layer element or group element (recursive)*/
    void addDrawingOrder( QDomElement elem, bool useDrawingOrder, QMap<int, QString>& orderedLayerList ) const;
    /**Returns project layers by id*/
    void projectLayerMap( QMap<QString, QgsMapLayer*>& layerMap ) const;

    static QString editTypeString( QgsVectorLayer::EditType type );

    /**Returns a complete string set with all the restricted layer names (layers/groups that are not to be published)*/
    QSet<QString> restrictedLayers() const;
    /**Adds sublayers of an embedded group to layer set*/
    static void sublayersOfEmbeddedGroup( const QString& projectFilePath, const QString& groupName, QSet<QString>& layerSet );
    /**Returns visible extent from the project file (or an empty rectangle in case of error)*/
    QgsRectangle projectExtent() const;

    void createTextAnnotationItems();
    void createSvgAnnotationItems();

    void cleanupSvgAnnotationItems();
    void cleanupTextAnnotationItems();

    /**Calculates annotation position to provide the same distance to the lower right corner as in the QGIS project file
    @param width output image pixel width
    @param height output image pixel height
    @param itemWidth item width in pixels in the QGIS project (screen pixels)
    @param itemHeight item height in pixels in the QGIS project (screen pixels)
    @param xPos out: x-coordinate of the item in the output image
    @param yPos out: y-coordinate of the item in the output image*/
    static bool annotationPosition( const QDomElement& elem, double scaleFactor, double& xPos, double& yPos );

    /**Draws background rectangle and frame for an annotation
    @param elem <Annotation> xml element
    @param scaleFactor dpi related scale factor
    @param xPos x-position of the item
    @param yPos y-position of the item
    @param itemWidth item width in pixels in the QGIS project (screen pixels)
    @param itemHeight item height in pixels in the QGIS project (screen pixels)*/
    static void drawAnnotationRectangle( QPainter* p, const QDomElement& elem, double scaleFactor, double xPos, double yPos, int itemWidth, int itemHeight );

    void addDrawingOrderEmbeddedGroup( const QDomElement& groupElem, QMap<int, QString>& orderedLayerList, bool useDrawingOrder ) const;

    /**Reads service metadata from projectfile or falls back to parent class method if not there
     * This is for WFS Services
     **/
    void serviceWFSCapabilities( QDomElement& parentElement, QDomDocument& doc ) const;
};

#endif // QGSPROJECTPARSER_H
