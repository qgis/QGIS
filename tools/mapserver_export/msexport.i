/***************************************************************************
     msexport.i
     --------------------------------------
    Date                 : Sun Sep 16 12:34:12 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* IF YOU REGENERATE THE SWIG WRAPPER FROM THIS INTERFACE FILE, IT PROBABLY
 * WON'T WORK SINCE THERE SEEMS TO BE A BUG IN THE INTERFACE GENERATION CODE
*/
%module msexport 
%{
/* Includes the header in the wrapper code */
#include "qgsmapserverexport.h"
%}

/* Parse the header file to generate wrappers */

class QgsMapserverExport:public QDialog, private Ui::QgsMapserverExportBase
{
public:
  QgsMapserverExport(QWidget* parent = 0,  Qt::WFlags fl = 0 );
  ~QgsMapserverExport();
  //! Read the file and create the map
  //bool read();
  //! Write the contents of the map to a file
  bool write();
  //void setMapCanvas(QgsMapCanvas *map);
  //! Open a file dialog, the type determined by action (SAVE AS or OPEN)
  //QString selectFileName();
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
	void showHelp();
  void on_btnChooseFile_clicked();
  void on_chkExpLayersOnly_clicked(bool);
  void on_btnChooseProjectFile_clicked();
private:
  void initPy();
  void writeMapFile(void);
  QString mapFile;
  QString qgisProjectFile;
  bool neverSaved;
  int action;
};



