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
#ifndef QGSPOINTDIALOG_H
#define QGSPOINTDIALOG_H

#include <vector>

#include <qcursor.h>

#include <qgsmapcanvas.h>

#include "qgsrasterlayer.h"

#include <qgspointdialogbase.uic.h>


class QgsPointDialog : public QgsPointDialogBase
{
Q_OBJECT
public:
  QgsPointDialog();
  QgsPointDialog(QgsRasterLayer* layer, QWidget* parent = 0, 
		 const char* name = 0, bool modal = FALSE, WFlags fl = 0);
  ~QgsPointDialog();

public slots:
  
  void handleCanvasClick(QgsPoint& pixelCoords);
  void addPoint(const QgsPoint& pixelCoords, const QgsPoint& mapCoords);
  void pbnCancel_clicked();
  void pbnGenerateWorldFile_clicked();
  void pbnGenerateAndLoad_clicked();
  void pbnSelectWorldFile_clicked();
  void pbnSelectModifiedRaster_clicked();
  void tbnZoomIn_changed(int);
  void tbnZoomOut_changed(int);
  void tbnZoomToLayer_clicked();
  void tbnPan_changed(int);
  void tbnAddPoint_changed(int);
  void tbnDeletePoint_changed(int);
  void enableRelevantControls(void);
  
signals:
  
  void loadLayer(QString);
  
private:

  void showCoordDialog(QgsPoint& pixelCoords);
  void deleteDataPoint(QgsPoint& pixelCoords);
  bool generateWorldFile();
  QString guessWorldFileName(const QString& raster);
  
  QgsMapCanvas* mCanvas;
  QCursor* mCursor;
  QgsRasterLayer* mLayer;
  
  std::vector<QgsPoint> mPixelCoords, mMapCoords;
  std::vector<QString> mAcetateIDs;

};

#endif
