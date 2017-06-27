/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/server/qgswmsprojectparser.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsWmsProjectParser : QgsWmsConfigParser
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
#include "qgswmsprojectparser.h"
%End
  public:
    QgsWmsProjectParser( const QString &filePath, const QgsAccessControl *accessControl );
    virtual ~QgsWmsProjectParser();

    virtual void layersAndStylesCapabilities( QDomElement &parentElement, QDomDocument &doc, const QString &version, const QString &serviceUrl, bool fullProjectSettings = false ) const;

%Docstring
 Adds layer and style specific capabilities elements to the parent node. This includes the individual layers and styles, their description, native CRS, bounding boxes, etc.
\param fullProjectInformation If true: add extended project information (does not validate against WMS schema)*
%End

    virtual QList<QgsMapLayer *> mapLayerFromStyle( const QString &lName, const QString &styleName, bool useCache = true ) const;


    virtual QStringList wfsLayerNames() const;


    virtual void owsGeneralAndResourceList( QDomElement &parentElement, QDomDocument &doc, const QString &strHref ) const;


    virtual double legendBoxSpace() const;

    virtual double legendLayerSpace() const;

    virtual double legendLayerTitleSpace() const;

    virtual double legendSymbolSpace() const;

    virtual double legendIconLabelSpace() const;

    virtual double legendSymbolWidth() const;

    virtual double legendSymbolHeight() const;

    virtual QFont legendLayerFont() const;

    virtual QFont legendItemFont() const;


    virtual double imageQuality() const;

    virtual int wmsPrecision() const;


    virtual bool wmsInspireActivated() const;

    virtual void inspireCapabilities( QDomElement &parentElement, QDomDocument &doc ) const;


    virtual void printCapabilities( QDomElement &parentElement, QDomDocument &doc ) const;


    virtual void setScaleDenominator( double );
    virtual void addExternalGMLData( const QString &, QDomDocument * );


    virtual int layersAndStyles( QStringList &layers, QStringList &styles ) const;

%Docstring
Fills a layer and a style list. The two list have the same number of entries and the style and the layer at a position belong together (similar to the HTTP parameters 'Layers' and 'Styles'. Returns 0 in case of success
 :rtype: int
%End

    virtual QDomDocument getStyle( const QString &styleName, const QString &layerName ) const;

%Docstring
Returns the xml fragment of a style
 :rtype: QDomDocument
%End

    virtual QDomDocument getStyles( QStringList &layerList ) const;

%Docstring
Returns the xml fragment of layers styles
 :rtype: QDomDocument
%End

    virtual QDomDocument describeLayer( QStringList &layerList, const QString &wfsHrefString, const QString &wcsHrefString ) const;

%Docstring
Returns the xml fragment of layers styles description
 :rtype: QDomDocument
%End

    virtual QgsUnitTypes::RenderUnit outputUnits() const;

%Docstring
Returns if output are MM or PIXEL
 :rtype: QgsUnitTypes.RenderUnit
%End

    virtual bool featureInfoWithWktGeometry() const;

%Docstring
True if the feature info response should contain the wkt geometry for vector features
 :rtype: bool
%End

    virtual bool segmentizeFeatureInfoWktGeometry() const;

%Docstring
True if the feature info wkt geometry is delivered with segmentized curve types
 :rtype: bool
%End

    virtual QHash<QString, QString> featureInfoLayerAliasMap() const;

%Docstring
Returns map with layer aliases for GetFeatureInfo (or 0 pointer if not supported). Key: layer name, Value: layer alias
 :rtype: QHash<str, QString>
%End

    virtual QString featureInfoDocumentElement( const QString &defaultValue ) const;


    virtual QString featureInfoDocumentElementNS() const;


    virtual QString featureInfoSchema() const;


    virtual bool featureInfoFormatSIA2045() const;

%Docstring
Return feature info in format SIA2045?
 :rtype: bool
%End

    virtual void drawOverlays( QPainter *p, int dpi, int width, int height ) const;

%Docstring
Draw text annotation items from the QGIS projectfile
%End

    virtual void loadLabelSettings() const;

%Docstring
Load PAL engine settings  into global project instance
%End

    virtual int nLayers() const;


    virtual void serviceCapabilities( QDomElement &parentElement, QDomDocument &doc ) const;


    virtual bool useLayerIds() const;

    virtual bool allowRequestDefinedDatasources() const;


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/server/qgswmsprojectparser.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
