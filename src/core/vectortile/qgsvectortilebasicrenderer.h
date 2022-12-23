/***************************************************************************
  qgsvectortilebasicrenderer.h
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORTILEBASICRENDERER_H
#define QGSVECTORTILEBASICRENDERER_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include "qgsvectortilerenderer.h"

class QgsLineSymbol;
class QgsFillSymbol;
class QgsMarkerSymbol;

class QgsSymbol;

/**
 * \ingroup core
 * \brief Definition of map rendering of a subset of vector tile data. The subset of data is defined by:
 *
 * - sub-layer name
 * - geometry type (a single sub-layer may have multiple geometry types)
 * - filter expression
 *
 * Rendering is determined by the associated symbol (QgsSymbol). Symbol has to be of the same
 * type as the chosen geometryType() - i.e. QgsMarkerSymbol for points, QgsLineSymbol for linestrings
 * and QgsFillSymbol for polygons.
 *
 * It is possible to further constrain when this style is applied by setting a range of allowed
 * zoom levels, or by disabling it.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsVectorTileBasicRendererStyle
{
  public:
    //! Constructs a style object
    QgsVectorTileBasicRendererStyle( const QString &stName = QString(), const QString &laName = QString(), QgsWkbTypes::GeometryType geomType = QgsWkbTypes::UnknownGeometry );
    //! Constructs a style object as a copy of another style
    QgsVectorTileBasicRendererStyle( const QgsVectorTileBasicRendererStyle &other );
    QgsVectorTileBasicRendererStyle &operator=( const QgsVectorTileBasicRendererStyle &other );
    ~QgsVectorTileBasicRendererStyle();

    //! Sets human readable name of this style
    void setStyleName( const QString &name ) { mStyleName = name; }
    //! Returns human readable name of this style
    QString styleName() const { return mStyleName; }

    //! Sets name of the sub-layer to render (empty layer means that all layers match)
    void setLayerName( const QString &name ) { mLayerName = name; }
    //! Returns name of the sub-layer to render (empty layer means that all layers match)
    QString layerName() const { return mLayerName; }

    //! Sets type of the geometry that will be used (point / line / polygon)
    void setGeometryType( QgsWkbTypes::GeometryType geomType ) { mGeometryType = geomType; }
    //! Returns type of the geometry that will be used (point / line / polygon)
    QgsWkbTypes::GeometryType geometryType() const { return mGeometryType; }

    //! Sets filter expression (empty filter means that all features match)
    void setFilterExpression( const QString &expr ) { mExpression = expr; }
    //! Returns filter expression (empty filter means that all features match)
    QString filterExpression() const { return mExpression; }

    //! Sets symbol for rendering. Takes ownership of the symbol.
    void setSymbol( QgsSymbol *sym SIP_TRANSFER );
    //! Returns symbol for rendering
    QgsSymbol *symbol() const { return mSymbol.get(); }

    //! Sets whether this style is enabled (used for rendering)
    void setEnabled( bool enabled ) { mEnabled = enabled; }
    //! Returns whether this style is enabled (used for rendering)
    bool isEnabled() const { return mEnabled; }

    //! Sets minimum zoom level index (negative number means no limit)
    void setMinZoomLevel( int minZoom ) { mMinZoomLevel = minZoom; }
    //! Returns minimum zoom level index (negative number means no limit)
    int minZoomLevel() const { return mMinZoomLevel; }

    //! Sets maximum zoom level index (negative number means no limit)
    void setMaxZoomLevel( int maxZoom ) { mMaxZoomLevel = maxZoom; }
    //! Returns maxnimum zoom level index (negative number means no limit)
    int maxZoomLevel() const { return mMaxZoomLevel; }

    //! Returns whether the style is active at given zoom level (also checks "enabled" flag)
    bool isActive( int zoomLevel ) const
    {
      return mEnabled && ( mMinZoomLevel == -1 || zoomLevel >= mMinZoomLevel ) && ( mMaxZoomLevel == -1 || zoomLevel <= mMaxZoomLevel );
    }

    //! Writes object content to given DOM element
    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const;
    //! Reads object content from given DOM element
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context );

  private:
    QString mStyleName;
    QString mLayerName;
    QgsWkbTypes::GeometryType mGeometryType;
    std::unique_ptr<QgsSymbol> mSymbol;
    bool mEnabled = true;
    QString mExpression;
    int mMinZoomLevel = -1;
    int mMaxZoomLevel = -1;
};


/**
 * \ingroup core
 * \brief The default vector tile renderer implementation. It has an ordered list of "styles",
 * each defines a rendering rule.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsVectorTileBasicRenderer : public QgsVectorTileRenderer
{
  public:
    //! Constructs renderer with no styles
    QgsVectorTileBasicRenderer();

    QString type() const override;
    QgsVectorTileBasicRenderer *clone() const override SIP_FACTORY;
    void startRender( QgsRenderContext &context, int tileZoom, const QgsTileRange &tileRange ) override;
    QMap<QString, QSet<QString> > usedAttributes( const QgsRenderContext & ) override SIP_SKIP;
    QSet< QString > requiredLayers( QgsRenderContext &context, int tileZoom ) const override;
    void stopRender( QgsRenderContext &context ) override;
    void renderTile( const QgsVectorTileRendererData &tile, QgsRenderContext &context ) override;
    void renderSelectedFeatures( const QList< QgsFeature > &selection, QgsRenderContext &context ) override;
    bool willRenderFeature( const QgsFeature &feature, int tileZoom, const QString &layerName, QgsRenderContext &context ) override;
    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;

    //! Sets list of styles of the renderer
    void setStyles( const QList<QgsVectorTileBasicRendererStyle> &styles );
    //! Returns list of styles of the renderer
    QList<QgsVectorTileBasicRendererStyle> styles() const;
    //! Updates style definition at the paricular index of the list (the index must be in interval [0,N-1] otherwise this function does nothing)
    void setStyle( int index, const QgsVectorTileBasicRendererStyle &style ) { mStyles[index] = style; }
    //! Returns style definition at the particular index
    QgsVectorTileBasicRendererStyle style( int index ) const { return mStyles[index]; }

    //! Returns a list of styles to render all layers with the given fill/stroke colors, stroke widths and marker sizes
    static QList<QgsVectorTileBasicRendererStyle> simpleStyle(
      const QColor &polygonFillColor, const QColor &polygonStrokeColor, double polygonStrokeWidth,
      const QColor &lineStrokeColor, double lineStrokeWidth,
      const QColor &pointFillColor, const QColor &pointStrokeColor, double pointSize );

    //! Returns a list of styles to render all layers, using random colors
    static QList<QgsVectorTileBasicRendererStyle> simpleStyleWithRandomColors();

  private:
    void setDefaultStyle();

  private:
    //! List of rendering styles
    QList<QgsVectorTileBasicRendererStyle> mStyles;

    // temporary bits

    //! Names of required fields for each sub-layer (only valid between startRender/stopRender calls)
    QMap<QString, QSet<QString> > mRequiredFields;

};

#endif // QGSVECTORTILEBASICRENDERER_H
