/** \brief The qgsworldmapimportergui class is used to import worldmap outputs.
 */
/* **************************************************************************
                          qgsworldmapimportergui.h  -  description
                             -------------------
    begin                : Sun Aug 11 2002
    copyright            : (C) 2002 by Tim Sutton
    email                : tim@linfiniti.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#ifndef QGSWORLDMAPIMPORTERGUI_H
#define QGSWORLDMAPIMPORTERGUI_H
#include "qgsworldmapimporterguibase.uic.h"
#include <qstring.h>

/**Gui subclass for importing worldmap results
  *@author Tim Sutton
  */

class QgsWorldMapImporterGui : public QgsWorldMapImporterGuiBase  
{
  Q_OBJECT 
  
  public:
        /** \brief Constructor
         */
        QgsWorldMapImporterGui();
        /** \brief Destructor */
        ~QgsWorldMapImporterGui();
        /** \brief Applies the settings made in the dialog without closing the box */
        void apply();
        void pbnInputFile_clicked();
        void pbnOutputFile_clicked();
        void cboInputFileType_textChanged( const QString & );
        void pbnOK_clicked();
        void leInputFileName_lostFocus();
        
  signals:
        void drawLayer(QString);
};

#endif
