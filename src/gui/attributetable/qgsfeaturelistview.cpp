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

#include "qgsactionmenu.h"
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

  mCurrentEditSelectionModel = new QItemSelectionModel( mModel->masterModel(), this );
  if ( !mFeatureSelectionManager )
  {
    mFeatureSelectionManager = new QgsVectorLayerSelectionManager( mModel->layerCache()->layer(), mModel );
  }

  mFeatureSelectionModel = new QgsFeatureSelectionModel( featureListModel, featureListModel, mFeatureSelectionManager, this );
  setSelectionModel( mFeatureSelectionModel );

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
  Q_FOREACH ( const QModelIndex &idx, mCurrentEditSelectionModel->selectedIndexes() )
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
    QgsDebugMsg( "No model assigned to this view" );
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

  Q_FOREACH ( QgsFeatureId fid, fids )
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

  if ( ok )
    mCurrentEditSelectionModel->select( index, command );
}

void QgsFeatureListView::repaintRequested( const QModelIndexList &indexes )
{
  Q_FOREACH ( const QModelIndex &index, indexes )
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
    setEditSelection( mModel->mapToMaster( index ), QItemSelectionModel::ClearAndSelect );
  }
  else
  {
    selectRow( index, false );
  }
}

void QgsFeatureListView::mouseReleaseEvent( QMouseEvent *event )
{
  Q_UNUSED( event );

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
  if ( Qt::Key_Up == event->key() || Qt::Key_Down == event->key() )
  {
    int currentRow = 0;
    if ( 0 != mCurrentEditSelectionModel->selectedIndexes().count() )
    {
      QModelIndex localIndex = mModel->mapFromMaster( mCurrentEditSelectionModel->selectedIndexes().first() );
      currentRow = localIndex.row();
    }

    QModelIndex newLocalIndex;
    QModelIndex newIndex;

    switch ( event->key() )
    {
      case Qt::Key_Up:
        newLocalIndex = mModel->index( currentRow - 1, 0 );
        newIndex = mModel->mapToMaster( newLocalIndex );
        if ( newIndex.isValid() )
        {
          setEditSelection( newIndex, QItemSelectionModel::ClearAndSelect );
          scrollTo( newLocalIndex );
        }
        break;

      case Qt::Key_Down:
        newLocalIndex = mModel->index( currentRow + 1, 0 );
        newIndex = mModel->mapToMaster( newLocalIndex );
        if ( newIndex.isValid() )
        {
          setEditSelection( newIndex, QItemSelectionModel::ClearAndSelect );
          scrollTo( newLocalIndex );
        }
        break;

      default:
        break;
    }
  }
  else
  {
    QListView::keyPressEvent( event );
  }
}

void QgsFeatureListView::contextMenuEvent( QContextMenuEvent *event )
{
  QModelIndex index = indexAt( event->pos() );

  if ( index.isValid() )
  {
    QgsFeature feature = mModel->data( index, QgsFeatureListModel::FeatureRole ).value<QgsFeature>();

    QgsActionMenu *menu = new QgsActionMenu( mModel->layerCache()->layer(), feature, QStringLiteral( "Feature" ), this );
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

void QgsFeatureListView::setFeatureSelectionManager( QgsIFeatureSelectionManager *featureSelectionManager )
{
  delete mFeatureSelectionManager;

  mFeatureSelectionManager = featureSelectionManager;

  if ( mFeatureSelectionModel )
    mFeatureSelectionModel->setFeatureSelectionManager( mFeatureSelectionManager );
}
