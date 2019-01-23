/***************************************************************************
    qgsguiutils.cpp - Constants used throughout the QGIS GUI.
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
#include "qgsguiutils.h"

#include "qgssettings.h"
#include "qgsencodingfiledialog.h"
#include "qgslogger.h"
#include "qgis_gui.h"
#include "qgis.h"

#include <QImageWriter>
#include <QFontDialog>
#include <QApplication>


namespace QgsGuiUtils
{

  bool GUI_EXPORT openFilesRememberingFilter( QString const &filterName,
      QString const &filters, QStringList &selectedFiles, QString &enc, QString &title,
      bool cancelAll )
  {
    Q_UNUSED( enc );

    QgsSettings settings;
    QString lastUsedFilter = settings.value( "/UI/" + filterName, "" ).toString();
    QString lastUsedDir = settings.value( "/UI/" + filterName + "Dir", QDir::homePath() ).toString();

    QgsDebugMsg( "Opening file dialog with filters: " + filters );
    if ( !cancelAll )
    {
      selectedFiles = QFileDialog::getOpenFileNames( nullptr, title, lastUsedDir, filters, &lastUsedFilter );
    }
    else //we have to use non-native dialog to add cancel all button
    {
      QgsEncodingFileDialog *openFileDialog = new QgsEncodingFileDialog( nullptr, title, lastUsedDir, filters, QString() );

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

  QPair<QString, QString> GUI_EXPORT getSaveAsImageName( QWidget *parent, const QString &message, const QString &defaultFilename )
  {
    // get a list of supported output image types
    QMap<QString, QString> filterMap;
    Q_FOREACH ( const QByteArray &format, QImageWriter::supportedImageFormats() )
    {
      //svg doesn't work so skip it
      if ( format == "svg" )
        continue;

      filterMap.insert( createFileFilter_( format ), format );
    }

#ifdef QGISDEBUG
    QgsDebugMsg( QStringLiteral( "Available Filters Map: " ) );
    for ( QMap<QString, QString>::iterator it = filterMap.begin(); it != filterMap.end(); ++it )
    {
      QgsDebugMsg( it.key() + "  :  " + it.value() );
    }
#endif

    QgsSettings settings;  // where we keep last used filter in persistent state
    QString lastUsedDir = settings.value( QStringLiteral( "UI/lastSaveAsImageDir" ), QDir::homePath() ).toString();

    // Prefer "png" format unless the user previously chose a different format
    QString pngExtension = QStringLiteral( "png" );
    QString pngFilter = createFileFilter_( pngExtension );
    QString selectedFilter = settings.value( QStringLiteral( "UI/lastSaveAsImageFilter" ), pngFilter ).toString();

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

    QString outputFileName;
    QString ext;
#if defined(Q_OS_WIN) || defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    outputFileName = QFileDialog::getSaveFileName( parent, message, initialPath, QStringList( filterMap.keys() ).join( QStringLiteral( ";;" ) ), &selectedFilter );

    if ( !outputFileName.isNull() )
    {
      ext = filterMap.value( selectedFilter, QString() );
      if ( !ext.isNull() )
        settings.setValue( QStringLiteral( "UI/lastSaveAsImageFilter" ), selectedFilter );
      settings.setValue( QStringLiteral( "UI/lastSaveAsImageDir" ), QFileInfo( outputFileName ).absolutePath() );
    }
#else

    //create a file dialog using the filter list generated above
    std::unique_ptr<QFileDialog> fileDialog( new QFileDialog( parent, message, initialPath, QStringList( filterMap.keys() ).join( ";;" ) ) );

    // allow for selection of more than one file
    fileDialog->setFileMode( QFileDialog::AnyFile );
    fileDialog->setAcceptMode( QFileDialog::AcceptSave );
    fileDialog->setOption( QFileDialog::DontConfirmOverwrite, false );

    if ( !selectedFilter.isEmpty() )     // set the filter to the last one used
    {
      fileDialog->selectNameFilter( selectedFilter );
    }

    //prompt the user for a fileName
    if ( fileDialog->exec() == QDialog::Accepted )
    {
      outputFileName = fileDialog->selectedFiles().first();
    }

    selectedFilter = fileDialog->selectedNameFilter();
    QgsDebugMsg( "Selected filter: " + selectedFilter );
    ext = filterMap.value( selectedFilter, QString() );

    if ( !ext.isNull() )
      settings.setValue( "/UI/lastSaveAsImageFilter", selectedFilter );

    settings.setValue( "/UI/lastSaveAsImageDir", fileDialog->directory().absolutePath() );
#endif

    // Add the file type suffix to the fileName if required
    if ( !ext.isNull() && !outputFileName.endsWith( '.' + ext.toLower(), Qt::CaseInsensitive ) )
    {
      outputFileName += '.' + ext;
    }

    return qMakePair<QString, QString>( outputFileName, ext );
  }

  QString createFileFilter_( QString const &longName, QString const &glob )
  {
    return QStringLiteral( "%1 (%2 %3)" ).arg( longName, glob.toLower(), glob.toUpper() );
  }

  QString createFileFilter_( QString const &format )
  {
    QString longName = format.toUpper() + " format";
    QString glob = "*." + format;
    return createFileFilter_( longName, glob );
  }

  QFont getFont( bool &ok, const QFont &initial, const QString &title )
  {
    // parent is intentionally not set to 'this' as
    // that would make it follow the style sheet font
    // see also #12233 and #4937
#if defined(Q_OS_MAC)
    // Native dialog broken on macOS with Qt5
    // probably only broken in Qt5.11.1 and .2
    //    (see https://successfulsoftware.net/2018/11/02/qt-is-broken-on-macos-right-now/ )
    // possible upstream bug: https://bugreports.qt.io/browse/QTBUG-69878 (fixed in Qt 5.12 ?)
    return QFontDialog::getFont( &ok, initial, nullptr, title, QFontDialog::DontUseNativeDialog );
#else
    return QFontDialog::getFont( &ok, initial, nullptr, title );
#endif
  }

  void saveGeometry( QWidget *widget, const QString &keyName )
  {
    QgsSettings settings;
    QString key = createWidgetKey( widget, keyName );
    settings.setValue( key, widget->saveGeometry() );
  }

  bool restoreGeometry( QWidget *widget, const QString &keyName )
  {
    QgsSettings settings;
    QString key = createWidgetKey( widget, keyName );
    return widget->restoreGeometry( settings.value( key ).toByteArray() );
  }

  QString createWidgetKey( QWidget *widget, const QString &keyName )
  {
    QString subKey;
    if ( !keyName.isEmpty() )
    {
      subKey = keyName;
    }
    else if ( widget->objectName().isEmpty() )
    {
      subKey = QString( widget->metaObject()->className() );
    }
    else
    {
      subKey = widget->objectName();
    }
    QString key = QStringLiteral( "Windows/%1/geometry" ).arg( subKey );
    return key;
  }

  int scaleIconSize( int standardSize )
  {
    QFontMetrics fm( ( QFont() ) );
    const double scale = 1.1 * standardSize / 24;
    return static_cast< int >( std::floor( std::max( Qgis::UI_SCALE_FACTOR * fm.height() * scale, static_cast< double >( standardSize ) ) ) );
  }

}

//
// QgsTemporaryCursorOverride
//

QgsTemporaryCursorOverride::QgsTemporaryCursorOverride( const QCursor &cursor )
{
  QApplication::setOverrideCursor( cursor );
}

QgsTemporaryCursorOverride::~QgsTemporaryCursorOverride()
{
  if ( mHasOverride )
    QApplication::restoreOverrideCursor();
}

void QgsTemporaryCursorOverride::release()
{
  if ( !mHasOverride )
    return;

  mHasOverride = false;
  QApplication::restoreOverrideCursor();
}
