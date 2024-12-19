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
#include "qgsgui.h"
#include "qgssettings.h"
#include "qgsproject.h"

#include <QTreeWidget>
#include <QPushButton>

///@cond PRIVATE

QgsStacDownloadAssetsDialog::QgsStacDownloadAssetsDialog( QWidget *parent ) :
  QDialog( parent )
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

  connect( mSelectAllButton, &QPushButton::clicked, this, &QgsStacDownloadAssetsDialog::selectAll );
  connect( mDeselectAllButton, &QPushButton::clicked, this, &QgsStacDownloadAssetsDialog::deselectAll );
}

void QgsStacDownloadAssetsDialog::setStacItem( QgsStacItem *stacItem )
{
  if ( ! stacItem )
    return;

  const QMap< QString, QgsStacAsset > assets = stacItem->assets();
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
