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
#include <QObject>

class QgsSymbol;

/** \ingroup MapComposer
 * A legend that can be placed onto a map composition
 */
class CORE_EXPORT QgsComposerLegend: public QObject, public QgsComposerItem
{
    Q_OBJECT

  public:
    QgsComposerLegend( QgsComposition* composition );
    ~QgsComposerLegend();

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

    QFont layerFont() const;
    void setLayerFont( const QFont& f );

    QFont itemFont() const;
    void setItemFont( const QFont& f );

    double boxSpace() const {return mBoxSpace;}
    void setBoxSpace( double s ) {mBoxSpace = s;}

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

    /**Updates the model and all legend entries*/
    void updateLegend();

    /** stores state in Dom node
       * @param elem is Dom element corresponding to 'Composer' tag
       * @param temp write template file
       */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
       * @param itemElem is Dom node corresponding to item tag
       */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

  public slots:
    /**Data changed*/
    void synchronizeWithModel();

  protected:
    QString mTitle;

    //different fonts for entries
    QFont mTitleFont;
    QFont mLayerFont;
    QFont mItemFont;

    /**Space between item box and contents*/
    double mBoxSpace;
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

    QgsLegendModel mLegendModel;


  private:
    QgsComposerLegend(); //forbidden

    /**Draws child items of a layer item
       @param layerItem parent model item (layer)
       @param currentYCoord in/out: current y position of legend item
       @param maxXCoord in/out: maximum x-coordinate of the whole legend
       @param layerOpacity opacity of the corresponding map layer
    */
    void drawLayerChildItems( QPainter* p, QStandardItem* layerItem, double& currentYCoord, double& maxXCoord, int layerOpacity = 255 );

    /**Draws a symbol at the current y position and returns the new x position. Returns real symbol height, because for points,
     it is possible that it differs from mSymbolHeight*/
    void drawSymbol( QPainter* p, QgsSymbol* s, double currentYCoord, double& currentXPosition, double& symbolHeight, int layerOpacity = 255 ) const;
    void drawPointSymbol( QPainter*, QgsSymbol* s, double currentYCoord, double& currentXPosition, double& symbolHeight, int opacity = 255 ) const;
    void drawLineSymbol( QPainter*, QgsSymbol* s, double currentYCoord, double& currentXPosition, int opacity = 255 ) const;
    void drawPolygonSymbol( QPainter* p, QgsSymbol* s, double currentYCoord, double& currentXPosition, int opacity = 255 ) const;

    /**Helper function that lists ids of layers contained in map canvas*/
    QStringList layerIdList() const;
};

#endif
