/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef MAPCOORDSDIALOG_H
#define MAPCOORDSDIALOG_H

#include <QDialog>
#include <QMouseEvent>

#include "qgsmaptoolemitpoint.h"
#include "qgssnappingutils.h"
#include "qgspoint.h"
#include "qgsvertexmarker.h"
#include "qgsmapcanvas.h"

#include <ui_qgsmapcoordsdialogbase.h>

class QPushButton;

class QgsGeorefMapToolEmitPoint : public QgsMapTool
{
    Q_OBJECT

  public:
    explicit QgsGeorefMapToolEmitPoint( QgsMapCanvas *canvas )
        : QgsMapTool( canvas )
        , mSnappingMarker( nullptr )
    {
    }

    virtual ~QgsGeorefMapToolEmitPoint()
    {
      delete mSnappingMarker;
      mSnappingMarker = nullptr;
    }

    void canvasMoveEvent( QgsMapMouseEvent *e ) override
    {
      MappedPoint mapped = mapPoint( e );

      if ( !mapped.snapped )
      {
        delete mSnappingMarker;
        mSnappingMarker = nullptr;
      }
      else
      {
        if ( !mSnappingMarker )
        {
          mSnappingMarker = new QgsVertexMarker( mCanvas );
          mSnappingMarker->setIconType( QgsVertexMarker::ICON_CROSS );
          mSnappingMarker->setColor( Qt::magenta );
          mSnappingMarker->setPenWidth( 3 );
        }
        mSnappingMarker->setCenter( mapped.point );
      }
    }

    void canvasPressEvent( QgsMapMouseEvent * e ) override
    {
      MappedPoint mapped = mapPoint( e );
      emit canvasClicked( mapped.point, e->button() );
    }

    void canvasReleaseEvent( QgsMapMouseEvent *e ) override
    {
      QgsMapTool::canvasReleaseEvent( e );
      emit mouseReleased();
    }

    void deactivate() override
    {
      delete mSnappingMarker;
      mSnappingMarker = 0;

      QgsMapTool::deactivate();
    }

  signals:
    void canvasClicked( const QgsPoint& point, Qt::MouseButton button );
    void mouseReleased();

  private:
    struct MappedPoint
    {
      MappedPoint() : snapped( false ) {}
      QgsPoint point;
      bool snapped;
    };

    MappedPoint mapPoint( QMouseEvent *e )
    {
      QgsPoint pnt = toMapCoordinates( e->pos() );
      QgsSnappingUtils* snappingUtils = canvas()->snappingUtils();
      QgsPointLocator::Match match = snappingUtils->snapToMap( pnt );

      MappedPoint ret;
      ret.snapped = match.isValid();
      ret.point = ret.snapped ? match.point() : pnt;
      return ret;
    }

    QgsVertexMarker* mSnappingMarker;
};

class QgsMapCoordsDialog : public QDialog, private Ui::QgsMapCoordsDialogBase
{
    Q_OBJECT

  public:
    QgsMapCoordsDialog( QgsMapCanvas *qgisCanvas, const QgsPoint &pixelCoords, QWidget *parent = nullptr );
    ~QgsMapCoordsDialog();

  private slots:
    void on_buttonBox_accepted();

    void setToolEmitPoint( bool isEnable );

    void maybeSetXY( const QgsPoint &, Qt::MouseButton );
    void updateOK();
    void setPrevTool();

  signals:
    void pointAdded( const QgsPoint &, const QgsPoint & );

  private:
    double dmsToDD( const QString& dms );

    QPushButton *mPointFromCanvasPushButton;

    QgsGeorefMapToolEmitPoint* mToolEmitPoint;
    QgsMapTool* mPrevMapTool;
    QgsMapCanvas* mQgisCanvas;

    QgsPoint mPixelCoords;
};

#endif
