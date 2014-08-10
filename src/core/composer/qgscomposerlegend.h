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
#include "qgslegendmodel.h"
#include "qgslegendsettings.h"

class QgsSymbolV2;
class QgsComposerGroupItem;
class QgsComposerLayerItem;
class QgsComposerMap;
class QgsLegendRenderer;


/** \ingroup MapComposer
 * A legend that can be placed onto a map composition
 */
class CORE_EXPORT QgsComposerLegend : public QgsComposerItem
{
    Q_OBJECT;

  public:
    QgsComposerLegend( QgsComposition* composition );
    ~QgsComposerLegend();

    /** return correct graphics item type. Added in v1.7 */
    virtual int type() const { return ComposerLegend; }

    /** \brief Reimplementation of QCanvasItem::paint*/
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    /**Paints the legend and calculates its size. If painter is 0, only size is calculated*/
    QSizeF paintAndDetermineSize( QPainter* painter );

    /**Sets item box to the whole content*/
    void adjustBoxSize();

    /**Returns pointer to the legend model*/
    QgsLegendModel* model() {return &mLegendModel;}

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
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
       * @param itemElem is Dom node corresponding to item tag
       * @param doc is Dom document
       */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    //Overriden to show legend title
    virtual QString displayName() const;

  public slots:
    /**Data changed*/
    void synchronizeWithModel();
    /**Sets mCompositionMap to 0 if the map is deleted*/
    void invalidateCurrentMap();

  private:
    QgsComposerLegend(); //forbidden

    QgsLegendModel mLegendModel;

    QgsLegendSettings mSettings;

    const QgsComposerMap* mComposerMap;
};

#endif

