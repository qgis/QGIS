/***************************************************************************
                              qgsserverprojectparser.sip
                              ------------------------
  begin                : March 2, 2015
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


class QgsServerProjectParser
{
%TypeHeaderCode
#include "qgsserverprojectparser.h"
%End

  public:
    QgsServerProjectParser( QDomDocument* xmlDoc, const QString& filePath );

    QString projectPath() const;

    const QDomDocument* xmlDocument() const;

    //! Returns project layers by id
    void projectLayerMap( QMap<QString, QgsMapLayer*>& layerMap ) const;

    //! Converts a (possibly relative) path to absolute
    QString convertToAbsolutePath( const QString& file ) const;

    /** Creates a maplayer object from <maplayer> element. The layer cash owns the maplayer, so don't delete it
    @return the maplayer or 0 in case of error*/
    QgsMapLayer* createLayerFromElement( const QDomElement& elem, bool useCache = true ) const;

    QgsMapLayer* mapLayerFromLayerId( const QString& lId, bool useCache = true ) const;

    //! Returns the layer id under a <legendlayer> tag in the QGIS projectfile
    QString layerIdFromLegendLayer( const QDomElement& legendLayer ) const;

    //! @param considerMapExtent Take user-defined map extent instead of data-calculated extent if present in project file
    void combineExtentAndCrsOfGroupChildren( QDomElement& groupElement, QDomDocument& doc, bool considerMapExtent = false ) const;

    void addLayerProjectSettings( QDomElement& layerElem, QDomDocument& doc, QgsMapLayer* currentLayer ) const;

    QgsRectangle layerBoundingBoxInProjectCrs( const QDomElement& layerElem, const QDomDocument &doc ) const;

    bool crsSetForLayer( const QDomElement& layerElement, QSet<QString> &crsSet ) const;

    QgsCoordinateReferenceSystem projectCrs() const;

    QgsRectangle mapRectangle() const;

    QStringList supportedOutputCrsList() const;

    QList<QDomElement> projectLayerElements() const;

    QList<QDomElement> legendGroupElements() const;

    QString projectTitle() const;

    QDomElement legendElem() const;

    QDomElement propertiesElem() const;

    QSet<QString> restrictedLayers() const;
    bool useLayerIds() const;

    QHash< QString, QDomElement > projectLayerElementsByName() const;
    QHash< QString, QDomElement > projectLayerElementsById() const;

    QStringList wfsLayerNames() const;
    QStringList wcsLayerNames() const;

    QDomElement firstComposerLegendElement() const;

    QList<QDomElement> publishedComposerElements() const;

    /** Returns the text of the <layername> element for a layer element
    @return name or a null string in case of error*/
    QString layerName( const QDomElement& layerElem ) const;

    QString serviceUrl() const;
    QString wfsServiceUrl() const;
    QString wcsServiceUrl() const;

    QStringList wfsLayers() const;
    QStringList wcsLayers() const;

    //! Add layers for vector joins
    void addJoinLayersForElement( const QDomElement& layerElem ) const;

    void addValueRelationLayersForLayer( const QgsVectorLayer *vl ) const;

    //! Add layers which are necessary for the evaluation of the expression function 'getFeature( layer, attributField, value)'
    void addGetFeatureLayers( const QDomElement& layerElem ) const;

    /** Returns the text of the <id> element for a layer element
    @return id or a null string in case of error*/
    QString layerId( const QDomElement& layerElem ) const;

    /** Returns the text of the <id> element for a layer element
    @return id or a null string in case of error*/
    QString layerShortName( const QDomElement& layerElem ) const;

    QgsRectangle projectExtent() const;

    int numberOfLayers() const;

    bool updateLegendDrawingOrder() const;

    void serviceCapabilities( QDomElement& parentElement, QDomDocument& doc, const QString& service, bool sia2045 = false ) const;

    QStringList customLayerOrder() const;
};

