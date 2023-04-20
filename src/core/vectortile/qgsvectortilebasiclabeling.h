/***************************************************************************
  qgsvectortilebasiclabeling.h
  --------------------------------------
  Date                 : April 2020
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

#ifndef QGSVECTORTILEBASICLABELING_H
#define QGSVECTORTILEBASICLABELING_H


#include "qgsvectortilelabeling.h"


class QgsPalLayerSettings;

/**
 * \ingroup core
 * \brief Configuration of a single style within QgsVectorTileBasicLabeling
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsVectorTileBasicLabelingStyle
{
  public:

    //! Sets labeling configuration of this style
    void setLabelSettings( const QgsPalLayerSettings &settings ) { mLabelSettings = settings; }
    //! Returns labeling configuration of this style
    QgsPalLayerSettings labelSettings() const { return mLabelSettings; }

    //! Sets human readable name of this style
    void setStyleName( const QString &name ) { mStyleName = name; }
    //! Returns human readable name of this style
    QString styleName() const { return mStyleName; }

    //! Sets name of the sub-layer to render (empty layer means that all layers match)
    void setLayerName( const QString &name ) { mLayerName = name; }
    //! Returns name of the sub-layer to render (empty layer means that all layers match)
    QString layerName() const { return mLayerName; }

    //! Sets type of the geometry that will be used (point / line / polygon)
    void setGeometryType( Qgis::GeometryType geomType ) { mGeometryType = geomType; }
    //! Returns type of the geometry that will be used (point / line / polygon)
    Qgis::GeometryType geometryType() const { return mGeometryType; }

    //! Sets filter expression (empty filter means that all features match)
    void setFilterExpression( const QString &expr ) { mExpression = expr; }
    //! Returns filter expression (empty filter means that all features match)
    QString filterExpression() const { return mExpression; }

    //! Sets whether this style is enabled (used for rendering)
    void setEnabled( bool enabled ) { mEnabled = enabled; }
    //! Returns whether this style is enabled (used for rendering)
    bool isEnabled() const { return mEnabled; }

    /**
     * Sets minimum zoom level index (negative number means no limit).
     *
     * The style will be rendered if the zoom level is greater than or equal
     * to \a minZoom.
     *
     * \see minZoomLevel()
     * \see setMaxZoomLevel()
     */
    void setMinZoomLevel( int minZoom ) { mMinZoomLevel = minZoom; }

    /**
     * Returns the minimum zoom level index (negative number means no limit).
     *
     * The style will be rendered if the zoom level is greater than or equal
     * to the this level.
     *
     * \see setMinZoomLevel()
     * \see maxZoomLevel()
     */
    int minZoomLevel() const { return mMinZoomLevel; }

    /**
     * Sets maximum zoom level index (negative number means no limit).
     *
     * The style will be rendered if the zoom level is less than or equal
     * to \a maxZoom.
     *
     * \warning This differs from the handling of the max zoom as defined
     * in the MapBox Style Specifications, where the style is rendered
     * only if the zoom level is less than the maximum zoom.
     *
     * \see maxZoomLevel()
     * \see setMinZoomLevel()
     */
    void setMaxZoomLevel( int maxZoom ) { mMaxZoomLevel = maxZoom; }

    /**
     * Returns the maximum zoom level index (negative number means no limit).
     *
     * The style will be rendered if the zoom level is less than or equal
     * to the maximum zoom.
     *
     * \warning This differs from the handling of the max zoom as defined
     * in the MapBox Style Specifications, where the style is rendered
     * only if the zoom level is less than the maximum zoom.
     *
     * \see setMaxZoomLevel()
     * \see minZoomLevel()
     */
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
    Qgis::GeometryType mGeometryType;
    bool mEnabled = true;
    QString mExpression;
    int mMinZoomLevel = -1;
    int mMaxZoomLevel = -1;

    QgsPalLayerSettings mLabelSettings;
};


/**
 * \ingroup core
 * \brief Basic labeling configuration for vector tile layers. It contains a definition
 * of a list of labeling styles, where each labeling style is a combination of
 * sub-layer name, geometry type, filter expression, zoom range and label settings.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsVectorTileBasicLabeling : public QgsVectorTileLabeling
{
  public:
    QgsVectorTileBasicLabeling();

    QString type() const override;
    QgsVectorTileLabeling *clone() const override SIP_FACTORY;
    QgsVectorTileLabelProvider *provider( QgsVectorTileLayer *layer ) const override SIP_SKIP;
    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;

    //! Sets list of styles of the renderer
    void setStyles( const QList<QgsVectorTileBasicLabelingStyle> &styles ) { mStyles = styles; }
    //! Returns list of styles of the renderer
    QList<QgsVectorTileBasicLabelingStyle> styles() const { return mStyles; }
    //! Updates style definition at the paricular index of the list (the index must be in interval [0,N-1] otherwise this function does nothing)
    void setStyle( int index, const QgsVectorTileBasicLabelingStyle &style ) { mStyles[index] = style; }
    //! Returns style definition at the particular index
    QgsVectorTileBasicLabelingStyle style( int index ) const { return mStyles[index]; }

  private:
    //! List of rendering styles
    QList<QgsVectorTileBasicLabelingStyle> mStyles;
};


#ifndef SIP_RUN

/**
 * \ingroup core
 * \brief Implementation class for QgsVectorTileBasicLabeling
 *
 * \since QGIS 3.14
 */
class QgsVectorTileBasicLabelProvider : public QgsVectorTileLabelProvider
{
  public:
    //! Constructs a label provider for the given vector tile layer and using styling from QgsVectorTileBasicLabeling
    QgsVectorTileBasicLabelProvider( QgsVectorTileLayer *layer, const QList<QgsVectorTileBasicLabelingStyle> &styles );

    QList<QgsAbstractLabelProvider *> subProviders() override;
    bool prepare( QgsRenderContext &context, QSet<QString> &attributeNames ) override;

    // virtual functions from QgsVectorTileLabelProvider
    void registerTileFeatures( const QgsVectorTileRendererData &tile, QgsRenderContext &context ) override;
    QMap<QString, QSet<QString> > usedAttributes( const QgsRenderContext &context, int tileZoom ) const override;
    QSet< QString > requiredLayers( QgsRenderContext &context, int tileZoom ) const override;
    void setFields( const QMap<QString, QgsFields> &perLayerFields ) override;

  private:
    QList<QgsVectorTileBasicLabelingStyle> mStyles;

    //! Label providers - one for each style
    QList<QgsVectorLayerLabelProvider *> mSubProviders;

  public:
    //! Names of required fields for each sub-layer (only valid between startRender/stopRender calls)
    QMap<QString, QgsFields> mPerLayerFields;
};

#endif


#endif // QGSVECTORTILEBASICLABELING_H
