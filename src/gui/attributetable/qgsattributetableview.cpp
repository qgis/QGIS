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
#include <QToolButton>
#include <QHBoxLayout>

#include "qgsactionmanager.h"
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
    , mMasterModel( nullptr )
    , mFilterModel( nullptr )
    , mFeatureSelectionModel( nullptr )
    , mFeatureSelectionManager( nullptr )
    , mModel( nullptr )
    , mActionPopup( nullptr )
    , mRowSectionAnchor( 0 )
    , mCtrlDragSelectionFlag( QItemSelectionModel::Select )
    , mActionWidget( nullptr )
{
  QSettings settings;
  restoreGeometry( settings.value( "/BetterAttributeTable/geometry" ).toByteArray() );

  //verticalHeader()->setDefaultSectionSize( 20 );
  horizontalHeader()->setHighlightSections( false );

  // We need mouse move events to create the action button on hover
  setMouseTracking( true );

  mTableDelegate = new QgsAttributeTableDelegate( this );
  setItemDelegate( mTableDelegate );

  setSelectionBehavior( QAbstractItemView::SelectRows );
  setSelectionMode( QAbstractItemView::ExtendedSelection );
  setSortingEnabled( true ); // At this point no data is in the model yet, so actually nothing is sorted.
  horizontalHeader()->setSortIndicatorShown( false ); // So hide the indicator to avoid confusion.

  verticalHeader()->viewport()->installEventFilter( this );

  connect( verticalHeader(), SIGNAL( sectionPressed( int ) ), this, SLOT( selectRow( int ) ) );
  connect( verticalHeader(), SIGNAL( sectionEntered( int ) ), this, SLOT( _q_selectRow( int ) ) );
  connect( horizontalHeader(), SIGNAL( sectionResized( int, int, int ) ), this, SLOT( columnSizeChanged( int, int, int ) ) );
  connect( horizontalHeader(), SIGNAL( sortIndicatorChanged( int, Qt::SortOrder ) ), this, SLOT( showHorizontalSortIndicator() ) );
}

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
  mFeatureSelectionModel = nullptr;

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

  mActionWidget = createActionWidget( 0 );
  mActionWidget->setVisible( false );
  updateActionImage( mActionWidget );
}

void QgsAttributeTableView::setFeatureSelectionManager( QgsIFeatureSelectionManager* featureSelectionManager )
{
  if ( mFeatureSelectionManager )
    delete mFeatureSelectionManager;

  mFeatureSelectionManager = featureSelectionManager;

  if ( mFeatureSelectionModel )
    mFeatureSelectionModel->setFeatureSelectionManager( mFeatureSelectionManager );
}

QWidget* QgsAttributeTableView::createActionWidget( QgsFeatureId fid )
{
  QgsAttributeTableConfig attributeTableConfig = mFilterModel->layer()->attributeTableConfig();
  QgsActionManager* actions = mFilterModel->layer()->actions();

  QToolButton* toolButton = nullptr;
  QWidget* container = nullptr;

  if ( attributeTableConfig.actionWidgetStyle() == QgsAttributeTableConfig::DropDown )
  {
    toolButton  = new QToolButton( this );
    toolButton->setPopupMode( QToolButton::MenuButtonPopup );
    container = toolButton;
  }
  else
  {
    container = new QWidget( this );
    container->setLayout( new QHBoxLayout() );
    container->layout()->setMargin( 0 );
  }

  for ( int i = 0; i < actions->size(); ++i )
  {
    const QgsAction& action = actions->at( i );

    if ( !action.showInAttributeTable() )
      continue;

    QString actionTitle = !action.shortTitle().isEmpty() ? action.shortTitle() : action.icon().isNull() ? action.name() : "";
    QAction* act = new QAction( action.icon(), actionTitle, container );
    act->setToolTip( action.name() );
    act->setData( i );
    act->setProperty( "fid", fid );

    connect( act, SIGNAL( triggered( bool ) ), this, SLOT( actionTriggered() ) );

    if ( attributeTableConfig.actionWidgetStyle() == QgsAttributeTableConfig::DropDown )
    {
      toolButton->addAction( act );

      if ( actions->defaultAction() == i )
        toolButton->setDefaultAction( act );

      container = toolButton;
    }
    else
    {
      QToolButton* btn = new QToolButton;
      btn->setDefaultAction( act );
      container->layout()->addWidget( btn );
    }
  }

  if ( toolButton && !toolButton->actions().isEmpty() && actions->defaultAction() == -1 )
    toolButton->setDefaultAction( toolButton->actions().at( 0 ) );

  return container;
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
  QModelIndex index = indexAt( event->pos() );
  if ( index.data( QgsAttributeTableFilterModel::TypeRole ) == QgsAttributeTableFilterModel::ColumnTypeActionButton )
  {
    Q_ASSERT( index.isValid() );

    if ( !indexWidget( index ) )
      setIndexWidget( index, createActionWidget( mFilterModel->data( index, QgsAttributeTableModel::FeatureIdRole ).toLongLong() ) );
  }

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

void QgsAttributeTableView::repaintRequested( const QModelIndexList& indexes )
{
  Q_FOREACH ( const QModelIndex& index, indexes )
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
  mActionPopup = nullptr;

  QModelIndex idx = indexAt( event->pos() );
  if ( !idx.isValid() )
  {
    return;
  }

  QgsVectorLayer *vlayer = mFilterModel->layer();
  if ( !vlayer )
    return;

  mActionPopup = new QMenu( this );

  mActionPopup->addAction( tr( "Select All" ), this, SLOT( selectAll() ), QKeySequence::SelectAll );

  // let some other parts of the application add some actions
  emit willShowContextMenu( mActionPopup, idx );

  if ( !mActionPopup->actions().isEmpty() )
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
  mFilterModel = nullptr;
  mFeatureSelectionManager = nullptr;
  mFeatureSelectionModel = nullptr;
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

void QgsAttributeTableView::showHorizontalSortIndicator()
{
  horizontalHeader()->setSortIndicatorShown( true );
}

void QgsAttributeTableView::actionTriggered()
{
  QAction* action = qobject_cast<QAction*>( sender() );
  QgsFeatureId fid = action->property( "fid" ).toLongLong();

  QgsFeature f;
  mFilterModel->layerCache()->getFeatures( QgsFeatureRequest( fid ) ).nextFeature( f );

  mFilterModel->layer()->actions()->doAction( action->data().toInt(), f );
}

void QgsAttributeTableView::columnSizeChanged( int index, int oldWidth, int newWidth )
{
  Q_UNUSED( oldWidth )
  if ( mFilterModel->actionColumnIndex() == index )
  {
    mActionWidget->resize( newWidth, mActionWidget->height() );
    updateActionImage( mActionWidget );
  }
}

void QgsAttributeTableView::updateActionImage( QWidget* widget )
{
  QImage image( widget->size(), QImage::Format_ARGB32_Premultiplied );
  QPainter painter( &image );
  widget->render( &painter );
  mTableDelegate->setActionWidgetImage( image );
}
