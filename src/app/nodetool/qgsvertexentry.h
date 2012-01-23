/***************************************************************************
    qgsvertexentry.h  - entry for vertex of nodetool
    ---------------------
    begin                : April 2009
    copyright            : (C) 2009 by Richard Kostecky
    email                : csf dot kostej at mail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVERTEXENTRY_H
#define QGSVERTEXENTRY_H

#include <qgspoint.h>
#include <qgsvertexmarker.h>
#include <qgsmapcanvas.h>
#include <qgsmaplayer.h>

class QgsVertexEntry
{
    bool mSelected;
    QgsPoint mPoint;
    int mEquals;
    bool mInRubberBand;
    int mRubberBandNr;
    int mRubberBandIndex;
    int mOriginalIndex;
    int mPenWidth;
    QString mToolTip;
    QgsVertexMarker::IconType mType;
    QgsVertexMarker *mMarker;
    QgsMapCanvas *mCanvas;
    QgsMapLayer *mLayer;

  public:
    QgsVertexEntry( QgsMapCanvas *canvas,
                    QgsMapLayer *layer,
                    QgsPoint p,
                    int originalIndex,
                    QString tooltip = QString::null,
                    QgsVertexMarker::IconType type = QgsVertexMarker::ICON_BOX,
                    int penWidth = 2 );
    ~QgsVertexEntry();

    QgsPoint point() const { return mPoint; }
    int equals() const { return mEquals; }
    bool isSelected() const { return mSelected; }
    bool isInRubberBand() const { return mInRubberBand; }

    void setCenter( QgsPoint p );
    void moveCenter( double x, double y );
    void setEqual( int index ) { mEquals = index; }
    void setSelected( bool selected = true );
    void setInRubberBand( bool inRubberBand = true ) { mInRubberBand = inRubberBand; }
    int rubberBandNr() const { return mRubberBandNr; }
    int rubberBandIndex() { return mRubberBandIndex; }

    void setRubberBandValues( bool inRubberBand, int rubberBandNr, int indexInRubberBand );
    void update();
};

#endif
