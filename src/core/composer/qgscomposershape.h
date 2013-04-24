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
    QgsComposerShape::Shape shapeType() const {return mShape;}
    void setShapeType( QgsComposerShape::Shape s ) {mShape = s;}

    /**Sets this items bound in scene coordinates such that 1 item size units
     corresponds to 1 scene size unit. Also, the shape is scaled*/
    void setSceneRect( const QRectF& rectangle );

  public slots:
    /**Sets item rotation and resizes item bounds such that the shape always has the same size*/
    virtual void setRotation( double r );


  protected:
    /* reimplement drawFrame, since it's not a rect, but a custom shape */
    virtual void drawFrame( QPainter* p );
    /* reimplement drawBackground, since it's not a rect, but a custom shape */
    virtual void drawBackground( QPainter* p );


  private:
    /**Ellipse, rectangle or triangle*/
    Shape mShape;

    /* draws the custom shape */
    void drawShape( QPainter* p );


    /**Returns a point on the line from startPoint to directionPoint that is a certain distance away from the starting point*/
    QPointF pointOnLineWithDistance( const QPointF& startPoint, const QPointF& directionPoint, double distance ) const;
};

#endif // QGSCOMPOSERSHAPEITEM_H
