/***************************************************************************
                              qgsannotationitem.h
                              ------------------------
  begin                : February 9, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco dot hugentobler at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSANNOTATIONITEM_H
#define QGSANNOTATIONITEM_H

#include "qgsmapcanvasitem.h"
#include "qgscoordinatereferencesystem.h"

class QDomDocument;
class QDomElement;
class QDialog;
class QgsVectorLayer;
class QgsMarkerSymbolV2;

/** \ingroup gui
 * An annotation item can be either placed either on screen corrdinates or on map coordinates.
  It may reference a feature and displays that associatiation with a balloon like appearance*/
class GUI_EXPORT QgsAnnotationItem: public QgsMapCanvasItem
{
  public:
    enum MouseMoveAction
    {
      NoAction,
      MoveMapPosition,
      MoveFramePosition,
      ResizeFrameUp,
      ResizeFrameDown,
      ResizeFrameLeft,
      ResizeFrameRight,
      ResizeFrameLeftUp,
      ResizeFrameRightUp,
      ResizeFrameLeftDown,
      ResizeFrameRightDown
    };

    QgsAnnotationItem( QgsMapCanvas* mapCanvas );
    virtual ~QgsAnnotationItem();

    void updatePosition() override;

    QRectF boundingRect() const override;

    virtual QSizeF minimumFrameSize() const;

    /** Returns the mouse move behaviour for a given position
      @param pos the position in scene coordinates*/
    QgsAnnotationItem::MouseMoveAction moveActionForPosition( QPointF pos ) const;
    /** Returns suitable cursor shape for mouse move action*/
    Qt::CursorShape cursorShapeForAction( MouseMoveAction moveAction ) const;

    //setters and getters
    void setMapPositionFixed( bool fixed );
    bool mapPositionFixed() const { return mMapPositionFixed; }

    virtual void setMapPosition( const QgsPoint& pos );
    QgsPoint mapPosition() const { return mMapPosition; }

    /** Sets the CRS of the map position.
      @param crs the CRS to set */
    virtual void setMapPositionCrs( const QgsCoordinateReferenceSystem& crs );
    /** Returns the CRS of the map position.*/
    QgsCoordinateReferenceSystem mapPositionCrs() const { return mMapPositionCrs; }

    void setFrameSize( QSizeF size );
    QSizeF frameSize() const { return mFrameSize; }

    void setOffsetFromReferencePoint( QPointF offset );
    QPointF offsetFromReferencePoint() const { return mOffsetFromReferencePoint; }

    /** Set symbol that is drawn on map position. Takes ownership*/
    void setMarkerSymbol( QgsMarkerSymbolV2* symbol );
    const QgsMarkerSymbolV2* markerSymbol() const {return mMarkerSymbol;}

    void setFrameBorderWidth( double w ) { mFrameBorderWidth = w; }
    double frameBorderWidth() const { return mFrameBorderWidth; }

    void setFrameColor( const QColor& c ) { mFrameColor = c; }
    QColor frameColor() const { return mFrameColor; }

    void setFrameBackgroundColor( const QColor& c ) { mFrameBackgroundColor = c; }
    QColor frameBackgroundColor() const { return mFrameBackgroundColor; }

    virtual void writeXML( QDomDocument& doc ) const = 0;
    virtual void readXML( const QDomDocument& doc, const QDomElement& itemElem ) = 0;

    void _writeXML( QDomDocument& doc, QDomElement& itemElem ) const;
    void _readXML( const QDomDocument& doc, const QDomElement& annotationElem );

  protected:
    /** True: the item stays at the same map position, False: the item stays on same screen position*/
    bool mMapPositionFixed;
    /** Map position (in case mMapPositionFixed is true)*/
    QgsPoint mMapPosition;
    /** CRS of the map position */
    QgsCoordinateReferenceSystem mMapPositionCrs;

    /** Describes the shift of the item content box to the reference point*/
    QPointF mOffsetFromReferencePoint;

    /** Size of the frame (without balloon)*/
    QSizeF mFrameSize;
    /** Bounding rect (including item frame and balloon)*/
    QRectF mBoundingRect;

    /** Point symbol that is to be drawn at the map reference location*/
    QgsMarkerSymbolV2* mMarkerSymbol;
    /** Width of the frame*/
    double mFrameBorderWidth;
    /** Frame / balloon color*/
    QColor mFrameColor;
    QColor mFrameBackgroundColor;

    /** Segment number where the connection to the map point is attached. -1 if no balloon needed (e.g. if point is contained in frame)*/
    int mBalloonSegment;
    /** First segment point for drawing the connection (ccw direction)*/
    QPointF mBalloonSegmentPoint1;
    /** Second segment point for drawing the balloon connection (ccw direction)*/
    QPointF mBalloonSegmentPoint2;

    void updateBoundingRect();
    /** Check where to attach the balloon connection between frame and map point*/
    void updateBalloon();

    void drawFrame( QPainter* p );
    void drawMarkerSymbol( QPainter* p );
    void drawSelectionBoxes( QPainter* p );
    /** Returns frame width in painter units*/
    //double scaledFrameWidth( QPainter* p) const;
    /** Gets the frame line (0 is the top line, 1 right, 2 bottom, 3 left)*/
    QLineF segment( int index );
    /** Returns a point on the line from startPoint to directionPoint that is a certain distance away from the starting point*/
    QPointF pointOnLineWithDistance( QPointF startPoint, QPointF directionPoint, double distance ) const;
    /** Returns the symbol size scaled in (mapcanvas) pixels. Used for the counding rect calculation*/
    double scaledSymbolSize() const;
};

#endif // QGSANNOTATIONITEM_H
