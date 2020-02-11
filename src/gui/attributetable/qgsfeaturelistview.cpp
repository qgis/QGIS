/***************************************************************************
     QgsAttributeTableView.cpp
     --------------------------------------
    Date                 : Feb 2009
    Copyright            : (C) 2009 Vita Cizek
    Email                : weetya (at) gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QHeaderView>
#include <QKeyEvent>
#include <QMenu>
#include <QSet>
#include <QSettings>

#include "qgsattributetabledelegate.h"
#include "qgsattributetablefiltermodel.h"
#include "qgsattributetablemodel.h"
#include "qgsfeaturelistmodel.h"
#include "qgsfeaturelistviewdelegate.h"
#include "qgsfeaturelistview.h"
#include "qgsfeatureselectionmodel.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerselectionmanager.h"

QgsFeatureListView::QgsFeatureListView( QWidget *parent )
  : QListView( parent )
{
  setSelectionMode( QAbstractItemView::ExtendedSelection );
}

QgsVectorLayerCache *QgsFeatureListView::layerCache()
{
  return mModel->layerCache();
}

void QgsFeatureListView::setModel( QgsFeatureListModel *featureListModel )
{
  QListView::setModel( featureListModel );
  mModel = featureListModel;

  delete mFeatureSelectionModel;
  delete mCurrentEditSelectionModel;

  mCurrentEditSelectionModel = new QItemSelectionModel( mModel->masterModel(), this );
  if ( !mFeatureSelectionManager )
  {
    mOwnedFeatureSelectionManager = new QgsVectorLayerSelectionManager( mModel->layerCache()->layer(), mModel );
    mFeatureSelectionManager = mOwnedFeatureSelectionManager;
  }

  mFeatureSelectionModel = new QgsFeatureSelectionModel( featureListModel, featureListModel, mFeatureSelectionManager, this );
  setSelectionModel( mFeatureSelectionModel );
  connect( featureListModel->layerCache()->layer(), &QgsVectorLayer::selectionChanged, this, [ this ]()
  {
    ensureEditSelection( true );
  } );

  if ( mItemDelegate && mItemDelegate->parent() == this )
  {
    delete mItemDelegate;
  }

  mItemDelegate = new QgsFeatureListViewDelegate( mModel, this );
  mItemDelegate->setEditSelectionModel( mCurrentEditSelectionModel );
  setItemDelegate( mItemDelegate );

  mItemDelegate->setFeatureSelectionModel( mFeatureSelectionModel );
  connect( mFeatureSelectionModel, static_cast<void ( QgsFeatureSelectionModel::* )( const QModelIndexList &indexes )>( &QgsFeatureSelectionModel::requestRepaint ),
           this, static_cast<void ( QgsFeatureListView::* )( const QModelIndexList &indexes )>( &QgsFeatureListView::repaintRequested ) );
  connect( mFeatureSelectionModel, static_cast<void ( QgsFeatureSelectionModel::* )()>( &QgsFeatureSelectionModel::requestRepaint ),
           this, static_cast<void ( QgsFeatureListView::* )()>( &QgsFeatureListView::repaintRequested ) );
  connect( mCurrentEditSelectionModel, &QItemSelectionModel::selectionChanged, this, &QgsFeatureListView::editSelectionChanged );
  connect( mModel->layerCache()->layer(), &QgsVectorLayer::attributeValueChanged, this, [ = ] { repaintRequested(); } );
  connect( featureListModel, &QgsFeatureListModel::rowsRemoved, this, [ this ]() { ensureEditSelection(); } );
  connect( featureListModel, &QgsFeatureListModel::rowsInserted, this, [ this ]() { ensureEditSelection(); } );
  connect( featureListModel, &QgsFeatureListModel::modelReset, this, [ this ]() { ensureEditSelection(); } );
}

bool QgsFeatureListView::setDisplayExpression( const QString &expression )
{
  if ( mModel->setDisplayExpression( expression ) )
  {
    emit displayExpressionChanged( expression );
    return true;
  }
  else
  {
    return false;
  }
}

const QString QgsFeatureListView::displayExpression() const
{
  return mModel->displayExpression();
}

QString QgsFeatureListView::parserErrorString()
{
  return mModel->parserErrorString();
}

QgsFeatureIds QgsFeatureListView::currentEditSelection()
{
  QgsFeatureIds selection;
  const QModelIndexList selectedIndexes = mCurrentEditSelectionModel->selectedIndexes();
  for ( const QModelIndex &idx : selectedIndexes )
  {
    selection << idx.data( QgsAttributeTableModel::FeatureIdRole ).value<QgsFeatureId>();
  }
  return selection;
}

void QgsFeatureListView::setCurrentFeatureEdited( bool state )
{
  mItemDelegate->setCurrentFeatureEdited( state );
  viewport()->update( visualRegionForSelection( mCurrentEditSelectionModel->selection() ) );
}

void QgsFeatureListView::mousePressEvent( QMouseEvent *event )
{
  if ( mModel )
  {
    QPoint pos = event->pos();

    QModelIndex index = indexAt( pos );

    if ( QgsFeatureListViewDelegate::EditElement == mItemDelegate->positionToElement( event->pos() ) )
    {

      mEditSelectionDrag = true;
      if ( index.isValid() )
        setEditSelection( mModel->mapToMaster( index ), QItemSelectionModel::ClearAndSelect );
    }
    else
    {
      mFeatureSelectionModel->enableSync( false );
      selectRow( index, true );
      repaintRequested();
    }
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "No model assigned to this view" ) );
  }
}

void QgsFeatureListView::editSelectionChanged( const QItemSelection &deselected, const QItemSelection &selected )
{
  if ( isVisible() && updatesEnabled() )
  {
    QItemSelection localDeselected = mModel->mapSelectionFromMaster( deselected );
    QItemSelection localSelected = mModel->mapSelectionFromMaster( selected );
    viewport()->update( visualRegionForSelection( localDeselected ) | visualRegionForSelection( localSelected ) );
  }

  QItemSelection currentSelection = mCurrentEditSelectionModel->selection();
  if ( currentSelection.size() == 1 )
  {
    QModelIndexList indexList = currentSelection.indexes();
    if ( !indexList.isEmpty() )
    {
      QgsFeature feat;
      mModel->featureByIndex( mModel->mapFromMaster( indexList.first() ), feat );

      emit currentEditSelectionChanged( feat );
      emit currentEditSelectionProgressChanged( mModel->mapFromMaster( indexList.first() ).row(), mModel->rowCount() );
    }
  }
}

void QgsFeatureListView::selectAll()
{
  QItemSelection selection;
  selection.append( QItemSelectionRange( mModel->index( 0, 0 ), mModel->index( mModel->rowCount() - 1, 0 ) ) );

  mFeatureSelectionModel->selectFeatures( selection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
}

void QgsFeatureListView::setEditSelection( const QgsFeatureIds &fids )
{
  QItemSelection selection;

  const auto constFids = fids;
  for ( QgsFeatureId fid : constFids )
  {
    selection.append( QItemSelectionRange( mModel->mapToMaster( mModel->fidToIdx( fid ) ) ) );
  }

  bool ok = true;
  emit aboutToChangeEditSelection( ok );

  if ( ok )
    mCurrentEditSelectionModel->select( selection, QItemSelectionModel::ClearAndSelect );
}

void QgsFeatureListView::setEditSelection( const QModelIndex &index, QItemSelectionModel::SelectionFlags command )
{
  bool ok = true;
  emit aboutToChangeEditSelection( ok );

  Q_ASSERT( index.model() == mModel->masterModel() || !index.isValid() );

  if ( ok )
    mCurrentEditSelectionModel->select( index, command );
}

void QgsFeatureListView::repaintRequested( const QModelIndexList &indexes )
{
  const auto constIndexes = indexes;
  for ( const QModelIndex &index : constIndexes )
  {
    update( index );
  }
}

void QgsFeatureListView::repaintRequested()
{
  setDirtyRegion( viewport()->rect() );
}

void QgsFeatureListView::mouseMoveEvent( QMouseEvent *event )
{
  QPoint pos = event->pos();

  QModelIndex index = indexAt( pos );

  if ( mEditSelectionDrag )
  {
    if ( index.isValid() )
      setEditSelection( mModel->mapToMaster( index ), QItemSelectionModel::ClearAndSelect );
  }
  else
  {
    selectRow( index, false );
  }
}

void QgsFeatureListView::mouseReleaseEvent( QMouseEvent *event )
{
  Q_UNUSED( event )

  if ( mEditSelectionDrag )
  {
    mEditSelectionDrag = false;
  }
  else
  {
    if ( mFeatureSelectionModel )
      mFeatureSelectionModel->enableSync( true );
  }
}

void QgsFeatureListView::keyPressEvent( QKeyEvent *event )
{
  switch ( event->key() )
  {
    case Qt::Key_Up:
      editOtherFeature( Previous );
      break;

    case Qt::Key_Down:
      editOtherFeature( Next );
      break;

    default:
      QListView::keyPressEvent( event );
  }
}

void QgsFeatureListView::editOtherFeature( QgsFeatureListView::PositionInList positionInList )
{
  int currentRow = 0;
  if ( 0 != mCurrentEditSelectionModel->selectedIndexes().count() )
  {
    QModelIndex localIndex = mModel->mapFromMaster( mCurrentEditSelectionModel->selectedIndexes().first() );
    currentRow = localIndex.row();
  }

  QModelIndex newLocalIndex;
  QModelIndex newIndex;

  switch ( positionInList )
  {
    case First:
      newLocalIndex = mModel->index( 0, 0 );
      break;

    case Previous:
      newLocalIndex = mModel->index( currentRow - 1, 0 );
      break;

    case Next:
      newLocalIndex = mModel->index( currentRow + 1, 0 );
      break;

    case Last:
      newLocalIndex = mModel->index( mModel->rowCount() - 1, 0 );
      break;
  }

  newIndex = mModel->mapToMaster( newLocalIndex );
  if ( newIndex.isValid() )
  {
    setEditSelection( newIndex, QItemSelectionModel::ClearAndSelect );
    scrollTo( newLocalIndex );
  }
}

void QgsFeatureListView::contextMenuEvent( QContextMenuEvent *event )
{
  QModelIndex index = indexAt( event->pos() );

  if ( index.isValid() )
  {
    QgsFeature feature = mModel->data( index, QgsFeatureListModel::FeatureRole ).value<QgsFeature>();

    QgsActionMenu *menu = new QgsActionMenu( mModel->layerCache()->layer(), feature, QStringLiteral( "Feature" ), this );

    // Index is from feature list model, but we need an index from the
    // filter model to be passed to listeners, using fid instead would
    // have been much better in term of bugs (and headaches) but this
    // belongs to the API unfortunately.
    emit willShowContextMenu( menu, mModel->mapToSource( index ) );

    menu->exec( event->globalPos() );
  }
}

void QgsFeatureListView::selectRow( const QModelIndex &index, bool anchor )
{
  QItemSelectionModel::SelectionFlags command = selectionCommand( index );
  int row = index.row();

  if ( anchor )
    mRowAnchor = row;

  if ( selectionMode() != QListView::SingleSelection
       && command.testFlag( QItemSelectionModel::Toggle ) )
  {
    if ( anchor )
      mCtrlDragSelectionFlag = mFeatureSelectionModel->isSelected( index )
                               ? QItemSelectionModel::Deselect : QItemSelectionModel::Select;
    command &= ~QItemSelectionModel::Toggle;
    command |= mCtrlDragSelectionFlag;
    if ( !anchor )
      command |= QItemSelectionModel::Current;
  }

  QModelIndex tl = model()->index( std::min( mRowAnchor, row ), 0 );
  QModelIndex br = model()->index( std::max( mRowAnchor, row ), model()->columnCount() - 1 );

  mFeatureSelectionModel->selectFeatures( QItemSelection( tl, br ), command );
}

void QgsFeatureListView::ensureEditSelection( bool inSelection )
{
  if ( !mModel->rowCount() )
    return;

  const QModelIndexList selectedIndexes = mCurrentEditSelectionModel->selectedIndexes();

  // We potentially want a new edit selection
  // If we it should be in the feature selection
  // but we don't find a matching one we might
  // still stick to the old edit selection
  bool editSelectionUpdateRequested = false;
  // There is a valid selection available which we
  // could fall back to
  bool validEditSelectionAvailable = false;

  if ( selectedIndexes.isEmpty() || !selectedIndexes.first().isValid() || mModel->mapFromMaster( selectedIndexes.first() ).row() == -1 )
  {
    validEditSelectionAvailable = false;
  }
  else
  {
    validEditSelectionAvailable = true;
  }

  // If we want to force the edit selection to be within the feature selection
  // let's do some additional checks
  if ( inSelection )
  {
    // no valid edit selection, update anyway
    if ( !validEditSelectionAvailable )
    {
      editSelectionUpdateRequested = true;
    }
    else
    {
      // valid selection: update only if it's not in the feature selection
      const QgsFeatureIds selectedFids = layerCache()->layer()->selectedFeatureIds();

      if ( !selectedFids.contains( mModel->idxToFid( mModel->mapFromMaster( selectedIndexes.first() ) ) ) )
      {
        editSelectionUpdateRequested = true;
      }
    }
  }
  else
  {
    // we don't care if the edit selection is in the feature selection?
    // well then, only update if there is no valid edit selection available
    if ( !validEditSelectionAvailable )
      editSelectionUpdateRequested = true;
  }

  if ( editSelectionUpdateRequested )
  {
    if ( !mUpdateEditSelectionTimer.isSingleShot() )
    {
      mUpdateEditSelectionTimer.setSingleShot( true );
      connect( &mUpdateEditSelectionTimer, &QTimer::timeout, this, [ this, inSelection, validEditSelectionAvailable ]()
      {
        // The layer might have been removed between timer start and timer triggered
        // in this case there is nothing left for us to do.
        if ( !layerCache() )
          return;

        int rowToSelect = -1;

        if ( inSelection )
        {
          const QgsFeatureIds selectedFids = layerCache()->layer()->selectedFeatureIds();
          const int rowCount = mModel->rowCount();

          for ( int i = 0; i < rowCount; i++ )
          {
            if ( selectedFids.contains( mModel->idxToFid( mModel->index( i, 0 ) ) ) )
            {
              rowToSelect = i;
              break;
            }

            if ( rowToSelect == -1 && !validEditSelectionAvailable )
              rowToSelect = 0;
          }
        }
        else
          rowToSelect = 0;

        if ( rowToSelect != -1 )
        {
          setEditSelection( mModel->mapToMaster( mModel->index( rowToSelect, 0 ) ), QItemSelectionModel::ClearAndSelect );
        }
      } );
      mUpdateEditSelectionTimer.setInterval( 0 );
    }
    mUpdateEditSelectionTimer.start();
  }
}

void QgsFeatureListView::setFeatureSelectionManager( QgsIFeatureSelectionManager *featureSelectionManager )
{
  mFeatureSelectionManager = featureSelectionManager;

  if ( mFeatureSelectionModel )
    mFeatureSelectionModel->setFeatureSelectionManager( mFeatureSelectionManager );

  // only delete the owner selection manager and not one created from outside
  if ( mOwnedFeatureSelectionManager )
  {
    mOwnedFeatureSelectionManager->deleteLater();
    mOwnedFeatureSelectionManager = nullptr;
  }
}
