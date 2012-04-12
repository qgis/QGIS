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

#include "qgscomposeritem.h"
#include "qgslegendmodel.h"

class QgsSymbol;
class QgsSymbolV2;
class QgsComposerGroupItem;
class QgsComposerLayerItem;
class QgsComposerMap;

/** \ingroup MapComposer
 * A legend that can be placed onto a map composition
 */
class CORE_EXPORT QgsComposerLegend: public QgsComposerItem
{
    Q_OBJECT

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

    QFont titleFont() const;
    void setTitleFont( const QFont& f );

    QFont groupFont() const;
    void setGroupFont( const QFont& f );

    QFont layerFont() const;
    void setLayerFont( const QFont& f );

    QFont itemFont() const;
    void setItemFont( const QFont& f );

    double boxSpace() const {return mBoxSpace;}
    void setBoxSpace( double s ) {mBoxSpace = s;}

    double groupSpace() const {return mGroupSpace;}
    void setGroupSpace( double s ) {mGroupSpace = s;}

    double layerSpace() const {return mLayerSpace;}
    void setLayerSpace( double s ) {mLayerSpace = s;}

    double symbolSpace() const {return mSymbolSpace;}
    void setSymbolSpace( double s ) {mSymbolSpace = s;}

    double iconLabelSpace() const {return mIconLabelSpace;}
    void setIconLabelSpace( double s ) {mIconLabelSpace = s;}

    double symbolWidth() const {return mSymbolWidth;}
    void setSymbolWidth( double w ) {mSymbolWidth = w;}

    double symbolHeight() const {return mSymbolHeight;}
    void setSymbolHeight( double h ) {mSymbolHeight = h;}

    void setWrapChar( const QString& t ) {mWrapChar = t;}
    QString wrapChar() const {return mWrapChar;}

    void setComposerMap( const QgsComposerMap* map );
    const QgsComposerMap* composerMap() const { return mComposerMap; }

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

    //different fonts for entries
    QFont mTitleFont;
    QFont mGroupFont;
    QFont mLayerFont;
    QFont mItemFont;

    /**Space between item box and contents*/
    double mBoxSpace;
    /**Vertical space between group entries*/
    double mGroupSpace;
    /**Vertical space between layer entries*/
    double mLayerSpace;
    /**Vertical space between symbol entries*/
    double mSymbolSpace;
    /**Horizontal space between item icon and label*/
    double mIconLabelSpace;
    /**Width of symbol icon*/
    double mSymbolWidth;
    /**Height of symbol icon*/
    double mSymbolHeight;

    /** Spacing between lines when wrapped */
    double mlineSpacing;

    QgsLegendModel mLegendModel;

    /**Reference to map (because symbols are sometimes in map units)*/
    const QgsComposerMap* mComposerMap;


  private:
    QgsComposerLegend(); //forbidden

    /**Draws a group item and all subitems*/
    void drawGroupItem( QPainter* p, QgsComposerGroupItem* groupItem, double& currentYCoord, double& maxXCoord );
    /**Draws a layer item and all subitems*/
    void drawLayerItem( QPainter* p, QgsComposerLayerItem* layerItem, double& currentYCoord, double& maxXCoord );

    /**Draws child items of a layer item
       @param p painter
       @param layerItem parent model item (layer)
       @param currentYCoord in/out: current y position of legend item
       @param maxXCoord in/out: maximum x-coordinate of the whole legend
       @param layerOpacity opacity of the corresponding map layer
    */
    void drawLayerChildItems( QPainter* p, QStandardItem* layerItem, double& currentYCoord, double& maxXCoord, int layerOpacity = 255 );

    /**Draws a symbol at the current y position and returns the new x position. Returns real symbol height, because for points,
     it is possible that it differs from mSymbolHeight*/
    void drawSymbol( QPainter* p, QgsSymbol* s, double currentYCoord, double& currentXPosition, double& symbolHeight, int layerOpacity = 255 ) const;
    void drawSymbolV2( QPainter* p, QgsSymbolV2* s, double currentYCoord, double& currentXPosition, double& symbolHeight, int layerOpacity = 255 ) const;
    void drawPointSymbol( QPainter*, QgsSymbol* s, double currentYCoord, double& currentXPosition, double& symbolHeight, int opacity = 255 ) const;
    void drawLineSymbol( QPainter*, QgsSymbol* s, double currentYCoord, double& currentXPosition, int opacity = 255 ) const;
    void drawPolygonSymbol( QPainter* p, QgsSymbol* s, double currentYCoord, double& currentXPosition, int opacity = 255 ) const;

    /**Helper function that lists ids of layers contained in map canvas*/
    QStringList layerIdList() const;

  private:
    /** Splits a string using the the wrap char taking into account handling empty
      wrap char which means no wrapping */
    QStringList splitStringForWrapping( QString stringToSplt );
};

#endif
