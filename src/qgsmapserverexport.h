/***************************************************************************
    qgsmapserverexport.h - Export QGIS MapCanvas to MapServer
     --------------------------------------
    Date                 : 08-Nov-2003
    Copyright            : (C) 2003 by Gary E.Sherman
    email                : sherman at mrcc.com
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSMAPSERVEREXPORT_H
#define QGSMAPSERVEREXPORT_H

class QgsMapCanvas;
/*! \class QgsMapServerExport
* \brief Class to handle reading and writing a Qgis project file
*/
class QgsMapServerExport
{

public:
  QgsMapServerExport(QgsMapCanvas *map=0, int action=SAVE);
  ~QgsMapServerExport();
  //! Read the file and create the map
  bool read();
  //! Write the contents of the map to a file
  bool write();
  void setMapCanvas(QgsMapCanvas *map);
  //! Open a file dialog, the type determined by action (SAVE AS or OPEN)
  QString selectFileName();
  //! get the basename of the file (no path, just the file name)
  QString baseName();
  //! get the full path name of the map file
  QString fullPathName();
  //! Set the full path to the file
  void setFileName(QString filename);
  enum ACTION {
  	SAVE,
	SAVEAS,
	OPEN
	};
  private:
  void writeMapFile(void);
  QString fileName;
  QString fullPath;
  bool neverSaved;
  QgsMapCanvas *map;
  int action;
};

#endif //QGSMAPSERVEREXPORT_H

