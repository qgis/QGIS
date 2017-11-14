/***************************************************************************
                         qgscomposerlegend.h  -  description
                         -------------------
    begin                : June 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERLEGEND_H
#define QGSCOMPOSERLEGEND_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgscomposeritem.h"
#include "qgslayertreemodel.h"
#include "qgslegendsettings.h"
#include "qgslayertreegroup.h"


class QgsLayerTreeModel;
class QgsSymbol;
class QgsComposerMap;
class QgsLegendRenderer;
class QgsLegendModel;


/**
 * \ingroup core
 * A legend that can be placed onto a map composition
 */
class CORE_EXPORT QgsComposerLegend : public QgsComposerItem
{
    Q_OBJECT

  public:
    QgsComposerLegend( QgsComposition *composition SIP_TRANSFERTHIS );
    ~QgsComposerLegend();

    //! Return correct graphics item type.
    virtual int type() const override { return ComposerLegend; }

    //! \brief Reimplementation of QCanvasItem::paint
    void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget ) override;

    //! Paints the legend and calculates its size. If painter is 0, only size is calculated
    QSizeF paintAndDetermineSize( QPainter *painter );

    //! Sets item box to the whole content
    void adjustBoxSize();

    /**
     * Sets whether the legend should automatically resize to fit its contents.
     * \param enabled set to false to disable automatic resizing. The legend frame will not
     * be expanded to fit legend items, and items may be cropped from display.
     * \see resizeToContents()
     * \since QGIS 3.0
     */
    void setResizeToContents( bool enabled );

    /**
     * Returns whether the legend should automatically resize to fit its contents.
     * \see setResizeToContents()
     * \since QGIS 3.0
     */
    bool resizeToContents() const;


    /**
     * Returns the legend model
     */
    QgsLegendModel *model();

    //! \since QGIS 2.6
    void setAutoUpdateModel( bool autoUpdate );
    //! \since QGIS 2.6
    bool autoUpdateModel() const;

    /**
     * Set whether legend items should be filtered to show just the ones visible in the associated map
     * \since QGIS 2.6
     */
    void setLegendFilterByMapEnabled( bool enabled );

    /**
     * Find out whether legend items are filtered to show just the ones visible in the associated map
     * \since QGIS 2.6
     */
    bool legendFilterByMapEnabled() const { return mLegendFilterByMap; }

    /**
     * Update() overloading. Use it rather than update()
     * \since QGIS 2.12
     */
    virtual void updateItem() override;

    /**
     * When set to true, during an atlas rendering, it will filter out legend elements
     * where features are outside the current atlas feature.
     * \since QGIS 2.14
     */
    void setLegendFilterOutAtlas( bool doFilter );

    /**
     * Whether to filter out legend elements outside of the current atlas feature
     * \see setLegendFilterOutAtlas()
     * \since QGIS 2.14
     */
    bool legendFilterOutAtlas() const;

    //setters and getters
    void setTitle( const QString &t );
    QString title() const;

    /**
     * Returns the alignment of the legend title
     * \returns Qt::AlignmentFlag for the legend title
     * \since QGIS 2.3
     * \see setTitleAlignment
     */
    Qt::AlignmentFlag titleAlignment() const;

    /**
     * Sets the alignment of the legend title
     * \param alignment Text alignment for drawing the legend title
     * \since QGIS 2.3
     * \see titleAlignment
     */
    void setTitleAlignment( Qt::AlignmentFlag alignment );

    //! Returns reference to modifiable style
    QgsLegendStyle &rstyle( QgsLegendStyle::Style s );
    //! Returns style
    QgsLegendStyle style( QgsLegendStyle::Style s ) const;
    void setStyle( QgsLegendStyle::Style s, const QgsLegendStyle &style );

    QFont styleFont( QgsLegendStyle::Style s ) const;
    //! Set style font
    void setStyleFont( QgsLegendStyle::Style s, const QFont &f );

    //! Set style margin
    void setStyleMargin( QgsLegendStyle::Style s, double margin );
    void setStyleMargin( QgsLegendStyle::Style s, QgsLegendStyle::Side side, double margin );

    /**
     * Returns the spacing in-between lines in mm
     * \since QGIS 3.0
     * \see setLineSpacing
     */
    double lineSpacing() const;

    /**
     * Sets the spacing in-between multiple lines
     * \param spacing Double value to use as spacing in between multiple lines
     * \since QGIS 3.0
     * \see lineSpacing
     */
    void setLineSpacing( double spacing );

    double boxSpace() const;
    void setBoxSpace( double s );

    double columnSpace() const;
    void setColumnSpace( double s );

    QColor fontColor() const;
    void setFontColor( const QColor &c );

    double symbolWidth() const;
    void setSymbolWidth( double w );

    double symbolHeight() const;
    void setSymbolHeight( double h );

    double wmsLegendWidth() const;
    void setWmsLegendWidth( double w );

    double wmsLegendHeight() const;
    void setWmsLegendHeight( double h );

    void setWrapChar( const QString &t );
    QString wrapChar() const;

    int columnCount() const;
    void setColumnCount( int c );

    bool splitLayer() const;
    void setSplitLayer( bool s );

    bool equalColumnWidth() const;
    void setEqualColumnWidth( bool s );

    /**
     * Returns whether a stroke will be drawn around raster symbol items.
     * \see setDrawRasterStroke()
     * \see rasterStrokeColor()
     * \see rasterStrokeWidth()
     * \since QGIS 2.12
     */
    bool drawRasterStroke() const;

    /**
     * Sets whether a stroke will be drawn around raster symbol items.
     * \param enabled set to true to draw borders
     * \see drawRasterStroke()
     * \see setRasterStrokeColor()
     * \see setRasterStrokeWidth()
     * \since QGIS 2.12
     */
    void setDrawRasterStroke( bool enabled );

    /**
     * Returns the stroke color for the stroke drawn around raster symbol items. The stroke is
     * only drawn if drawRasterStroke() is true.
     * \see setRasterStrokeColor()
     * \see drawRasterStroke()
     * \see rasterStrokeWidth()
     * \since QGIS 2.12
     */
    QColor rasterStrokeColor() const;

    /**
     * Sets the stroke color for the stroke drawn around raster symbol items. The stroke is
     * only drawn if drawRasterStroke() is true.
     * \param color stroke color
     * \see rasterStrokeColor()
     * \see setDrawRasterStroke()
     * \see setRasterStrokeWidth()
     * \since QGIS 2.12
     */
    void setRasterStrokeColor( const QColor &color );

    /**
     * Returns the stroke width (in millimeters) for the stroke drawn around raster symbol items. The stroke is
     * only drawn if drawRasterStroke() is true.
     * \see setRasterStrokeWidth()
     * \see drawRasterStroke()
     * \see rasterStrokeColor()
     * \since QGIS 2.12
     */
    double rasterStrokeWidth() const;

    /**
     * Sets the stroke width for the stroke drawn around raster symbol items. The stroke is
     * only drawn if drawRasterStroke() is true.
     * \param width stroke width in millimeters
     * \see rasterStrokeWidth()
     * \see setDrawRasterStroke()
     * \see setRasterStrokeColor()
     * \since QGIS 2.12
     */
    void setRasterStrokeWidth( double width );

    void setComposerMap( const QgsComposerMap *map );
    const QgsComposerMap *composerMap() const { return mComposerMap;}

    //! Updates the model and all legend entries
    void updateLegend();

    /**
     * Stores state in Dom node
       * \param elem is Dom element corresponding to 'Composer' tag
       * \param doc Dom document
       */
    bool writeXml( QDomElement &elem, QDomDocument &doc ) const override;

    /**
     * Sets state from Dom document
       * \param itemElem is Dom node corresponding to item tag
       * \param doc is Dom document
       */
    bool readXml( const QDomElement &itemElem, const QDomDocument &doc ) override;

    //Overridden to show legend title
    virtual QString displayName() const override;

    /**
     * Returns the legend's renderer settings object.
     * \since QGIS 3.0
     */
    const QgsLegendSettings &legendSettings() const { return mSettings; }

  public slots:
    //! Data changed
    void synchronizeWithModel();
    //! Sets mCompositionMap to 0 if the map is deleted
    void invalidateCurrentMap();

    virtual void refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property = QgsComposerObject::AllProperties, const QgsExpressionContext *context = nullptr ) override;


  private slots:

    void updateFilterByMapAndRedraw();

    void updateFilterByMap( bool redraw = true );

    //! update legend in case style of associated map has changed
    void mapLayerStyleOverridesChanged();

    //! react to atlas
    void onAtlasEnded();
    void onAtlasFeature( QgsFeature * );

    void nodeCustomPropertyChanged( QgsLayerTreeNode *node, const QString &key );

  private:
    QgsComposerLegend(); //forbidden

    //! use new custom layer tree and update model. if new root is null pointer, will use project's tree
    void setCustomLayerTree( QgsLayerTree *rootGroup );

    QgsLegendModel *mLegendModel = nullptr;
    std::unique_ptr< QgsLayerTreeGroup > mCustomLayerTree;

    QgsLegendSettings mSettings;

    QString mTitle;
    int mColumnCount = 1;

    const QgsComposerMap *mComposerMap = nullptr;

    bool mLegendFilterByMap = false;
    bool mLegendFilterByExpression = false;

    //! whether to filter out legend elements outside of the atlas feature
    bool mFilterOutAtlas = false;

    //! tag for update request
    bool mFilterAskedForUpdate = false;
    //! actual filter update
    void doUpdateFilterByMap();

    bool mInAtlas = false;

    //! Will be false until the associated map scale and DPI have been calculated
    bool mInitialMapScaleCalculated = false;

    //! Will be true if the legend size should be totally reset at next paint
    bool mForceResize = false;

    //! Will be true if the legend should be resized automatically to fit contents
    bool mSizeToContents = true;
};

#endif

