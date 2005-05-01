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

#include <mapcoordsdialogbase.uic.h>

class MapCoordsDialog : public MapCoordsDialogBase
{
Q_OBJECT
public:
  MapCoordsDialog();
  MapCoordsDialog(const QgsPoint& pixelCoords,
		  QWidget* parent = 0, const char* name = 0, 
		  bool modal = FALSE, WFlags fl = 0);
  ~MapCoordsDialog();
  
public slots:
  
  void pbnOK_clicked();
  
private:
  
  QgsPoint mPixelCoords;

signals:

  void pointAdded(const QgsPoint& pixelCoords, const QgsPoint& mapCoords);

};

#endif
