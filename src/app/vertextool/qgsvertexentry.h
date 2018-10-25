/***************************************************************************
    qgsvertexentry.h  - entry for vertex of vertextool
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

#include "qgspoint.h"
#include "qgsvertexmarker.h"

class QgsMapLayer;
class QgsMapCanvas;

class QgsVertexEntry
{
    bool mSelected;
    QgsPoint mPoint;
    QgsVertexId mVertexId;
    int mPenWidth;
    QString mToolTip;
    QgsVertexMarker::IconType mType;
    QgsVertexMarker *mMarker = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMapLayer *mLayer = nullptr;

  public:
    QgsVertexEntry( QgsMapCanvas *canvas,
                    QgsMapLayer *layer,
                    const QgsPoint &p,
                    QgsVertexId vertexId,
                    const QString &tooltip = QString(),
                    QgsVertexMarker::IconType type = QgsVertexMarker::ICON_BOX,
                    int penWidth = 2 );
    ~QgsVertexEntry();

    QgsVertexEntry( const QgsVertexEntry &rh ) = delete;
    QgsVertexEntry &operator=( const QgsVertexEntry &rh ) = delete;

    const QgsPoint &point() const { return mPoint; }
    QgsPointXY pointV1() const { return QgsPointXY( mPoint.x(), mPoint.y() ); }
    QgsVertexId vertexId() const { return mVertexId; }
    bool isSelected() const { return mSelected; }

    void placeMarker();

    void setSelected( bool selected = true );

};

#endif
