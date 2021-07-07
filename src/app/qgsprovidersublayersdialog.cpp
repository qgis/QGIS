/***************************************************************************
    qgsprovidersublayersdialog.h
    ---------------------
    begin                : July 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprovidersublayersdialog.h"
#include "qgssettings.h"
#include "qgsgui.h"
#include "qgsprovidersublayermodel.h"
#include "qgsproviderutils.h"
#include "qgsprovidersublayertask.h"
#include "qgsapplication.h"
#include "qgstaskmanager.h"
#include "qgsnative.h"

#include <QPushButton>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>

QgsProviderSublayerDialogModel::QgsProviderSublayerDialogModel( QObject *parent )
  : QgsProviderSublayerModel( parent )
{

}

QVariant QgsProviderSublayerDialogModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  if ( index.row() < 0 || index.row() >= rowCount( QModelIndex() ) )
    return QVariant();

  if ( index.row() < mSublayers.count() )
  {
    const QgsProviderSublayerDetails details = mSublayers.at( index.row() );

    if ( details.type() == QgsMapLayerType::VectorLayer && details.wkbType() == QgsWkbTypes::Unknown )
    {
      switch ( role )
      {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
        {
          if ( index.column() == static_cast< int >( Column::Description ) )
            return tr( "Scanning…" );
          break;
        }

        case Qt::FontRole:
        {
          QFont f =  QgsProviderSublayerModel::data( index, role ).value< QFont >();
          f.setItalic( true );
          return f;
        }
      }
    }
  }
  return QgsProviderSublayerModel::data( index, role );
}

Qt::ItemFlags QgsProviderSublayerDialogModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QgsProviderSublayerModel::flags( index );

  if ( index.row() < 0 || index.row() >= rowCount( QModelIndex() ) )
    return QgsProviderSublayerModel::flags( index );

  if ( index.row() < mSublayers.count() )
  {
    const QgsProviderSublayerDetails details = mSublayers.at( index.row() );

    if ( details.type() == QgsMapLayerType::VectorLayer && details.wkbType() == QgsWkbTypes::Unknown )
    {
      // unknown geometry item can't be selected
      return Qt::ItemFlags();
    }
  }
  return QgsProviderSublayerModel::flags( index );
}

QgsProviderSublayersDialog::QgsProviderSublayersDialog( const QString &uri, const QList<QgsProviderSublayerDetails> initialDetails, QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  const QFileInfo fileInfo( uri );
  mFilePath = fileInfo.isFile() && fileInfo.exists() ? uri : QString();
  mFileName = !mFilePath.isEmpty() ? fileInfo.fileName() : QString();

  setWindowTitle( mFileName.isEmpty() ? tr( "Select Items to Add" ) : QStringLiteral( "%1 | %2" ).arg( tr( "Select Items to Add" ), mFileName ) );

  mLblFilePath->setText( QStringLiteral( "<a href=\"%1\">%2</a>" )
                         .arg( QUrl::fromLocalFile( mFilePath ).toString(), QDir::toNativeSeparators( QFileInfo( mFilePath ).canonicalFilePath() ) ) );
  mLblFilePath->setVisible( !mFileName.isEmpty() );
  mLblFilePath->setWordWrap( true );
  mLblFilePath->setTextInteractionFlags( Qt::TextBrowserInteraction );
  connect( mLblFilePath, &QLabel::linkActivated, this, [ = ]( const QString & link )
  {
    const QUrl url( link );
    QFileInfo file( url.toLocalFile() );
    if ( file.exists() && !file.isDir() )
      QgsGui::instance()->nativePlatformInterface()->openFileExplorerAndSelectFile( url.toLocalFile() );
    else
      QDesktopServices::openUrl( url );
  } );

  mModel = new QgsProviderSublayerDialogModel( this );
  mModel->setSublayerDetails( initialDetails );
  mProxyModel = new QgsProviderSublayerProxyModel( this );
  mProxyModel->setSourceModel( mModel );
  mLayersTree->setModel( mProxyModel );

  // resize columns
  QgsSettings settings;
  QByteArray ba = settings.value( "/Windows/SubLayers/headerState" ).toByteArray();
  if ( !ba.isNull() )
  {
    mLayersTree->header()->restoreState( ba );
  }
  else
  {
    for ( int i = 0; i < mModel->columnCount(); i++ )
      mLayersTree->resizeColumnToContents( i );
    mLayersTree->setColumnWidth( 1, mLayersTree->columnWidth( 1 ) + 10 );
  }

  if ( QgsProviderUtils::sublayerDetailsAreIncomplete( initialDetails, false ) )
  {
    // initial details are incomplete, so fire up a task in the background to fully populate the model...
    mTask = new QgsProviderSublayerTask( uri );
    connect( mTask.data(), &QgsProviderSublayerTask::taskCompleted, this, [ = ]
    {
      mModel->setSublayerDetails( mTask->results() );
      mTask = nullptr;
    } );
    QgsApplication::taskManager()->addTask( mTask.data() );
  }

  connect( mBtnSelectAll, &QAbstractButton::pressed, mLayersTree, &QTreeView::selectAll );
  connect( mBtnDeselectAll, &QAbstractButton::pressed, this, [ = ] { mLayersTree->selectionModel()->clear(); } );
  connect( mLayersTree->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsProviderSublayersDialog::treeSelectionChanged );
  connect( mSearchLineEdit, &QgsFilterLineEdit::textChanged, mProxyModel, &QgsProviderSublayerProxyModel::setFilterString );

  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, [ = ]
  {
    emit layersAdded( selectedLayers() );
    accept();
  } );
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
}

QgsProviderSublayersDialog::~QgsProviderSublayersDialog()
{
  QgsSettings settings;
  settings.setValue( "/Windows/SubLayers/headerState", mLayersTree->header()->saveState() );
  if ( mTask )
    mTask->cancel();
}

QList<QgsProviderSublayerDetails> QgsProviderSublayersDialog::selectedLayers() const
{
  const QModelIndexList selection = mLayersTree->selectionModel()->selectedRows();
  QList< QgsProviderSublayerDetails > selectedSublayers;
  for ( const QModelIndex &index : selection )
  {
    const QModelIndex sourceIndex = mProxyModel->mapToSource( index );
    if ( !mModel->data( sourceIndex, static_cast< int >( QgsProviderSublayerModel::Role::IsNonLayerItem ) ).toBool() )
    {
      selectedSublayers << mModel->indexToSublayer( sourceIndex );
    }
  }
  return selectedSublayers;
}

QString QgsProviderSublayersDialog::groupName() const
{
  if ( !mCbxAddToGroup->isChecked() )
    return QString();

  const QFileInfo fi( mFilePath );
  QString res = fi.completeBaseName();

  QgsSettings settings;
  if ( settings.value( QStringLiteral( "qgis/formatLayerName" ), false ).toBool() )
  {
    res = QgsMapLayer::formatLayerName( res );
  }
  return res;
}

void QgsProviderSublayersDialog::treeSelectionChanged( const QItemSelection &, const QItemSelection & )
{
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( !mLayersTree->selectionModel()->selectedRows().empty() );
}
