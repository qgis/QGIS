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

#include <qgspoint.h>

class QgsMapCanvas;
class QgsMapTool;

#include <ui_mapcoordsdialogbase.h>
#include <QDialog>
class MapCoordsDialog : public QDialog, private Ui::MapCoordsDialogBase
{
    Q_OBJECT
  public:
    MapCoordsDialog();
    MapCoordsDialog( const QgsPoint& pixelCoords, QgsMapCanvas* qgisCanvas,
                     QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~MapCoordsDialog();

  public slots:

    void on_buttonOk_clicked();
    void on_buttonCancel_clicked();

    void on_btnPointFromCanvas_clicked();

    void maybeSetXY( const QgsPoint &, Qt::MouseButton );
    void updateOK();

  private:

    QgsPoint mPixelCoords;

    QgsMapTool* mToolEmitPoint;
    QgsMapTool* mPrevMapTool;

    QgsMapCanvas* mQgisCanvas;

  signals:

    void pointAdded( const QgsPoint& pixelCoords, const QgsPoint& mapCoords );

};

#endif
