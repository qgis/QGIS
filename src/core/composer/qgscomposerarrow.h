/***************************************************************************
                         qgscomposerarrow.h
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

#ifndef QGSCOMPOSERARROW_H
#define QGSCOMPOSERARROW_H

#include "qgscomposeritem.h"

/**An item that draws an arrow between to points*/
class QgsComposerArrow: public QgsComposerItem
{
  public:
    QgsComposerArrow( QgsComposition* c );
    QgsComposerArrow( const QPointF& startPoint, const QPointF& stopPoint, QgsComposition* c );
    ~QgsComposerArrow();

    /** \brief Reimplementation of QCanvasItem::paint - draw on canvas */
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    /**Modifies position of start and endpoint and calls QgsComposerItem::setSceneRect*/
    void setSceneRect( const QRectF& rectangle );

    /**Sets the width of the arrow head in mm*/
    void setArrowHeadWidth( double width );
    double arrowHeadWidth() const {return mArrowHeadWidth;}

    void setOutlineWidth( double width );
    double outlineWidth() const {return mPen.widthF();}

    void setStartMarker( const QString& svgPath );
    void setEndMarker( const QString& svgPath );

    bool showArrowMarker() const { return mShowArrowMarker;}
    void setShowArrowMarker( bool show );

    QColor arrowColor() const { return mArrowColor; }
    void setArrowColor( const QColor& c ) { mArrowColor = c; }

    /** stores state in Dom node
    * @param node is Dom node corresponding to 'Composer' tag
    * @param temp write template file
    */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
    * @param itemElem is Dom node corresponding to item tag
    */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

  private:

    enum MarkerType
    {
      StartMarker,
      EndMarker
    };

    QPointF mStartPoint;
    QPointF mStopPoint;

    QPen mPen;
    QBrush mBrush;

    /**Width of the arrow marker in mm. May be specified by the user. The height is automatically adapted*/
    double mArrowHeadWidth;
    /**Height of the arrow marker in mm. Is calculated from arrow marker width and apsect ratio of svg*/
    double mStartArrowHeadHeight;
    double mStopArrowHeadHeight;

    /**Path to the start marker file*/
    QString mStartMarkerFile;
    /**Path to the end marker file*/
    QString mEndMarkerFile;

    /**True if arrow head marker is drawn*/
    bool mShowArrowMarker;
    QColor mArrowColor;

    /**Adapts the item scene rect to contain the start point, the stop point including the arrow marker and the outline.
        Needs to be called whenever the arrow width/height, the outline with or the endpoints are changed*/
    void adaptItemSceneRect();

    void drawHardcodedMarker( QPainter* p, MarkerType type );
    void drawSVGMarker( QPainter* p, MarkerType type, const QString& markerPath );
    /**Calculates arrow angle (for marker rotation)*/
    double arrowAngle() const;
    /**Apply default graphics settings*/
    void initGraphicsSettings();
};

#endif // QGSCOMPOSERARROW_H
