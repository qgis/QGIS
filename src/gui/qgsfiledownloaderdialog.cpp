/***************************************************************************
  qgsfiledownloaderdialog.cpp
  --------------------------------------
  Date                 : November 2016
  Copyright            : (C) 2016 by Alessandro Pasotti
  Email                : apasotti at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfiledownloaderdialog.h"
#include "moc_qgsfiledownloaderdialog.cpp"
#include "qgsfiledownloader.h"
#include "qgsfileutils.h"
#include <QMessageBox>

QgsFileDownloaderDialog::QgsFileDownloaderDialog( const QUrl &url, const QString &outputFileName, const QString &authcfg )
  : mOutputFileName( outputFileName ), mDownloader( new QgsFileDownloader( url, outputFileName, authcfg, true ) )
{
  setWindowTitle( tr( "Download" ) );
  setLabelText( tr( "Downloading %1." ).arg( outputFileName ) );
  show();

  connect( this, &QProgressDialog::canceled, mDownloader, &QgsFileDownloader::cancelDownload );
  connect( mDownloader, &QgsFileDownloader::downloadError, this, &QgsFileDownloaderDialog::onError );
  connect( mDownloader, &QgsFileDownloader::downloadProgress, this, &QgsFileDownloaderDialog::onDownloadProgress );
  connect( mDownloader, &QgsFileDownloader::downloadExited, this, &QgsFileDownloaderDialog::deleteLater );

  connect( mDownloader, &QgsFileDownloader::downloadCompleted, this, &QgsFileDownloaderDialog::downloadCompleted );
  connect( mDownloader, &QgsFileDownloader::downloadCanceled, this, &QgsFileDownloaderDialog::downloadCanceled );
  connect( mDownloader, &QgsFileDownloader::downloadExited, this, &QgsFileDownloaderDialog::downloadExited );
  connect( mDownloader, &QgsFileDownloader::downloadError, this, &QgsFileDownloaderDialog::downloadError );
  connect( mDownloader, &QgsFileDownloader::downloadProgress, this, &QgsFileDownloaderDialog::downloadProgress );
  mDownloader->startDownload();
}

void QgsFileDownloaderDialog::onError( const QStringList &errors )
{
  QMessageBox::warning( nullptr, tr( "Download File" ), errors.join( QLatin1String( "<br>" ) ) );
}

void QgsFileDownloaderDialog::onDownloadProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  setMaximum( bytesTotal );
  setValue( bytesReceived );
  setLabelText( tr( "Downloading %1 of %2 %3." ).arg( QgsFileUtils::representFileSize( bytesReceived ), QgsFileUtils::representFileSize( bytesTotal ), mOutputFileName ) );
}
