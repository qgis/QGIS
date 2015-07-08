/***************************************************************************
                         qgsgeometryrubberband.h
                         -----------------------
    begin                : December 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOMETRYRUBBERBAND_H
#define QGSGEOMETRYRUBBERBAND_H

#include "qgsmapcanvasitem.h"
#include <QBrush>
#include <QPen>


class QgsAbstractGeometryV2;
class QgsPointV2;
struct QgsVertexId;

class GUI_EXPORT QgsGeometryRubberBand: public QgsMapCanvasItem
{
  public:
    enum IconType
    {
      /**
      * No icon is used
      */
      ICON_NONE,
      /**
       * A cross is used to highlight points (+)
       */
      ICON_CROSS,
      /**
       * A cross is used to highlight points (x)
       */
      ICON_X,
      /**
       * A box is used to highlight points (□)
       */
      ICON_BOX,
      /**
       * A circle is used to highlight points (○)
       */
      ICON_CIRCLE,
      /**
       * A full box is used to highlight points (■)
       */
      ICON_FULL_BOX
    };

    QgsGeometryRubberBand( QgsMapCanvas* mapCanvas, QGis::GeometryType geomType = QGis::Line );
    ~QgsGeometryRubberBand();

    /** Sets geometry (takes ownership). Geometry is expected to be in map coordinates */
    void setGeometry( QgsAbstractGeometryV2* geom );
    const QgsAbstractGeometryV2* geometry() { return mGeometry; }

    void moveVertex( const QgsVertexId& id, const QgsPointV2& newPos );

    void setFillColor( const QColor& c );
    void setOutlineColor( const QColor& c );
    void setOutlineWidth( int width );
    void setLineStyle( Qt::PenStyle penStyle );
    void setBrushStyle( Qt::BrushStyle brushStyle );
    void setIconType( IconType iconType ) { mIconType = iconType; }

  protected:
    virtual void paint( QPainter* painter );

  private:
    QgsAbstractGeometryV2* mGeometry;
    QBrush mBrush;
    QPen mPen;
    int mIconSize;
    IconType mIconType;
    QGis::GeometryType mGeometryType;

    void drawVertex( QPainter* p, double x, double y );
    QgsRectangle rubberBandRectangle() const;
};

#endif // QGSGEOMETRYRUBBERBAND_H
