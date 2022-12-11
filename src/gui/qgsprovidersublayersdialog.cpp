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
#include <QUrl>

QgsProviderSublayerDialogModel::QgsProviderSublayerDialogModel( QObject *parent )
  : QgsProviderSublayerModel( parent )
{

}

QVariant QgsProviderSublayerDialogModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  QgsProviderSublayerModelNode *node = index2node( index );
  if ( !node )
    return QVariant();

  if ( QgsProviderSublayerModelSublayerNode *sublayerNode = dynamic_cast<QgsProviderSublayerModelSublayerNode *>( node ) )
  {
    const QgsProviderSublayerDetails details = sublayerNode->sublayer();

    if ( details.type() == QgsMapLayerType::VectorLayer && details.wkbType() == QgsWkbTypes::Unknown && !mGeometryTypesResolved )
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
    else if ( details.flags() & Qgis::SublayerFlag::SystemTable )
    {
      switch ( role )
      {
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

    if ( details.type() == QgsMapLayerType::VectorLayer && details.wkbType() == QgsWkbTypes::Unknown && !mGeometryTypesResolved )
    {
      // unknown geometry item can't be selected
      return Qt::ItemFlags();
    }
  }
  return QgsProviderSublayerModel::flags( index );
}

void QgsProviderSublayerDialogModel::setGeometryTypesResolved( bool resolved )
{
  mGeometryTypesResolved = resolved;
  emit dataChanged( index( 0, 0 ), index( rowCount( QModelIndex() ), columnCount() ) );
}

QgsProviderSublayersDialog::QgsProviderSublayersDialog( const QString &uri, const QString &providerKey, const QString &filePathIn, const QList<QgsProviderSublayerDetails> initialDetails, const QList<QgsMapLayerType> &acceptableTypes, QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  const QFileInfo fileInfo( filePathIn );
  const QString filePath = ( fileInfo.isFile() || fileInfo.isDir() ) && fileInfo.exists() ? filePathIn : QString();
  const QString fileName = !filePath.isEmpty() ? QgsProviderUtils::suggestLayerNameFromFilePath( filePath ) : QString();

  if ( !fileName.isEmpty() )
  {
    setGroupName( fileName );
  }

  setWindowTitle( fileName.isEmpty() ? tr( "Select Items to Add" ) : QStringLiteral( "%1 | %2" ).arg( tr( "Select Items to Add" ), fileName ) );

  mLblFilePath->setText( QStringLiteral( "<a href=\"%1\">%2</a>" )
                         .arg( QUrl::fromLocalFile( filePath ).toString(), QDir::toNativeSeparators( QFileInfo( filePath ).canonicalFilePath() ) ) );
  mLblFilePath->setVisible( !filePath.isEmpty() );
  mLblFilePath->setWordWrap( true );
  mLblFilePath->setTextInteractionFlags( Qt::TextBrowserInteraction );
  connect( mLblFilePath, &QLabel::linkActivated, this, [ = ]( const QString & link )
  {
    const QUrl url( link );
    const QFileInfo file( url.toLocalFile() );
    if ( file.exists() && !file.isDir() )
      QgsGui::nativePlatformInterface()->openFileExplorerAndSelectFile( url.toLocalFile() );
    else
      QDesktopServices::openUrl( url );
  } );

  mModel = new QgsProviderSublayerDialogModel( this );
  mModel->setSublayerDetails( initialDetails );
  mProxyModel = new QgsProviderSublayerProxyModel( this );
  mProxyModel->setSourceModel( mModel );
  mLayersTree->setModel( mProxyModel );

  mLayersTree->expandAll();

  const QgsSettings settings;
  const bool addToGroup = settings.value( QStringLiteral( "/qgis/openSublayersInGroup" ), false ).toBool();
  mCbxAddToGroup->setChecked( addToGroup );
  mCbxAddToGroup->setVisible( !fileName.isEmpty() );

  // resize columns
  const QByteArray ba = settings.value( "/Windows/SubLayers/headerState" ).toByteArray();
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

  if ( QgsProviderUtils::sublayerDetailsAreIncomplete( initialDetails ) )
  {
    // initial details are incomplete, so fire up a task in the background to fully populate the model...
    mTask = new QgsProviderSublayerTask( uri, providerKey, true );
    connect( mTask.data(), &QgsProviderSublayerTask::taskCompleted, this, [ = ]
    {
      QList< QgsProviderSublayerDetails > res = mTask->results();
      res.erase( std::remove_if( res.begin(), res.end(), [acceptableTypes]( const QgsProviderSublayerDetails & sublayer )
      {
        return !acceptableTypes.empty() && !acceptableTypes.contains( sublayer.type() );
      } ), res.end() );

      mModel->setSublayerDetails( res );
      mModel->setGeometryTypesResolved( true );
      mTask = nullptr;
      mLayersTree->expandAll();
      selectAll();
    } );
    QgsApplication::taskManager()->addTask( mTask.data() );
  }

  connect( mBtnSelectAll, &QAbstractButton::pressed, this, &QgsProviderSublayersDialog::selectAll );
  connect( mBtnDeselectAll, &QAbstractButton::pressed, this, [ = ] { mLayersTree->selectionModel()->clear(); } );
  connect( mLayersTree->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsProviderSublayersDialog::treeSelectionChanged );
  connect( mSearchLineEdit, &QgsFilterLineEdit::textChanged, mProxyModel, &QgsProviderSublayerProxyModel::setFilterString );
  connect( mCheckShowSystem, &QCheckBox::toggled, mProxyModel, &QgsProviderSublayerProxyModel::setIncludeSystemTables );
  connect( mCheckShowEmpty, &QCheckBox::toggled, mProxyModel, &QgsProviderSublayerProxyModel::setIncludeEmptyLayers );
  connect( mLayersTree, &QTreeView::doubleClicked, this, [ = ]( const QModelIndex & index )
  {
    const QModelIndex left = mLayersTree->model()->index( index.row(), 0, index.parent() );
    if ( !( left.flags() & Qt::ItemIsSelectable ) )
      return;

    mLayersTree->selectionModel()->select( QItemSelection( left,
                                           mLayersTree->model()->index( index.row(), mLayersTree->model()->columnCount() - 1, index.parent() ) ),
                                           QItemSelectionModel::ClearAndSelect );
    emit layersAdded( selectedLayers() );
    accept();
  } );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, [ = ]
  {
    emit layersAdded( selectedLayers() );
    accept();
  } );
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
  mButtonBox->button( QDialogButtonBox::Ok )->setText( tr( "Add Layers" ) );

  selectAll();
}

void QgsProviderSublayersDialog::setNonLayerItems( const QList<QgsProviderSublayerModel::NonLayerItem> &items )
{
  for ( const QgsProviderSublayerModel::NonLayerItem &item : items )
  {
    mModel->addNonLayerItem( item );
  }
}

QgsProviderSublayersDialog::~QgsProviderSublayersDialog()
{
  QgsSettings settings;
  settings.setValue( "/Windows/SubLayers/headerState", mLayersTree->header()->saveState() );
  settings.setValue( QStringLiteral( "/qgis/openSublayersInGroup" ), mCbxAddToGroup->isChecked() );

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

QList<QgsProviderSublayerModel::NonLayerItem> QgsProviderSublayersDialog::selectedNonLayerItems() const
{
  const QModelIndexList selection = mLayersTree->selectionModel()->selectedRows();
  QList< QgsProviderSublayerModel::NonLayerItem > selectedItems;
  for ( const QModelIndex &index : selection )
  {
    const QModelIndex sourceIndex = mProxyModel->mapToSource( index );
    if ( mModel->data( sourceIndex, static_cast< int >( QgsProviderSublayerModel::Role::IsNonLayerItem ) ).toBool() )
    {
      selectedItems << mModel->indexToNonLayerItem( sourceIndex );
    }
  }
  return selectedItems;
}

void QgsProviderSublayersDialog::setGroupName( const QString &groupNameIn )
{
  mGroupName = groupNameIn;
  const QgsSettings settings;
  if ( settings.value( QStringLiteral( "qgis/formatLayerName" ), false ).toBool() )
  {
    mGroupName = QgsMapLayer::formatLayerName( mGroupName );
  }

  mCbxAddToGroup->setVisible( !mGroupName.isEmpty() );
}

QString QgsProviderSublayersDialog::groupName() const
{
  if ( !mCbxAddToGroup->isChecked() )
    return QString();
  return mGroupName;
}

void QgsProviderSublayersDialog::treeSelectionChanged( const QItemSelection &selected, const QItemSelection & )
{
  if ( mBlockSelectionChanges )
    return;

  mBlockSelectionChanges = true;
  bool selectedANonLayerItem = false;
  QModelIndex firstSelectedNonLayerItem;
  bool selectedALayerItem = false;
  for ( const QModelIndex &index : selected.indexes() )
  {
    if ( index.column() != 0 )
      continue;

    if ( mProxyModel->data( index, static_cast< int >( QgsProviderSublayerModel::Role::IsNonLayerItem ) ).toBool() )
    {
      if ( !selectedANonLayerItem )
      {
        selectedANonLayerItem = true;
        firstSelectedNonLayerItem = index;
      }
      else
      {
        // only one non-layer item can be selected
        mLayersTree->selectionModel()->select( QItemSelection( mLayersTree->model()->index( index.row(), 0, index.parent() ),
                                               mLayersTree->model()->index( index.row(), mLayersTree->model()->columnCount() - 1, index.parent() ) ),
                                               QItemSelectionModel::Deselect );
      }
    }
    else
    {
      selectedALayerItem = true;
    }
  }

  for ( int row = 0; row < mProxyModel->rowCount(); ++row )
  {
    const QModelIndex index = mProxyModel->index( row, 0 );
    if ( mProxyModel->data( index, static_cast< int >( QgsProviderSublayerModel::Role::IsNonLayerItem ) ).toBool() )
    {
      if ( ( selectedANonLayerItem && index != firstSelectedNonLayerItem ) || selectedALayerItem )
      {
        mLayersTree->selectionModel()->select( QItemSelection( mLayersTree->model()->index( index.row(), 0, index.parent() ),
                                               mLayersTree->model()->index( index.row(), mLayersTree->model()->columnCount() - 1, index.parent() ) ),
                                               QItemSelectionModel::Deselect );
      }
    }
    else
    {
      if ( selectedANonLayerItem )
      {
        mLayersTree->selectionModel()->select( QItemSelection( mLayersTree->model()->index( index.row(), 0, index.parent() ),
                                               mLayersTree->model()->index( index.row(), mLayersTree->model()->columnCount() - 1, index.parent() ) ),
                                               QItemSelectionModel::Deselect );
      }
    }
  }
  mBlockSelectionChanges = false;

  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( !mLayersTree->selectionModel()->selectedRows().empty() );

  mCbxAddToGroup->setEnabled( !selectedANonLayerItem );
  mButtonBox->button( QDialogButtonBox::Ok )->setText( selectedANonLayerItem ? tr( "Open" ) : tr( "Add Layers" ) );
}

void QgsProviderSublayersDialog::selectAll()
{
  mLayersTree->selectionModel()->clear();

  std::function< void( const QModelIndex & ) > selectAllInParent;

  selectAllInParent = [this, &selectAllInParent]( const QModelIndex & parent )
  {
    for ( int row = 0; row < mProxyModel->rowCount( parent ); ++row )
    {
      const QModelIndex index = mProxyModel->index( row, 0, parent );
      if ( !mProxyModel->data( index, static_cast< int >( QgsProviderSublayerModel::Role::IsNonLayerItem ) ).toBool() )
      {
        mLayersTree->selectionModel()->select( QItemSelection( mLayersTree->model()->index( index.row(), 0, index.parent() ),
                                               mLayersTree->model()->index( index.row(), mLayersTree->model()->columnCount() - 1, index.parent() ) ),
                                               QItemSelectionModel::Select );
      }
      selectAllInParent( index );
    }
  };
  selectAllInParent( QModelIndex() );

  mButtonBox->button( QDialogButtonBox::Ok )->setFocus();
}
