/***************************************************************************
                              qgsserverprojectparser.h
                              ------------------------
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

#ifndef QGSSERVERPROJECTPARSER_H
#define QGSSERVERPROJECTPARSER_H

#include "qgsvectorlayer.h"
#include <QDomElement>
#include <QHash>
#include <QMap>
#include <QString>

class QgsCoordinateReferenceSystem;
class QgsMapLayer;
class QgsRectangle;
class QDomDocument;

class QgsServerProjectParser
{
  public:
    /**Takes ownership of the document*/
    QgsServerProjectParser( QDomDocument* xmlDoc, const QString& filePath );
    ~QgsServerProjectParser();

    QString projectPath() const { return mProjectPath; }

    const QDomDocument* xmlDocument() const { return mXMLDoc; }

    /**Returns project layers by id*/
    void projectLayerMap( QMap<QString, QgsMapLayer*>& layerMap ) const;

    /**Converts a (possibly relative) path to absolute*/
    QString convertToAbsolutePath( const QString& file ) const;

    /**Creates a maplayer object from <maplayer> element. The layer cash owns the maplayer, so don't delete it
    @return the maplayer or 0 in case of error*/
    QgsMapLayer* createLayerFromElement( const QDomElement& elem, bool useCache = true ) const;

    QgsMapLayer* mapLayerFromLayerId( const QString& lId, bool useCache = true ) const;

    /**Returns the layer id under a <legendlayer> tag in the QGIS projectfile*/
    QString layerIdFromLegendLayer( const QDomElement& legendLayer ) const;

    /**@param considerMapExtent Take user-defined map extent instead of data-calculated extent if present in project file*/
    void combineExtentAndCrsOfGroupChildren( QDomElement& groupElement, QDomDocument& doc, bool considerMapExtent = false ) const;

    void addLayerProjectSettings( QDomElement& layerElem, QDomDocument& doc, QgsMapLayer* currentLayer ) const;

    QgsRectangle layerBoundingBoxInProjectCRS( const QDomElement& layerElem, const QDomDocument &doc ) const;

    bool crsSetForLayer( const QDomElement& layerElement, QSet<QString> &crsSet ) const;

    const QgsCoordinateReferenceSystem& projectCRS() const;

    QgsRectangle mapRectangle() const;

    QStringList supportedOutputCrsList() const;

    const QList<QDomElement>& projectLayerElements() const { return mProjectLayerElements; }

    const QList<QDomElement>& legendGroupElements() const { return mLegendGroupElements; }

    QString projectTitle() const;

    QDomElement legendElem() const;

    QDomElement propertiesElem() const;

    const QSet<QString>& restrictedLayers() const { return mRestrictedLayers; }
    bool useLayerIDs() const { return mUseLayerIDs; }

    const QHash< QString, QDomElement >& projectLayerElementsByName() const { return mProjectLayerElementsByName; }
    const QHash< QString, QDomElement >& projectLayerElementsById() const { return mProjectLayerElementsById; }

    void layerFromLegendLayer( const QDomElement& legendLayerElem, QMap< int, QgsMapLayer*>& layers, bool useCache = true ) const;

    QStringList wfsLayerNames() const;
    QStringList wcsLayerNames() const;

    QDomElement firstComposerLegendElement() const;

    QList<QDomElement> publishedComposerElements() const;

    QList< QPair< QString, QgsLayerCoordinateTransform > > layerCoordinateTransforms() const;

    /**Returns the text of the <layername> element for a layer element
    @return id or a null string in case of error*/
    QString layerName( const QDomElement& layerElem ) const;

    QString serviceUrl() const;
    QString wfsServiceUrl() const;
    QString wcsServiceUrl() const;

    QStringList wfsLayers() const;
    QStringList wcsLayers() const;

    void addJoinLayersForElement( const QDomElement& layerElem ) const;

    void addValueRelationLayersForLayer( const QgsVectorLayer *vl ) const;
    /**Add layers which are necessary for the evaluation of the expression function 'getFeature( layer, attributField, value)'*/
    void addGetFeatureLayers( const QDomElement& layerElem ) const;

    /**Returns the text of the <id> element for a layer element
    @return id or a null string in case of error*/
    QString layerId( const QDomElement& layerElem ) const;

    QgsRectangle projectExtent() const;

    int numberOfLayers() const;

    bool updateLegendDrawingOrder() const;

    void serviceCapabilities( QDomElement& parentElement, QDomDocument& doc, const QString& service, bool sia2045 = false ) const;

    QStringList customLayerOrder() const { return mCustomLayerOrder; }

  private:

    /**Content of project file*/
    QDomDocument* mXMLDoc;

    /**Absolute project file path (including file name)*/
    QString mProjectPath;

    /**List of project layer (ordered same as in the project file)*/
    QList<QDomElement> mProjectLayerElements;

    /**Project layer elements, accessible by layer id*/
    QHash< QString, QDomElement > mProjectLayerElementsById;

    /**Project layer elements, accessible by layer name*/
    QHash< QString, QDomElement > mProjectLayerElementsByName;

    /**List of all legend group elements*/
    QList<QDomElement> mLegendGroupElements;

    /**Names of layers and groups which should not be published*/
    QSet<QString> mRestrictedLayers;

    bool mUseLayerIDs;

    QgsServerProjectParser(); //forbidden

    /**Returns a complete string set with all the restricted layer names (layers/groups that are not to be published)*/
    QSet<QString> findRestrictedLayers() const;

    QStringList mCustomLayerOrder;

    bool findUseLayerIDs() const;

    /**Adds sublayers of an embedded group to layer set*/
    static void sublayersOfEmbeddedGroup( const QString& projectFilePath, const QString& groupName, QSet<QString>& layerSet );
};

#endif // QGSSERVERPROJECTPARSER_H
