/***************************************************************************
    qgsprojectio.h - Save/Restore QGIS Project files
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
/* qgsprojectio.h,v 1.6 2004/06/10 23:07:08 timlinux Exp */

#ifndef _QGSPROJECTIO_H_
#define _QGSPROJECTIO_H_

class QgisApp;
class QgsMapLayerRegistry;
class QgsRect;
/*! \class QgsProjectIo
* \brief Class to handle reading and writing a Qgis project file
*/
class QgsProjectIo
{

public:
  QgsProjectIo(int action=SAVE, QgisApp *qgis=0);
  ~QgsProjectIo();
  //! Read the file and create the map
  bool read(QString path=0);
  //! Write the contents of the map to a file
  bool write(QgsRect theExtent);
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
  void writeXML(QgsRect theExtent);
  QString fileName;
  QString fullPath;
  bool neverSaved;
  //! pointer to the main app for connecting slots
  QgisApp *qgisApp;
  QgsMapLayerRegistry * mMapLayerRegistry;
  int action;
};

#endif
