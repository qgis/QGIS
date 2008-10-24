/***************************************************************************
                         qgscomposeritem.h
                             -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOMPOSERITEM_H
#define QGSCOMPOSERITEM_H

#include "qgscomposition.h"
#include <QGraphicsRectItem>

class QWidget;
class QDomDocument;
class QDomElement;

class QqsComposition;

/** \ingroup MapComposer
 * A item that forms part of a map composition.
 */
class CORE_EXPORT QgsComposerItem: public QGraphicsRectItem
{

  public:

    /**Describes the action (move or resize in different directon) to be done during mouse move*/
    enum mouseMoveAction
    {
      moveItem,
      resizeUp,
      resizeDown,
      resizeLeft,
      resizeRight,
      resizeDLeftUp,
      resizeDRightUp,
      resizeDLeftDown,
      resizeDRightDown
    };

    QgsComposerItem( QgsComposition* composition );
    QgsComposerItem( qreal x, qreal y, qreal width, qreal height, QgsComposition* composition );
    virtual ~QgsComposerItem();

    /** \brief Set selected, selected item should be highlighted */
    virtual void setSelected( bool s );

    /** \brief Is selected */
    virtual bool selected( void ) {return QGraphicsRectItem::isSelected();}

    /** stores state in project */
    virtual bool writeSettings( void );

    /** read state from project */
    virtual bool readSettings( void );

    /** delete settings from project file  */
    virtual bool removeSettings( void );

    /**Moves item in canvas coordinates*/
    void move( double dx, double dy );

    /**Move Content of item. Does nothing per default (but implemented in composer map)
       @param dx move in x-direction (canvas coordinates)
       @param dy move in y-direction(canvas coordinates)*/
    virtual void moveContent( double dx, double dy ) {}

    /**Zoom content of item. Does nothing per default (but implemented in composer map)
     @param delta value from wheel event that describes magnitude and direction (positive /negative number)
    @param x x-position of mouse cursor (in item coordinates)
    @param y y-position of mouse cursor (in item coordinates)*/
    virtual void zoomContent( int delta, double x, double y ) {}

    /**Sets this items bound in scene coordinates such that 1 item size units
     corresponds to 1 scene size unit*/
    virtual void setSceneRect( const QRectF& rectangle );

    /** stores state in Dom node
     * @param node is Dom node corresponding to 'Composer' tag
     * @param temp write template file
     */
    virtual bool writeXML( QDomElement& elem, QDomDocument & doc ) = 0;

    /**Writes parameter that are not subclass specific in document. Usually called from writeXML methods of subclasses*/
    bool _writeXML( QDomElement& itemElem, QDomDocument& doc );

    /** sets state from Dom document
     * @param itemElem is Dom node corresponding to item tag
     */
    virtual bool readXML( const QDomElement& itemElem, const QDomDocument& doc ) = 0;

    /**Reads parameter that are not subclass specific in document. Usually called from readXML methods of subclasses*/
    bool _readXML( const QDomElement& itemElem, const QDomDocument& doc );



    bool frame() const {return mFrame;}
    void setFrame( bool drawFrame ) {mFrame = drawFrame;}

    /**Composite operations for item groups do nothing per default*/
    virtual void addItem( QgsComposerItem* item ) {}
    virtual void removeItems() {}

    const QgsComposition* composition() const {return mComposition;}

    //functions that encapsulate the workaround for the Qt font bug (that is to scale the font size up and then scale the
    //painter down by the same factor for drawing

    /**Draws Text. Takes care about all the composer specific issues (calculation to pixel, scaling of font and painter
     to work arount the Qt font bug)*/
    void drawText( QPainter* p, int x, int y, const QString& text, const QFont& font ) const;

    /**Like the above, but with a rectangle for multiline text*/
    void drawText( QPainter* p, const QRectF& rect, const QString& text, const QFont& font ) const;

    /**Returns the font width in Millimeters (considers upscaling and downscaling with FONT_WORKAROUND_SCALE*/
    double textWidthMillimeters( const QFont& font, const QString& text ) const;

    /**Returns the font ascent in Millimeters (considers upscaling and downscaling with FONT_WORKAROUND_SCALE*/
    double fontAscentMillimeters( const QFont& font ) const;

    /**Calculates font to from point size to pixel size*/
    double pixelFontSize( double pointSize ) const;

    /**Returns a font where size is in pixel and font size is upscaled with FONT_WORKAROUND_SCALE*/
    QFont scaledFontPixelSize( const QFont& font ) const;

  protected:

    QgsComposition* mComposition;

    QgsComposerItem::mouseMoveAction mCurrentMouseMoveAction;
    /**Start point of the last mouse move action (in scene coordinates)*/
    QPointF mMouseMoveStartPos;
    /**Position of the last mouse move event (in item coordinates)*/
    QPointF mLastMouseEventPos;

    /**Rectangle used during move and resize actions*/
    QGraphicsRectItem* mBoundingResizeRectangle;

    /**True if item fram needs to be painted*/
    bool mFrame;

    //event handlers
    virtual void mouseMoveEvent( QGraphicsSceneMouseEvent * event );
    virtual void mousePressEvent( QGraphicsSceneMouseEvent * event );
    virtual void mouseReleaseEvent( QGraphicsSceneMouseEvent * event );

    virtual void hoverMoveEvent( QGraphicsSceneHoverEvent * event );

    /**Finds out the appropriate cursor for the current mouse position in the widget (e.g. move in the middle, resize at border)*/
    Qt::CursorShape cursorForPosition( const QPointF& itemCoordPos );

    /**Finds out which mouse move action to choose depending on the cursor position inside the widget*/
    QgsComposerItem::mouseMoveAction mouseMoveActionForPosition( const QPointF& itemCoordPos );

    /**Calculate rectangle changes according to mouse move (dx, dy) and the current mouse move action
       @param dx x-coordinate move of cursor
       @param dy y-coordinate move of cursor
       @param mx out: rectangle should be moved by mx in x-direction
       @param my out: rectangle should be moved by my in y-direction
       @param rx out: width of rectangle should be resized by rx
       @param ry out: height of rectangle should be resized by ry*/
    void rectangleChange( double dx, double dy, double& mx, double& my, double& rx, double& ry ) const;

    /**Draw selection boxes around item*/
    virtual void drawSelectionBoxes( QPainter* p );

    /**Draw black frame around item*/
    virtual void drawFrame( QPainter* p );

    /**Draw background*/
    virtual void drawBackground( QPainter* p );
};

#endif
