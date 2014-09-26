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
    , mModel( 0 )
    , mCurrentEditSelectionModel( 0 )
    , mFeatureSelectionModel( 0 )
    , mItemDelegate( 0 )
    , mEditSelectionDrag( false )
{
  setSelectionMode( QAbstractItemView::ExtendedSelection );
}

QgsVectorLayerCache *QgsFeatureListView::layerCache()
{
  return mModel->layerCache();
}

void QgsFeatureListView::setModel( QgsFeatureListModel* featureListModel )
{
  QListView::setModel( featureListModel );
  mModel = featureListModel;

  delete mFeatureSelectionModel;
  mFeatureSelectionModel = new QgsFeatureSelectionModel( featureListModel, featureListModel, new QgsVectorLayerSelectionManager( featureListModel->layerCache()->layer(), this ), this );
  setSelectionModel( mFeatureSelectionModel );

  mCurrentEditSelectionModel = new QItemSelectionModel( mModel->masterModel(), this );

  if ( mItemDelegate && mItemDelegate->parent() == this )
  {
    delete mItemDelegate;
  }

  mItemDelegate = new QgsFeatureListViewDelegate( mModel, this );
  mItemDelegate->setEditSelectionModel( mCurrentEditSelectionModel );
  setItemDelegate( mItemDelegate );

  mItemDelegate->setFeatureSelectionModel( mFeatureSelectionModel );
  connect( mFeatureSelectionModel, SIGNAL( requestRepaint( QModelIndexList ) ), this, SLOT( repaintRequested( QModelIndexList ) ) );
  connect( mFeatureSelectionModel, SIGNAL( requestRepaint() ), this, SLOT( repaintRequested() ) );

  connect( mCurrentEditSelectionModel, SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), SLOT( editSelectionChanged( QItemSelection, QItemSelection ) ) );
}

bool QgsFeatureListView::setDisplayExpression( const QString expression )
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
  Q_FOREACH ( QModelIndex idx, mCurrentEditSelectionModel->selectedIndexes() )
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

void QgsFeatureListView::editSelectionChanged( QItemSelection deselected, QItemSelection selected )
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

  foreach ( QgsFeatureId fid, fids )
  {
    selection.append( QItemSelectionRange( mModel->mapToMaster( mModel->fidToIdx( fid ) ) ) );
  }

  bool ok = true;
  emit aboutToChangeEditSelection( ok );

  if ( ok )
    mCurrentEditSelectionModel->select( selection, QItemSelectionModel::ClearAndSelect );
}

void QgsFeatureListView::setEditSelection( const QModelIndex& index, QItemSelectionModel::SelectionFlags command )
{
  bool ok = true;
  emit aboutToChangeEditSelection( ok );

  if ( ok )
    mCurrentEditSelectionModel->select( index, command );
}

void QgsFeatureListView::repaintRequested( QModelIndexList indexes )
{
  foreach ( const QModelIndex index, indexes )
  {
    update( index );
  }
}

void QgsFeatureListView::repaintRequested()
{
  setDirtyRegion( viewport()->rect() );
}

/*!
    This function is called with the given \a event when a mouse move event is
    sent to the widget. If a selection is in progress and new items are moved
    over the selection is extended; if a drag is in progress it is continued.
*/

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

/*!
    This function is called with the given \a event when a mouse button is released,
    after a mouse press event on the widget. If a user presses the mouse inside your
    widget and then drags the mouse to another location before releasing the mouse button,
    your widget receives the release event. The function will emit the clicked() signal if an
    item was being pressed.
*/
void QgsFeatureListView::mouseReleaseEvent( QMouseEvent *event )
{
  Q_UNUSED( event );

  if ( mEditSelectionDrag )
  {
    mEditSelectionDrag = false;
  }
  else
  {
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

    QgsActionMenu menu( mModel->layerCache()->layer(), &feature, this );
    menu.exec( event->globalPos() );
  }
}

void QgsFeatureListView::selectRow( const QModelIndex& index, bool anchor )
{
  QItemSelectionModel::SelectionFlags command =  selectionCommand( index );
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

  QModelIndex tl = model()->index( qMin( mRowAnchor, row ), 0 );
  QModelIndex br = model()->index( qMax( mRowAnchor, row ), model()->columnCount() - 1 );

  mFeatureSelectionModel->selectFeatures( QItemSelection( tl, br ), command );
}
