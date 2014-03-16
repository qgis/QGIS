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
#include <QObject> //for tr
#include <QImageWriter>
#include "qgsencodingfiledialog.h"
#include "qgslogger.h"
#include <memory> //for auto_ptr


namespace QgisGui
{

  bool GUI_EXPORT openFilesRememberingFilter( QString const &filterName,
      QString const &filters, QStringList & selectedFiles, QString& enc, QString &title,
      bool cancelAll )
  {
    Q_UNUSED( enc );

    QSettings settings;
    QString lastUsedFilter = settings.value( "/UI/" + filterName, "" ).toString();
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

      if ( !lastUsedFilter.isEmpty() )
      {
        openFileDialog->selectNameFilter( lastUsedFilter );
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
      QString firstFileName = selectedFiles.first();
      QFileInfo fi( firstFileName );
      QString path = fi.path();

      QgsDebugMsg( "Writing last used dir: " + path );

      settings.setValue( "/UI/" + filterName, lastUsedFilter );
      settings.setValue( "/UI/" + filterName + "Dir", path );
    }
    return false;
  }

  QPair<QString, QString> GUI_EXPORT getSaveAsImageName( QWidget *theParent, QString theMessage, QString defaultFilename )
  {
    // get a list of supported output image types
    QMap<QString, QString> filterMap;
    foreach ( QByteArray format, QImageWriter::supportedImageFormats() )
    {
      //svg doesnt work so skip it
      if ( format ==  "svg" )
        continue;

      filterMap.insert( createFileFilter_( format.toUpper() + " format", "*." + format ), format );
    }

#ifdef QGISDEBUG
    QgsDebugMsg( "Available Filters Map: " );
    for ( QMap<QString, QString>::iterator it = filterMap.begin(); it != filterMap.end(); ++it )
    {
      QgsDebugMsg( it.key() + "  :  " + it.value() );
    }
#endif

    //find out the last used filter
    QSettings settings;  // where we keep last used filter in persistent state
    QString lastUsedFilter = settings.value( "/UI/lastSaveAsImageFilter" ).toString();
    QString lastUsedDir = settings.value( "/UI/lastSaveAsImageDir", "." ).toString();

    QString outputFileName;
    QString selectedFilter = lastUsedFilter;
    QString ext;

    QString initialPath;
    if ( defaultFilename.isNull() )
    {
      //no default filename provided, just use last directory
      initialPath = lastUsedDir;
    }
    else
    {
      //a default filename was provided, so use it to build the initial path
      initialPath = QDir( lastUsedDir ).filePath( defaultFilename );
    }

#if defined(Q_OS_WIN) || defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    outputFileName = QFileDialog::getSaveFileName( theParent, theMessage, initialPath, QStringList( filterMap.keys() ).join( ";;" ), &selectedFilter );

    if ( !outputFileName.isNull() )
    {
      ext = filterMap.value( selectedFilter, QString::null );
      if ( !ext.isNull() )
        settings.setValue( "/UI/lastSaveAsImageFilter", selectedFilter );
      settings.setValue( "/UI/lastSaveAsImageDir", QFileInfo( outputFileName ).absolutePath() );
    }
#else
    //create a file dialog using the filter list generated above
    std::auto_ptr<QFileDialog> fileDialog( new QFileDialog( theParent, theMessage, initialPath, QStringList( filterMap.keys() ).join( ";;" ) ) );

    // allow for selection of more than one file
    fileDialog->setFileMode( QFileDialog::AnyFile );
    fileDialog->setAcceptMode( QFileDialog::AcceptSave );
    fileDialog->setConfirmOverwrite( true );

    if ( !lastUsedFilter.isEmpty() )     // set the filter to the last one used
    {
      fileDialog->selectNameFilter( lastUsedFilter );
    }

    //prompt the user for a fileName
    if ( fileDialog->exec() == QDialog::Accepted )
    {
      outputFileName = fileDialog->selectedFiles().first();
    }

    selectedFilter = fileDialog->selectedFilter();
    QgsDebugMsg( "Selected filter: " + selectedFilter );
    ext = filterMap.value( selectedFilter, QString::null );

    if ( !ext.isNull() )
      settings.setValue( "/UI/lastSaveAsImageFilter", selectedFilter );

    settings.setValue( "/UI/lastSaveAsImageDir", fileDialog->directory().absolutePath() );
#endif

    // Add the file type suffix to the fileName if required
    if ( !ext.isNull() && !outputFileName.toLower().endsWith( "." + ext.toLower() ) )
    {
      outputFileName += "." + ext;
    }

    return qMakePair<QString, QString>( outputFileName, ext );
  }

  QString createFileFilter_( QString const &longName, QString const &glob )
  {
    return longName + " (" + glob.toLower() + " " + glob.toUpper() + ")";
  }

} // end of QgisGui namespace
