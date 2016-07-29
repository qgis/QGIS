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

#include <qgspointv2.h>
#include <qgsvertexmarker.h>
#include <qgsmapcanvas.h>
#include <qgsmaplayer.h>

class QgsVertexEntry
{
    bool mSelected;
    QgsPointV2 mPoint;
    QgsVertexId mVertexId;
    int mPenWidth;
    QString mToolTip;
    QgsVertexMarker::IconType mType;
    QgsVertexMarker *mMarker;
    QgsMapCanvas *mCanvas;
    QgsMapLayer *mLayer;

  public:
    QgsVertexEntry( QgsMapCanvas *canvas,
                    QgsMapLayer *layer,
                    const QgsPointV2& p,
                    QgsVertexId vertexId,
                    const QString& tooltip = QString::null,
                    QgsVertexMarker::IconType type = QgsVertexMarker::ICON_BOX,
                    int penWidth = 2 );
    ~QgsVertexEntry();

    const QgsPointV2& point() const { return mPoint; }
    QgsPoint pointV1() const { return QgsPoint( mPoint.x(), mPoint.y() ); }
    QgsVertexId vertexId() const { return mVertexId; }
    bool isSelected() const { return mSelected; }

    void placeMarker();

    void setSelected( bool selected = true );

  private:

    QgsVertexEntry( const QgsVertexEntry& rh );
    QgsVertexEntry& operator=( const QgsVertexEntry& rh );
};

#endif
