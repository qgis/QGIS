
/**
 * 
 * Gary Sherman
 **/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _QGSPROJECTIO_H_
#define _QGSPROJECTIO_H_

class QgsMapCanvas;
/*! \class QgsProjectIo
* \brief Class to handle reading and writing a Qgis project file
*/
class QgsProjectIo
{

public:
  QgsProjectIo(QgsMapCanvas *map=0, int action=SAVE);
  ~QgsProjectIo();
  void read();
  void write();
  void setMapCanvas(QgsMapCanvas *map);
  QString selectFileName();
  enum ACTION {
  	SAVE,
	SAVEAS,
	OPEN
	};
  private:
  void writeXML(void);
  QString fileName;
  QString fullPath;
  bool neverSaved;
  QgsMapCanvas *map;
  int action;
};

#endif
