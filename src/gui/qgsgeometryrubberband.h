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
#include "qgswkbtypes.h"
#include <QBrush>
#include <QPen>
#include "qgis_gui.h"

#include "qgscompoundcurve.h"
#include "qgscurvepolygon.h"
#include "qgscircularstring.h"
#include "qgslinestring.h"
#include "qgspoint.h"

#ifdef SIP_RUN
% ModuleHeaderCode
// For ConvertToSubClassCode.
#include <qgsgeometryrubberband.h>
% End
#endif

class QgsAbstractGeometry;
class QgsPoint;
struct QgsVertexId;

/**
 * \ingroup gui
 * \brief A rubberband class for QgsAbstractGeometry (considering curved geometries).
*/
class GUI_EXPORT QgsGeometryRubberBand: public QgsMapCanvasItem
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast<QgsGeometryRubberBand *>( sipCpp ) )
      sipType = sipType_QgsGeometryRubberBand;
    else
      sipType = nullptr;
    SIP_END
#endif

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

    QgsGeometryRubberBand( QgsMapCanvas *mapCanvas, QgsWkbTypes::GeometryType geomType = QgsWkbTypes::LineGeometry );
    ~QgsGeometryRubberBand() override;

    //! Sets geometry (takes ownership). Geometry is expected to be in map coordinates
    virtual void setGeometry( QgsAbstractGeometry *geom SIP_TRANSFER );
    //! Returns a pointer to the geometry
    const QgsAbstractGeometry *geometry() { return mGeometry.get(); }
    //! Moves vertex to new position (in map coordinates)
    void moveVertex( QgsVertexId id, const QgsPoint &newPos );
    //! Sets fill color for vertex markers
    void setFillColor( const QColor &c );
    //! Sets stroke color for vertex markers
    void setStrokeColor( const QColor &c );
    //! Sets stroke width
    void setStrokeWidth( int width );
    //! Sets pen style
    void setLineStyle( Qt::PenStyle penStyle );
    //! Sets brush style
    void setBrushStyle( Qt::BrushStyle brushStyle );
    //! Sets vertex marker icon type
    void setIconType( IconType iconType ) { mIconType = iconType; }
    //! Sets whether the vertices are drawn
    void setVertexDrawingEnabled( bool isVerticesDrawn );
    void updatePosition() override;

  protected:
    void paint( QPainter *painter ) override;

    //! Returns which geometry is handled by the rubber band, polygon or line
    QgsWkbTypes::GeometryType geometryType() const;

    //! Sets which geometry is handled by the rubber band, polygon or line
    void setGeometryType( const QgsWkbTypes::GeometryType &geometryType );

  private:
    std::unique_ptr<QgsAbstractGeometry> mGeometry = nullptr;
    QBrush mBrush;
    QPen mPen;
    int mIconSize;
    IconType mIconType;
    QgsWkbTypes::GeometryType mGeometryType;
    bool mDrawVertices = true;

    void drawVertex( QPainter *p, double x, double y );
    QgsRectangle rubberBandRectangle() const;
};


#endif // QGSGEOMETRYRUBBERBAND_H
