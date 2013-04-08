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
#include "qgsvectordataprovider.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"

QgsAttributeTableView::QgsAttributeTableView( QWidget *parent )
    : QTableView( parent ),
    mMasterModel( NULL ),
    mFilterModel( NULL ),
    mActionPopup( NULL )
{
  QSettings settings;
  restoreGeometry( settings.value( "/BetterAttributeTable/geometry" ).toByteArray() );

  verticalHeader()->setDefaultSectionSize( 20 );
  horizontalHeader()->setHighlightSections( false );

  setItemDelegate( new QgsAttributeTableDelegate( this ) );

  setSelectionBehavior( QAbstractItemView::SelectRows );
  setSelectionMode( QAbstractItemView::ExtendedSelection );
  setSortingEnabled( true );

  verticalHeader()->viewport()->installEventFilter( this );
}

QgsAttributeTableView::~QgsAttributeTableView()
{
  if ( mActionPopup )
  {
    delete mActionPopup;
  }
}

void QgsAttributeTableView::setCanvasAndLayerCache( QgsMapCanvas *canvas, QgsVectorLayerCache *layerCache )
{
  QgsAttributeTableModel* oldModel = mMasterModel;
  QgsAttributeTableFilterModel* filterModel = mFilterModel;

  mMasterModel = new QgsAttributeTableModel( layerCache, this );

  mLayerCache = layerCache;

  mMasterModel->loadLayer();

  mFilterModel = new QgsAttributeTableFilterModel( canvas, mMasterModel, mMasterModel );
  setModel( mFilterModel );

  delete oldModel;
  delete filterModel;
}

bool QgsAttributeTableView::eventFilter(QObject *object, QEvent *event)
{
  if ( object == verticalHeader()->viewport() )
  {
    switch ( event->type() )
    {
      case QEvent::MouseButtonPress:
        mFilterModel->disableSelectionSync();
        break;

      case QEvent::MouseButtonRelease:
        mFilterModel->enableSelectionSync();
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
    disconnect( mMasterSelection, SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), this, SLOT( onMasterSelectionChanged( QItemSelection, QItemSelection ) ) );
    disconnect( selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), this, SLOT( onSelectionChanged( QItemSelection, QItemSelection ) ) );

    disconnect( mFilterModel, SIGNAL( filterAboutToBeInvalidated() ), this, SLOT( onFilterAboutToBeInvalidated() ) );
    disconnect( mFilterModel, SIGNAL( filterInvalidated() ), this, SLOT( onFilterInvalidated() ) );
  }

  mFilterModel = filterModel;
  QTableView::setModel( filterModel );

  if ( filterModel )
  {
    // Connect new model stuff
    mMasterSelection = mFilterModel->masterSelection();

    connect( mMasterSelection, SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), SLOT( onMasterSelectionChanged( QItemSelection, QItemSelection ) ) );
    connect( selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), SLOT( onSelectionChanged( QItemSelection, QItemSelection ) ) );

    connect( mFilterModel, SIGNAL( filterAboutToBeInvalidated() ), SLOT( onFilterAboutToBeInvalidated() ) );
    connect( mFilterModel, SIGNAL( filterInvalidated() ), SLOT( onFilterInvalidated() ) );
  }
}

QItemSelectionModel* QgsAttributeTableView::masterSelection()
{
  return mMasterSelection;
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

void QgsAttributeTableView::onFilterAboutToBeInvalidated()
{
  disconnect( selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), this, SLOT( onSelectionChanged( QItemSelection, QItemSelection ) ) );
}

void QgsAttributeTableView::onFilterInvalidated()
{
  QItemSelection localSelection = mFilterModel->mapSelectionFromSource( mMasterSelection->selection() );
  selectionModel()->select( localSelection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
  connect( selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), this, SLOT( onSelectionChanged( QItemSelection, QItemSelection ) ) );
}

void QgsAttributeTableView::onSelectionChanged( const QItemSelection& selected, const QItemSelection& deselected )
{
  disconnect( mMasterSelection, SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), this, SLOT( onMasterSelectionChanged( QItemSelection, QItemSelection ) ) );
  QItemSelection masterSelected = mFilterModel->mapSelectionToSource( selected );
  QItemSelection masterDeselected = mFilterModel->mapSelectionToSource( deselected );

  mMasterSelection->select( masterSelected, QItemSelectionModel::Select );
  mMasterSelection->select( masterDeselected, QItemSelectionModel::Deselect );
  connect( mMasterSelection, SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), SLOT( onMasterSelectionChanged( QItemSelection, QItemSelection ) ) );
}

void QgsAttributeTableView::onMasterSelectionChanged( const QItemSelection& selected, const QItemSelection& deselected )
{
  Q_UNUSED( selected )
  Q_UNUSED( deselected )
  disconnect( selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), this, SLOT( onSelectionChanged( QItemSelection, QItemSelection ) ) );

  // Synchronizing the whole selection seems to work faster than using the deltas (Deselecting takes pretty long)
  QItemSelection localSelection = mFilterModel->mapSelectionFromSource( mMasterSelection->selection() );
  selectionModel()->select( localSelection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );

  connect( selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), this, SLOT( onSelectionChanged( QItemSelection, QItemSelection ) ) );
}

void QgsAttributeTableView::selectAll()
{
  QItemSelection selection;
  selection.append( QItemSelectionRange( mFilterModel->index( 0, 0 ), mFilterModel->index( mFilterModel->rowCount() - 1, 0 ) ) );
  selectionModel()->select( selection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
}

void QgsAttributeTableView::contextMenuEvent( QContextMenuEvent* event )
{
  if ( mActionPopup )
  {
    delete mActionPopup;
    mActionPopup = 0;
  }

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
