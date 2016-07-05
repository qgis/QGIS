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
#include "qgsmaplayeractionregistry.h"

QgsAttributeTableView::QgsAttributeTableView( QWidget *parent )
    : QTableView( parent )
    , mFilterModel( nullptr )
    , mFeatureSelectionModel( nullptr )
    , mFeatureSelectionManager( nullptr )
    , mActionPopup( nullptr )
    , mRowSectionAnchor( 0 )
    , mCtrlDragSelectionFlag( QItemSelectionModel::Select )
{
  QSettings settings;
  restoreGeometry( settings.value( "/BetterAttributeTable/geometry" ).toByteArray() );

  //verticalHeader()->setDefaultSectionSize( 20 );
  horizontalHeader()->setHighlightSections( false );

  // We need mouse move events to create the action button on hover
  mTableDelegate = new QgsAttributeTableDelegate( this );
  setItemDelegate( mTableDelegate );

  setEditTriggers( QAbstractItemView::AllEditTriggers );

  setSelectionBehavior( QAbstractItemView::SelectRows );
  setSelectionMode( QAbstractItemView::ExtendedSelection );
  setSortingEnabled( true ); // At this point no data is in the model yet, so actually nothing is sorted.
  horizontalHeader()->setSortIndicatorShown( false ); // So hide the indicator to avoid confusion.

  verticalHeader()->viewport()->installEventFilter( this );

  connect( verticalHeader(), SIGNAL( sectionPressed( int ) ), this, SLOT( selectRow( int ) ) );
  connect( verticalHeader(), SIGNAL( sectionEntered( int ) ), this, SLOT( _q_selectRow( int ) ) );
  connect( horizontalHeader(), SIGNAL( sectionResized( int, int, int ) ), this, SLOT( columnSizeChanged( int, int, int ) ) );
  connect( horizontalHeader(), SIGNAL( sortIndicatorChanged( int, Qt::SortOrder ) ), this, SLOT( showHorizontalSortIndicator() ) );
  connect( QgsMapLayerActionRegistry::instance(), SIGNAL( changed() ), this, SLOT( recreateActionWidgets() ) );
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

void QgsAttributeTableView::setAttributeTableConfig( const QgsAttributeTableConfig& config )
{
  int i = 0;
  Q_FOREACH ( const QgsAttributeTableConfig::ColumnConfig& columnConfig, config.columns() )
  {
    if ( columnConfig.hidden )
      continue;

    if ( columnConfig.width >= 0 )
    {
      setColumnWidth( i, columnConfig.width );
    }
    else
    {
      setColumnWidth( i, horizontalHeader()->defaultSectionSize() );
    }
    i++;
  }
}

void QgsAttributeTableView::setModel( QgsAttributeTableFilterModel* filterModel )
{
  mFilterModel = filterModel;
  QTableView::setModel( filterModel );

  if ( mFilterModel )
  {
    connect( mFilterModel, SIGNAL( destroyed() ), this, SLOT( modelDeleted() ) );
    connect( mTableDelegate, SIGNAL( actionColumnItemPainted( QModelIndex ) ), this, SLOT( onActionColumnItemPainted( QModelIndex ) ) );
  }

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

  QToolButton* toolButton = nullptr;
  QWidget* container = nullptr;

  if ( attributeTableConfig.actionWidgetStyle() == QgsAttributeTableConfig::DropDown )
  {
    toolButton  = new QToolButton();
    toolButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
    toolButton->setPopupMode( QToolButton::MenuButtonPopup );
    container = toolButton;
  }
  else
  {
    container = new QWidget();
    container->setLayout( new QHBoxLayout() );
    container->layout()->setMargin( 0 );
  }

  QList< QAction* > actionList;
  QAction* defaultAction = nullptr;

  // first add user created layer actions
  QgsActionManager* actions = mFilterModel->layer()->actions();
  for ( int i = 0; i < actions->size(); ++i )
  {
    const QgsAction& action = actions->at( i );

    if ( !action.showInAttributeTable() )
      continue;

    QString actionTitle = !action.shortTitle().isEmpty() ? action.shortTitle() : action.icon().isNull() ? action.name() : "";
    QAction* act = new QAction( action.icon(), actionTitle, container );
    act->setToolTip( action.name() );
    act->setData( "user_action" );
    act->setProperty( "action_id", i );
    act->setProperty( "fid", fid );
    connect( act, SIGNAL( triggered( bool ) ), this, SLOT( actionTriggered() ) );
    actionList << act;

    if ( actions->defaultAction() == i )
      defaultAction = act;
  }

  // next add any registered actions for this layer
  Q_FOREACH ( QgsMapLayerAction* mapLayerAction,
              QgsMapLayerActionRegistry::instance()->mapLayerActions( mFilterModel->layer(),
                  QgsMapLayerAction::SingleFeature ) )
  {
    QAction* action = new QAction( mapLayerAction->icon(), mapLayerAction->text(), container );
    action->setData( "map_layer_action" );
    action->setToolTip( mapLayerAction->text() );
    action->setProperty( "fid", fid );
    action->setProperty( "action", qVariantFromValue( qobject_cast<QObject *>( mapLayerAction ) ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( actionTriggered() ) );
    actionList << action;

    if ( !defaultAction &&
         QgsMapLayerActionRegistry::instance()->defaultActionForLayer( mFilterModel->layer() ) == mapLayerAction )
      defaultAction = action;
  }

  if ( !defaultAction && !actionList.isEmpty() )
    defaultAction = actionList.at( 0 );

  Q_FOREACH ( QAction* act, actionList )
  {
    if ( attributeTableConfig.actionWidgetStyle() == QgsAttributeTableConfig::DropDown )
    {
      toolButton->addAction( act );

      if ( act == defaultAction )
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

  if ( attributeTableConfig.actionWidgetStyle() == QgsAttributeTableConfig::ButtonList )
  {
    static_cast< QHBoxLayout* >( container->layout() )->addStretch();
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

  if ( action->data().toString() == "user_action" )
  {
    mFilterModel->layer()->actions()->doAction( action->property( "action_id" ).toInt(), f );
  }
  else if ( action->data().toString() == "map_layer_action" )
  {
    QObject* object = action->property( "action" ).value<QObject *>();
    QgsMapLayerAction* layerAction = qobject_cast<QgsMapLayerAction *>( object );
    if ( layerAction )
    {
      layerAction->triggerForFeature( mFilterModel->layer(), &f );
    }
  }
}

void QgsAttributeTableView::columnSizeChanged( int index, int oldWidth, int newWidth )
{
  Q_UNUSED( oldWidth )
  emit columnResized( index, newWidth );
}

void QgsAttributeTableView::onActionColumnItemPainted( const QModelIndex& index )
{
  if ( !indexWidget( index ) )
  {
    QWidget* widget = createActionWidget( mFilterModel->data( index, QgsAttributeTableModel::FeatureIdRole ).toLongLong() );
    mActionWidgets.insert( index, widget );
    setIndexWidget( index, widget );
  }
}

void QgsAttributeTableView::recreateActionWidgets()
{
  QMap< QModelIndex, QWidget* > newWidgets;
  QMap< QModelIndex, QWidget* >::const_iterator it = mActionWidgets.constBegin();
  for ( ; it != mActionWidgets.constEnd(); ++it )
  {
    it.value()->deleteLater(); //?
    QWidget* widget = createActionWidget( mFilterModel->data( it.key(), QgsAttributeTableModel::FeatureIdRole ).toLongLong() );
    newWidgets.insert( it.key(), widget );
    setIndexWidget( it.key(), widget );
  }
  mActionWidgets = newWidgets;
}
