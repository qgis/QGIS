/***************************************************************************
    qgsmapserverexport.h - Export QGIS MapCanvas to MapServer
     --------------------------------------
    Date                 : 08-Nov-2003
    Copyright            : (C) 2003 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************

  $Id$

*/

#ifndef QGSMAPSERVEREXPORT_H
#define QGSMAPSERVEREXPORT_H
#include "ui_qgsmapserverexportbase.h"
#include "qgisgui.h"
class QgsMapCanvas;

/*! \class QgsMapServerExport
* \brief Class to handle reading and writing a Qgis project file
*/
class QgsMapserverExport: public QDialog, private Ui::QgsMapserverExportBase
{
    Q_OBJECT
  public:
    QgsMapserverExport( QgsMapCanvas *map = 0, QWidget* parent = 0,
                        Qt::WFlags fl = QgisGui::ModalDialogFlags );
    ~QgsMapserverExport();
    //! Read the file and create the map
    bool read();
    //! Write the contents of the map to a file
    bool write();
    void setMapCanvas( QgsMapCanvas *map );
    //! Open a file dialog, the type determined by action (SAVE AS or OPEN)
    QString selectFileName();
    //! get the basename of the file (no path, just the file name)
    QString baseName();
    //! get the full path name of the map file
    QString fullPathName();
    //! Set the full path to the file
    void setFileName( QString fileName );
    enum ACTION
    {
      SAVE,
      SAVEAS,
      OPEN
    };
  public slots:
    void on_chkExpLayersOnly_clicked();
    void on_btnChooseFile_clicked();
    void help();

  private:
    void writeMapFile( void );
    QString fileName;
    QString fullPath;
    bool neverSaved;
    QgsMapCanvas *map;
    int action;
    static const int context_id = 0;
};

#endif //QGSMAPSERVEREXPORT_H

