/***************************************************************************
                             qgsddirectoryitem.cpp
                             -------------------
    begin                : 2011-04-01
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdirectoryitem.h"
#include "qgssettings.h"
#include "qgsapplication.h"
#include "qgsdataitemprovider.h"
#include "qgsdataitemproviderregistry.h"
#include "qgsdataprovider.h"
#include "qgszipitem.h"
#include "qgsprojectitem.h"
#include "qgsfileutils.h"
#include <QFileSystemWatcher>
#include <QDir>
#include <QMouseEvent>
#include <QTimer>
#include <QMenu>
#include <QAction>

//
// QgsDirectoryItem
//

QgsDirectoryItem::QgsDirectoryItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, QDir::toNativeSeparators( name ), path )
  , mDirPath( path )
{
  init();
}

QgsDirectoryItem::QgsDirectoryItem( QgsDataItem *parent, const QString &name,
                                    const QString &dirPath, const QString &path,
                                    const QString &providerKey )
  : QgsDataCollectionItem( parent, QDir::toNativeSeparators( name ), path, providerKey )
  , mDirPath( dirPath )
{
  init();
}

void QgsDirectoryItem::init()
{
  mType = Qgis::BrowserItemType::Directory;
  setToolTip( QDir::toNativeSeparators( mDirPath ) );

  QgsSettings settings;

  mMonitoring = monitoringForPath( mDirPath );
  switch ( mMonitoring )
  {
    case Qgis::BrowserDirectoryMonitoring::Default:
      mMonitored = pathShouldByMonitoredByDefault( mDirPath );
      break;
    case Qgis::BrowserDirectoryMonitoring::NeverMonitor:
      mMonitored = false;
      break;
    case Qgis::BrowserDirectoryMonitoring::AlwaysMonitor:
      mMonitored = true;
      break;
  }

  settings.beginGroup( QStringLiteral( "qgis/browserPathColors" ) );
  QString settingKey = mDirPath;
  settingKey.replace( '/', QLatin1String( "|||" ) );
  if ( settings.childKeys().contains( settingKey ) )
  {
    const QString colorString = settings.value( settingKey ).toString();
    mIconColor = QColor( colorString );
  }
  settings.endGroup();
}

void QgsDirectoryItem::reevaluateMonitoring()
{
  mMonitoring = monitoringForPath( mDirPath );
  switch ( mMonitoring )
  {
    case Qgis::BrowserDirectoryMonitoring::Default:
      mMonitored = pathShouldByMonitoredByDefault( mDirPath );
      break;
    case Qgis::BrowserDirectoryMonitoring::NeverMonitor:
      mMonitored = false;
      break;
    case Qgis::BrowserDirectoryMonitoring::AlwaysMonitor:
      mMonitored = true;
      break;
  }

  const QVector<QgsDataItem *> childItems = children();
  for ( QgsDataItem *child : childItems )
  {
    if ( QgsDirectoryItem *dirItem = qobject_cast< QgsDirectoryItem *>( child ) )
      dirItem->reevaluateMonitoring();
  }

  createOrDestroyFileSystemWatcher();
}

void QgsDirectoryItem::createOrDestroyFileSystemWatcher()
{
  if ( !mMonitored && mFileSystemWatcher )
  {
    mFileSystemWatcher->deleteLater();
    mFileSystemWatcher = nullptr;
  }
  else if ( mMonitored && state() == Qgis::BrowserItemState::Populated && !mFileSystemWatcher )
  {
    mFileSystemWatcher = new QFileSystemWatcher( this );
    mFileSystemWatcher->addPath( mDirPath );
    connect( mFileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &QgsDirectoryItem::directoryChanged );
  }
}

QColor QgsDirectoryItem::iconColor() const
{
  return mIconColor;
}

void QgsDirectoryItem::setIconColor( const QColor &color )
{
  if ( color == mIconColor )
    return;

  mIconColor = color;
  emit dataChanged( this );
}

void QgsDirectoryItem::setCustomColor( const QString &directory, const QColor &color )
{
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "qgis/browserPathColors" ) );
  QString settingKey = directory;
  settingKey.replace( '/', QLatin1String( "|||" ) );
  if ( color.isValid() )
    settings.setValue( settingKey, color.name( QColor::HexArgb ) );
  else
    settings.remove( settingKey );
  settings.endGroup();
}

QIcon QgsDirectoryItem::icon()
{
  if ( mDirPath == QDir::homePath() )
    return homeDirIcon( mIconColor, mIconColor.darker() );

  // still loading? show the spinner
  if ( state() == Qgis::BrowserItemState::Populating )
    return QgsDataItem::icon();

  // symbolic link? use link icon
  const QFileInfo fi( mDirPath );
  if ( fi.isDir() && fi.isSymLink() )
  {
    return mIconColor.isValid()
           ? QgsApplication::getThemeIcon( QStringLiteral( "/mIconFolderLinkParams.svg" ), mIconColor, mIconColor.darker() )
           : QgsApplication::getThemeIcon( QStringLiteral( "/mIconFolderLink.svg" ) );
  }

  // loaded? show the open dir icon
  if ( state() == Qgis::BrowserItemState::Populated )
    return openDirIcon( mIconColor, mIconColor.darker() );

  // show the closed dir icon
  return iconDir( mIconColor, mIconColor.darker() );
}

Qgis::BrowserDirectoryMonitoring QgsDirectoryItem::monitoring() const
{
  return mMonitoring;
}

void QgsDirectoryItem::setMonitoring( Qgis::BrowserDirectoryMonitoring monitoring )
{
  mMonitoring = monitoring;

  QgsSettings settings;
  QStringList noMonitorDirs = settings.value( QStringLiteral( "qgis/disableMonitorItemUris" ), QStringList() ).toStringList();
  QStringList alwaysMonitorDirs = settings.value( QStringLiteral( "qgis/alwaysMonitorItemUris" ), QStringList() ).toStringList();

  switch ( mMonitoring )
  {
    case Qgis::BrowserDirectoryMonitoring::Default:
    {
      // remove disable/always setting for this path, so that default behavior is used
      noMonitorDirs.removeAll( mDirPath );
      settings.setValue( QStringLiteral( "qgis/disableMonitorItemUris" ), noMonitorDirs );

      alwaysMonitorDirs.removeAll( mDirPath );
      settings.setValue( QStringLiteral( "qgis/alwaysMonitorItemUris" ), alwaysMonitorDirs );

      mMonitored = pathShouldByMonitoredByDefault( mDirPath );
      break;
    }

    case Qgis::BrowserDirectoryMonitoring::NeverMonitor:
    {
      if ( !noMonitorDirs.contains( mDirPath ) )
      {
        noMonitorDirs.append( mDirPath );
        settings.setValue( QStringLiteral( "qgis/disableMonitorItemUris" ), noMonitorDirs );
      }

      alwaysMonitorDirs.removeAll( mDirPath );
      settings.setValue( QStringLiteral( "qgis/alwaysMonitorItemUris" ), alwaysMonitorDirs );

      mMonitored = false;
      break;
    }

    case Qgis::BrowserDirectoryMonitoring::AlwaysMonitor:
    {
      noMonitorDirs.removeAll( mDirPath );
      settings.setValue( QStringLiteral( "qgis/disableMonitorItemUris" ), noMonitorDirs );

      if ( !alwaysMonitorDirs.contains( mDirPath ) )
      {
        alwaysMonitorDirs.append( mDirPath );
        settings.setValue( QStringLiteral( "qgis/alwaysMonitorItemUris" ), alwaysMonitorDirs );
      }

      mMonitored = true;
      break;
    }
  }

  const QVector<QgsDataItem *> childItems = children();
  for ( QgsDataItem *child : childItems )
  {
    if ( QgsDirectoryItem *dirItem = qobject_cast< QgsDirectoryItem *>( child ) )
      dirItem->reevaluateMonitoring();
  }

  createOrDestroyFileSystemWatcher();
}

QVector<QgsDataItem *> QgsDirectoryItem::createChildren()
{
  QVector<QgsDataItem *> children;
  const QDir dir( mDirPath );

  const QList<QgsDataItemProvider *> providers = QgsApplication::dataItemProviderRegistry()->providers();

  const QStringList entries = dir.entryList( QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase );
  for ( const QString &subdir : entries )
  {
    if ( mRefreshLater )
    {
      deleteLater( children );
      return children;
    }

    const QString subdirPath = dir.absoluteFilePath( subdir );

    QgsDebugMsgLevel( QStringLiteral( "creating subdir: %1" ).arg( subdirPath ), 2 );

    const QString path = mPath + '/' + subdir; // may differ from subdirPath
    if ( QgsDirectoryItem::hiddenPath( path ) )
      continue;

    bool handledByProvider = false;
    for ( QgsDataItemProvider *provider : providers )
    {
      if ( provider->handlesDirectoryPath( path ) )
      {
        handledByProvider = true;
        break;
      }
    }
    if ( handledByProvider )
      continue;

    QgsDirectoryItem *item = new QgsDirectoryItem( this, subdir, subdirPath, path );

    // we want directories shown before files
    item->setSortKey( QStringLiteral( "  %1" ).arg( subdir ) );

    // propagate signals up to top

    children.append( item );
  }

  const QStringList fileEntries = dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot | QDir::Files, QDir::Name );
  for ( const QString &name : fileEntries )
  {
    if ( mRefreshLater )
    {
      deleteLater( children );
      return children;
    }

    const QString path = dir.absoluteFilePath( name );
    const QFileInfo fileInfo( path );

    if ( fileInfo.suffix().compare( QLatin1String( "zip" ), Qt::CaseInsensitive ) == 0 ||
         fileInfo.suffix().compare( QLatin1String( "tar" ), Qt::CaseInsensitive ) == 0 )
    {
      QgsDataItem *item = QgsZipItem::itemFromPath( this, path, name, path );
      if ( item )
      {
        children.append( item );
        continue;
      }
    }

    bool createdItem = false;
    for ( QgsDataItemProvider *provider : providers )
    {
      const int capabilities = provider->capabilities();

      if ( !( ( fileInfo.isFile() && ( capabilities & QgsDataProvider::File ) ) ||
              ( fileInfo.isDir() && ( capabilities & QgsDataProvider::Dir ) ) ) )
      {
        continue;
      }

      QgsDataItem *item = provider->createDataItem( path, this );
      if ( item )
      {
        // 3rd party providers may not correctly set the ItemRepresentsFile capability, so force it here if we
        // see that the item's path does match the original file path
        if ( item->path() == path )
          item->setCapabilities( item->capabilities2() | Qgis::BrowserItemCapability::ItemRepresentsFile );

        children.append( item );
        createdItem = true;
      }
    }

    if ( !createdItem )
    {
      // if item is a QGIS project, and no specific item provider has overridden handling of
      // project items, then use the default project item behavior
      if ( fileInfo.suffix().compare( QLatin1String( "qgs" ), Qt::CaseInsensitive ) == 0 ||
           fileInfo.suffix().compare( QLatin1String( "qgz" ), Qt::CaseInsensitive ) == 0 )
      {
        QgsDataItem *item = new QgsProjectItem( this, fileInfo.completeBaseName(), path );
        item->setCapabilities( item->capabilities2() | Qgis::BrowserItemCapability::ItemRepresentsFile );
        children.append( item );
        continue;
      }
    }

  }
  return children;
}

void QgsDirectoryItem::setState( Qgis::BrowserItemState state )
{
  QgsDataCollectionItem::setState( state );

  if ( state == Qgis::BrowserItemState::Populated && mMonitored )
  {
    if ( !mFileSystemWatcher )
    {
      mFileSystemWatcher = new QFileSystemWatcher( this );
      mFileSystemWatcher->addPath( mDirPath );
      connect( mFileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &QgsDirectoryItem::directoryChanged );
    }
    mLastScan = QDateTime::currentDateTime();
  }
  else if ( state == Qgis::BrowserItemState::NotPopulated )
  {
    if ( mFileSystemWatcher )
    {
      delete mFileSystemWatcher;
      mFileSystemWatcher = nullptr;
    }
  }
}

void QgsDirectoryItem::directoryChanged()
{
  // If the last scan was less than 10 seconds ago, skip this
  if ( mLastScan.msecsTo( QDateTime::currentDateTime() ) < QgsSettings().value( QStringLiteral( "browser/minscaninterval" ), 10000 ).toInt() )
  {
    return;
  }
  if ( state() == Qgis::BrowserItemState::Populating )
  {
    // schedule to refresh later, because refresh() simply returns if Populating
    mRefreshLater = true;
  }
  else
  {
    // We definintely don't want the temporary files created by sqlite
    // to re-trigger a refresh in an infinite loop.
    disconnect( mFileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &QgsDirectoryItem::directoryChanged );
    // QFileSystemWhatcher::directoryChanged is emitted when a
    // file is created and not when it is closed/flushed.
    //
    // Delay to give to OS the time to complete writing the file
    // this happens when a new file appears in the directory and
    // the item's children thread will try to open the file with
    // GDAL or OGR even if it is still being written.
    QTimer::singleShot( 100, this, [ = ] { refresh(); } );
  }
}

bool QgsDirectoryItem::hiddenPath( const QString &path )
{
  const QgsSettings settings;
  const QStringList hiddenItems = settings.value( QStringLiteral( "browser/hiddenPaths" ),
                                  QStringList() ).toStringList();
  const int idx = hiddenItems.indexOf( path );
  return ( idx > -1 );
}

Qgis::BrowserDirectoryMonitoring QgsDirectoryItem::monitoringForPath( const QString &path )
{
  const QgsSettings settings;
  if ( settings.value( QStringLiteral( "qgis/disableMonitorItemUris" ), QStringList() ).toStringList().contains( path ) )
    return Qgis::BrowserDirectoryMonitoring::NeverMonitor;
  else if ( settings.value( QStringLiteral( "qgis/alwaysMonitorItemUris" ), QStringList() ).toStringList().contains( path ) )
    return Qgis::BrowserDirectoryMonitoring::AlwaysMonitor;
  return Qgis::BrowserDirectoryMonitoring::Default;
}

bool QgsDirectoryItem::pathShouldByMonitoredByDefault( const QString &path )
{
  // check through path's parent directories, to see if any have an explicit
  // always/never monitor setting. If so, this path will inherit that setting
  const QString originalPath = QDir::cleanPath( path );
  QString currentPath = originalPath;
  QString prevPath;
  while ( currentPath != prevPath )
  {
    prevPath = currentPath;
    currentPath = QFileInfo( currentPath ).path();

    switch ( monitoringForPath( currentPath ) )
    {
      case Qgis::BrowserDirectoryMonitoring::NeverMonitor:
        return false;
      case Qgis::BrowserDirectoryMonitoring::AlwaysMonitor:
        return true;
      case Qgis::BrowserDirectoryMonitoring::Default:
        break;
    }
  }

  // else if we know that the path is on a slow device, we don't monitor by default
  // as this can be very expensive and slow down QGIS
  if ( QgsFileUtils::pathIsSlowDevice( path ) )
    return false;

  // paths are monitored by default if no explicit setting is in place, and the user hasn't
  // completely opted out of all browser monitoring
  return QgsSettings().value( QStringLiteral( "/qgis/monitorDirectoriesInBrowser" ), true ).toBool();
}

void QgsDirectoryItem::childrenCreated()
{
  QgsDebugMsgLevel( QStringLiteral( "mRefreshLater = %1" ).arg( mRefreshLater ), 3 );

  if ( mRefreshLater )
  {
    QgsDebugMsgLevel( QStringLiteral( "directory changed during createChidren() -> refresh() again" ), 3 );
    mRefreshLater = false;
    setState( Qgis::BrowserItemState::Populated );
    refresh();
  }
  else
  {
    QgsDataCollectionItem::childrenCreated();
  }
  // Re-connect the file watcher after all children have been created
  if ( mFileSystemWatcher && mMonitored )
    connect( mFileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &QgsDirectoryItem::directoryChanged );
}

bool QgsDirectoryItem::equal( const QgsDataItem *other )
{
  //QgsDebugMsg ( mPath + " x " + other->mPath );
  if ( type() != other->type() )
  {
    return false;
  }
  return ( path() == other->path() );
}

QWidget *QgsDirectoryItem::paramWidget()
{
  return new QgsDirectoryParamWidget( mPath );
}

QgsMimeDataUtils::UriList QgsDirectoryItem::mimeUris() const
{
  QgsMimeDataUtils::Uri u;
  u.layerType = QStringLiteral( "directory" );
  u.name = mName;
  u.uri = mDirPath;
  u.filePath = path();
  return { u };
}

//
// QgsDirectoryParamWidget
//
QgsDirectoryParamWidget::QgsDirectoryParamWidget( const QString &path, QWidget *parent )
  : QTreeWidget( parent )
{
  setRootIsDecorated( false );

  // name, size, date, permissions, owner, group, type
  setColumnCount( 7 );
  QStringList labels;
  labels << tr( "Name" ) << tr( "Size" ) << tr( "Date" ) << tr( "Permissions" ) << tr( "Owner" ) << tr( "Group" ) << tr( "Type" );
  setHeaderLabels( labels );

  const QIcon iconDirectory = QgsApplication::getThemeIcon( QStringLiteral( "mIconFolder.svg" ) );
  const QIcon iconFile = QgsApplication::getThemeIcon( QStringLiteral( "mIconFile.svg" ) );
  const QIcon iconDirLink = QgsApplication::getThemeIcon( QStringLiteral( "mIconFolderLink.svg" ) );
  const QIcon iconFileLink = QgsApplication::getThemeIcon( QStringLiteral( "mIconFileLink.svg" ) );

  QList<QTreeWidgetItem *> items;

  const QDir dir( path );
  const QStringList entries = dir.entryList( QDir::AllEntries | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase );
  for ( const QString &name : entries )
  {
    const QFileInfo fi( dir.absoluteFilePath( name ) );
    QStringList texts;
    texts << name;
    QString size;
    if ( fi.size() > 1024 )
    {
      size = QStringLiteral( "%1 KiB" ).arg( QLocale().toString( fi.size() / 1024.0, 'f', 1 ) );
    }
    else if ( fi.size() > 1.048576e6 )
    {
      size = QStringLiteral( "%1 MiB" ).arg( QLocale().toString( fi.size() / 1.048576e6, 'f', 1 ) );
    }
    else
    {
      size = QStringLiteral( "%1 B" ).arg( fi.size() );
    }
    texts << size;
    texts << QLocale().toString( fi.lastModified(), QLocale::ShortFormat );
    QString perm;
    perm += fi.permission( QFile::ReadOwner ) ? 'r' : '-';
    perm += fi.permission( QFile::WriteOwner ) ? 'w' : '-';
    perm += fi.permission( QFile::ExeOwner ) ? 'x' : '-';
    // QFile::ReadUser, QFile::WriteUser, QFile::ExeUser
    perm += fi.permission( QFile::ReadGroup ) ? 'r' : '-';
    perm += fi.permission( QFile::WriteGroup ) ? 'w' : '-';
    perm += fi.permission( QFile::ExeGroup ) ? 'x' : '-';
    perm += fi.permission( QFile::ReadOther ) ? 'r' : '-';
    perm += fi.permission( QFile::WriteOther ) ? 'w' : '-';
    perm += fi.permission( QFile::ExeOther ) ? 'x' : '-';
    texts << perm;

    texts << fi.owner();
    texts << fi.group();

    QString type;
    QIcon icon;
    if ( fi.isDir() && fi.isSymLink() )
    {
      type = tr( "folder" );
      icon = iconDirLink;
    }
    else if ( fi.isDir() )
    {
      type = tr( "folder" );
      icon = iconDirectory;
    }
    else if ( fi.isFile() && fi.isSymLink() )
    {
      type = tr( "file" );
      icon = iconFileLink;
    }
    else if ( fi.isFile() )
    {
      type = tr( "file" );
      icon = iconFile;
    }

    texts << type;

    QTreeWidgetItem *item = new QTreeWidgetItem( texts );
    item->setIcon( 0, icon );
    items << item;
  }

  addTopLevelItems( items );

  // hide columns that are not requested
  const QgsSettings settings;
  const QList<QVariant> lst = settings.value( QStringLiteral( "dataitem/directoryHiddenColumns" ) ).toList();
  for ( const QVariant &colVariant : lst )
  {
    setColumnHidden( colVariant.toInt(), true );
  }
}

void QgsDirectoryParamWidget::mousePressEvent( QMouseEvent *event )
{
  if ( event->button() == Qt::RightButton )
  {
    // show the popup menu
    QMenu popupMenu;

    QStringList labels;
    labels << tr( "Name" ) << tr( "Size" ) << tr( "Date" ) << tr( "Permissions" ) << tr( "Owner" ) << tr( "Group" ) << tr( "Type" );
    for ( int i = 0; i < labels.count(); i++ )
    {
      QAction *action = popupMenu.addAction( labels[i], this, &QgsDirectoryParamWidget::showHideColumn );
      action->setObjectName( QString::number( i ) );
      action->setCheckable( true );
      action->setChecked( !isColumnHidden( i ) );
    }

    popupMenu.exec( event->globalPos() );
  }
}

void QgsDirectoryParamWidget::showHideColumn()
{
  QAction *action = qobject_cast<QAction *>( sender() );
  if ( !action )
    return; // something is wrong

  const int columnIndex = action->objectName().toInt();
  setColumnHidden( columnIndex, !isColumnHidden( columnIndex ) );

  // save in settings
  QgsSettings settings;
  QList<QVariant> lst;
  for ( int i = 0; i < columnCount(); i++ )
  {
    if ( isColumnHidden( i ) )
      lst.append( QVariant( i ) );
  }
  settings.setValue( QStringLiteral( "dataitem/directoryHiddenColumns" ), lst );
}

//
// QgsProjectHomeItem
//

QgsProjectHomeItem::QgsProjectHomeItem( QgsDataItem *parent, const QString &name, const QString &dirPath, const QString &path )
  : QgsDirectoryItem( parent, name, dirPath, path, QStringLiteral( "special:ProjectHome" ) )
{
}

QIcon QgsProjectHomeItem::icon()
{
  if ( state() == Qgis::BrowserItemState::Populating )
    return QgsDirectoryItem::icon();
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconFolderProject.svg" ) );
}

QVariant QgsProjectHomeItem::sortKey() const
{
  return QStringLiteral( " 1" );
}


