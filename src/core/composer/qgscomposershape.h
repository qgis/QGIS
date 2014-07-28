/***************************************************************************
                         qgscomposershape.h
                         ----------------------
    begin                : November 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco@hugis.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERSHAPE_H
#define QGSCOMPOSERSHAPE_H

#include "qgscomposeritem.h"
#include <QBrush>
#include <QPen>

class QgsFillSymbolV2;

/**A composer items that draws common shapes (ellipse, triangle, rectangle)*/
class CORE_EXPORT QgsComposerShape: public QgsComposerItem
{
    Q_OBJECT
  public:

    enum Shape
    {
      Ellipse,
      Rectangle,
      Triangle
    };

    QgsComposerShape( QgsComposition* composition );
    QgsComposerShape( qreal x, qreal y, qreal width, qreal height, QgsComposition* composition );
    ~QgsComposerShape();

    /** return correct graphics item type. Added in v1.7 */
    virtual int type() const { return ComposerShape; }

    /** \brief Reimplementation of QCanvasItem::paint - draw on canvas */
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    /** stores state in Dom element
     * @param elem is Dom element corresponding to 'Composer' tag
     * @param doc write template file
     */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
     * @param itemElem is Dom node corresponding to item tag
     * @param doc is Dom document
     */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    //setters and getters
    QgsComposerShape::Shape shapeType() const { return mShape; }
    void setShapeType( QgsComposerShape::Shape s );

    /**Sets radius for rounded rectangle corners. Added in v2.1 */
    void setCornerRadius( double radius );
    /**Returns the radius for rounded rectangle corners*/
    double cornerRadius() const { return mCornerRadius; };

    /**Sets the QgsFillSymbolV2 used to draw the shape. Must also call setUseSymbolV2( true ) to
     * enable drawing with a symbol.
     * Note: added in version 2.1*/
    void setShapeStyleSymbol( QgsFillSymbolV2* symbol );
    /**Returns the QgsFillSymbolV2 used to draw the shape.
     * Note: added in version 2.1*/
    QgsFillSymbolV2* shapeStyleSymbol() { return mShapeStyleSymbol; }

    /**Controls whether the shape should be drawn using a QgsFillSymbolV2.
     * Note: Added in v2.1 */
    void setUseSymbolV2( bool useSymbolV2 );

    /**Depending on the symbol style, the bounding rectangle can be larger than the shape
    @note this function was added in version 2.3*/
    QRectF boundingRect() const;

    /**Sets new scene rectangle bounds and recalculates hight and extent. Reimplemented from
     * QgsComposerItem as it needs to call updateBoundingRect after the shape's size changes
    */
    void setSceneRect( const QRectF& rectangle );

    //Overriden to return shape type
    virtual QString displayName() const;

  protected:
    /* reimplement drawFrame, since it's not a rect, but a custom shape */
    virtual void drawFrame( QPainter* p );
    /* reimplement drawBackground, since it's not a rect, but a custom shape */
    virtual void drawBackground( QPainter* p );
    /**reimplement estimatedFrameBleed, since frames on shapes are drawn using symbology
     * rather than the item's pen */
    virtual double estimatedFrameBleed() const;

  public slots:
    /**Should be called after the shape's symbol is changed. Redraws the shape and recalculates
     * its selection bounds.
     * Note: added in version 2.1*/
    void refreshSymbol();

  private:
    /**Ellipse, rectangle or triangle*/
    Shape mShape;

    double mCornerRadius;

    bool mUseSymbolV2;

    QgsFillSymbolV2* mShapeStyleSymbol;
    double mMaxSymbolBleed;
    /**Current bounding rectangle of shape*/
    QRectF mCurrentRectangle;

    /* draws the custom shape */
    void drawShape( QPainter* p );

    /* draws the custom shape using symbol v2*/
    void drawShapeUsingSymbol( QPainter* p );

    /* creates the default shape symbol */
    void createDefaultShapeStyleSymbol();

    /**Returns a point on the line from startPoint to directionPoint that is a certain distance away from the starting point*/
    QPointF pointOnLineWithDistance( const QPointF& startPoint, const QPointF& directionPoint, double distance ) const;

    /**Updates the bounding rect of this item*/
    void updateBoundingRect();
};

#endif // QGSCOMPOSERSHAPEITEM_H
