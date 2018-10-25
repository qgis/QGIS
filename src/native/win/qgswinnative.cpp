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
#include <QAbstractEventDispatcher>
#include <QtWinExtras/QWinTaskbarButton>
#include <QtWinExtras/QWinTaskbarProgress>
#include <QtWinExtras/QWinJumpList>
#include <QtWinExtras/QWinJumpListItem>
#include <QtWinExtras/QWinJumpListCategory>
#include "wintoastlib.h"
#include <Dbt.h>

QgsNative::Capabilities QgsWinNative::capabilities() const
{
  return mCapabilities;
}

void QgsWinNative::initializeMainWindow( QWindow *window,
    const QString &applicationName,
    const QString &organizationName,
    const QString &version )
{
  if ( mTaskButton )
    return; // already initialized!

  mTaskButton = new QWinTaskbarButton( window );
  mTaskButton->setWindow( window );
  mTaskProgress = mTaskButton->progress();
  mTaskProgress->setVisible( false );

  WinToastLib::WinToast::instance()->setAppName( applicationName.toStdWString() );
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
}

void QgsWinNative::openFileExplorerAndSelectFile( const QString &path )
{
  const QString nativePath = QDir::toNativeSeparators( path );

  wchar_t *pathArray = new wchar_t[static_cast< uint>( nativePath.length() + 1 )];
  nativePath.toWCharArray( pathArray );
  pathArray[nativePath.length()] = 0;

  ITEMIDLIST *pidl = ILCreateFromPathW( pathArray );
  if ( pidl )
  {
    SHOpenFolderAndSelectItems( pidl, 0, nullptr, 0 );
    ILFree( pidl );
  }

  delete[] pathArray;
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
  mTaskProgress->setValue( progress );
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

bool QgsWinNativeEventFilter::nativeEventFilter( const QByteArray &, void *message, long * )
{
  MSG *pWindowsMessage = static_cast<MSG *>( message );
  unsigned int wParam = pWindowsMessage->wParam;
  if ( wParam == DBT_DEVICEARRIVAL || wParam == DBT_DEVICEREMOVECOMPLETE )
  {
    long lParam = pWindowsMessage->lParam;
    unsigned long deviceType = reinterpret_cast<DEV_BROADCAST_HDR *>( lParam )->dbch_devicetype;
    if ( deviceType == DBT_DEVTYP_VOLUME )
    {
      // need to handle disks with multiple partitions -- these are given by a single event
      unsigned long unitmask = reinterpret_cast<DEV_BROADCAST_VOLUME *>( lParam )->dbcv_unitmask;
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
    }
  }
  return false;
}
