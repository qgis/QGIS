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

#include "qgscomposerlegendstyle.h"
#include "qgscomposeritem.h"
#include "qgscomposerlegenditem.h"
#include "qgslayertreemodel.h"
#include "qgslegendmodel.h"
#include "qgslegendsettings.h"

class QgsLayerTreeModel;
class QgsSymbolV2;
class QgsComposerGroupItem;
class QgsComposerLayerItem;
class QgsComposerMap;
class QgsLegendRenderer;


/** \ingroup MapComposer
 * Item model implementation based on layer tree model for composer legend.
 * Overrides some functionality of QgsLayerTreeModel to better fit the needs of composer legend.
 *
 * @note added in 2.6
 */
class CORE_EXPORT QgsLegendModelV2 : public QgsLayerTreeModel
{
  public:
    QgsLegendModelV2( QgsLayerTreeGroup* rootNode, QObject *parent = 0 );

    QVariant data( const QModelIndex& index, int role ) const override;

    Qt::ItemFlags flags( const QModelIndex &index ) const override;
};


/** \ingroup MapComposer
 * A legend that can be placed onto a map composition
 */
class CORE_EXPORT QgsComposerLegend : public QgsComposerItem
{
    Q_OBJECT;

  public:
    QgsComposerLegend( QgsComposition* composition );
    ~QgsComposerLegend();

    /** return correct graphics item type. */
    virtual int type() const override { return ComposerLegend; }

    /** \brief Reimplementation of QCanvasItem::paint*/
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget ) override;

    /**Paints the legend and calculates its size. If painter is 0, only size is calculated*/
    QSizeF paintAndDetermineSize( QPainter* painter );

    /**Sets item box to the whole content*/
    void adjustBoxSize();

    /**Returns pointer to the legend model*/
    //! @note deprecated in 2.6 - use modelV2()
    Q_DECL_DEPRECATED QgsLegendModel* model() {return &mLegendModel;}

    //! @note added in 2.6
    QgsLegendModelV2* modelV2() { return mLegendModel2; }

    //! @note added in 2.6
    void setAutoUpdateModel( bool autoUpdate );
    //! @note added in 2.6
    bool autoUpdateModel() const;

    //! Set whether legend items should be filtered to show just the ones visible in the associated map
    //! @note added in 2.6
    void setLegendFilterByMapEnabled( bool enabled );
    //! Find out whether legend items are filtered to show just the ones visible in the associated map
    //! @note added in 2.6
    bool legendFilterByMapEnabled() const { return mLegendFilterByMap; }

    //setters and getters
    void setTitle( const QString& t );
    QString title() const;

    /**Returns the alignment of the legend title
     * @returns Qt::AlignmentFlag for the legend title
     * @note added in 2.3
     * @see setTitleAlignment
    */
    Qt::AlignmentFlag titleAlignment() const;
    /**Sets the alignment of the legend title
     * @param alignment Text alignment for drawing the legend title
     * @note added in 2.3
     * @see titleAlignment
    */
    void setTitleAlignment( Qt::AlignmentFlag alignment );

    /** Returns reference to modifiable style */
    QgsComposerLegendStyle & rstyle( QgsComposerLegendStyle::Style s );
    /** Returns style */
    QgsComposerLegendStyle style( QgsComposerLegendStyle::Style s ) const;
    void setStyle( QgsComposerLegendStyle::Style s, const QgsComposerLegendStyle style );

    QFont styleFont( QgsComposerLegendStyle::Style s ) const;
    /** Set style font */
    void setStyleFont( QgsComposerLegendStyle::Style s, const QFont& f );

    /** Set style margin*/
    void setStyleMargin( QgsComposerLegendStyle::Style s, double margin );
    void setStyleMargin( QgsComposerLegendStyle::Style s, QgsComposerLegendStyle::Side side, double margin );

    double boxSpace() const;
    void setBoxSpace( double s );

    double columnSpace() const;
    void setColumnSpace( double s );

    QColor fontColor() const;
    void setFontColor( const QColor& c );

    double symbolWidth() const;
    void setSymbolWidth( double w );

    double symbolHeight() const;
    void setSymbolHeight( double h );

    double wmsLegendWidth() const;
    void setWmsLegendWidth( double w );

    double wmsLegendHeight() const;
    void setWmsLegendHeight( double h );

    void setWrapChar( const QString& t );
    QString wrapChar() const;

    int columnCount() const;
    void setColumnCount( int c );

    int splitLayer() const;
    void setSplitLayer( bool s );

    int equalColumnWidth() const;
    void setEqualColumnWidth( bool s );

    void setComposerMap( const QgsComposerMap* map );
    const QgsComposerMap* composerMap() const { return mComposerMap;}

    /**Updates the model and all legend entries*/
    void updateLegend();

    /** stores state in Dom node
       * @param elem is Dom element corresponding to 'Composer' tag
       * @param doc Dom document
       */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const override;

    /** sets state from Dom document
       * @param itemElem is Dom node corresponding to item tag
       * @param doc is Dom document
       */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc ) override;

    //Overriden to show legend title
    virtual QString displayName() const override;

  public slots:
    /**Data changed*/
    void synchronizeWithModel();
    /**Sets mCompositionMap to 0 if the map is deleted*/
    void invalidateCurrentMap();

  private slots:
    void updateFilterByMap();

  private:
    QgsComposerLegend(); //forbidden

    //! use new custom layer tree and update model. if new root is null pointer, will use project's tree
    void setCustomLayerTree( QgsLayerTreeGroup* rootGroup );

    QgsLegendModel mLegendModel;

    QgsLegendModelV2* mLegendModel2;
    QgsLayerTreeGroup* mCustomLayerTree;

    QgsLegendSettings mSettings;

    const QgsComposerMap* mComposerMap;

    bool mLegendFilterByMap;
};

#endif

