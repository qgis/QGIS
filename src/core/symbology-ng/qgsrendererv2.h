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
#include "qgsrectangle.h"
#include "qgsrendercontext.h"
#include "qgssymbolv2.h"

#include <QList>
#include <QString>
#include <QVariant>
#include <QPair>
#include <QPixmap>
#include <QDomDocument>
#include <QDomElement>

class QgsFeature;
class QgsFields;
class QgsVectorLayer;
class QgsPaintEffect;

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

    /** To be overridden
     * @param feature feature
     * @return returns pointer to symbol or 0 if symbol was not found
     */
    Q_DECL_DEPRECATED virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature );

    /** To be overridden
     * @param feature feature
     * @param context render context
     * @return returns pointer to symbol or 0 if symbol was not found
     * @note added in QGIS 2.12
     */
    //TODO - make pure virtual when above method is removed
    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature, QgsRenderContext& context );

    /**
     * Return symbol for feature. The difference compared to symbolForFeature() is that it returns original
     * symbol which can be used as an identifier for renderer's rule - the former may return a temporary replacement
     * of a symbol for use in rendering.
     * @note added in 2.6
     */
    Q_DECL_DEPRECATED virtual QgsSymbolV2* originalSymbolForFeature( QgsFeature& feature );

    /**
     * Return symbol for feature. The difference compared to symbolForFeature() is that it returns original
     * symbol which can be used as an identifier for renderer's rule - the former may return a temporary replacement
     * of a symbol for use in rendering.
     * @note added in 2.12
     */
    virtual QgsSymbolV2* originalSymbolForFeature( QgsFeature& feature, QgsRenderContext& context );

    /**
     * Needs to be called when a new render cycle is started
     *
     * @param context  Additional information passed to the renderer about the job which will be rendered
     * @param fields   The fields available for rendering
     * @return         Information passed back from the renderer that can e.g. be used to reduce the amount of requested features
     */
    virtual void startRender( QgsRenderContext& context, const QgsFields& fields ) = 0;

    //! @deprecated since 2.4 - not using QgsVectorLayer directly anymore
    Q_DECL_DEPRECATED virtual void startRender( QgsRenderContext& context, const QgsVectorLayer* vlayer );

    virtual void stopRender( QgsRenderContext& context ) = 0;

    /**
     * If a renderer does not require all the features this method may be overridden
     * and return an expression used as where clause.
     * This will be called once after {@link startRender()} and before the first call
     * to {@link renderFeature()}.
     * By default this returns a null string and all features will be requested.
     * You do not need to specify the extent in here, this is taken care of separately and
     * will be combined with a filter returned from this method.
     *
     * @return An expresion used as where clause
     */
    virtual QString filter() { return QString::null; }

    virtual QList<QString> usedAttributes() = 0;

    virtual ~QgsFeatureRendererV2();

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
    virtual int capabilities() { return 0; }

    //! for symbol levels
    Q_DECL_DEPRECATED virtual QgsSymbolV2List symbols();

    /** Returns list of symbols used by the renderer.
     * @param context render context
     * @note added in QGIS 2.12
     */
    virtual QgsSymbolV2List symbols( QgsRenderContext& context );

    bool usingSymbolLevels() const { return mUsingSymbolLevels; }
    void setUsingSymbolLevels( bool usingSymbolLevels ) { mUsingSymbolLevels = usingSymbolLevels; }

    //! create a renderer from XML element
    static QgsFeatureRendererV2* load( QDomElement& symbologyElem );

    //! store renderer info to XML element
    virtual QDomElement save( QDomDocument& doc );

    //! create the SLD UserStyle element following the SLD v1.1 specs
    //! @deprecated since 2.8 - use the other override with styleName
    Q_DECL_DEPRECATED virtual QDomElement writeSld( QDomDocument& doc, const QgsVectorLayer &layer ) const;
    //! create the SLD UserStyle element following the SLD v1.1 specs with the given name
    //! @note added in 2.8
    virtual QDomElement writeSld( QDomDocument& doc, const QString& styleName ) const;

    /** Create a new renderer according to the information contained in
     * the UserStyle element of a SLD style document
     * @param node the node in the SLD document whose the UserStyle element
     * is a child
     * @param geomType the geometry type of the features, used to convert
     * Symbolizer elements
     * @param errorMessage it will contain the error message if something
     * went wrong
     * @return the renderer
     */
    static QgsFeatureRendererV2* loadSld( const QDomNode &node, QGis::GeometryType geomType, QString &errorMessage );

    //! used from subclasses to create SLD Rule elements following SLD v1.1 specs
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
    //! @deprecated use the symbol's methods instead
    Q_DECL_DEPRECATED virtual QString rotationField() const { return QString(); }
    //! sets rotation field of renderer (if supported by the renderer)
    //! @deprecated use the symbol's methods instead
    Q_DECL_DEPRECATED virtual void setRotationField( QString fieldName ) { Q_UNUSED( fieldName ); }

    //! return whether the renderer will render a feature or not.
    //! Must be called between startRender() and stopRender() calls.
    //! Default implementation uses symbolForFeature().
    Q_DECL_DEPRECATED virtual bool willRenderFeature( QgsFeature& feat );

    /** Returns whether the renderer will render a feature or not.
     * Must be called between startRender() and stopRender() calls.
     * Default implementation uses symbolForFeature().
     * @note added in QGIS 2.12
     */
    virtual bool willRenderFeature( QgsFeature& feat, QgsRenderContext& context );

    //! return list of symbols used for rendering the feature.
    //! For renderers that do not support MoreSymbolsPerFeature it is more efficient
    //! to use symbolForFeature()
    Q_DECL_DEPRECATED virtual QgsSymbolV2List symbolsForFeature( QgsFeature& feat );

    //! return list of symbols used for rendering the feature.
    //! For renderers that do not support MoreSymbolsPerFeature it is more efficient
    //! to use symbolForFeature()
    virtual QgsSymbolV2List symbolsForFeature( QgsFeature& feat, QgsRenderContext& context );

    //! Equivalent of originalSymbolsForFeature() call
    //! extended to support renderers that may use more symbols per feature - similar to symbolsForFeature()
    //! @note added in 2.6
    Q_DECL_DEPRECATED virtual QgsSymbolV2List originalSymbolsForFeature( QgsFeature& feat );

    //! Equivalent of originalSymbolsForFeature() call
    //! extended to support renderers that may use more symbols per feature - similar to symbolsForFeature()
    //! @note added in 2.6
    virtual QgsSymbolV2List originalSymbolsForFeature( QgsFeature& feat, QgsRenderContext& context );

    /** Allows for a renderer to modify the extent of a feature request prior to rendering
     * @param extent reference to request's filter extent. Modify extent to change the
     * extent of feature request
     * @param context render context
     * @note added in QGIS 2.7
     */
    virtual void modifyRequestExtent( QgsRectangle& extent, QgsRenderContext& context ) { Q_UNUSED( extent ); Q_UNUSED( context ); }

    /** Returns the current paint effect for the renderer.
     * @returns paint effect
     * @note added in QGIS 2.9
     * @see setPaintEffect
     */
    QgsPaintEffect* paintEffect() const;

    /** Sets the current paint effect for the renderer.
     * @param effect paint effect. Ownership is transferred to the renderer.
     * @note added in QGIS 2.9
     * @see paintEffect
     */
    void setPaintEffect( QgsPaintEffect* effect );

    /** Returns whether the renderer must render as a raster.
     * @note added in QGIS 2.12
     * @see setForceRasterRender
     */
    bool forceRasterRender() const { return mForceRaster; }

    /** Sets whether the renderer should be rendered to a raster destination.
     * @param forceRaster set to true if renderer must be drawn on a raster surface.
     * This may be desirable for highly detailed layers where rendering as a vector
     * would result in a large, complex vector output.
     * @see forceRasterRender
     * @note added in QGIS 2.12
     */
    void setForceRasterRender( bool forceRaster ) { mForceRaster = forceRaster; }

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
    static const unsigned char* _getLineString( QPolygonF& pts, QgsRenderContext& context, const unsigned char* wkb, bool clipToExtent = true );
    static const unsigned char* _getPolygon( QPolygonF& pts, QList<QPolygonF>& holes, QgsRenderContext& context, const unsigned char* wkb, bool clipToExtent = true );

    void setScaleMethodToSymbol( QgsSymbolV2* symbol, int scaleMethod );

    /** Copies paint effect of this renderer to another renderer
     * @param destRenderer destination renderer for copied effect
     */
    void copyPaintEffect( QgsFeatureRendererV2 *destRenderer ) const;

    QString mType;

    bool mUsingSymbolLevels;

    /** The current type of editing marker */
    int mCurrentVertexMarkerType;
    /** The current size of editing marker */
    int mCurrentVertexMarkerSize;

    QgsPaintEffect* mPaintEffect;

    bool mForceRaster;

    /** @note this function is used to convert old sizeScale expresssions to symbol
     * level DataDefined size
     */
    static void convertSymbolSizeScale( QgsSymbolV2 * symbol, QgsSymbolV2::ScaleMethod method, const QString & field );
    /** @note this function is used to convert old rotations expresssions to symbol
     * level DataDefined angle
     */
    static void convertSymbolRotation( QgsSymbolV2 * symbol, const QString & field );

  private:
    Q_DISABLE_COPY( QgsFeatureRendererV2 )
};

// for some reason SIP compilation fails if these lines are not included:
class QgsRendererV2Widget;
class QgsPaintEffectWidget;

#endif // QGSRENDERERV2_H
