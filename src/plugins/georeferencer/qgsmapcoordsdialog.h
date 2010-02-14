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
/* $Id$ */
#ifndef MAPCOORDSDIALOG_H
#define MAPCOORDSDIALOG_H

#include <QDialog>

#include "qgsmaptoolemitpoint.h"
#include "qgspoint.h"

#include <ui_qgsmapcoordsdialogbase.h>

class QPushButton;

class QgsGeorefMapToolEmitPoint : public QgsMapToolEmitPoint
{
  Q_OBJECT;

public:
  QgsGeorefMapToolEmitPoint(QgsMapCanvas *canvas)
    : QgsMapToolEmitPoint(canvas)
  {
  }

  void canvasReleaseEvent(QMouseEvent *e)
  {
    QgsMapToolEmitPoint::canvasReleaseEvent(e);
    emit mouseReleased();
  }

signals:
  void mouseReleased();
};

class QgsMapCoordsDialog : public QDialog, private Ui::QgsMapCoordsDialogBase
{
  Q_OBJECT

public:
  QgsMapCoordsDialog( QgsMapCanvas* qgisCanvas, const QgsPoint pixelCoords, QWidget* parent = 0 );
  ~QgsMapCoordsDialog();

private slots:
  void on_buttonBox_accepted();

  void setToolEmitPoint(bool isEnable);

  void maybeSetXY( const QgsPoint &, Qt::MouseButton );
  void updateOK();
  void setPrevTool();

signals:
  void pointAdded(const QgsPoint &, const QgsPoint &);

private:
  double dmsToDD(QString dms);

  QPushButton *mPointFromCanvasPushButton;

  QgsGeorefMapToolEmitPoint* mToolEmitPoint;
  QgsMapTool* mPrevMapTool;
  QgsMapCanvas* mQgisCanvas;

  QgsPoint mPixelCoords;
};

#endif
