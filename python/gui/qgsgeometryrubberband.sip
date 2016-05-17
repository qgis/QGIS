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

/** A rubberband class for QgsAbstractGeometryV2 (considering curved geometries)*/
class QgsGeometryRubberBand: QgsMapCanvasItem
{
%TypeHeaderCode
#include "qgsgeometryrubberband.h"
%End
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
    /** Returns a pointer to the geometry*/
    const QgsAbstractGeometryV2* geometry();
    /** Moves vertex to new position (in map coordinates)*/
    void moveVertex( QgsVertexId id, const QgsPointV2& newPos );
    /** Sets fill color for vertex markers*/
    void setFillColor( const QColor& c );
    /** Sets outline color for vertex markes*/
    void setOutlineColor( const QColor& c );
    /** Sets outline width*/
    void setOutlineWidth( int width );
    /** Sets pen style*/
    void setLineStyle( Qt::PenStyle penStyle );
    /** Sets brush style*/
    void setBrushStyle( Qt::BrushStyle brushStyle );
    /** Sets vertex marker icon type*/
    void setIconType( IconType iconType );

  protected:
    virtual void paint( QPainter* painter );
};
