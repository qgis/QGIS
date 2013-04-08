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

class QgsSymbolV2;
class QgsComposerGroupItem;
class QgsComposerLayerItem;
class QgsComposerMap;

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
    void setTitle( const QString& t ) {mTitle = t;}
    QString title() const {return mTitle;}

    /** Returns reference to modifiable style */
    QgsComposerLegendStyle & rstyle( QgsComposerLegendStyle::Style s ) { return mStyleMap[s]; }
    /** Returns style */
    QgsComposerLegendStyle style( QgsComposerLegendStyle::Style s ) const { return mStyleMap.value( s ); }
    void setStyle( QgsComposerLegendStyle::Style s, const QgsComposerLegendStyle style ) { mStyleMap[s] = style; }

    QFont styleFont( QgsComposerLegendStyle::Style s ) const { return style( s ).font(); }
    /** Set style font */
    void setStyleFont( QgsComposerLegendStyle::Style s, const QFont& f );

    /** Set style margin*/
    void setStyleMargin( QgsComposerLegendStyle::Style s, double margin );
    void setStyleMargin( QgsComposerLegendStyle::Style s, QgsComposerLegendStyle::Side side, double margin );

    double boxSpace() const {return mBoxSpace;}
    void setBoxSpace( double s ) {mBoxSpace = s;}

    double columnSpace() const {return mColumnSpace;}
    void setColumnSpace( double s ) { mColumnSpace = s;}

    QColor fontColor() const {return mFontColor;}
    void setFontColor( const QColor& c ) {mFontColor = c;}

    double symbolWidth() const {return mSymbolWidth;}
    void setSymbolWidth( double w ) {mSymbolWidth = w;}

    double symbolHeight() const {return mSymbolHeight;}
    void setSymbolHeight( double h ) {mSymbolHeight = h;}

    void setWrapChar( const QString& t ) {mWrapChar = t;}
    QString wrapChar() const {return mWrapChar;}

    int columnCount() const { return mColumnCount; }
    void setColumnCount( int c ) { mColumnCount = c;}

    int splitLayer() const { return mSplitLayer; }
    void setSplitLayer( bool s ) { mSplitLayer = s;}

    int equalColumnWidth() const { return mEqualColumnWidth; }
    void setEqualColumnWidth( bool s ) { mEqualColumnWidth = s;}

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

  public slots:
    /**Data changed*/
    void synchronizeWithModel();
    /**Sets mCompositionMap to 0 if the map is deleted*/
    void invalidateCurrentMap();

  protected:
    QString mTitle;
    QString mWrapChar;

    QColor mFontColor;

    /**Space between item box and contents*/
    qreal mBoxSpace;
    /**Space between columns*/
    double mColumnSpace;

    /**Width of symbol icon*/
    double mSymbolWidth;
    /**Height of symbol icon*/
    double mSymbolHeight;

    /** Spacing between lines when wrapped */
    double mlineSpacing;

    /** Number of legend columns */
    int mColumnCount;

    QgsLegendModel mLegendModel;

    /**Reference to map (because symbols are sometimes in map units)*/
    const QgsComposerMap* mComposerMap;

    /** Allow splitting layers into multiple columns */
    bool mSplitLayer;

    /** Use the same width (maximum) for all columns */
    bool mEqualColumnWidth;

  private:
    /** Nucleon is either group title, layer title or layer child item.
     *  Nucleon is similar to QgsComposerLegendItem but it does not have
     *  the same hierarchy. E.g. layer title nucleon is just title, it does not
     *  include all layer subitems, the same with groups.
     */
    class Nucleon
    {
      public:
        QgsComposerLegendItem* item;
        // Symbol size size without any space around for symbol item
        QSizeF symbolSize;
        // Label size without any space around for symbol item
        QSizeF labelSize;
        QSizeF size;
        // Offset of symbol label, this offset is the same for all symbol labels
        // of the same layer in the same column
        double labelXOffset;
    };

    /** Atom is indivisible set (indivisible into more columns). It may consists
     *  of one or more Nucleon, depending on layer splitting mode:
     *  1) no layer split: [group_title ...] layer_title layer_item [layer_item ...]
     *  2) layer split:    [group_title ...] layer_title layer_item
     *              or:    layer_item
     *  It means that group titles must not be split from layer title and layer title
     *  must not be split from first item, because it would look bad and it would not
     *  be readable to leave group or layer title at the bottom of column.
     */
    class Atom
    {
      public:
        Atom(): size( QSizeF( 0, 0 ) ), column( 0 ) {}
        QList<Nucleon> nucleons;
        // Atom size including nucleons interspaces but without any space around atom.
        QSizeF size;
        int column;
    };

    /** Create list of atoms according to current layer splitting mode */
    QList<Atom> createAtomList( QStandardItem* rootItem, bool splitLayer );

    /** Divide atoms to columns and set columns on atoms */
    void setColumns( QList<Atom>& atomList );

    QgsComposerLegend(); //forbidden

    QSizeF drawTitle( QPainter* painter = 0, QPointF point = QPointF(), Qt::AlignmentFlag halignment = Qt::AlignLeft );

    /**Draws a group item and all subitems
     * Returns list of sizes of layers and groups including this group.
     */
    QSizeF drawGroupItemTitle( QgsComposerGroupItem* groupItem, QPainter* painter = 0, QPointF point = QPointF() );
    /**Draws a layer item and all subitems*/
    QSizeF drawLayerItemTitle( QgsComposerLayerItem* layerItem, QPainter* painter = 0, QPointF point = QPointF() );

    Nucleon drawSymbolItem( QgsComposerLegendItem* symbolItem, QPainter* painter = 0, QPointF point = QPointF(), double labelXOffset = 0. );

    /**Draws a symbol at the current y position and returns the new x position. Returns real symbol height, because for points,
     it is possible that it differs from mSymbolHeight*/
    void drawSymbolV2( QPainter* p, QgsSymbolV2* s, double currentYCoord, double& currentXPosition, double& symbolHeight ) const;

    /** Draw atom and return its actual size */
    QSizeF drawAtom( Atom atom, QPainter* painter = 0, QPointF point = QPointF() );

    double spaceAboveAtom( Atom atom );

    /**Helper function that lists ids of layers contained in map canvas*/
    QStringList layerIdList() const;

    /** Splits a string using the wrap char taking into account handling empty
      wrap char which means no wrapping */
    QStringList splitStringForWrapping( QString stringToSplt );

    QMap<QgsComposerLegendStyle::Style, QgsComposerLegendStyle> mStyleMap;
};

#endif
