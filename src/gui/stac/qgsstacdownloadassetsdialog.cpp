/***************************************************************************
    qgsstacdownloadassetsdialog.cpp
    ---------------------
    begin                : September 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstacdownloadassetsdialog.h"
#include "moc_qgsstacdownloadassetsdialog.cpp"
#include "qgsgui.h"
#include "qgsnetworkcontentfetchertask.h"
#include "qgssettings.h"
#include "qgsproject.h"
#include "qgsmessagebar.h"
#include "qgsapplication.h"

#include <QTreeWidget>
#include <QPushButton>
#include <QAction>
#include <QMenu>
#include <QClipboard>

///@cond PRIVATE

QgsStacDownloadAssetsDialog::QgsStacDownloadAssetsDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );
  mFileWidget->setStorageMode( QgsFileWidget::StorageMode::GetDirectory );

  QString defPath = QDir::cleanPath( QFileInfo( QgsProject::instance()->absoluteFilePath() ).path() );
  defPath = QgsSettings().value( QStringLiteral( "UI/lastFileNameWidgetDir" ), defPath ).toString();
  if ( defPath.isEmpty() )
    defPath = QDir::homePath();
  mFileWidget->setFilePath( defPath );
  mFileWidget->lineEdit()->setReadOnly( true );

  mContextMenu = new QMenu( this );

  connect( mSelectAllButton, &QPushButton::clicked, this, &QgsStacDownloadAssetsDialog::selectAll );
  connect( mDeselectAllButton, &QPushButton::clicked, this, &QgsStacDownloadAssetsDialog::deselectAll );

  mTreeWidget->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( mTreeWidget, &QWidget::customContextMenuRequested, this, &QgsStacDownloadAssetsDialog::showContextMenu );
}

void QgsStacDownloadAssetsDialog::accept()
{
  const QString folder = selectedFolder();
  const QStringList urls = selectedUrls();
  for ( const QString &url : urls )
  {
    QgsNetworkContentFetcherTask *fetcher = new QgsNetworkContentFetcherTask( url, mAuthCfg, QgsTask::CanCancel, tr( "Downloading STAC asset" ) );

    connect( fetcher, &QgsNetworkContentFetcherTask::errorOccurred, fetcher, [bar = mMessageBar]( QNetworkReply::NetworkError, const QString &errorMsg ) {
      if ( bar )
        bar->pushMessage(
          tr( "Error downloading STAC asset" ),
          errorMsg,
          Qgis::MessageLevel::Critical
        );
    } );

    connect( fetcher, &QgsNetworkContentFetcherTask::fetched, fetcher, [fetcher, folder, bar = mMessageBar] {
      QNetworkReply *reply = fetcher->reply();
      if ( !reply || reply->error() != QNetworkReply::NoError )
      {
        // canceled or failed
        return;
      }
      else
      {
        const QString fileName = fetcher->contentDispositionFilename().isEmpty() ? reply->url().fileName() : fetcher->contentDispositionFilename();
        QFileInfo fi( fileName );
        QFile file( QStringLiteral( "%1/%2" ).arg( folder, fileName ) );
        int i = 1;
        while ( file.exists() )
        {
          QString uniqueName = QStringLiteral( "%1/%2(%3)" ).arg( folder, fi.baseName() ).arg( i++ );
          if ( !fi.completeSuffix().isEmpty() )
            uniqueName.append( QStringLiteral( ".%1" ).arg( fi.completeSuffix() ) );
          file.setFileName( uniqueName );
        }

        bool failed = false;
        if ( file.open( QIODevice::WriteOnly ) )
        {
          const QByteArray data = reply->readAll();
          if ( file.write( data ) < 0 )
            failed = true;

          file.close();
        }
        else
        {
          failed = true;
        }

        if ( failed )
        {
          if ( bar )
            bar->pushMessage(
              tr( "Error downloading STAC asset" ),
              tr( "Could not write to file %1" ).arg( file.fileName() ),
              Qgis::MessageLevel::Critical
            );
        }
        else
        {
          if ( bar )
            bar->pushMessage(
              tr( "STAC asset downloaded" ),
              file.fileName(),
              Qgis::MessageLevel::Success
            );
        }
      }
    } );

    QgsApplication::taskManager()->addTask( fetcher );
  }

  QDialog::accept();
}

void QgsStacDownloadAssetsDialog::setAuthCfg( const QString &authCfg )
{
  mAuthCfg = authCfg;
}

void QgsStacDownloadAssetsDialog::setMessageBar( QgsMessageBar *bar )
{
  mMessageBar = bar;
}

void QgsStacDownloadAssetsDialog::setStacItem( QgsStacItem *stacItem )
{
  if ( !stacItem )
    return;

  const QMap<QString, QgsStacAsset> assets = stacItem->assets();
  for ( auto it = assets.constBegin(); it != assets.constEnd(); ++it )
  {
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText( 0, it.key() );
    item->setToolTip( 0, it.key() );
    item->setCheckState( 0, Qt::Checked );
    item->setText( 1, it->title() );
    item->setToolTip( 1, it->title() );
    item->setText( 2, it->description() );
    item->setToolTip( 2, it->description() );
    item->setText( 3, it->roles().join( "," ) );
    item->setToolTip( 3, it->roles().join( "," ) );
    item->setText( 4, it->mediaType() );
    item->setToolTip( 4, it->mediaType() );
    item->setText( 5, it->href() );
    item->setToolTip( 5, it->href() );

    mTreeWidget->addTopLevelItem( item );
  }
}

QString QgsStacDownloadAssetsDialog::selectedFolder()
{
  return mFileWidget->filePath();
}

QStringList QgsStacDownloadAssetsDialog::selectedUrls()
{
  QStringList urls;
  for ( int i = 0; i < mTreeWidget->topLevelItemCount(); ++i )
  {
    QTreeWidgetItem *item = mTreeWidget->topLevelItem( i );
    if ( item->checkState( 0 ) == Qt::Checked )
      urls.append( item->text( 5 ) );
  }
  return urls;
}

void QgsStacDownloadAssetsDialog::showContextMenu( QPoint p )
{
  QTreeWidgetItem *item = mTreeWidget->itemAt( p );
  if ( !item )
    return;

  mTreeWidget->setCurrentItem( item );
  mContextMenu->clear();

  const QString url = item->text( 5 );
  QAction *copyUrlAction = new QAction( tr( "Copy URL" ), mContextMenu );
  connect( copyUrlAction, &QAction::triggered, this, [url] {
    QApplication::clipboard()->setText( url );
  } );
  mContextMenu->addAction( copyUrlAction );
  mContextMenu->exec( QCursor::pos() );
}

void QgsStacDownloadAssetsDialog::selectAll()
{
  for ( int i = 0; i < mTreeWidget->topLevelItemCount(); ++i )
  {
    QTreeWidgetItem *item = mTreeWidget->topLevelItem( i );
    item->setCheckState( 0, Qt::Checked );
  }
}

void QgsStacDownloadAssetsDialog::deselectAll()
{
  for ( int i = 0; i < mTreeWidget->topLevelItemCount(); ++i )
  {
    QTreeWidgetItem *item = mTreeWidget->topLevelItem( i );
    item->setCheckState( 0, Qt::Unchecked );
  }
}

///@endcond
