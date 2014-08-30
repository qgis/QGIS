/***************************************************************************
    qgsrendererv2.h
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRENDERERV2_H
#define QGSRENDERERV2_H

#include "qgis.h"

#include <QList>
#include <QString>
#include <QVariant>
#include <QPair>
#include <QPixmap>
#include <QDomDocument>
#include <QDomElement>

class QgsSymbolV2;
class QgsRenderContext;
class QgsFeature;
class QgsFields;
class QgsVectorLayer;

typedef QMap<QString, QString> QgsStringMap;

typedef QList<QgsSymbolV2*> QgsSymbolV2List;
typedef QMap<QString, QgsSymbolV2* > QgsSymbolV2Map;

typedef QList< QPair<QString, QPixmap> > QgsLegendSymbologyList;
typedef QList< QPair<QString, QgsSymbolV2*> > QgsLegendSymbolList;

#include "qgslegendsymbolitemv2.h"


#define RENDERER_TAG_NAME   "renderer-v2"

////////
// symbol levels

class CORE_EXPORT QgsSymbolV2LevelItem
{
  public:
    QgsSymbolV2LevelItem( QgsSymbolV2* symbol, int layer ) : mSymbol( symbol ), mLayer( layer ) {}
    QgsSymbolV2* symbol() { return mSymbol; }
    int layer() { return mLayer; }
  protected:
    QgsSymbolV2* mSymbol;
    int mLayer;
};

// every level has list of items: symbol + symbol layer num
typedef QList< QgsSymbolV2LevelItem > QgsSymbolV2Level;

// this is a list of levels
typedef QList< QgsSymbolV2Level > QgsSymbolV2LevelOrder;


//////////////
// renderers

class CORE_EXPORT QgsFeatureRendererV2
{
  public:
    // renderer takes ownership of its symbols!

    //! return a new renderer - used by default in vector layers
    static QgsFeatureRendererV2* defaultRenderer( QGis::GeometryType geomType );

    QString type() const { return mType; }

    /** to be overridden
     * @param feature feature
     * @return returns pointer to symbol or 0 if symbol was not found
     */
    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature ) = 0;

    virtual void startRender( QgsRenderContext& context, const QgsFields& fields ) = 0;

    //! @deprecated since 2.4 - not using QgsVectorLayer directly anymore
    Q_DECL_DEPRECATED virtual void startRender( QgsRenderContext& context, const QgsVectorLayer* vlayer );

    virtual void stopRender( QgsRenderContext& context ) = 0;

    virtual QList<QString> usedAttributes() = 0;

    virtual ~QgsFeatureRendererV2() {}

    virtual QgsFeatureRendererV2* clone() const = 0;

    virtual bool renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer = -1, bool selected = false, bool drawVertexMarker = false );

    //! for debugging
    virtual QString dump() const;

    enum Capabilities
    {
      SymbolLevels = 1,               // rendering with symbol levels (i.e. implements symbols(), symbolForFeature())
      RotationField = 1 <<  1,        // rotate symbols by attribute value
      MoreSymbolsPerFeature = 1 << 2, // may use more than one symbol to render a feature: symbolsForFeature() will return them
      Filter         = 1 << 3,        // features may be filtered, i.e. some features may not be rendered (categorized, rule based ...)
      ScaleDependent = 1 << 4         // depends on scale if feature will be rendered (rule based )
    };

    //! returns bitwise OR-ed capabilities of the renderer
    //! \note added in 2.0
    virtual int capabilities() { return 0; }

    //! for symbol levels
    virtual QgsSymbolV2List symbols() = 0;

    bool usingSymbolLevels() const { return mUsingSymbolLevels; }
    void setUsingSymbolLevels( bool usingSymbolLevels ) { mUsingSymbolLevels = usingSymbolLevels; }

    //! create a renderer from XML element
    static QgsFeatureRendererV2* load( QDomElement& symbologyElem );

    //! store renderer info to XML element
    virtual QDomElement save( QDomDocument& doc );

    //! create the SLD UserStyle element following the SLD v1.1 specs
    //! @note added in 1.9
    virtual QDomElement writeSld( QDomDocument& doc, const QgsVectorLayer &layer ) const;

    /** create a new renderer according to the information contained in
     * the UserStyle element of a SLD style document
     * @param node the node in the SLD document whose the UserStyle element
     * is a child
     * @param geomType the geometry type of the features, used to convert
     * Symbolizer elements
     * @param errorMessage it will contain the error message if something
     * went wrong
     * @return the renderer
     * @note added in 1.9
     */
    static QgsFeatureRendererV2* loadSld( const QDomNode &node, QGis::GeometryType geomType, QString &errorMessage );

    //! used from subclasses to create SLD Rule elements following SLD v1.1 specs
    //! @note added in 1.9
    virtual void toSld( QDomDocument& doc, QDomElement &element ) const
    { element.appendChild( doc.createComment( QString( "FeatureRendererV2 %1 not implemented yet" ).arg( type() ) ) ); }

    //! return a list of symbology items for the legend
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize );

    //! items of symbology items in legend should be checkable
    //! @note added in 2.5
    virtual bool legendSymbolItemsCheckable() const;

    //! items of symbology items in legend is checked
    //! @note added in 2.5
    virtual bool legendSymbolItemChecked( QString key );

    //! item in symbology was checked
    //! @note added in 2.5
    virtual void checkLegendSymbolItem( QString key, bool state = true );

    //! return a list of item text / symbol
    //! @note: this method was added in version 1.5
    //! @note not available in python bindings
    virtual QgsLegendSymbolList legendSymbolItems( double scaleDenominator = -1, QString rule = "" );

    //! Return a list of symbology items for the legend. Better choice than legendSymbolItems().
    //! Default fallback implementation just uses legendSymbolItems() implementation
    //! @note added in 2.6
    virtual QgsLegendSymbolListV2 legendSymbolItemsV2() const;

    //! If supported by the renderer, return classification attribute for the use in legend
    //! @note added in 2.6
    virtual QString legendClassificationAttribute() const { return QString(); }

    //! set type and size of editing vertex markers for subsequent rendering
    void setVertexMarkerAppearance( int type, int size );

    //! return rotation field name (or empty string if not set or not supported by renderer)
    //! @note added in 1.9
    virtual QString rotationField() const { return ""; }
    //! sets rotation field of renderer (if supported by the renderer)
    //! @note added in 1.9
    virtual void setRotationField( QString fieldName ) { Q_UNUSED( fieldName ); }

    //! return whether the renderer will render a feature or not.
    //! Must be called between startRender() and stopRender() calls.
    //! Default implementation uses symbolForFeature().
    //! @note added in 1.9
    virtual bool willRenderFeature( QgsFeature& feat ) { return symbolForFeature( feat ) != NULL; }

    //! return list of symbols used for rendering the feature.
    //! For renderers that do not support MoreSymbolsPerFeature it is more efficient
    //! to use symbolForFeature()
    //! @note added in 1.9
    virtual QgsSymbolV2List symbolsForFeature( QgsFeature& feat );

  protected:
    QgsFeatureRendererV2( QString type );

    void renderFeatureWithSymbol( QgsFeature& feature,
                                  QgsSymbolV2* symbol,
                                  QgsRenderContext& context,
                                  int layer,
                                  bool selected,
                                  bool drawVertexMarker );

    //! render editing vertex marker at specified point
    void renderVertexMarker( QPointF& pt, QgsRenderContext& context );
    //! render editing vertex marker for a polyline
    void renderVertexMarkerPolyline( QPolygonF& pts, QgsRenderContext& context );
    //! render editing vertex marker for a polygon
    void renderVertexMarkerPolygon( QPolygonF& pts, QList<QPolygonF>* rings, QgsRenderContext& context );

    static const unsigned char* _getPoint( QPointF& pt, QgsRenderContext& context, const unsigned char* wkb );
    static const unsigned char* _getLineString( QPolygonF& pts, QgsRenderContext& context, const unsigned char* wkb );
    static const unsigned char* _getPolygon( QPolygonF& pts, QList<QPolygonF>& holes, QgsRenderContext& context, const unsigned char* wkb );

    void setScaleMethodToSymbol( QgsSymbolV2* symbol, int scaleMethod );

    QString mType;

    bool mUsingSymbolLevels;

    /** The current type of editing marker */
    int mCurrentVertexMarkerType;
    /** The current size of editing marker */
    int mCurrentVertexMarkerSize;

  private:
    Q_DISABLE_COPY( QgsFeatureRendererV2 )
};

class QgsRendererV2Widget;  // why does SIP fail, when this isn't here

#endif // QGSRENDERERV2_H
