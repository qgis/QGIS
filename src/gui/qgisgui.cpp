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

    bool haveLastUsedFilter = false; // by default, there is no last
    // used filter

    QSettings settings;         // where we keep last used filter in
    // persistent state

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

  QPair<QString, QString> GUI_EXPORT getSaveAsImageName( QWidget * theParent, QString theMessage )
  {
    Q_UNUSED( theMessage );
    //create a map to hold the QImageIO names and the filter names
    //the QImageIO name must be passed to the mapcanvas saveas image function
    typedef QMap<QString, QString> FilterMap;
    FilterMap myFilterMap;

    //find out the last used filter
    QSettings myQSettings;  // where we keep last used filter in persistent state
    QString myLastUsedFilter = myQSettings.value( "/UI/lastSaveAsImageFilter" ).toString();
    QString myLastUsedDir = myQSettings.value( "/UI/lastSaveAsImageDir", "." ).toString();

    // get a list of supported output image types
    int myCounterInt = 0;
    QString myFilters;
    QList<QByteArray> formats = QImageWriter::supportedImageFormats();

    for ( ; myCounterInt < formats.count(); myCounterInt++ )
    {
      QString myFormat = QString( formats.at( myCounterInt ) );
      //svg doesnt work so skip it
      if ( myFormat ==  "svg" )
        continue;

      QString myFilter = createFileFilter_( myFormat + " format", "*." + myFormat );
      if ( !myFilters.isEmpty() )
        myFilters += ";;";
      myFilters += myFilter;
      myFilterMap[myFilter] = myFormat;
    }
#ifdef QGISDEBUG
    QgsDebugMsg( "Available Filters Map: " );
    FilterMap::Iterator myIterator;
    for ( myIterator = myFilterMap.begin(); myIterator != myFilterMap.end(); ++myIterator )
    {
      QgsDebugMsg( myIterator.key() + "  :  " + myIterator.value() );
    }
#endif

    //create a file dialog using the the filter list generated above
    std::auto_ptr < QFileDialog > myQFileDialog( new QFileDialog( theParent,
        QObject::tr( "Choose a file name to save the map image as" ),
        myLastUsedDir, myFilters ) );

    // allow for selection of more than one file
    myQFileDialog->setFileMode( QFileDialog::AnyFile );

    myQFileDialog->setAcceptMode( QFileDialog::AcceptSave );

    myQFileDialog->setConfirmOverwrite( true );


    if ( !myLastUsedFilter.isEmpty() )     // set the filter to the last one used
    {
      myQFileDialog->selectFilter( myLastUsedFilter );
    }

    //prompt the user for a fileName
    QString myOutputFileName; // = myQFileDialog->getSaveFileName(); //delete this
    if ( myQFileDialog->exec() == QDialog::Accepted )
    {
      myOutputFileName = myQFileDialog->selectedFiles().first();
    }

    QString myFilterString = myQFileDialog->selectedFilter();
    QgsDebugMsg( "Selected filter: " + myFilterString );
    QgsDebugMsg( "Image type: " + myFilterMap[myFilterString] );

    // Add the file type suffix to the fileName if required
    if ( !myOutputFileName.endsWith( myFilterMap[myFilterString] ) )
    {
      myOutputFileName += "." + myFilterMap[myFilterString];
    }

    myQSettings.setValue( "/UI/lastSaveAsImageFilter", myFilterString );
    myQSettings.setValue( "/UI/lastSaveAsImageDir", myQFileDialog->directory().absolutePath() );
    QPair <QString, QString> myPair;
    myPair.first = myOutputFileName;
    myPair.second = myFilterMap[myFilterString];
    return myPair;
  } //

  QString createFileFilter_( QString const &longName, QString const &glob )
  {
    return longName + " (" + glob.toLower() + " " + glob.toUpper() + ")";
  }                               // createFileFilter_

} // end of QgisGui namespace
