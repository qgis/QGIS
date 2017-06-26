/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/server/qgsserverprojectparser.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsServerProjectParser
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
#include "qgsserverprojectparser.h"
%End
  public:

    QgsServerProjectParser( QDomDocument *xmlDoc, const QString &filePath );

    QString projectPath() const;
%Docstring
 :rtype: str
%End

    const QDomDocument *xmlDocument() const;
%Docstring
 :rtype: QDomDocument
%End

    void projectLayerMap( QMap<QString, QgsMapLayer *> &layerMap ) const;
%Docstring
Returns project layers by id
%End

    QString convertToAbsolutePath( const QString &file ) const;
%Docstring
Converts a (possibly relative) path to absolute
 :rtype: str
%End

    QgsMapLayer *createLayerFromElement( const QDomElement &elem, bool useCache = true ) const;
%Docstring
 Creates a maplayer object from <maplayer> element. The layer cash owns the maplayer, so don't delete it
:return: the maplayer or 0 in case of error*
 :rtype: QgsMapLayer
%End

    QgsMapLayer *mapLayerFromLayerId( const QString &lId, bool useCache = true ) const;
%Docstring
 :rtype: QgsMapLayer
%End

    QString layerIdFromLegendLayer( const QDomElement &legendLayer ) const;
%Docstring
Returns the layer id under a <legendlayer> tag in the QGIS projectfile
 :rtype: str
%End

    void combineExtentAndCrsOfGroupChildren( QDomElement &groupElement, QDomDocument &doc, bool considerMapExtent = false ) const;
%Docstring
\param considerMapExtent Take user-defined map extent instead of data-calculated extent if present in project file
%End

    void addLayerProjectSettings( QDomElement &layerElem, QDomDocument &doc, QgsMapLayer *currentLayer ) const;

    QgsRectangle layerBoundingBoxInProjectCrs( const QDomElement &layerElem, const QDomDocument &doc ) const;
%Docstring
 :rtype: QgsRectangle
%End

    bool crsSetForLayer( const QDomElement &layerElement, QSet<QString> &crsSet ) const;
%Docstring
 :rtype: bool
%End

    QgsCoordinateReferenceSystem projectCrs() const;
%Docstring
 :rtype: QgsCoordinateReferenceSystem
%End

    QgsRectangle mapRectangle() const;
%Docstring
 :rtype: QgsRectangle
%End

    QStringList supportedOutputCrsList() const;
%Docstring
 :rtype: list of str
%End

    QList<QDomElement> projectLayerElements() const;
%Docstring
 :rtype: list of QDomElement
%End

    QList<QDomElement> legendGroupElements() const;
%Docstring
 :rtype: list of QDomElement
%End

    QString projectTitle() const;
%Docstring
 :rtype: str
%End

    QDomElement legendElem() const;
%Docstring
 :rtype: QDomElement
%End

    QDomElement propertiesElem() const;
%Docstring
 :rtype: QDomElement
%End

    QSet<QString> restrictedLayers() const;
%Docstring
 :rtype: set of str
%End
    bool useLayerIds() const;
%Docstring
 :rtype: bool
%End

    QHash< QString, QDomElement > projectLayerElementsByName() const;
%Docstring
 :rtype: QHash< str, QDomElement >
%End
    QHash< QString, QDomElement > projectLayerElementsById() const;
%Docstring
 :rtype: QHash< str, QDomElement >
%End


    QStringList wfsLayerNames() const;
%Docstring
 :rtype: list of str
%End
    QStringList wcsLayerNames() const;
%Docstring
 :rtype: list of str
%End

    QStringList layersNames() const;
%Docstring
 Gets a list containing names of layers. If a layer has a short name,
  then it's used instead of it's name.
 :return: A list of layers' names or short name if defined
.. versionadded:: 3.0
 :rtype: list of str
%End

    QDomElement firstComposerLegendElement() const;
%Docstring
 :rtype: QDomElement
%End

    QList<QDomElement> publishedComposerElements() const;
%Docstring
 :rtype: list of QDomElement
%End

    QList< QPair< QString, QgsDatumTransformStore::Entry > > layerCoordinateTransforms() const;
%Docstring
 :rtype: list of QPair< str, QgsDatumTransformStore.Entry >
%End

    QStringList wfsLayers() const;
%Docstring
 :rtype: list of str
%End
    QStringList wcsLayers() const;
%Docstring
 :rtype: list of str
%End

    void addJoinLayersForElement( const QDomElement &layerElem ) const;
%Docstring
Add layers for vector joins
%End

    void addValueRelationLayersForLayer( const QgsVectorLayer *vl ) const;
    void addGetFeatureLayers( const QDomElement &layerElem ) const;
%Docstring
Add layers which are necessary for the evaluation of the expression function 'getFeature( layer, attributeField, value)'
%End

    QString layerId( const QDomElement &layerElem ) const;
%Docstring
 Returns the text of the <id> element for a layer element
:return: id or a null string in case of error*
 :rtype: str
%End

    QgsRectangle projectExtent() const;
%Docstring
 :rtype: QgsRectangle
%End

    int numberOfLayers() const;
%Docstring
 :rtype: int
%End

    bool updateLegendDrawingOrder() const;
%Docstring
 :rtype: bool
%End

    void serviceCapabilities( QDomElement &parentElement, QDomDocument &doc, const QString &service, bool sia2045 = false ) const;

    QStringList customLayerOrder() const;
%Docstring
 :rtype: list of str
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/server/qgsserverprojectparser.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
