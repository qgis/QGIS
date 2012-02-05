/***************************************************************************
                              qgsconfigparser.h
                              -----------------
  begin                : May 26, 2010
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

#ifndef QGSCONFIGPARSER_H
#define QGSCONFIGPARSER_H

#include "qgsmaprenderer.h"
#include <QDomDocument>
#include <QFont>
#include <QList>
#include <QSet>
#include <QTemporaryFile>

class QgsComposition;
class QgsComposerLabel;
class QgsComposerMap;
class QDomElement;

/**Interface class for configuration parsing, e.g. SLD configuration or QGIS project file*/
class QgsConfigParser
{
  public:
    QgsConfigParser();

    virtual ~QgsConfigParser();

    /**Adds layer and style specific capabilities elements to the parent node. This includes the individual layers and styles, their description, native CRS, bounding boxes, etc.*/
    virtual void layersAndStylesCapabilities( QDomElement& parentElement, QDomDocument& doc ) const = 0;

    /**Returns one or possibly several maplayers for a given layer name and style. If there are several layers, the layers should be drawn in inverse list order.
       If no layers/style are found, an empty list is returned
      @param allowCache true if layer can be read from / written to cache*/
    virtual QList<QgsMapLayer*> mapLayerFromStyle( const QString& layerName, const QString& styleName, bool useCache = true ) const = 0;

    /**Returns number of layers in configuration*/
    virtual int numberOfLayers() const = 0;

    /**Fills a layer and a style list. The two list have the same number of entries and the style and the layer at a position belong together (similar to the HTTP parameters 'Layers' and 'Styles'. Returns 0 in case of success*/
    virtual int layersAndStyles( QStringList& layers, QStringList& styles ) const = 0;

    /**Returns the xml fragment of a style*/
    virtual QDomDocument getStyle( const QString& styleName, const QString& layerName ) const = 0;

    /**Possibility to add a parameter map to the config parser. This is used by the SLD parser. Default implementation does nothing*/
    virtual void setParameterMap( const QMap<QString, QString>& parameterMap )
    { Q_UNUSED( parameterMap ); }

    /**Returns if output are MM or PIXEL*/
    QgsMapRenderer::OutputUnits outputUnits() const { return mOutputUnits; }
    void setOutputUnits( QgsMapRenderer::OutputUnits u ) { mOutputUnits = u; }

    /**Sets fallback parser (does not take ownership)*/
    void setFallbackParser( QgsConfigParser* p );
    const QgsConfigParser* fallbackParser() { return mFallbackParser; }

    void setScaleDenominator( double denom ) {mScaleDenominator = denom;}

    void addExternalGMLData( const QString& layerName, QDomDocument* gmlDoc );

    void setLegendLayerFont( const QFont& f ) { mLegendLayerFont = f; }
    const QFont& legendLayerFont() const { return mLegendLayerFont; }

    void setLegendItemFont( const QFont& f ) { mLegendItemFont = f; }
    const QFont& legendItemFont() const { return mLegendItemFont; }

    double legendBoxSpace() const { return mLegendBoxSpace; }
    double legendLayerSpace() const { return mLegendLayerSpace; }
    double legendSymbolSpace() const { return mLegendSymbolSpace; }
    double legendIconLabelSpace() const { return mLegendIconLabelSpace; }
    double legendSymbolWidth() const { return mLegendSymbolWidth; }
    double legendSymbolHeight() const { return mLegendSymbolHeight; }

    /**Returns an ID-list of layers which are not queryable*/
    virtual QStringList identifyDisabledLayers() const { return QStringList(); }

    /**Returns a set of supported epsg codes for the capabilities document. An empty list means
       that all possible CRS should be advertised (which could result in very long capabilities documents)*/
    virtual QStringList supportedOutputCrsList() const { return QStringList(); }

    /**True if the feature info response should contain the wkt geometry for vector features*/
    virtual bool featureInfoWithWktGeometry() const { return false; }

    /**Returns information about vector layer aliases. First key is the layer id, (second) key is the field id, value the alias.
       Default implementation returns an empty map*/
    virtual QMap< QString, QMap< int, QString > > layerAliasInfo() const { return QMap< QString, QMap<int, QString> > (); }

    /**Returns information about vector attributes with hidden edit type. Key is layer id, value is a set containing the names of the hidden attributes*/
    virtual QMap< QString, QSet<QString> > hiddenAttributes() const { return QMap< QString, QSet<QString> >(); }

    /**Creates a print composition, usually for a GetPrint request. Replaces map and label parameters*/
    QgsComposition* createPrintComposition( const QString& composerTemplate, QgsMapRenderer* mapRenderer, const QMap< QString, QString >& parameterMap ) const;

    /**Creates a composition from the project file (probably delegated to the fallback parser)*/
    virtual QgsComposition* initComposition( const QString& composerTemplate, QgsMapRenderer* mapRenderer, QList< QgsComposerMap*>& mapList, QList< QgsComposerLabel* >& labelList ) const = 0;

    /**Adds print capabilities to xml document. ParentElem usually is the <Capabilities> element*/
    virtual void printCapabilities( QDomElement& parentElement, QDomDocument& doc ) const = 0;

    /**Appends service metadata to the capabilities document*/
    virtual void serviceCapabilities( QDomElement& parentElement, QDomDocument& doc ) const;

  protected:
    /**Parser to forward not resolved requests (e.g. SLD parser based on user request might have a fallback parser with admin configuration)*/
    QgsConfigParser* mFallbackParser;
    /**Indicates the current scale and is used for scale dependent symbology. Defaults to 0 (means that the scale is
       ignored*/
    double mScaleDenominator;

    /**Output units (pixel or mm)*/
    QgsMapRenderer::OutputUnits mOutputUnits;

    /**List of GML datasets passed outside SLD (e.g. in a SOAP request). Key of the map is the layer name*/
    QMap<QString, QDomDocument*> mExternalGMLDatasets;

    //todo: leave this to the layer cash?
    /**Stores pointers to layers that have to be removed in the destructor of QgsSLDParser*/
    mutable QList<QgsMapLayer*> mLayersToRemove;

    /**Stores the temporary file objects. The class takes ownership of the objects and deletes them in the destructor*/
    mutable QList<QTemporaryFile*> mFilesToRemove;

    /**Stores paths of files that need to be removed after each request (necessary because of contours shapefiles that
      cannot be handles with QTemporaryFile*/
    mutable QList<QString> mFilePathsToRemove;

    /**Layer font for GetLegendGraphics*/
    QFont mLegendLayerFont;
    /**Item font for GetLegendGraphics*/
    QFont mLegendItemFont;

    //various parameters used for GetLegendGraphics
    double mLegendBoxSpace;
    double mLegendLayerSpace;
    double mLegendSymbolSpace;
    double mLegendIconLabelSpace;
    double mLegendSymbolWidth;
    double mLegendSymbolHeight;

    /**Transforms layer extent to epsg 4326 and appends ExGeographicBoundingBox and BoundingBox elements to the layer element*/
    void appendLayerBoundingBoxes( QDomElement& layerElem, QDomDocument& doc, const QgsRectangle& layerExtent, const QgsCoordinateReferenceSystem& layerCRS ) const;

#if 0
    /**Returns the <Ex_GeographicalBoundingBox of a layer element as a rectangle
        @param layerElement <Layer> element in capabilities
        @param rect out: bounding box as rectangle
        @return true in case of success*/
    bool exGeographicBoundingBox( const QDomElement& layerElement, QgsRectangle& rect ) const;
    bool latlonGeographicBoundingBox( const QDomElement& layerElement, QgsRectangle& rect ) const;
#endif

    /**Returns a list of supported EPSG coordinate system numbers from a layer*/
    QStringList createCRSListForLayer( QgsMapLayer* theMapLayer ) const;
    /**Reads all the epsg numbers from a capabilities layer
    @param layerElement <Layer> element in capabilities
    @param crsSet out: set containing the epsg numbers on successfull completion
    @return true in case of success*/
    bool crsSetForLayer( const QDomElement& layerElement, QSet<QString> &crsSet ) const;
    void appendCRSElementsToLayer( QDomElement& layerElement, QDomDocument& doc, const QStringList &crsList ) const;
    void appendCRSElementToLayer( QDomElement& layerElement, const QDomElement& precedingElement, const QString& crsText, QDomDocument& doc ) const;

    void setDefaultLegendSettings();
};

#endif // QGSCONFIGPARSER_H
