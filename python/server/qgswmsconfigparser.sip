/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/server/qgswmsconfigparser.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsWmsConfigParser
{
%Docstring
*************************************************************************
                                                                         *
   This program is free software; you can redistribute it and/or modify  *
   it under the terms of the GNU General Public License as published by  *
   the Free Software Foundation; either version 2 of the License, or     *
   (at your option) any later version.                                   *
                                                                         *
**************************************************************************
%End

%TypeHeaderCode
#include "qgswmsconfigparser.h"
%End
  public:

    virtual void layersAndStylesCapabilities( QDomElement &parentElement, QDomDocument &doc, const QString &version, const QString &serviceUrl, bool fullProjectSettings = false ) const = 0;
%Docstring
 Adds layer and style specific capabilities elements to the parent node. This includes the individual layers and styles, their description, native CRS, bounding boxes, etc.
\param fullProjectInformation If true: add extended project information (does not validate against WMS schema)*
%End

    virtual QList<QgsMapLayer *> mapLayerFromStyle( const QString &lName, const QString &styleName, bool useCache = true ) const = 0;
%Docstring
Returns one or possibly several maplayers for a given layer name and style. If no layers/style are found, an empty list is returned
 :rtype: list of QgsMapLayer
%End

    virtual int layersAndStyles( QStringList &layers, QStringList &styles ) const = 0;
%Docstring
Fills a layer and a style list. The two list have the same number of entries and the style and the layer at a position belong together (similar to the HTTP parameters 'Layers' and 'Styles'. Returns 0 in case of success
 :rtype: int
%End

    virtual QDomDocument getStyle( const QString &styleName, const QString &layerName ) const = 0;
%Docstring
Returns the xml fragment of a style
 :rtype: QDomDocument
%End

    virtual QDomDocument getStyles( QStringList &layerList ) const = 0;
%Docstring
Returns the xml fragment of layers styles
 :rtype: QDomDocument
%End

    virtual QDomDocument describeLayer( QStringList &layerList, const QString &wfsHrefString, const QString &wcsHrefString ) const = 0;
%Docstring
Returns the xml fragment of layers styles description
 :rtype: QDomDocument
%End

    virtual QgsUnitTypes::RenderUnit outputUnits() const = 0;
%Docstring
Returns if output are MM or PIXEL
 :rtype: QgsUnitTypes.RenderUnit
%End

    virtual QStringList identifyDisabledLayers() const = 0;
%Docstring
Returns an ID-list of layers which are not queryable (comes from <properties> -> <Identify> -> <disabledLayers in the project file
 :rtype: list of str
%End

    virtual bool featureInfoWithWktGeometry() const = 0;
%Docstring
True if the feature info response should contain the wkt geometry for vector features
 :rtype: bool
%End

    virtual bool segmentizeFeatureInfoWktGeometry() const = 0;
%Docstring
True if the feature info wkt geometry is delivered with segmentized curve types
 :rtype: bool
%End

    virtual QHash<QString, QString> featureInfoLayerAliasMap() const = 0;
%Docstring
Returns map with layer aliases for GetFeatureInfo (or 0 pointer if not supported). Key: layer name, Value: layer alias
 :rtype: QHash<str, QString>
%End

    virtual QString featureInfoDocumentElement( const QString &defaultValue ) const = 0;
%Docstring
 :rtype: str
%End

    virtual QString featureInfoDocumentElementNS() const = 0;
%Docstring
 :rtype: str
%End

    virtual QString featureInfoSchema() const = 0;
%Docstring
 :rtype: str
%End

    virtual bool featureInfoFormatSIA2045() const = 0;
%Docstring
Return feature info in format SIA2045?
 :rtype: bool
%End

    virtual void drawOverlays( QPainter *p, int dpi, int width, int height ) const = 0;
%Docstring
Draw text annotation items from the QGIS projectfile
%End

    virtual void loadLabelSettings() const = 0;
%Docstring
Load PAL engine settings  into global project instance
%End

    virtual QStringList wfsLayerNames() const = 0;
%Docstring
 :rtype: list of str
%End

    virtual void owsGeneralAndResourceList( QDomElement &parentElement, QDomDocument &doc, const QString &strHref ) const = 0;

    virtual double legendBoxSpace() const = 0;
%Docstring
 :rtype: float
%End
    virtual double legendLayerSpace() const = 0;
%Docstring
 :rtype: float
%End
    virtual double legendLayerTitleSpace() const = 0;
%Docstring
 :rtype: float
%End
    virtual double legendSymbolSpace() const = 0;
%Docstring
 :rtype: float
%End
    virtual double legendIconLabelSpace() const = 0;
%Docstring
 :rtype: float
%End
    virtual double legendSymbolWidth() const = 0;
%Docstring
 :rtype: float
%End
    virtual double legendSymbolHeight() const = 0;
%Docstring
 :rtype: float
%End
    virtual QFont legendLayerFont() const = 0;
%Docstring
 :rtype: QFont
%End
    virtual QFont legendItemFont() const = 0;
%Docstring
 :rtype: QFont
%End

    virtual double imageQuality() const = 0;
%Docstring
 :rtype: float
%End

    virtual int wmsPrecision() const = 0;
%Docstring
 :rtype: int
%End

    virtual bool wmsInspireActivated() const = 0;
%Docstring
 :rtype: bool
%End
    virtual void inspireCapabilities( QDomElement &parentElement, QDomDocument &doc ) const = 0;
%Docstring
Adds inspire capabilities to xml document. ParentElem usually is the <Capabilities> element
%End

    QgsComposition *createPrintComposition( const QString &composerTemplate, const QgsMapSettings &mapSettings, const QMap< QString, QString > &parameterMap, QStringList &highlightLayers ) const;
%Docstring
 :rtype: QgsComposition
%End


    virtual void printCapabilities( QDomElement &parentElement, QDomDocument &doc ) const = 0;
%Docstring
Adds print capabilities to xml document. ParentElem usually is the <Capabilities> element
%End

    virtual void setScaleDenominator( double denom ) = 0;
    virtual void addExternalGMLData( const QString &layerName, QDomDocument *gmlDoc ) = 0;


    virtual int nLayers() const = 0;
%Docstring
 :rtype: int
%End

    virtual void serviceCapabilities( QDomElement &parentElement, QDomDocument &doc ) const = 0;

    virtual bool useLayerIds() const = 0;
%Docstring
 :rtype: bool
%End

    virtual bool allowRequestDefinedDatasources() const;
%Docstring
 :rtype: bool
%End

    static QStringList addHighlightLayers( const QMap<QString, QString> &parameterMap, QStringList &layerSet, const QString &parameterPrefix = QString() );
%Docstring
Adds highlight layers to the layer registry and to the layer set. Returns the ids of the newly created layers (for later removal)
 :rtype: list of str
%End
    static void removeHighlightLayers( const QStringList &layerIds );


  private:
    QgsWmsConfigParser();
    virtual ~QgsWmsConfigParser();
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/server/qgswmsconfigparser.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
