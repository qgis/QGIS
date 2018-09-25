/***************************************************************************
                          qgshandlebadlayers.cpp  -  description
                             -------------------
    begin                : Sat 5 Mar 2011
    copyright            : (C) 2011 by Juergen E. Fischer, norBIT GmbH
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgshandlebadlayers.h"
#include "qgisapp.h"
#include "qgsauthconfigselect.h"
#include "qgsdataprovider.h"
#include "qgsguiutils.h"
#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "qgsrasterlayer.h"
#include "qgsproviderregistry.h"
#include "qgsmessagebar.h"
#include "qgssettings.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFileDialog>
#include <QPushButton>
#include <QToolButton>
#include <QMessageBox>
#include <QUrl>

void QgsHandleBadLayersHandler::handleBadLayers( const QList<QDomNode> &layers )
{
  QApplication::setOverrideCursor( Qt::ArrowCursor );
  QgsHandleBadLayers *dialog = new QgsHandleBadLayers( layers );

  if ( dialog->layerCount() < layers.size() )
    QgisApp::instance()->messageBar()->pushMessage(
      tr( "Handle bad layers" ),
      tr( "%1 of %2 bad layers were not fixable." )
      .arg( layers.size() - dialog->layerCount() )
      .arg( layers.size() ),
      Qgis::Warning, QgisApp::instance()->messageTimeout() );

  if ( dialog->layerCount() > 0 )
    dialog->exec();

  delete dialog;
  QApplication::restoreOverrideCursor();
}


QgsHandleBadLayers::QgsHandleBadLayers( const QList<QDomNode> &layers )
  : QDialog( QgisApp::instance() )
  , mLayers( layers )
{
  setupUi( this );

  mVectorFileFilter = QgsProviderRegistry::instance()->fileVectorFilters();
  mRasterFileFilter = QgsProviderRegistry::instance()->fileRasterFilters();

  mBrowseButton = new QPushButton( tr( "Browse" ) );
  buttonBox->addButton( mBrowseButton, QDialogButtonBox::ActionRole );
  mBrowseButton->setDisabled( true );

  connect( mLayerList, &QTableWidget::itemSelectionChanged, this, &QgsHandleBadLayers::selectionChanged );
  connect( mBrowseButton, &QAbstractButton::clicked, this, &QgsHandleBadLayers::browseClicked );
  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsHandleBadLayers::apply );

  mLayerList->clear();
  mLayerList->setSortingEnabled( true );
  mLayerList->setSelectionBehavior( QAbstractItemView::SelectRows );
  mLayerList->setColumnCount( 5 );
  mLayerList->setColumnWidth( 3, 75 );

  mLayerList->setHorizontalHeaderLabels( QStringList()
                                         << tr( "Layer name" )
                                         << tr( "Type" )
                                         << tr( "Provider" )
                                         << tr( "Auth config" )
                                         << tr( "Datasource" )
                                       );

  int j = 0;
  for ( int i = 0; i < mLayers.size(); i++ )
  {
    const QDomNode &node = mLayers[i];

    QString name = node.namedItem( QStringLiteral( "layername" ) ).toElement().text();
    QString type = node.toElement().attribute( QStringLiteral( "type" ) );
    QString datasource = node.namedItem( QStringLiteral( "datasource" ) ).toElement().text();
    QString provider = node.namedItem( QStringLiteral( "provider" ) ).toElement().text();
    QString vectorProvider = type == QLatin1String( "vector" ) ? provider : tr( "none" );
    bool providerFileBased = ( QgsProviderRegistry::instance()->providerCapabilities( provider ) & QgsDataProvider::File ) != 0;

    QgsDebugMsg( QString( "name=%1 type=%2 provider=%3 datasource='%4'" )
                 .arg( name,
                       type,
                       vectorProvider,
                       datasource ) );

    mLayerList->setRowCount( j + 1 );

    QTableWidgetItem *item = nullptr;

    item = new QTableWidgetItem( name );
    item->setData( Qt::UserRole + 0, i );
    item->setFlags( item->flags() & ~Qt::ItemIsEditable );
    mLayerList->setItem( j, 0, item );

    item = new QTableWidgetItem( type );
    item->setData( Qt::UserRole + 0, providerFileBased );
    item->setFlags( item->flags() & ~Qt::ItemIsEditable );
    mLayerList->setItem( j, 1, item );

    item = new QTableWidgetItem( vectorProvider );
    item->setFlags( item->flags() & ~Qt::ItemIsEditable );
    mLayerList->setItem( j, 2, item );

    if ( QgsAuthConfigUriEdit::hasConfigId( datasource ) )
    {
      QToolButton *btn = new QToolButton( this );
      btn->setMaximumWidth( 75 );
      btn->setMinimumHeight( 24 );
      btn->setText( tr( "Edit" ) );
      btn->setProperty( "row", j );
      connect( btn, &QAbstractButton::clicked, this, &QgsHandleBadLayers::editAuthCfg );
      mLayerList->setCellWidget( j, 3, btn );
    }
    else
    {
      item = new QTableWidgetItem( QString() );
      mLayerList->setItem( j, 3, item );
    }

    item = new QTableWidgetItem( datasource );
    mLayerList->setItem( j, 4, item );

    j++;
  }

  // mLayerList->resizeColumnsToContents();
}

void QgsHandleBadLayers::selectionChanged()
{

  mRows.clear();

  Q_FOREACH ( QTableWidgetItem *item, mLayerList->selectedItems() )
  {
    if ( item->column() != 0 )
      continue;

    bool providerFileBased = mLayerList->item( item->row(), 1 )->data( Qt::UserRole + 0 ).toBool();
    if ( !providerFileBased )
      continue;

    mRows << item->row();
  }

  mBrowseButton->setEnabled( !mRows.isEmpty() );
}

QString QgsHandleBadLayers::filename( int row )
{
  QString type = mLayerList->item( row, 1 )->text();
  QString provider = mLayerList->item( row, 2 )->text();
  QString datasource = mLayerList->item( row, 4 )->text();

  if ( type == QLatin1String( "vector" ) )
  {
    const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( provider, datasource );
    return parts.value( QLatin1String( "path" ) ).toString();
  }
  else
  {
    return datasource;
  }
}

void QgsHandleBadLayers::setFilename( int row, const QString &filename )
{
  if ( !QFileInfo::exists( filename ) )
    return;

  QString type = mLayerList->item( row, 1 )->text();
  QString provider = mLayerList->item( row, 2 )->text();
  QTableWidgetItem *item = mLayerList->item( row, 4 );

  QString datasource = item->text();

  if ( type == QLatin1String( "vector" ) )
  {
    if ( provider == QLatin1String( "spatialite" ) )
    {
      QgsDataSourceUri uri( datasource );
      uri.setDatabase( filename );
      datasource = uri.uri();
    }
    else if ( provider == QLatin1String( "ogr" ) )
    {
      QStringList theURIParts = datasource.split( '|' );
      theURIParts[0] = filename;
      datasource = theURIParts.join( QStringLiteral( "|" ) );
    }
    else if ( provider == QLatin1String( "delimitedtext" ) )
    {
      QUrl uriSource = QUrl::fromEncoded( datasource.toLatin1() );
      QUrl uriDest = QUrl::fromLocalFile( filename );
      uriDest.setQueryItems( uriSource.queryItems() );
      datasource = QString::fromLatin1( uriDest.toEncoded() );
    }
  }
  else
  {
    datasource = filename;
  }

  item->setText( datasource );
}

void QgsHandleBadLayers::browseClicked()
{

  if ( mRows.size() == 1 )
  {
    int row = mRows.at( 0 );
    QString type = mLayerList->item( row, 1 )->text();

    QString memoryQualifier, fileFilter;
    if ( type == QLatin1String( "vector" ) )
    {
      memoryQualifier = QStringLiteral( "lastVectorFileFilter" );
      fileFilter = mVectorFileFilter;
    }
    else
    {
      memoryQualifier = QStringLiteral( "lastRasterFileFilter" );
      fileFilter = mRasterFileFilter;
    }

    QString fn = filename( row );
    if ( fn.isNull() )
      return;

    QStringList selectedFiles;
    QString enc;
    QString title = tr( "Select File to Replace '%1'" ).arg( fn );

    QgsGuiUtils::openFilesRememberingFilter( memoryQualifier, fileFilter, selectedFiles, enc, title );
    if ( selectedFiles.size() != 1 )
    {
      QMessageBox::information( this, title, tr( "Please select exactly one file." ) );
      return;
    }

    setFilename( row, selectedFiles[0] );
  }
  else if ( mRows.size() > 1 )
  {
    QString title = tr( "Select New Directory of Selected Files" );

    QgsSettings settings;
    QString lastDir = settings.value( QStringLiteral( "UI/missingDirectory" ), QDir::homePath() ).toString();
    QString selectedFolder = QFileDialog::getExistingDirectory( this, title, lastDir );
    if ( selectedFolder.isEmpty() )
    {
      return;
    }

    QDir dir( selectedFolder );
    if ( !dir.exists() )
    {
      return;
    }

    Q_FOREACH ( int row, mRows )
    {
      bool providerFileBased = mLayerList->item( row, 1 )->data( Qt::UserRole + 0 ).toBool();
      if ( !providerFileBased )
        continue;

      QString fn = filename( row );
      if ( fn.isEmpty() )
        continue;

      QFileInfo fi( fn );
      fi.setFile( dir, fi.fileName() );
      if ( !fi.exists() )
        continue;

      setFilename( row, fi.absoluteFilePath() );
    }
  }
}

void QgsHandleBadLayers::editAuthCfg()
{
  QToolButton *btn = qobject_cast<QToolButton *>( sender() );
  int row = -1;
  for ( int i = 0; i < mLayerList->rowCount(); i++ )
  {
    if ( mLayerList->cellWidget( i, 3 ) == btn )
    {
      row = i;
      break;
    }
  }

  if ( row == -1 )
    return;

  QString provider = mLayerList->item( row, 2 )->text();
  if ( provider == QLatin1String( "none" ) )
    provider.clear();

  QString prevuri = mLayerList->item( row, 4 )->text();

  QgsAuthConfigUriEdit *dlg = new QgsAuthConfigUriEdit( this, prevuri, provider );
  dlg->setWindowModality( Qt::WindowModal );
  dlg->resize( 500, 500 );
  if ( dlg->exec() )
  {
    QString newuri( dlg->dataSourceUri() );
    if ( newuri != prevuri )
    {
      mLayerList->item( row, 4 )->setText( newuri );
    }
  }
  dlg->deleteLater();
}

void QgsHandleBadLayers::apply()
{
  for ( int i = 0; i < mLayerList->rowCount(); i++ )
  {
    int idx = mLayerList->item( i, 0 )->data( Qt::UserRole ).toInt();
    QDomNode &node = const_cast<QDomNode &>( mLayers[ idx ] );

    QTableWidgetItem *item = mLayerList->item( i, 4 );
    QString datasource = item->text();

    node.namedItem( QStringLiteral( "datasource" ) ).toElement().firstChild().toText().setData( datasource );
    if ( QgsProject::instance()->readLayer( node ) )
    {
      mLayerList->removeRow( i-- );
    }
    else
    {
      item->setForeground( QBrush( Qt::red ) );
    }
  }
}

void QgsHandleBadLayers::accept()
{
  apply();

  if ( mLayerList->rowCount() > 0  &&
       QMessageBox::warning( this,
                             tr( "Unhandled layer will be lost." ),
                             tr( "There are still %n unhandled layer(s), that will be lost if you closed now.",
                                 "unhandled layers",
                                 mLayerList->rowCount() ),
                             QMessageBox::Ok | QMessageBox::Cancel,
                             QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  QDialog::accept();
}

void QgsHandleBadLayers::reject()
{

  if ( mLayerList->rowCount() > 0  &&
       QMessageBox::warning( this,
                             tr( "Unhandled layer will be lost." ),
                             tr( "There are still %n unhandled layer(s), that will be lost if you closed now.",
                                 "unhandled layers",
                                 mLayerList->rowCount() ),
                             QMessageBox::Ok | QMessageBox::Cancel,
                             QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  QDialog::reject();
}

int QgsHandleBadLayers::layerCount()
{
  return mLayerList->rowCount();
}
