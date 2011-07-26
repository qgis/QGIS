/***************************************************************************
                         qgscomposerview.cpp
                         -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QMainWindow>
#include <QMouseEvent>
#include <QKeyEvent>

#include "qgscomposerview.h"
#include "qgscomposerarrow.h"
#include "qgscomposerlabel.h"
#include "qgscomposerlegend.h"
#include "qgscomposermap.h"
#include "qgscomposeritemgroup.h"
#include "qgscomposerpicture.h"
#include "qgscomposerscalebar.h"
#include "qgscomposershape.h"
#include "qgscomposerattributetable.h"
#include "qgslogger.h"

#ifndef QT_NO_OPENGL
#include <QtOpenGL/QGLWidget>
#endif

QgsComposerView::QgsComposerView( QWidget* parent, const char* name, Qt::WFlags f )
    : QGraphicsView( parent )
    , mShiftKeyPressed( false )
    , mRubberBandItem( 0 )
    , mRubberBandLineItem( 0 )
    , mMoveContentItem( 0 )
    , mPaintingEnabled( true )
{
  Q_UNUSED( f );
  Q_UNUSED( name );

  setResizeAnchor( QGraphicsView::AnchorViewCenter );
  setMouseTracking( true );
  
#ifndef QT_NO_OPENGL
  setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
  viewport()->setMouseTracking(true);
  setRenderHint(QPainter::HighQualityAntialiasing, true);
#endif
  
//  viewport()->setMouseTracking( true );
}

void QgsComposerView::mousePressEvent( QMouseEvent* e )
{
  if ( !composition() )
  {
    return;
  }

  QPointF scenePoint = mapToScene( e->pos() );
  QPointF snappedScenePoint = composition()->snapPointToGrid( scenePoint );

  //lock/unlock position of item with right click
  if ( e->button() == Qt::RightButton )
  {
    QgsComposerItem* selectedItem = composition()->composerItemAt( scenePoint );
    if ( selectedItem )
    {
      bool lock = selectedItem->positionLock() ? false : true;
      selectedItem->setPositionLock( lock );
      selectedItem->update();
      //make sure the new cursor is correct
      QPointF itemPoint = selectedItem->mapFromScene( scenePoint );
      selectedItem->updateCursor( itemPoint );
    }
    return;
  }

  switch ( mCurrentTool )
  {
      //select/deselect items and pass mouse event further
    case Select:
    {
      if ( !mShiftKeyPressed ) //keep selection if shift key pressed
      {
        composition()->clearSelection();
      }

      //select topmost item at position of event
      QgsComposerItem* selectedItem = composition()->composerItemAt( scenePoint );
      if ( !selectedItem )
      {
        break;
      }

      selectedItem->setSelected( true );
      QGraphicsView::mousePressEvent( e );
      emit selectedItemChanged( selectedItem );
      break;
    }

    case MoveItemContent:
    {
      //store item as member if it is selected and cursor is over item
      QgsComposerItem* item = dynamic_cast<QgsComposerItem *>( itemAt( e->pos() ) );
      if ( item )
      {
        mMoveContentStartPos = scenePoint;
      }
      mMoveContentItem = item;
      break;
    }

    case AddArrow:
    {
      mRubberBandStartPos = QPointF( snappedScenePoint.x(), snappedScenePoint.y() );
      mRubberBandLineItem = new QGraphicsLineItem( snappedScenePoint.x(), snappedScenePoint.y(), snappedScenePoint.x(), snappedScenePoint.y() );
      mRubberBandLineItem->setZValue( 100 );
      scene()->addItem( mRubberBandLineItem );
      scene()->update();
      break;
    }

    //create rubber band for map and ellipse items
    case AddMap:
    case AddShape:
    {
      QTransform t;
      mRubberBandItem = new QGraphicsRectItem( 0, 0, 0, 0 );
      mRubberBandStartPos = QPointF( snappedScenePoint.x(), snappedScenePoint.y() );
      t.translate( snappedScenePoint.x(), snappedScenePoint.y() );
      mRubberBandItem->setTransform( t );
      mRubberBandItem->setZValue( 100 );
      scene()->addItem( mRubberBandItem );
      scene()->update();
    }
    break;

    case AddLabel:
    {
      QgsComposerLabel* newLabelItem = new QgsComposerLabel( composition() );
      newLabelItem->setText( tr( "Quantum GIS" ) );
      newLabelItem->adjustSizeToText();
      newLabelItem->setSceneRect( QRectF( snappedScenePoint.x(), snappedScenePoint.y(), newLabelItem->rect().width(), newLabelItem->rect().height() ) );
      addComposerLabel( newLabelItem );
      emit actionFinished();
    }
    break;

    case AddScalebar:
    {
      QgsComposerScaleBar* newScaleBar = new QgsComposerScaleBar( composition() );
      newScaleBar->setSceneRect( QRectF( snappedScenePoint.x(), snappedScenePoint.y(), 20, 20 ) );
      addComposerScaleBar( newScaleBar );
      emit actionFinished();
    }
    break;

    case AddLegend:
    {
      QgsComposerLegend* newLegend = new QgsComposerLegend( composition() );
      newLegend->setSceneRect( QRectF( snappedScenePoint.x(), snappedScenePoint.y(), newLegend->rect().width(), newLegend->rect().height() ) );
      addComposerLegend( newLegend );
      emit actionFinished();
      break;
    }
    case AddPicture:
    {
      QgsComposerPicture* newPicture = new QgsComposerPicture( composition() );
      newPicture->setSceneRect( QRectF( snappedScenePoint.x(), snappedScenePoint.y(), 30, 30 ) );
      addComposerPicture( newPicture );
      emit actionFinished();
      break;
    }
    case AddTable:
    {
      QgsComposerAttributeTable* newTable = new QgsComposerAttributeTable( composition() );
      newTable->setSceneRect( QRectF( snappedScenePoint.x(), snappedScenePoint.y(), 50, 50 ) );
      addComposerTable( newTable );
      emit actionFinished();
      break;
    }

    default:
      break;
  }
}

void QgsComposerView::mouseReleaseEvent( QMouseEvent* e )
{
  if ( !composition() )
  {
    return;
  }

  QPointF scenePoint = mapToScene( e->pos() );

  switch ( mCurrentTool )
  {
    case Select:
    {
      QGraphicsView::mouseReleaseEvent( e );
      break;
    }

    case MoveItemContent:
    {
      if ( mMoveContentItem )
      {
        //update map preview if composer map
        QgsComposerMap* composerMap = dynamic_cast<QgsComposerMap *>( mMoveContentItem );
        if ( composerMap )
        {
          composerMap->setOffset( 0, 0 );
        }

        double moveX = scenePoint.x() - mMoveContentStartPos.x();
        double moveY = scenePoint.y() - mMoveContentStartPos.y();

        composition()->beginCommand( mMoveContentItem, tr( "Move item content" ) );
        mMoveContentItem->moveContent( -moveX, -moveY );
        composition()->endCommand();
        mMoveContentItem = 0;
      }
      break;
    }
    case AddArrow:
    {
      QPointF scenePoint = mapToScene( e->pos() );
      QPointF snappedScenePoint = composition()->snapPointToGrid( scenePoint );
      QgsComposerArrow* composerArrow = new QgsComposerArrow( mRubberBandStartPos, QPointF( snappedScenePoint.x(), snappedScenePoint.y() ), composition() );
      addComposerArrow( composerArrow );
      scene()->removeItem( mRubberBandLineItem );
      delete mRubberBandLineItem;
      mRubberBandLineItem = 0;
      emit actionFinished();
      break;
    }

    case AddShape:
    {
      if ( !mRubberBandItem || mRubberBandItem->rect().width() < 0.1 || mRubberBandItem->rect().width() < 0.1 )
      {
        scene()->removeItem( mRubberBandItem );
        delete mRubberBandItem;
        mRubberBandItem = 0;
        return;
      }

      QgsComposerShape* composerShape = new QgsComposerShape( mRubberBandItem->transform().dx(), mRubberBandItem->transform().dy(), mRubberBandItem->rect().width(), mRubberBandItem->rect().height(), composition() );
      addComposerShape( composerShape );
      scene()->removeItem( mRubberBandItem );
      delete mRubberBandItem;
      mRubberBandItem = 0;
      emit actionFinished();
      break;
    }

    case AddMap:
    {
      if ( !mRubberBandItem || mRubberBandItem->rect().width() < 0.1 || mRubberBandItem->rect().width() < 0.1 )
      {
        if ( mRubberBandItem )
        {
          scene()->removeItem( mRubberBandItem );
          delete mRubberBandItem;
          mRubberBandItem = 0;
        }
        return;
      }

      QgsComposerMap* composerMap = new QgsComposerMap( composition(), mRubberBandItem->transform().dx(), mRubberBandItem->transform().dy(), mRubberBandItem->rect().width(), mRubberBandItem->rect().height() );
      addComposerMap( composerMap );
      scene()->removeItem( mRubberBandItem );
      delete mRubberBandItem;
      mRubberBandItem = 0;
      emit actionFinished();
    }
    break;

    default:
      break;
  }
}

void QgsComposerView::mouseMoveEvent( QMouseEvent* e )
{
  if ( !composition() )
  {
    return;
  }

  if ( e->buttons() == Qt::NoButton )
  {
    if ( mCurrentTool == Select )
    {
      QGraphicsView::mouseMoveEvent( e );
    }
  }
  else
  {
    QPointF scenePoint = mapToScene( e->pos() );

    switch ( mCurrentTool )
    {
      case Select:
        QGraphicsView::mouseMoveEvent( e );
        break;

      case AddArrow:
      {
        if ( mRubberBandLineItem )
        {
          mRubberBandLineItem->setLine( mRubberBandStartPos.x(), mRubberBandStartPos.y(),  scenePoint.x(),  scenePoint.y() );
        }
        break;
      }

      case AddMap:
      case AddShape:
        //adjust rubber band item
      {
        double x = 0;
        double y = 0;
        double width = 0;
        double height = 0;

        double dx = scenePoint.x() - mRubberBandStartPos.x();
        double dy = scenePoint.y() - mRubberBandStartPos.y();

        if ( dx < 0 )
        {
          x = scenePoint.x();
          width = -dx;
        }
        else
        {
          x = mRubberBandStartPos.x();
          width = dx;
        }

        if ( dy < 0 )
        {
          y = scenePoint.y();
          height = -dy;
        }
        else
        {
          y = mRubberBandStartPos.y();
          height = dy;
        }

        if ( mRubberBandItem )
        {
          mRubberBandItem->setRect( 0, 0, width, height );
          QTransform t;
          t.translate( x, y );
          mRubberBandItem->setTransform( t );
        }
        break;
      }

      case MoveItemContent:
      {
        //update map preview if composer map
        QgsComposerMap* composerMap = dynamic_cast<QgsComposerMap *>( mMoveContentItem );
        if ( composerMap )
        {
          composerMap->setOffset( scenePoint.x() - mMoveContentStartPos.x(), scenePoint.y() - mMoveContentStartPos.y() );
          composerMap->update();
        }
        break;
      }
      default:
        break;
    }
  }
}

void QgsComposerView::mouseDoubleClickEvent( QMouseEvent* e )
{
  e->ignore();
}

void QgsComposerView::keyPressEvent( QKeyEvent * e )
{
  if ( e->key() == Qt::Key_Shift )
  {
    mShiftKeyPressed = true;
  }

  if ( !composition() )
  {
    return;
  }

  QList<QgsComposerItem*> composerItemList = composition()->selectedComposerItems();
  QList<QgsComposerItem*>::iterator itemIt = composerItemList.begin();

  //delete selected items
  if ( e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backspace )
  {
    for ( ; itemIt != composerItemList.end(); ++itemIt )
    {
      QgsComposerMap* map = dynamic_cast<QgsComposerMap *>( *itemIt );
      if ( !map || !map->isDrawing() ) //don't delete a composer map while it draws
      {
        composition()->removeItem( *itemIt );
        QgsComposerItemGroup* itemGroup = dynamic_cast<QgsComposerItemGroup*>( *itemIt );
        if ( itemGroup && composition() )
        {
          //add add/remove item command for every item in the group
          QUndoCommand* parentCommand = new QUndoCommand( tr( "Remove item group" ) );

          QSet<QgsComposerItem*> groupedItems = itemGroup->items();
          QSet<QgsComposerItem*>::iterator it = groupedItems.begin();
          for ( ; it != groupedItems.end(); ++it )
          {
            QgsAddRemoveItemCommand* subcommand = new QgsAddRemoveItemCommand( QgsAddRemoveItemCommand::Removed, *it, composition(), "", parentCommand );
            connectAddRemoveCommandSignals( subcommand );
            emit itemRemoved( *it );
          }

          composition()->undoStack()->push( parentCommand );
          delete itemGroup;
          emit itemRemoved( itemGroup );
        }
        else
        {
          emit itemRemoved( *itemIt );
          pushAddRemoveCommand( *itemIt, tr( "Item deleted" ), QgsAddRemoveItemCommand::Removed );
        }
      }
    }
  }

  else if ( e->key() == Qt::Key_Left )
  {
    for ( ; itemIt != composerItemList.end(); ++itemIt )
    {
      ( *itemIt )->move( -1.0, 0.0 );
    }
  }
  else if ( e->key() == Qt::Key_Right )
  {
    for ( ; itemIt != composerItemList.end(); ++itemIt )
    {
      ( *itemIt )->move( 1.0, 0.0 );
    }
  }
  else if ( e->key() == Qt::Key_Down )
  {
    for ( ; itemIt != composerItemList.end(); ++itemIt )
    {
      ( *itemIt )->move( 0.0, 1.0 );
    }
  }
  else if ( e->key() == Qt::Key_Up )
  {
    for ( ; itemIt != composerItemList.end(); ++itemIt )
    {
      ( *itemIt )->move( 0.0, -1.0 );
    }
  }
}

void QgsComposerView::keyReleaseEvent( QKeyEvent * e )
{
  if ( e->key() == Qt::Key_Shift )
  {
    mShiftKeyPressed = false;
  }
}

void QgsComposerView::wheelEvent( QWheelEvent* event )
{
  QPointF scenePoint = mapToScene( event->pos() );

  //select topmost item at position of event
  QgsComposerItem* theItem = composition()->composerItemAt( scenePoint );
  if ( theItem )
  {
    if ( theItem->isSelected() )
    {
      QPointF itemPoint = theItem->mapFromScene( scenePoint );
      theItem->beginCommand( tr( "Zoom item content" ) );
      theItem->zoomContent( event->delta(), itemPoint.x(), itemPoint.y() );
      theItem->endCommand();
    }
  }
}

void QgsComposerView::paintEvent( QPaintEvent* event )
{
  if ( mPaintingEnabled )
  {
    QGraphicsView::paintEvent( event );
    event->accept();
  }
  else
  {
    event->ignore();
  }
}

void QgsComposerView::hideEvent( QHideEvent* e )
{
  emit( composerViewShow( this ) );
  e->ignore();
}

void QgsComposerView::showEvent( QShowEvent* e )
{
  emit( composerViewHide( this ) );
  e->ignore();
}

void QgsComposerView::setComposition( QgsComposition* c )
{
  setScene( c );
}

QgsComposition* QgsComposerView::composition()
{
  if ( scene() )
  {
    QgsComposition* c = dynamic_cast<QgsComposition *>( scene() );
    if ( c )
    {
      return c;
    }
  }
  return 0;
}

void QgsComposerView::addComposerArrow( QgsComposerArrow* arrow )
{
  composition()->addItem( arrow );
  emit composerArrowAdded( arrow );
  scene()->clearSelection();
  arrow->setSelected( true );
  emit selectedItemChanged( arrow );
  pushAddRemoveCommand( arrow, tr( "Arrow added" ) );
}

void QgsComposerView::addComposerLabel( QgsComposerLabel* label )
{
  composition()->addItem( label );
  emit composerLabelAdded( label );
  scene()->clearSelection();
  label->setSelected( true );
  emit selectedItemChanged( label );
  pushAddRemoveCommand( label, tr( "Label added" ) );
}

void QgsComposerView::addComposerMap( QgsComposerMap* map )
{
  scene()->addItem( map );
  //set default preview mode to cache. Must be done here between adding composer map to scene and emiting signal
  map->setPreviewMode( QgsComposerMap::Cache );
  map->cache();
  emit composerMapAdded( map );
  scene()->clearSelection();
  map->setSelected( true );
  emit selectedItemChanged( map );
  pushAddRemoveCommand( map, tr( "Map added" ) );
}

void QgsComposerView::addComposerScaleBar( QgsComposerScaleBar* scaleBar )
{
  //take first available map
  QList<const QgsComposerMap*> mapItemList = composition()->composerMapItems();
  if ( mapItemList.size() > 0 )
  {
    scaleBar->setComposerMap( mapItemList.at( 0 ) );
  }
  scaleBar->applyDefaultSize(); //4 segments, 1/5 of composer map width
  scene()->addItem( scaleBar );
  emit composerScaleBarAdded( scaleBar );
  scene()->clearSelection();
  scaleBar->setSelected( true );
  emit selectedItemChanged( scaleBar );
  pushAddRemoveCommand( scaleBar, tr( "Scale bar added" ) );
}

void QgsComposerView::addComposerLegend( QgsComposerLegend* legend )
{
  //take first available map
  QList<const QgsComposerMap*> mapItemList = composition()->composerMapItems();
  if ( mapItemList.size() > 0 )
  {
    legend->setComposerMap( mapItemList.at( 0 ) );
  }
  scene()->addItem( legend );
  emit composerLegendAdded( legend );
  scene()->clearSelection();
  legend->setSelected( true );
  emit selectedItemChanged( legend );
  pushAddRemoveCommand( legend, tr( "Legend added" ) );
}

void QgsComposerView::addComposerPicture( QgsComposerPicture* picture )
{
  scene()->addItem( picture );
  emit composerPictureAdded( picture );
  scene()->clearSelection();
  picture->setSelected( true );
  emit selectedItemChanged( picture );
  pushAddRemoveCommand( picture, tr( "Picture added" ) );
}

void QgsComposerView::addComposerShape( QgsComposerShape* shape )
{
  scene()->addItem( shape );
  emit composerShapeAdded( shape );
  scene()->clearSelection();
  shape->setSelected( true );
  emit selectedItemChanged( shape );
  pushAddRemoveCommand( shape, tr( "Shape added" ) );
}

void QgsComposerView::addComposerTable( QgsComposerAttributeTable* table )
{
  scene()->addItem( table );
  emit composerTableAdded( table );
  scene()->clearSelection();
  table->setSelected( true );
  emit selectedItemChanged( table );
  pushAddRemoveCommand( table, tr( "Table added" ) );
}

void QgsComposerView::groupItems()
{
  if ( !composition() )
  {
    return;
  }

  QList<QgsComposerItem*> selectionList = composition()->selectedComposerItems();
  if ( selectionList.size() < 2 )
  {
    return; //not enough items for a group
  }
  QgsComposerItemGroup* itemGroup = new QgsComposerItemGroup( composition() );

  QList<QgsComposerItem*>::iterator itemIter = selectionList.begin();
  for ( ; itemIter != selectionList.end(); ++itemIter )
  {
    itemGroup->addItem( *itemIter );
  }

  composition()->addItem( itemGroup );
  itemGroup->setSelected( true );
  emit selectedItemChanged( itemGroup );
}

void QgsComposerView::ungroupItems()
{
  if ( !composition() )
  {
    return;
  }

  QList<QgsComposerItem*> selectionList = composition()->selectedComposerItems();
  QList<QgsComposerItem*>::iterator itemIter = selectionList.begin();
  for ( ; itemIter != selectionList.end(); ++itemIter )
  {
    QgsComposerItemGroup* itemGroup = dynamic_cast<QgsComposerItemGroup *>( *itemIter );
    if ( itemGroup )
    {
      itemGroup->removeItems();
      composition()->removeItem( *itemIter );
      delete( *itemIter );
      emit itemRemoved( *itemIter );
    }
  }
}

void QgsComposerView::sendItemAddedSignal( QgsComposerItem* item )
{
  //cast and send proper signal
  item->setSelected( true );
  QgsComposerArrow* arrow = dynamic_cast<QgsComposerArrow*>( item );
  if ( arrow )
  {
    emit composerArrowAdded( arrow );
    emit selectedItemChanged( arrow );
    return;
  }
  QgsComposerLabel* label = dynamic_cast<QgsComposerLabel*>( item );
  if ( label )
  {
    emit composerLabelAdded( label );
    emit selectedItemChanged( label );
    return;
  }
  QgsComposerMap* map = dynamic_cast<QgsComposerMap*>( item );
  if ( map )
  {
    emit composerMapAdded( map );
    emit selectedItemChanged( map );
    return;
  }
  QgsComposerScaleBar* scalebar = dynamic_cast<QgsComposerScaleBar*>( item );
  if ( scalebar )
  {
    emit composerScaleBarAdded( scalebar );
    emit selectedItemChanged( scalebar );
    return;
  }
  QgsComposerLegend* legend = dynamic_cast<QgsComposerLegend*>( item );
  if ( legend )
  {
    emit composerLegendAdded( legend );
    emit selectedItemChanged( legend );
    return;
  }
  QgsComposerPicture* picture = dynamic_cast<QgsComposerPicture*>( item );
  if ( picture )
  {
    emit composerPictureAdded( picture );
    emit selectedItemChanged( picture );
    return;
  }
  QgsComposerShape* shape = dynamic_cast<QgsComposerShape*>( item );
  if ( shape )
  {
    emit composerShapeAdded( shape );
    emit selectedItemChanged( shape );
    return;
  }
  QgsComposerAttributeTable* table = dynamic_cast<QgsComposerAttributeTable*>( item );
  if ( table )
  {
    emit composerTableAdded( table );
    emit selectedItemChanged( table );
    return;
  }
}

QMainWindow* QgsComposerView::composerWindow()
{
  QMainWindow* composerObject = 0;
  QObject* currentObject = parent();
  if ( !currentObject )
  {
    return qobject_cast<QMainWindow *>( currentObject );
  }

  while ( true )
  {
    composerObject = qobject_cast<QMainWindow*>( currentObject );
    if ( composerObject || currentObject->parent() == 0 )
    {
      return composerObject;
    }
    currentObject = currentObject->parent();
  }

  return 0;
}

void QgsComposerView::connectAddRemoveCommandSignals( QgsAddRemoveItemCommand* c )
{
  if ( !c )
  {
    return;
  }
  QObject::connect( c, SIGNAL( itemRemoved( QgsComposerItem* ) ), this, SIGNAL( itemRemoved( QgsComposerItem* ) ) );
  QObject::connect( c, SIGNAL( itemAdded( QgsComposerItem* ) ), this, SLOT( sendItemAddedSignal( QgsComposerItem* ) ) );
}

void QgsComposerView::pushAddRemoveCommand( QgsComposerItem* item, const QString& text, QgsAddRemoveItemCommand::State state )
{
  if ( !composition() )
  {
    return;
  }

  QgsAddRemoveItemCommand* c = new QgsAddRemoveItemCommand( state, item, composition(), text );
  connectAddRemoveCommandSignals( c );
  composition()->undoStack()->push( c );
}
