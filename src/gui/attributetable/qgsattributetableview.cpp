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

#include <QKeyEvent>
#include <QSettings>
#include <QHeaderView>
#include <QMenu>

#include "qgsattributetableview.h"
#include "qgsattributetablemodel.h"
#include "qgsattributetabledelegate.h"
#include "qgsattributetablefiltermodel.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayercache.h"
#include "qgsvectorlayerselectionmanager.h"
#include "qgsvectordataprovider.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsfeatureselectionmodel.h"

QgsAttributeTableView::QgsAttributeTableView( QWidget *parent )
    : QTableView( parent )
    , mMasterModel( NULL )
    , mFilterModel( NULL )
    , mFeatureSelectionModel( NULL )
    , mFeatureSelectionManager( NULL )
    , mActionPopup( NULL )
{
  QSettings settings;
  restoreGeometry( settings.value( "/BetterAttributeTable/geometry" ).toByteArray() );

  verticalHeader()->setDefaultSectionSize( 20 );
  horizontalHeader()->setHighlightSections( false );

  mTableDelegate = new QgsAttributeTableDelegate( this );
  setItemDelegate( mTableDelegate );

  setSelectionBehavior( QAbstractItemView::SelectRows );
  setSelectionMode( QAbstractItemView::ExtendedSelection );
  setSortingEnabled( true );

  verticalHeader()->viewport()->installEventFilter( this );

  connect( verticalHeader(), SIGNAL( sectionPressed( int ) ), this, SLOT( selectRow( int ) ) );
  connect( verticalHeader(), SIGNAL( sectionEntered( int ) ), this, SLOT( _q_selectRow( int ) ) );
}

QgsAttributeTableView::~QgsAttributeTableView()
{
  delete mActionPopup;
}
#if 0
void QgsAttributeTableView::setCanvasAndLayerCache( QgsMapCanvas *canvas, QgsVectorLayerCache *layerCache )
{
  QgsAttributeTableModel* oldModel = mMasterModel;
  QgsAttributeTableFilterModel* filterModel = mFilterModel;

  mMasterModel = new QgsAttributeTableModel( layerCache, this );

  mLayerCache = layerCache;

  mMasterModel->loadLayer();

  mFilterModel = new QgsAttributeTableFilterModel( canvas, mMasterModel, mMasterModel );
  setModel( mFilterModel );
  delete mFeatureSelectionModel;
  mFeatureSelectionModel = new QgsFeatureSelectionModel( mFilterModel, mFilterModel, new QgsVectorLayerSelectionManager( layerCache->layer(), mFilterModel ), mFilterModel );
  connect( mFeatureSelectionModel, SIGNAL( requestRepaint( QModelIndexList ) ), this, SLOT( repaintRequested( QModelIndexList ) ) );
  connect( mFeatureSelectionModel, SIGNAL( requestRepaint() ), this, SLOT( repaintRequested() ) );
  setSelectionModel( mFeatureSelectionModel );

  delete oldModel;
  delete filterModel;
}
#endif
bool QgsAttributeTableView::eventFilter( QObject *object, QEvent *event )
{
  if ( object == verticalHeader()->viewport() )
  {
    switch ( event->type() )
    {
      case QEvent::MouseButtonPress:
        mFeatureSelectionModel->enableSync( false );
        break;

      case QEvent::MouseButtonRelease:
        mFeatureSelectionModel->enableSync( true );
        break;

      default:
        break;
    }
  }
  return false;
}

void QgsAttributeTableView::setModel( QgsAttributeTableFilterModel* filterModel )
{
  if ( mFilterModel )
  {
    // Cleanup old model stuff if present
    disconnect( mFilterModel, SIGNAL( filterAboutToBeInvalidated() ), this, SLOT( onFilterAboutToBeInvalidated() ) );
    disconnect( mFilterModel, SIGNAL( filterInvalidated() ), this, SLOT( onFilterInvalidated() ) );
  }

  mFilterModel = filterModel;
  QTableView::setModel( filterModel );

  connect( mFilterModel, SIGNAL( destroyed() ), this, SLOT( modelDeleted() ) );

  delete mFeatureSelectionModel;
  mFeatureSelectionModel = 0;

  if ( filterModel )
  {
    if ( !mFeatureSelectionManager )
    {
      mFeatureSelectionManager = new QgsVectorLayerSelectionManager( mFilterModel->layer(), mFilterModel );
    }

    mFeatureSelectionModel = new QgsFeatureSelectionModel( mFilterModel, mFilterModel, mFeatureSelectionManager, mFilterModel );
    setSelectionModel( mFeatureSelectionModel );
    mTableDelegate->setFeatureSelectionModel( mFeatureSelectionModel );
    connect( mFeatureSelectionModel, SIGNAL( requestRepaint( QModelIndexList ) ), this, SLOT( repaintRequested( QModelIndexList ) ) );
    connect( mFeatureSelectionModel, SIGNAL( requestRepaint() ), this, SLOT( repaintRequested() ) );
  }
}

void QgsAttributeTableView::setFeatureSelectionManager( QgsIFeatureSelectionManager* featureSelectionManager )
{
  if ( mFeatureSelectionManager )
    delete mFeatureSelectionManager;

  mFeatureSelectionManager = featureSelectionManager;

  if ( mFeatureSelectionModel )
    mFeatureSelectionModel->setFeatureSelectionManager( mFeatureSelectionManager );
}

void QgsAttributeTableView::closeEvent( QCloseEvent *e )
{
  Q_UNUSED( e );
  QSettings settings;
  settings.setValue( "/BetterAttributeTable/geometry", QVariant( saveGeometry() ) );
}

void QgsAttributeTableView::mousePressEvent( QMouseEvent *event )
{
  setSelectionMode( QAbstractItemView::NoSelection );
  QTableView::mousePressEvent( event );
  setSelectionMode( QAbstractItemView::ExtendedSelection );
}

void QgsAttributeTableView::mouseReleaseEvent( QMouseEvent *event )
{
  setSelectionMode( QAbstractItemView::NoSelection );
  QTableView::mouseReleaseEvent( event );
  setSelectionMode( QAbstractItemView::ExtendedSelection );
}

void QgsAttributeTableView::mouseMoveEvent( QMouseEvent *event )
{
  setSelectionMode( QAbstractItemView::NoSelection );
  QTableView::mouseMoveEvent( event );
  setSelectionMode( QAbstractItemView::ExtendedSelection );
}

void QgsAttributeTableView::keyPressEvent( QKeyEvent *event )
{
  switch ( event->key() )
  {

      // Default Qt behavior would be to change the selection.
      // We don't make it that easy for the user to trash his selection.
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Left:
    case Qt::Key_Right:
      setSelectionMode( QAbstractItemView::NoSelection );
      QTableView::keyPressEvent( event );
      setSelectionMode( QAbstractItemView::ExtendedSelection );
      break;

    default:
      QTableView::keyPressEvent( event );
      break;
  }
}

void QgsAttributeTableView::repaintRequested( QModelIndexList indexes )
{
  foreach ( const QModelIndex index, indexes )
  {
    update( index );
  }
}

void QgsAttributeTableView::repaintRequested()
{
  setDirtyRegion( viewport()->rect() );
}

void QgsAttributeTableView::selectAll()
{
  QItemSelection selection;
  selection.append( QItemSelectionRange( mFilterModel->index( 0, 0 ), mFilterModel->index( mFilterModel->rowCount() - 1, 0 ) ) );
  mFeatureSelectionModel->selectFeatures( selection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
}

void QgsAttributeTableView::contextMenuEvent( QContextMenuEvent* event )
{
  delete mActionPopup;
  mActionPopup = 0;

  QModelIndex idx = indexAt( event->pos() );
  if ( !idx.isValid() )
  {
    return;
  }

  QgsVectorLayer *vlayer = mFilterModel->layer();
  if ( !vlayer )
    return;

  mActionPopup = new QMenu();

  mActionPopup->addAction( tr( "Select All" ), this, SLOT( selectAll() ), QKeySequence::SelectAll );

  // let some other parts of the application add some actions
  emit willShowContextMenu( mActionPopup, idx );

  if ( mActionPopup->actions().count() > 0 )
  {
    mActionPopup->popup( event->globalPos() );
  }
}

void QgsAttributeTableView::selectRow( int row )
{
  selectRow( row, true );
}

void QgsAttributeTableView::_q_selectRow( int row )
{
  selectRow( row, false );
}

void QgsAttributeTableView::modelDeleted()
{
  mFilterModel = 0;
  mFeatureSelectionManager = 0;
  mFeatureSelectionModel = 0;
}

void QgsAttributeTableView::selectRow( int row, bool anchor )
{
  if ( selectionBehavior() == QTableView::SelectColumns
       || ( selectionMode() == QTableView::SingleSelection
            && selectionBehavior() == QTableView::SelectItems ) )
    return;

  if ( row >= 0 && row < model()->rowCount() )
  {
    int column = horizontalHeader()->logicalIndexAt( isRightToLeft() ? viewport()->width() : 0 );
    QModelIndex index = model()->index( row, column );
    QItemSelectionModel::SelectionFlags command =  selectionCommand( index );
    selectionModel()->setCurrentIndex( index, QItemSelectionModel::NoUpdate );
    if (( anchor && !( command & QItemSelectionModel::Current ) )
        || ( selectionMode() == QTableView::SingleSelection ) )
      mRowSectionAnchor = row;

    if ( selectionMode() != QTableView::SingleSelection
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

    QModelIndex tl = model()->index( qMin( mRowSectionAnchor, row ), 0 );
    QModelIndex br = model()->index( qMax( mRowSectionAnchor, row ), model()->columnCount() - 1 );
    if ( verticalHeader()->sectionsMoved() && tl.row() != br.row() )
      setSelection( visualRect( tl ) | visualRect( br ), command );
    else
      mFeatureSelectionModel->selectFeatures( QItemSelection( tl, br ), command );
  }
}
