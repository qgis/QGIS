/***************************************************************************
    qgsmaptoolextraitem.cpp
    ---------------------
    begin                : 2026/01/22
    copyright            : (C) 2026 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolextraitem.h"

#include "qgsextraitemutils.h"
#include "qgslinesymbol.h"
#include "qgslinesymbollayer.h"
#include "qgsmapmouseevent.h"
#include "qgsmarkersymbol.h"
#include "qgsrubberband.h"
#include "qgssnappingutils.h"

#include <qgraphicssceneevent.h>

///////////

/**
 * \ingroup gui
 * \brief Rubber band for one extra item.
 *
 * Provides methods to move and rotate the represented extra item.
 *
 * \note not available in Python bindings
 *
 * \since QGIS 4.2
 */
class QgsExtraItemRubberBand : public QgsRubberBand
{
  public:
    /**
     * Constructor
     * \param position extra item position in layer CRS
     * \param rotation rotation angle in degree between 0 and 360
     * \param mapCanvas map canvas
     * \param layer vector layer to which belong this extra item belongs
     * \param symbolLayer symbol layer to which belong this extra item belongs
     */
    QgsExtraItemRubberBand( QgsPointXY position, double rotation, QgsMapCanvas *mapCanvas, QgsVectorLayer *layer, QgsTemplatedLineSymbolLayerBase *symbolLayer );

    /**
     * Attempt to move this item by \a deltaX horizontally and \a deltaY vertically.
     * \a deltaX and \a deltaY are expressed in pixels.
     */
    void attemptMoveBy( double deltaX, double deltaY );

    /**
     * Attempt to rotate this item by \a deltaDegree around its center.
     * \a deltaDegree is expressed in degree
     */
    void attemptRotateBy( double deltaDegree );

    /**
     * Returns position in layer CRS
     */
    const QgsPointXY &position() const;

    /**
     * Returns rotation angle in degree between 0 and 360
     */
    double rotation() const;

    /**
     * Update item according to its current rotation angle and position
     */
    void update();

  private:
    QgsPointXY mPosition;
    double mRotation;
    QPointer<QgsVectorLayer> mLayer;
    QgsTemplatedLineSymbolLayerBase *mSymbolLayer = nullptr;
};

///////////

QgsMapToolExtraItemBase::QgsMapToolExtraItemBase( QgsMapCanvas *canvas, QgsVectorLayer *layer, QgsTemplatedLineSymbolLayerBase *symbolLayer, int extraItemsFieldIndex )
  : QgsMapTool( canvas )
  , mLayer( layer )
  , mSymbolLayer( symbolLayer )
  , mExtraItemsFieldIndex( extraItemsFieldIndex )
{
}

void QgsMapToolExtraItemBase::loadFeatureExtraItems()
{
  if ( FID_IS_NULL( mFeatureId ) || !mLayer )
    return;

  QgsFeature feature = mLayer->getFeature( mFeatureId );
  if ( !feature.isValid() )
    return;

  QString currentExtraItems = feature.attribute( mExtraItemsFieldIndex ).toString();

  QString error;
  QgsExtraItemUtils::ExtraItems extraItems = QgsExtraItemUtils::parseExtraItems( currentExtraItems, error );
  if ( !error.isEmpty() )
  {
    emit messageEmitted( tr( "Error while parsing feature extra items: %1" ).arg( error ), Qgis::MessageLevel::Critical );
    return;
  }

  mExtraItems.clear();

  for ( std::tuple<double, double, double> extraItem : extraItems )
  {
    const double x = std::get<0>( extraItem );
    const double y = std::get<1>( extraItem );
    const double angle = std::get<2>( extraItem );

    QgsExtraItemRubberBand *rubberBand = new QgsExtraItemRubberBand( QgsPointXY( x, y ), angle, mCanvas, mLayer, mSymbolLayer );
    mExtraItems.push_back( rubberBand );
  }
}

void QgsMapToolExtraItemBase::updateAttribute()
{
  if ( !mLayer )
    return;

  QStringList strExtraItems;
  for ( const QObjectUniquePtr<QgsExtraItemRubberBand> &extraItem : mExtraItems )
  {
    QgsPointXY point = extraItem->position();
    strExtraItems << qgsDoubleToString( point.x() ) + " "
                       + qgsDoubleToString( point.y() ) + " "
                       + qgsDoubleToString( extraItem->rotation() );
  }

  QString strNewExtraItems = strExtraItems.join( "," );

  mLayer->beginEditCommand( tr( "Set extra item list" ) );
  if ( mLayer->changeAttributeValue( mFeatureId, mExtraItemsFieldIndex, strNewExtraItems ) )
  {
    mLayer->endEditCommand();
    mLayer->triggerRepaint();
  }
  else
  {
    mLayer->destroyEditCommand();
  }
}

void QgsMapToolExtraItemBase::selectFeature( QgsMapMouseEvent *event )
{
  // find the closest feature to the pressed position
  const QgsPointLocator::Match m = mCanvas->snappingUtils()->snapToCurrentLayer( event->pos(), QgsPointLocator::Area );
  if ( !m.isValid() )
  {
    emit messageEmitted( tr( "No feature was detected at the clicked position. Please click closer to the feature or enhance the search tolerance under Settings->Options->Digitizing->Search radius for vertex edits" ), Qgis::MessageLevel::Critical );
    return;
  }

  mFeatureId = m.featureId();
  loadFeatureExtraItems();
  mState = State::FeatureSelected;
  event->ignore();
}

///////////

QgsMapToolAddExtraItem::QgsMapToolAddExtraItem( QgsMapCanvas *canvas, QgsVectorLayer *layer, QgsTemplatedLineSymbolLayerBase *symbolLayer, int extraItemsFieldIndex )
  : QgsMapToolExtraItemBase( canvas, layer, symbolLayer, extraItemsFieldIndex )
{
  mToolName = tr( "Extra item add tool" );
}

void QgsMapToolAddExtraItem::canvasPressEvent( QgsMapMouseEvent *e )
{
  if ( !mLayer || e->button() != Qt::LeftButton )
    return;

  switch ( mState )
  {
    case State::SelectFeature:
      selectFeature( e );
      break;

    case State::FeatureSelected:
    {
      const QgsPointXY layerPoint = toLayerCoordinates( mLayer, e->mapPoint() );
      QgsExtraItemRubberBand *newRubberBand = new QgsExtraItemRubberBand( layerPoint, 0, mCanvas, mLayer, mSymbolLayer );
      mExtraItems.push_back( newRubberBand );
      updateAttribute();
      e->ignore();
      break;
    }
  }
}

void QgsMapToolAddExtraItem::keyPressEvent( QKeyEvent *event )
{
  switch ( mState )
  {
    case State::SelectFeature:
      break;

    case State::FeatureSelected:
      if ( event->matches( QKeySequence::StandardKey::Cancel ) )
      {
        mFeatureId = FID_NULL;
        mState = State::SelectFeature;
        mExtraItems.clear();
        event->ignore();
      }
  }
}

///////////


QgsMapToolModifyExtraItems::QgsMapToolModifyExtraItemMouseHandles::QgsMapToolModifyExtraItemMouseHandles( QgsMapToolModifyExtraItems *mapTool, QgsMapCanvas *canvas )
  : QgsGraphicsViewMouseHandles( canvas )
  , mMapTool( mapTool )
  , mCanvas( canvas )
{
  setZValue( 100 );

  connect( mMapTool, &QgsMapToolModifyExtraItems::selectedItemsChanged, this, [this] { updateHandles(); } );
  mCanvas->scene()->addItem( this );

  setRotationEnabled( true );
  setResizeEnabled( false );
};

void QgsMapToolModifyExtraItems::QgsMapToolModifyExtraItemMouseHandles::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
  paintInternal( painter, true, true, true, option, widget );
}

void QgsMapToolModifyExtraItems::QgsMapToolModifyExtraItemMouseHandles::setViewportCursor( Qt::CursorShape cursor )
{
  if ( dynamic_cast<QgsMapToolModifyExtraItems *>( mCanvas->mapTool() ) )
  {
    mCanvas->viewport()->setCursor( cursor );
  }
}

QList<QGraphicsItem *> QgsMapToolModifyExtraItems::QgsMapToolModifyExtraItemMouseHandles::sceneItemsAtPoint( QPointF scenePoint )
{
  QList<QGraphicsItem *> graphicsItems;
  const QList<QGraphicsItem *> items = mMapTool->selectedItems();
  for ( auto item : items )
  {
    if ( item->sceneBoundingRect().contains( scenePoint ) )
    {
      graphicsItems << item;
    }
  }
  return graphicsItems;
}

QList<QGraphicsItem *> QgsMapToolModifyExtraItems::QgsMapToolModifyExtraItemMouseHandles::selectedSceneItems( bool ) const
{
  return mMapTool->selectedItems();
}

QRectF QgsMapToolModifyExtraItems::QgsMapToolModifyExtraItemMouseHandles::itemRect( QGraphicsItem *item ) const
{
  return item->boundingRect();
}

void QgsMapToolModifyExtraItems::QgsMapToolModifyExtraItemMouseHandles::moveItem( QGraphicsItem *item, double deltaX, double deltaY )
{
  mMapTool->attemptMoveBy( item, deltaX, deltaY );
}

void QgsMapToolModifyExtraItems::QgsMapToolModifyExtraItemMouseHandles::rotateItem( QGraphicsItem *item, double deltaDegree, double deltaCenterX, double deltaCenterY )
{
  mMapTool->attemptRotateBy( item, deltaDegree, deltaCenterX, deltaCenterY );
}

void QgsMapToolModifyExtraItems::QgsMapToolModifyExtraItemMouseHandles::setItemRect( QGraphicsItem *, QRectF )
{
  QgsDebugError( "Resize is not supported for extra item" );
}

///////////

QgsExtraItemRubberBand::QgsExtraItemRubberBand( QgsPointXY position, double rotation, QgsMapCanvas *mapCanvas, QgsVectorLayer *layer, QgsTemplatedLineSymbolLayerBase *symbolLayer )
  : QgsRubberBand( mapCanvas )
  , mPosition( position )
  , mRotation( rotation )
  , mLayer( layer )
  , mSymbolLayer( symbolLayer )
{
  setFlags( flags() | QGraphicsItem::ItemIsSelectable );

  setWidth( mapCanvas->fontMetrics().xHeight() * .2 );
  setStrokeColor( QColor( 255, 0, 0, 255 ) );
  setFillColor( QColor( 0, 0, 0, 0 ) );
  setZValue( 10 );

  update();

  connect( mMapCanvas, &QgsMapCanvas::mapCanvasRefreshed, this, &QgsExtraItemRubberBand::update );
}

void QgsExtraItemRubberBand::attemptMoveBy( double deltaX, double deltaY )
{
  const double mupp = mMapCanvas->mapSettings().mapUnitsPerPixel();
  QgsVector translation( deltaX * mupp, -deltaY * mupp );

  QgsPointXY mapPoint = mMapCanvas->mapSettings().layerToMapCoordinates( mLayer, mPosition );
  mapPoint += translation;
  mPosition = mMapCanvas->mapSettings().mapToLayerCoordinates( mLayer, mapPoint );

  update();
}

void QgsExtraItemRubberBand::attemptRotateBy( double deltaDegree )
{
  mRotation = std::fmod( mRotation + deltaDegree + 360., 360.0 );
  update();
}

const QgsPointXY &QgsExtraItemRubberBand::position() const
{
  return mPosition;
}

double QgsExtraItemRubberBand::rotation() const
{
  return mRotation;
}


void QgsExtraItemRubberBand::update()
{
  if ( !mLayer )
    return;

  QgsRenderContext renderContext = QgsRenderContext::fromMapSettings( mMapCanvas->mapSettings() );

  QgsPointXY mapPoint = mMapCanvas->mapSettings().layerToMapCoordinates( mLayer, mPosition );
  const QPointF canvasPoint = toCanvasCoordinates( mapPoint );

  QRectF boundingRect;
  if ( QgsMarkerSymbol *symbol = dynamic_cast<QgsMarkerSymbol *>( mSymbolLayer->subSymbol() ) )
  {
    boundingRect = symbol->bounds( canvasPoint, renderContext );
  }
  else if ( QgsHashedLineSymbolLayer *slHashedLine = dynamic_cast<QgsHashedLineSymbolLayer *>( mSymbolLayer ) )
  {
    QgsLineSymbol *symbol = static_cast<QgsLineSymbol *>( slHashedLine->subSymbol() );
    const double length = renderContext.convertToPainterUnits( slHashedLine->hashLength(), slHashedLine->hashLengthUnit() );
    const double width = symbol->width( renderContext );
    boundingRect = QRectF( canvasPoint - QPointF( width / 2, length / 2 + width / 2 ), QSizeF( width, length + width ) );
  }
  else // Shall never happen, extra items are only supported for marker and hashed line symbol layer
  {
    Q_ASSERT( false );
    return;
  }

  QTransform t = QTransform().translate( canvasPoint.x(), canvasPoint.y() ).rotate( mRotation ).translate( -canvasPoint.x(), -canvasPoint.y() );
  QPolygonF bounds = t.map( boundingRect );

  bool selected = isSelected();
  reset( Qgis::GeometryType::Polygon );
  const QgsMapToPixel *transform = mMapCanvas->getCoordinateTransform();
  for ( int i = 0; i < bounds.count(); i++ )
  {
    QPointF canvasPt = bounds.at( i );
    QgsPointXY mapPt = transform->toMapCoordinates( canvasPt.x(), canvasPt.y() );
    addPoint( mapPt, i == bounds.count() - 1 ); // update only last one
  }

  // selected is reinitialized during reset, we need to restore it
  setSelected( selected );
}


///////////

QgsMapToolModifyExtraItems::QgsMapToolModifyExtraItems( QgsMapCanvas *canvas, QgsVectorLayer *layer, QgsTemplatedLineSymbolLayerBase *symbolLayer, int extraItemsFieldIndex )
  : QgsMapToolExtraItemBase( canvas, layer, symbolLayer, extraItemsFieldIndex )
{
  mToolName = tr( "Extra item modify tool" );
}


void QgsMapToolModifyExtraItems::activate()
{
  connect( mCanvas, &QgsMapCanvas::extentsChanged, this, &QgsMapToolModifyExtraItems::onMapCanvasExtentsChanged );
  QgsMapToolExtraItemBase::activate();
}

void QgsMapToolModifyExtraItems::deactivate()
{
  QgsMapToolExtraItemBase::deactivate();
  disconnect( mCanvas, &QgsMapCanvas::extentsChanged, this, &QgsMapToolModifyExtraItems::onMapCanvasExtentsChanged );
}


QList<QGraphicsItem *> QgsMapToolModifyExtraItems::selectedItems() const
{
  return mSelectedItems;
}

void QgsMapToolModifyExtraItems::canvasPressEvent( QgsMapMouseEvent *e )
{
  if ( !mLayer || e->button() != Qt::LeftButton )
    return;

  switch ( mState )
  {
    case State::SelectFeature:
      selectFeature( e );
      mMouseHandles.reset( new QgsMapToolModifyExtraItemMouseHandles( this, mCanvas ) );
      break;

    case State::FeatureSelected:
    {
      const bool toggleSelection = e->modifiers() & Qt::ShiftModifier;
      QPointF scenePos = mCanvas->mapToScene( e->pos() );
      if ( !toggleSelection && !mSelectedItems.empty() && mMouseHandles->sceneBoundingRect().contains( scenePos ) )
      {
        QGraphicsSceneMouseEvent forwardedEvent( QEvent::GraphicsSceneMousePress );
        forwardedEvent.setPos( mMouseHandles->mapFromScene( scenePos ) );
        forwardedEvent.setScenePos( scenePos );
        forwardedEvent.setLastScenePos( mCanvas->mapToScene( mLastPos ) );
        forwardedEvent.setButton( Qt::LeftButton );
        mMouseHandles->mousePressEvent( &forwardedEvent );
      }
      else
      {
        mSelectionRect.setTopLeft( e->pos() );
        mSelectionRect.setBottomRight( e->pos() );
      }

      mLastPos = e->pos();

      break;
    }
  }
}

void QgsMapToolModifyExtraItems::canvasMoveEvent( QgsMapMouseEvent *event )
{
  if ( !mLayer )
    return;

  switch ( mState )
  {
    case State::SelectFeature:
      break;

    case State::FeatureSelected:
    {
      const QPointF scenePos = mCanvas->mapToScene( event->pos() );
      if ( event->buttons() == Qt::NoButton )
      {
        if ( mMouseHandles->sceneBoundingRect().contains( scenePos ) )
        {
          QGraphicsSceneHoverEvent forwardedEvent( QEvent::GraphicsSceneHoverMove );
          forwardedEvent.setPos( mMouseHandles->mapFromScene( scenePos ) );
          forwardedEvent.setScenePos( scenePos );
          mMouseHandles->hoverMoveEvent( &forwardedEvent );
          mHoveringMouseHandles = true;
        }
        else if ( mHoveringMouseHandles )
        {
          QGraphicsSceneHoverEvent forwardedEvent( QEvent::GraphicsSceneHoverLeave );
          forwardedEvent.setPos( mMouseHandles->mapFromScene( scenePos ) );
          forwardedEvent.setScenePos( scenePos );
          mMouseHandles->hoverMoveEvent( &forwardedEvent );
          mHoveringMouseHandles = false;
        }
      }
      else if ( event->buttons() == Qt::LeftButton )
      {
        if ( mMouseHandles->shouldBlockEvent( event ) )
        {
          QGraphicsSceneMouseEvent forwardedEvent( QEvent::GraphicsSceneMouseMove );
          forwardedEvent.setPos( mMouseHandles->mapFromScene( scenePos ) );
          forwardedEvent.setScenePos( scenePos );
          forwardedEvent.setLastScenePos( mCanvas->mapToScene( mLastPos ) );
          forwardedEvent.setButton( Qt::LeftButton );
          mMouseHandles->mouseMoveEvent( &forwardedEvent );
        }
        else
        {
          if ( !mDragging )
          {
            mDragging = true;
            mSelectionRubberBand.reset( new QgsRubberBand( mCanvas, Qgis::GeometryType::Polygon ) );
            QColor color( Qt::blue );
            color.setAlpha( 63 );
            mSelectionRubberBand->setColor( color );
            mSelectionRect.setTopLeft( event->pos() );
          }

          mSelectionRect.setBottomRight( event->pos() );
          if ( mSelectionRubberBand )
          {
            mSelectionRubberBand->setToCanvasRectangle( mSelectionRect );
            mSelectionRubberBand->show();
          }
        }
      }
      mLastPos = event->pos();
      break;
    }
  }
}

void QgsMapToolModifyExtraItems::canvasReleaseEvent( QgsMapMouseEvent *event )
{
  if ( !mLayer || event->button() != Qt::LeftButton )
    return;

  switch ( mState )
  {
    case State::SelectFeature:
      break;

    case State::FeatureSelected:
      if ( mMouseHandles && mMouseHandles->shouldBlockEvent( event ) )
      {
        const QPointF scenePos = mCanvas->mapToScene( event->pos() );
        QGraphicsSceneMouseEvent forwardedEvent( QEvent::GraphicsSceneMouseRelease );
        forwardedEvent.setPos( event->pos() );
        forwardedEvent.setScenePos( scenePos );
        forwardedEvent.setLastScenePos( mCanvas->mapToScene( mLastPos ) );
        forwardedEvent.setButton( Qt::LeftButton );
        mMouseHandles->mouseReleaseEvent( &forwardedEvent );
      }
      else
      {
        if ( mCanceled )
        {
          mCanceled = false;
          mDragging = false;
          return;
        }

        if ( mDragging )
        {
          mDragging = false;
          mSelectionRubberBand.reset();
        }

        bool selectedItemsHasChanged = false;
        if ( !( event->modifiers() & Qt::ShiftModifier ) )
        {
          selectedItemsHasChanged = true;
          mSelectedItems.clear();
          for ( const QObjectUniquePtr<QgsExtraItemRubberBand> &extraItem : mExtraItems )
          {
            extraItem->setSelected( false );
          }
        }

        for ( const QObjectUniquePtr<QgsExtraItemRubberBand> &extraItem : mExtraItems )
        {
          QRectF rect = extraItem->boundingRect();
          rect.translate( extraItem->pos().x(), extraItem->pos().y() );
          const bool isSelected = rect.intersects( mSelectionRect );
          selectedItemsHasChanged |= ( isSelected != extraItem->isSelected() );
          extraItem->setSelected( isSelected );

          if ( isSelected )
          {
            mSelectedItems << extraItem;
          }
        }

        if ( selectedItemsHasChanged )
          emit selectedItemsChanged();

        mMouseHandles->setSelected( !mSelectedItems.empty() );
      }
      break;
  }
}

void QgsMapToolModifyExtraItems::keyPressEvent( QKeyEvent *event )
{
  switch ( mState )
  {
    case State::SelectFeature:
      break;

    case State::FeatureSelected:
      if ( event->matches( QKeySequence::StandardKey::Cancel ) )
      {
        if ( !mSelectedItems.empty() )
        {
          mSelectedItems.clear();
          mMouseHandles->updateHandles();
          event->ignore();
          return;
        }
        else
        {
          mFeatureId = FID_NULL;
          mState = State::SelectFeature;
          mExtraItems.clear();
          event->ignore();
          return;
        }
      }

      if ( event->key() == Qt::Key_V )
      {
        const QPointF delta = mLastPos - mCopiedItemsTopLeft;
        mSelectedItems.clear();
        for ( const std::tuple<double, double, double> &extraItem : std::as_const( mCopiedItems ) )
        {
          QgsExtraItemRubberBand *newRubberBand = new QgsExtraItemRubberBand( QgsPointXY( std::get<0>( extraItem ), std::get<1>( extraItem ) ), std::get<2>( extraItem ), mCanvas, mLayer, mSymbolLayer );
          newRubberBand->setSelected( true );
          attemptMoveBy( newRubberBand, delta.x(), delta.y() );
          mSelectedItems << newRubberBand;
          mExtraItems.push_back( newRubberBand );
        }
        updateAttribute();
        mMouseHandles->updateHandles();
        event->ignore();
        return;
      }

      if ( mSelectedItems.empty() )
        return;

      if ( event->key() == Qt::Key_C || event->key() == Qt::Key_X )
      {
        mCopiedItems.clear();
        for ( QGraphicsItem *extraItem : std::as_const( mSelectedItems ) )
        {
          QgsExtraItemRubberBand *rubberBand = qgis::down_cast<QgsExtraItemRubberBand *>( extraItem );
          mCopiedItems.push_back( { rubberBand->position().x(), rubberBand->position().y(), rubberBand->rotation() } );
        }
        mCopiedItemsTopLeft = mMouseHandles->sceneBoundingRect().topLeft();
        event->ignore();
      }
      else if ( event->key() == Qt::Key_Left
                || event->key() == Qt::Key_Right
                || event->key() == Qt::Key_Up
                || event->key() == Qt::Key_Down )
      {
        const int pixels = ( event->modifiers() & Qt::ShiftModifier ) ? 1 : 50;
        int deltaX = 0;
        int deltaY = 0;
        if ( event->key() == Qt::Key_Up )
        {
          deltaY = -pixels;
        }
        else if ( event->key() == Qt::Key_Down )
        {
          deltaY = pixels;
        }
        else if ( event->key() == Qt::Key_Left )
        {
          deltaX = -pixels;
        }
        else if ( event->key() == Qt::Key_Right )
        {
          deltaX = pixels;
        }

        for ( QGraphicsItem *extraItem : mSelectedItems )
        {
          attemptMoveBy( extraItem, deltaX, deltaY );
        }
        mMouseHandles->updateHandles();

        event->ignore();
      }

      if ( event->matches( QKeySequence::StandardKey::Delete ) || event->key() == Qt::Key_X )
      {
        for ( QGraphicsItem *extraItem : mSelectedItems )
        {
          auto it = std::find( mExtraItems.cbegin(), mExtraItems.cend(), extraItem );
          mExtraItems.erase( it );
        }

        mSelectedItems.clear();
        updateAttribute();
        mMouseHandles->updateHandles();
        event->ignore();
      }
  }
}

void QgsMapToolModifyExtraItems::attemptMoveBy( QGraphicsItem *item, double deltaX, double deltaY )
{
  QgsExtraItemRubberBand *rubberBand = qgis::down_cast<QgsExtraItemRubberBand *>( item );
  rubberBand->attemptMoveBy( deltaX, deltaY );
  updateAttribute();
}

void QgsMapToolModifyExtraItems::attemptRotateBy( QGraphicsItem *item, double deltaDegree, double deltaCenterX, double deltaCenterY )
{
  QgsExtraItemRubberBand *rubberBand = qgis::down_cast<QgsExtraItemRubberBand *>( item );
  rubberBand->attemptMoveBy( deltaCenterX, deltaCenterY );
  rubberBand->attemptRotateBy( deltaDegree );
  updateAttribute();
}

void QgsMapToolModifyExtraItems::onMapCanvasExtentsChanged()
{
  for ( const QObjectUniquePtr<QgsExtraItemRubberBand> &extraItem : mExtraItems )
  {
    extraItem->update();
  }

  if ( mMouseHandles )
    mMouseHandles->updateHandles();
}
