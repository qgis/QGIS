/***************************************************************************
    qgswinnative.cpp - abstracted interface to native system calls
                             -------------------
    begin                : January 2017
    copyright            : (C) 2017 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswinnative.h"
#include <QCoreApplication>
#include <QDebug>
#include <QString>
#include <QDir>
#include <QWindow>
#include <QProcess>
#include <QAbstractEventDispatcher>
#include <QtWinExtras/QWinTaskbarButton>
#include <QtWinExtras/QWinTaskbarProgress>
#include <QtWinExtras/QWinJumpList>
#include <QtWinExtras/QWinJumpListItem>
#include <QtWinExtras/QWinJumpListCategory>
#include "wintoastlib.h"
#include <Dbt.h>
#include <memory>
#include <type_traits>


struct LPITEMIDLISTDeleter
{
  void operator()( LPITEMIDLIST pidl ) const
  {
    ILFree( pidl );
  }
};

using ITEMIDLIST_unique_ptr = std::unique_ptr< std::remove_pointer_t< LPITEMIDLIST >, LPITEMIDLISTDeleter>;



QgsNative::Capabilities QgsWinNative::capabilities() const
{
  return mCapabilities;
}

void QgsWinNative::initializeMainWindow( QWindow *window,
    const QString &applicationName,
    const QString &organizationName,
    const QString &version )
{
  mWindow = window;
  if ( mTaskButton )
    return; // already initialized!

  mTaskButton = new QWinTaskbarButton( window );
  mTaskButton->setWindow( window );
  mTaskProgress = mTaskButton->progress();
  mTaskProgress->setVisible( false );

  QString appName = qgetenv( "QGIS_WIN_APP_NAME" );
  if ( appName.isEmpty() )
    appName = applicationName;

  WinToastLib::WinToast::instance()->setAppName( appName.toStdWString() );
  WinToastLib::WinToast::instance()->setAppUserModelId(
    WinToastLib::WinToast::configureAUMI( organizationName.toStdWString(),
                                          applicationName.toStdWString(),
                                          applicationName.toStdWString(),
                                          version.toStdWString() ) );
  if ( WinToastLib::WinToast::instance()->initialize() )
  {
    mWinToastInitialized = true;
    mCapabilities = mCapabilities | NativeDesktopNotifications;
  }

  mNativeEventFilter = new QgsWinNativeEventFilter();
  QAbstractEventDispatcher::instance()->installNativeEventFilter( mNativeEventFilter );
  connect( mNativeEventFilter, &QgsWinNativeEventFilter::usbStorageNotification, this, &QgsNative::usbStorageNotification );
}

void QgsWinNative::cleanup()
{
  if ( mWinToastInitialized )
    WinToastLib::WinToast::instance()->clear();
  mWindow = nullptr;
}

std::unique_ptr< wchar_t[] > pathToWChar( const QString &path )
{
  const QString nativePath = QDir::toNativeSeparators( path );

  std::unique_ptr< wchar_t[] > pathArray( new wchar_t[static_cast< uint>( nativePath.length() + 1 )] );
  nativePath.toWCharArray( pathArray.get() );
  pathArray[static_cast< size_t >( nativePath.length() )] = 0;
  return pathArray;
}

void QgsWinNative::openFileExplorerAndSelectFile( const QString &path )
{
  std::unique_ptr< wchar_t[] > pathArray = pathToWChar( path );
  ITEMIDLIST_unique_ptr pidl( ILCreateFromPathW( pathArray.get() ) );
  if ( pidl )
  {
    SHOpenFolderAndSelectItems( pidl.get(), 0, nullptr, 0 );
    pidl.reset();
  }
}

void QgsWinNative::showFileProperties( const QString &path )
{
  std::unique_ptr< wchar_t[] > pathArray = pathToWChar( path );
  ITEMIDLIST_unique_ptr pidl( ILCreateFromPathW( pathArray.get() ) );
  if ( pidl )
  {
    SHELLEXECUTEINFO info{ sizeof( info ) };
    if ( mWindow )
      info.hwnd = reinterpret_cast<HWND>( mWindow->winId() );
    info.nShow = SW_SHOWNORMAL;
    info.fMask = SEE_MASK_INVOKEIDLIST;
    info.lpIDList = pidl.get();
    info.lpVerb = "properties";

    ShellExecuteEx( &info );
  }
}

void QgsWinNative::showUndefinedApplicationProgress()
{
  mTaskProgress->setMaximum( 0 );
  mTaskProgress->show();
}

void QgsWinNative::setApplicationProgress( double progress )
{
  mTaskProgress->setMaximum( 100 );
  mTaskProgress->show();
  mTaskProgress->setValue( static_cast< int >( std::round( progress ) ) );
}

void QgsWinNative::hideApplicationProgress()
{
  mTaskProgress->hide();
}

void QgsWinNative::onRecentProjectsChanged( const std::vector<QgsNative::RecentProjectProperties> &recentProjects )
{
  QWinJumpList jumplist;
  jumplist.recent()->clear();
  for ( const RecentProjectProperties &recentProject : recentProjects )
  {
    QString name = recentProject.title != recentProject.path ? recentProject.title : QFileInfo( recentProject.path ).baseName();
    QWinJumpListItem *newProject = new QWinJumpListItem( QWinJumpListItem::Link );
    newProject->setTitle( name );
    newProject->setFilePath( QDir::toNativeSeparators( QCoreApplication::applicationFilePath() ) );
    newProject->setArguments( QStringList( recentProject.path ) );
    jumplist.recent()->addItem( newProject );
  }
}

class NotificationHandler : public WinToastLib::IWinToastHandler
{
  public:

    void toastActivated() const override {}

    void toastActivated( int ) const override {}

    void toastFailed() const override
    {
      qWarning() << "Error showing notification";
    }

    void toastDismissed( WinToastDismissalReason ) const override {}
};


QgsNative::NotificationResult QgsWinNative::showDesktopNotification( const QString &summary, const QString &body, const QgsNative::NotificationSettings &settings )
{
  NotificationResult result;
  if ( !mWinToastInitialized )
  {
    result.successful = false;
    return result;
  }

  WinToastLib::WinToastTemplate templ = WinToastLib::WinToastTemplate( WinToastLib::WinToastTemplate::ImageAndText02 );
  templ.setImagePath( settings.pngAppIconPath.toStdWString() );
  templ.setTextField( summary.toStdWString(), WinToastLib::WinToastTemplate::FirstLine );
  templ.setTextField( body.toStdWString(), WinToastLib::WinToastTemplate::SecondLine );
  templ.setDuration( WinToastLib::WinToastTemplate::Short );


  if ( WinToastLib::WinToast::instance()->showToast( templ, new NotificationHandler ) < 0 )
    result.successful = false;
  else
    result.successful = true;

  return result;
}

bool QgsWinNative::openTerminalAtPath( const QString &path )
{
  // logic from https://github.com/Microsoft/vscode/blob/fec1775aa52e2124d3f09c7b2ac8f69c57309549/src/vs/workbench/parts/execution/electron-browser/terminal.ts#L44
  const bool isWow64 = qEnvironmentVariableIsSet( "PROCESSOR_ARCHITEW6432" );
  QString windir = qgetenv( "WINDIR" );
  if ( windir.isEmpty() )
    windir = QStringLiteral( "C:\\Windows" );
  const QString term = QStringLiteral( "%1\\%2\\cmd.exe" ).arg( windir, isWow64 ? QStringLiteral( "Sysnative" ) : QStringLiteral( "System32" ) );

  QProcess process;
  process.setProgram( term );
  process.setCreateProcessArgumentsModifier( []( QProcess::CreateProcessArguments * args )
  {
    args->flags |= CREATE_NEW_CONSOLE;
    args->startupInfo->dwFlags &= ~ STARTF_USESTDHANDLES;
  } );
  process.setWorkingDirectory( path );

  qint64 pid;
  return process.startDetached( &pid );
}

bool QgsWinNativeEventFilter::nativeEventFilter( const QByteArray &eventType, void *message, long * )
{
  static const QByteArray sWindowsGenericMSG{ "windows_generic_MSG" };
  if ( !message || eventType != sWindowsGenericMSG )
    return false;

  MSG *pWindowsMessage = static_cast<MSG *>( message );
  if ( pWindowsMessage->message != WM_DEVICECHANGE )
  {
    return false;
  }

  unsigned int wParam = pWindowsMessage->wParam;
  if ( wParam == DBT_DEVICEARRIVAL || wParam == DBT_DEVICEREMOVECOMPLETE )
  {
    if ( !pWindowsMessage->lParam )
      return false;

    unsigned long deviceType = reinterpret_cast<DEV_BROADCAST_HDR *>( pWindowsMessage->lParam )->dbch_devicetype;
    if ( deviceType == DBT_DEVTYP_VOLUME )
    {
      const DEV_BROADCAST_VOLUME *broadcastVolume = reinterpret_cast<const DEV_BROADCAST_VOLUME *>( pWindowsMessage->lParam );
      if ( !broadcastVolume )
        return false;

      // Seen in qfilesystemwatcher_win.cpp from Qt:
      // WM_DEVICECHANGE/DBT_DEVTYP_VOLUME messages are sent to all toplevel windows. Compare a hash value to ensure
      // it is handled only once.
      const quintptr newHash = reinterpret_cast<quintptr>( broadcastVolume ) + pWindowsMessage->wParam
                               + quintptr( broadcastVolume->dbcv_flags ) + quintptr( broadcastVolume->dbcv_unitmask );
      if ( newHash == mLastMessageHash )
        return false;
      mLastMessageHash = newHash;

      // need to handle disks with multiple partitions -- these are given by a single event
      unsigned long unitmask = broadcastVolume->dbcv_unitmask;
      std::vector< QString > drives;
      char driveName[] = "A:/";
      unitmask &= 0x3ffffff;
      while ( unitmask )
      {
        if ( unitmask & 0x1 )
          drives.emplace_back( QString::fromLatin1( driveName ) );
        ++driveName[0];
        unitmask >>= 1;
      }

      for ( const QString &drive : drives )
      {
        emit usbStorageNotification( QStringLiteral( "%1:/" ).arg( drive ), wParam == DBT_DEVICEARRIVAL );
      }
      return false;
    }
  }
  return false;
}
