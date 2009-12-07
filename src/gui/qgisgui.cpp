/***************************************************************************
    qgisgui.cpp - Constants used throughout the QGIS GUI.
     --------------------------------------
    Date                 : 11-Jan-2006
    Copyright            : (C) 2006 by Tom Elwertowski
    Email                : telwertowski at users dot sourceforge dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgisgui.h"

#include <QSettings>
#include "qgsencodingfiledialog.h"
#include "qgslogger.h"

namespace QgisGui
{

  bool openFilesRememberingFilter( QString const &filterName,
                                   QString const &filters, QStringList & selectedFiles, QString& enc, QString &title,
                                   bool cancelAll )
  {

    bool haveLastUsedFilter = false; // by default, there is no last
    // used filter

    QSettings settings;         // where we keep last used filter in
    // persistant state

    haveLastUsedFilter = settings.contains( "/UI/" + filterName );
    QString lastUsedFilter = settings.value( "/UI/" + filterName,
                             QVariant( QString::null ) ).toString();

    QString lastUsedDir = settings.value( "/UI/" + filterName + "Dir", "." ).toString();

    QgsDebugMsg( "Opening file dialog with filters: " + filters );
    if ( !cancelAll )
    {
      selectedFiles = QFileDialog::getOpenFileNames( 0, title, lastUsedDir, filters, &lastUsedFilter );
    }
    else //we have to use non-native dialog to add cancel all button
    {
      QgsEncodingFileDialog* openFileDialog = new QgsEncodingFileDialog( 0, title, lastUsedDir, filters, QString( "" ) );
      // allow for selection of more than one file
      openFileDialog->setFileMode( QFileDialog::ExistingFiles );
      if ( haveLastUsedFilter )     // set the filter to the last one used
      {
        openFileDialog->selectFilter( lastUsedFilter );
      }
      openFileDialog->addCancelAll();
      if ( openFileDialog->exec() == QDialog::Accepted )
      {
        selectedFiles = openFileDialog->selectedFiles();
      }
      else
      {
        //cancel or cancel all?
        if ( openFileDialog->cancelAll() )
        {
          return true;
        }
      }
    }

    if ( !selectedFiles.isEmpty() )
    {
      // Fix by Tim - getting the dirPath from the dialog
      // directly truncates the last node in the dir path.
      // This is a workaround for that
      QString myFirstFileName = selectedFiles.first();
      QFileInfo myFI( myFirstFileName );
      QString myPath = myFI.path();

      QgsDebugMsg( "Writing last used dir: " + myPath );

      settings.setValue( "/UI/" + filterName, lastUsedFilter );
      settings.setValue( "/UI/" + filterName + "Dir", myPath );
    }
    return false;
  }


} // end of QgisGui namespace
