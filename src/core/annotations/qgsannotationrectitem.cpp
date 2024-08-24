/***************************************************************************
    qgsannotationrectitem.cpp
    ----------------
    begin                : July 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotationrectitem.h"
#include "qgsapplication.h"
#include "qgsgeometry.h"
#include "qgsrendercontext.h"
#include "qgsannotationitemnode.h"
#include "qgsannotationitemeditoperation.h"
#include "qgspainting.h"
#include "qgsfillsymbol.h"
#include "qgssymbollayerutils.h"
#include "qgsfillsymbollayer.h"
#include "qgslinesymbollayer.h"
#include "qgscalloutsregistry.h"
#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgsunittypes.h"

QgsAnnotationRectItem::QgsAnnotationRectItem( const QgsRectangle &bounds )
  : QgsAnnotationItem()
  , mBounds( bounds )
{
  mBackgroundSymbol = std::make_unique< QgsFillSymbol >( QgsSymbolLayerList
  {
    new QgsSimpleFillSymbolLayer( QColor( 255, 255, 255 ), Qt::BrushStyle::SolidPattern, QColor( 0, 0, 0 ), Qt::PenStyle::NoPen )
  } );
  QgsSimpleLineSymbolLayer *borderSymbol = new QgsSimpleLineSymbolLayer( QColor( 0, 0, 0 ) );
  borderSymbol->setPenJoinStyle( Qt::MiterJoin );
  mFrameSymbol = std::make_unique< QgsFillSymbol >( QgsSymbolLayerList{ borderSymbol } );
}

QgsAnnotationRectItem::~QgsAnnotationRectItem() = default;

Qgis::AnnotationItemFlags QgsAnnotationRectItem::flags() const
{
  switch ( mPlacementMode )
  {
    case Qgis::AnnotationPlacementMode::SpatialBounds:
      return Qgis::AnnotationItemFlag::SupportsCallouts;
    case Qgis::AnnotationPlacementMode::FixedSize:
      return Qgis::AnnotationItemFlag::ScaleDependentBoundingBox
             | Qgis::AnnotationItemFlag::SupportsCallouts;
    case Qgis::AnnotationPlacementMode::RelativeToMapFrame:
      return Qgis::AnnotationItemFlag::ScaleDependentBoundingBox;
  }
  BUILTIN_UNREACHABLE
}

void QgsAnnotationRectItem::render( QgsRenderContext &context, QgsFeedback *feedback )
{
  QgsRectangle bounds = mBounds;
  if ( mPlacementMode != Qgis::AnnotationPlacementMode::RelativeToMapFrame && context.coordinateTransform().isValid() )
  {
    try
    {
      bounds = context.coordinateTransform().transformBoundingBox( mBounds );
    }
    catch ( QgsCsException & )
    {
      return;
    }
  }

  QRectF painterBounds;

  switch ( mPlacementMode )
  {
    case Qgis::AnnotationPlacementMode::SpatialBounds:
      painterBounds = context.mapToPixel().transformBounds( bounds.toRectF() );
      break;

    case Qgis::AnnotationPlacementMode::FixedSize:
    {
      const double widthPixels = context.convertToPainterUnits( mFixedSize.width(), mFixedSizeUnit );
      const double heightPixels = context.convertToPainterUnits( mFixedSize.height(), mFixedSizeUnit );

      if ( callout() && !calloutAnchor().isEmpty() )
      {
        QgsGeometry anchor = calloutAnchor();

        const double calloutOffsetWidthPixels = context.convertToPainterUnits( offsetFromCallout().width(), offsetFromCalloutUnit() );
        const double calloutOffsetHeightPixels = context.convertToPainterUnits( offsetFromCallout().height(), offsetFromCalloutUnit() );

        QPointF anchorPoint = anchor.asQPointF();
        if ( context.coordinateTransform().isValid() )
        {
          double x = anchorPoint.x();
          double y = anchorPoint.y();
          double z = 0.0;
          context.coordinateTransform().transformInPlace( x, y, z );
          anchorPoint = QPointF( x, y );
        }

        context.mapToPixel().transformInPlace( anchorPoint.rx(), anchorPoint.ry() );

        painterBounds = QRectF( anchorPoint.x() + calloutOffsetWidthPixels,
                                anchorPoint.y() + calloutOffsetHeightPixels, widthPixels, heightPixels );
      }
      else
      {
        QPointF center = bounds.center().toQPointF();

        context.mapToPixel().transformInPlace( center.rx(), center.ry() );
        painterBounds = QRectF( center.x() - widthPixels * 0.5,
                                center.y() - heightPixels * 0.5,
                                widthPixels, heightPixels );
      }
      break;
    }

    case Qgis::AnnotationPlacementMode::RelativeToMapFrame:
    {
      const double widthPixels = context.convertToPainterUnits( mFixedSize.width(), mFixedSizeUnit );
      const double heightPixels = context.convertToPainterUnits( mFixedSize.height(), mFixedSizeUnit );

      QPointF center = bounds.center().toQPointF();
      center.rx() *= context.outputSize().width();
      center.ry() *= context.outputSize().height();

      painterBounds = QRectF( center.x() - widthPixels * 0.5,
                              center.y() - heightPixels * 0.5,
                              widthPixels, heightPixels );
      break;
    }
  }

  if ( painterBounds.width() < 1 || painterBounds.height() < 1 )
    return;

  if ( mDrawBackground && mBackgroundSymbol )
  {
    mBackgroundSymbol->startRender( context );
    mBackgroundSymbol->renderPolygon( painterBounds, nullptr, nullptr, context );
    mBackgroundSymbol->stopRender( context );
  }

  if ( mPlacementMode != Qgis::AnnotationPlacementMode::RelativeToMapFrame && callout() )
  {
    QgsCallout::QgsCalloutContext calloutContext;
    renderCallout( context, painterBounds, 0, calloutContext, feedback );
  }

  renderInBounds( context, painterBounds, feedback );

  if ( mDrawFrame && mFrameSymbol )
  {
    mFrameSymbol->startRender( context );
    mFrameSymbol->renderPolygon( painterBounds, nullptr, nullptr, context );
    mFrameSymbol->stopRender( context );
  }
}

QList<QgsAnnotationItemNode> QgsAnnotationRectItem::nodesV2( const QgsAnnotationItemEditContext &context ) const
{
  QList<QgsAnnotationItemNode> res;
  switch ( mPlacementMode )
  {
    case Qgis::AnnotationPlacementMode::SpatialBounds:
    {
      res =
      {
        QgsAnnotationItemNode( QgsVertexId( 0, 0, 0 ), QgsPointXY( mBounds.xMinimum(), mBounds.yMinimum() ), Qgis::AnnotationItemNodeType::VertexHandle, Qt::CursorShape::SizeBDiagCursor ),
        QgsAnnotationItemNode( QgsVertexId( 0, 0, 1 ), QgsPointXY( mBounds.xMaximum(), mBounds.yMinimum() ), Qgis::AnnotationItemNodeType::VertexHandle, Qt::CursorShape::SizeFDiagCursor ),
        QgsAnnotationItemNode( QgsVertexId( 0, 0, 2 ), QgsPointXY( mBounds.xMaximum(), mBounds.yMaximum() ), Qgis::AnnotationItemNodeType::VertexHandle, Qt::CursorShape::SizeBDiagCursor ),
        QgsAnnotationItemNode( QgsVertexId( 0, 0, 3 ), QgsPointXY( mBounds.xMinimum(), mBounds.yMaximum() ), Qgis::AnnotationItemNodeType::VertexHandle, Qt::CursorShape::SizeFDiagCursor ),
      };

      QgsPointXY calloutNodePoint;
      if ( !calloutAnchor().isEmpty() )
      {
        calloutNodePoint = calloutAnchor().asPoint();
      }
      else
      {
        calloutNodePoint = mBounds.center();
      }
      res.append( QgsAnnotationItemNode( QgsVertexId( 1, 0, 0 ), calloutNodePoint, Qgis::AnnotationItemNodeType::CalloutHandle ) );

      return res;
    }

    case Qgis::AnnotationPlacementMode::FixedSize:
    {
      res =
      {
        QgsAnnotationItemNode( QgsVertexId( 0, 0, 0 ), mBounds.center(), Qgis::AnnotationItemNodeType::VertexHandle )
      };

      QgsPointXY calloutNodePoint;
      if ( !calloutAnchor().isEmpty() )
      {
        calloutNodePoint = calloutAnchor().asPoint();
      }
      else
      {
        calloutNodePoint = QgsPointXY( context.currentItemBounds().xMinimum(), context.currentItemBounds().yMinimum() );
      }
      res.append( QgsAnnotationItemNode( QgsVertexId( 1, 0, 0 ), calloutNodePoint, Qgis::AnnotationItemNodeType::CalloutHandle ) );

      return res;
    }

    case Qgis::AnnotationPlacementMode::RelativeToMapFrame:
    {
      return
      {
        QgsAnnotationItemNode( QgsVertexId( 0, 0, 0 ),
                               context.currentItemBounds().center(), Qgis::AnnotationItemNodeType::VertexHandle )
      };
    }
  }
  BUILTIN_UNREACHABLE
}

Qgis::AnnotationItemEditOperationResult QgsAnnotationRectItem::applyEditV2( QgsAbstractAnnotationItemEditOperation *operation, const QgsAnnotationItemEditContext &context )
{
  switch ( operation->type() )
  {
    case QgsAbstractAnnotationItemEditOperation::Type::MoveNode:
    {
      QgsAnnotationItemEditOperationMoveNode *moveOperation = dynamic_cast< QgsAnnotationItemEditOperationMoveNode * >( operation );
      if ( moveOperation->nodeId().part == 0 )
      {
        switch ( mPlacementMode )
        {
          case Qgis::AnnotationPlacementMode::SpatialBounds:
          {
            switch ( moveOperation->nodeId().vertex )
            {
              case 0:
                mBounds = QgsRectangle( moveOperation->after().x(),
                                        moveOperation->after().y(),
                                        mBounds.xMaximum(),
                                        mBounds.yMaximum() );
                break;
              case 1:
                mBounds = QgsRectangle( mBounds.xMinimum(),
                                        moveOperation->after().y(),
                                        moveOperation->after().x(),
                                        mBounds.yMaximum() );
                break;
              case 2:
                mBounds = QgsRectangle( mBounds.xMinimum(),
                                        mBounds.yMinimum(),
                                        moveOperation->after().x(),
                                        moveOperation->after().y() );
                break;
              case 3:
                mBounds = QgsRectangle( moveOperation->after().x(),
                                        mBounds.yMinimum(),
                                        mBounds.xMaximum(),
                                        moveOperation->after().y() );
                break;
              default:
                break;
            }
            return Qgis::AnnotationItemEditOperationResult::Success;
          }

          case Qgis::AnnotationPlacementMode::FixedSize:
          {
            mBounds = QgsRectangle::fromCenterAndSize( moveOperation->after(),
                      mBounds.width(),
                      mBounds.height() );
            return Qgis::AnnotationItemEditOperationResult::Success;
          }

          case Qgis::AnnotationPlacementMode::RelativeToMapFrame:
          {
            const double deltaX = moveOperation->translationXPixels() / context.renderContext().outputSize().width();
            const double deltaY = moveOperation->translationYPixels() / context.renderContext().outputSize().height();
            mBounds = QgsRectangle::fromCenterAndSize( QgsPointXY( mBounds.center().x() + deltaX, mBounds.center().y() + deltaY ),
                      mBounds.width(), mBounds.height() );
            return Qgis::AnnotationItemEditOperationResult::Success;
          }
        }
      }
      else if ( moveOperation->nodeId().part == 1 )
      {
        setCalloutAnchor( QgsGeometry::fromPoint( moveOperation->after() ) );
        if ( !callout() )
        {
          setCallout( QgsApplication::calloutRegistry()->defaultCallout() );
        }
        return Qgis::AnnotationItemEditOperationResult::Success;
      }
      break;
    }

    case QgsAbstractAnnotationItemEditOperation::Type::TranslateItem:
    {
      QgsAnnotationItemEditOperationTranslateItem *moveOperation = qgis::down_cast< QgsAnnotationItemEditOperationTranslateItem * >( operation );
      switch ( mPlacementMode )
      {

        case Qgis::AnnotationPlacementMode::SpatialBounds:
          mBounds = QgsRectangle( mBounds.xMinimum() + moveOperation->translationX(),
                                  mBounds.yMinimum() + moveOperation->translationY(),
                                  mBounds.xMaximum() + moveOperation->translationX(),
                                  mBounds.yMaximum() + moveOperation->translationY() );
          break;

        case Qgis::AnnotationPlacementMode::FixedSize:
        {
          if ( callout() && !calloutAnchor().isEmpty() )
          {
            const double xOffset = context.renderContext().convertFromPainterUnits( moveOperation->translationXPixels(), offsetFromCalloutUnit() );
            const double yOffset = context.renderContext().convertFromPainterUnits( moveOperation->translationYPixels(), offsetFromCalloutUnit() );
            setOffsetFromCallout( QSizeF( offsetFromCallout().width() + xOffset, offsetFromCallout().height() + yOffset ) );
          }
          else
          {
            mBounds = QgsRectangle( mBounds.xMinimum() + moveOperation->translationX(),
                                    mBounds.yMinimum() + moveOperation->translationY(),
                                    mBounds.xMaximum() + moveOperation->translationX(),
                                    mBounds.yMaximum() + moveOperation->translationY() );
          }
          break;
        }

        case Qgis::AnnotationPlacementMode::RelativeToMapFrame:
        {
          const double deltaX = moveOperation->translationXPixels() / context.renderContext().outputSize().width();
          const double deltaY = moveOperation->translationYPixels() / context.renderContext().outputSize().height();
          mBounds = QgsRectangle::fromCenterAndSize( QgsPointXY( mBounds.center().x() + deltaX, mBounds.center().y() + deltaY ),
                    mBounds.width(), mBounds.height() );
          break;
        }
      }
      return Qgis::AnnotationItemEditOperationResult::Success;
    }

    case QgsAbstractAnnotationItemEditOperation::Type::DeleteNode:
    case QgsAbstractAnnotationItemEditOperation::Type::AddNode:
      break;
  }
  return Qgis::AnnotationItemEditOperationResult::Invalid;
}

QgsAnnotationItemEditOperationTransientResults *QgsAnnotationRectItem::transientEditResultsV2( QgsAbstractAnnotationItemEditOperation *operation, const QgsAnnotationItemEditContext &context )
{
  switch ( operation->type() )
  {
    case QgsAbstractAnnotationItemEditOperation::Type::MoveNode:
    {
      QgsAnnotationItemEditOperationMoveNode *moveOperation = dynamic_cast< QgsAnnotationItemEditOperationMoveNode * >( operation );
      if ( moveOperation->nodeId().part == 0 )
      {
        switch ( mPlacementMode )
        {
          case Qgis::AnnotationPlacementMode::SpatialBounds:
          {
            QgsRectangle modifiedBounds = mBounds;
            switch ( moveOperation->nodeId().vertex )
            {
              case 0:
                modifiedBounds.setXMinimum( moveOperation->after().x() );
                modifiedBounds.setYMinimum( moveOperation->after().y() );
                break;
              case 1:
                modifiedBounds.setXMaximum( moveOperation->after().x() );
                modifiedBounds.setYMinimum( moveOperation->after().y() );
                break;
              case 2:
                modifiedBounds.setXMaximum( moveOperation->after().x() );
                modifiedBounds.setYMaximum( moveOperation->after().y() );
                break;
              case 3:
                modifiedBounds.setXMinimum( moveOperation->after().x() );
                modifiedBounds.setYMaximum( moveOperation->after().y() );
                break;
              default:
                break;
            }
            return new QgsAnnotationItemEditOperationTransientResults( QgsGeometry::fromRect( modifiedBounds ) );
          }
          case Qgis::AnnotationPlacementMode::FixedSize:
          {
            const QgsRectangle currentBounds = context.currentItemBounds();
            const QgsRectangle newBounds = QgsRectangle::fromCenterAndSize( moveOperation->after(), currentBounds.width(), currentBounds.height() );
            return new QgsAnnotationItemEditOperationTransientResults( QgsGeometry::fromRect( newBounds ) );
          }
          case Qgis::AnnotationPlacementMode::RelativeToMapFrame:
          {
            const QgsRectangle currentBounds = context.currentItemBounds();
            const QgsRectangle newBounds = QgsRectangle::fromCenterAndSize( currentBounds.center() + ( moveOperation->after() - moveOperation->before() ),
                                           currentBounds.width(), currentBounds.height() );
            return new QgsAnnotationItemEditOperationTransientResults( QgsGeometry::fromRect( newBounds ) );
          }
        }
      }
      else
      {
        QgsAnnotationItemEditOperationMoveNode *moveOperation = dynamic_cast< QgsAnnotationItemEditOperationMoveNode * >( operation );
        return new QgsAnnotationItemEditOperationTransientResults( QgsGeometry( moveOperation->after().clone() ) );
      }
      break;
    }

    case QgsAbstractAnnotationItemEditOperation::Type::TranslateItem:
    {
      QgsAnnotationItemEditOperationTranslateItem *moveOperation = qgis::down_cast< QgsAnnotationItemEditOperationTranslateItem * >( operation );
      switch ( mPlacementMode )
      {
        case Qgis::AnnotationPlacementMode::SpatialBounds:
        {
          const QgsRectangle modifiedBounds( mBounds.xMinimum() + moveOperation->translationX(),
                                             mBounds.yMinimum() + moveOperation->translationY(),
                                             mBounds.xMaximum() + moveOperation->translationX(),
                                             mBounds.yMaximum() + moveOperation->translationY() );
          return new QgsAnnotationItemEditOperationTransientResults( QgsGeometry::fromRect( modifiedBounds ) );
        }

        case Qgis::AnnotationPlacementMode::FixedSize:
        {
          if ( callout() && !calloutAnchor().isEmpty() )
          {
            QgsGeometry anchor = calloutAnchor();

            const double calloutOffsetWidthPixels = context.renderContext().convertToPainterUnits( offsetFromCallout().width(), offsetFromCalloutUnit() )
                                                    + moveOperation->translationXPixels();
            const double calloutOffsetHeightPixels = context.renderContext().convertToPainterUnits( offsetFromCallout().height(), offsetFromCalloutUnit() )
                + moveOperation->translationYPixels();

            QPointF anchorPoint = anchor.asQPointF();
            if ( context.renderContext().coordinateTransform().isValid() )
            {
              double x = anchorPoint.x();
              double y = anchorPoint.y();
              double z = 0.0;
              context.renderContext().coordinateTransform().transformInPlace( x, y, z );
              anchorPoint = QPointF( x, y );
            }

            context.renderContext().mapToPixel().transformInPlace( anchorPoint.rx(), anchorPoint.ry() );

            const double textOriginXPixels = anchorPoint.x() + calloutOffsetWidthPixels;
            const double textOriginYPixels = anchorPoint.y() + calloutOffsetHeightPixels;

            const double widthPixels = context.renderContext().convertToPainterUnits( mFixedSize.width(), mFixedSizeUnit );
            const double heightPixels = context.renderContext().convertToPainterUnits( mFixedSize.height(), mFixedSizeUnit );

            QgsLineString ls( QVector<QgsPointXY> { QgsPointXY( textOriginXPixels, textOriginYPixels ),
                                                    QgsPointXY( textOriginXPixels + widthPixels, textOriginYPixels ),
                                                    QgsPointXY( textOriginXPixels + widthPixels, textOriginYPixels + heightPixels ),
                                                    QgsPointXY( textOriginXPixels, textOriginYPixels + heightPixels ),
                                                    QgsPointXY( textOriginXPixels, textOriginYPixels )
                                                  } );

            QgsGeometry g( new QgsPolygon( ls.clone() ) );
            g.transform( context.renderContext().mapToPixel().transform().inverted() );
            g.transform( context.renderContext().coordinateTransform(), Qgis::TransformDirection::Reverse );
            return new QgsAnnotationItemEditOperationTransientResults( g );
          }
          else
          {
            const QgsRectangle currentBounds = context.currentItemBounds();
            const QgsRectangle newBounds = QgsRectangle::fromCenterAndSize( mBounds.center() + QgsVector( moveOperation->translationX(), moveOperation->translationY() ),
                                           currentBounds.width(), currentBounds.height() );
            return new QgsAnnotationItemEditOperationTransientResults( QgsGeometry::fromRect( newBounds ) );
          }
        }

        case Qgis::AnnotationPlacementMode::RelativeToMapFrame:
        {
          const QgsRectangle currentBounds = context.currentItemBounds();
          const QgsRectangle newBounds = QgsRectangle::fromCenterAndSize( currentBounds.center() + QgsVector( moveOperation->translationX(), moveOperation->translationY() ),
                                         currentBounds.width(), currentBounds.height() );
          return new QgsAnnotationItemEditOperationTransientResults( QgsGeometry::fromRect( newBounds ) );
        }
      }
      break;
    }

    case QgsAbstractAnnotationItemEditOperation::Type::DeleteNode:
    case QgsAbstractAnnotationItemEditOperation::Type::AddNode:
      break;
  }
  return nullptr;
}

QgsRectangle QgsAnnotationRectItem::boundingBox() const
{
  QgsRectangle bounds;
  switch ( mPlacementMode )
  {
    case Qgis::AnnotationPlacementMode::SpatialBounds:
    {
      bounds = mBounds;
      if ( callout() && !calloutAnchor().isEmpty() )
      {
        QgsGeometry anchor = calloutAnchor();
        bounds.combineExtentWith( anchor.boundingBox() );
      }
      break;
    }

    case Qgis::AnnotationPlacementMode::FixedSize:
      if ( callout() && !calloutAnchor().isEmpty() )
      {
        bounds = calloutAnchor().boundingBox();
      }
      else
      {
        bounds = QgsRectangle( mBounds.center(), mBounds.center() );
      }
      break;

    case Qgis::AnnotationPlacementMode::RelativeToMapFrame:
      bounds = mBounds;
      break;
  }

  return bounds;
}

QgsRectangle QgsAnnotationRectItem::boundingBox( QgsRenderContext &context ) const
{
  switch ( mPlacementMode )
  {
    case Qgis::AnnotationPlacementMode::SpatialBounds:
      return QgsAnnotationRectItem::boundingBox();

    case Qgis::AnnotationPlacementMode::FixedSize:
    {
      const double widthPixels = context.convertToPainterUnits( mFixedSize.width(), mFixedSizeUnit );
      const double heightPixels = context.convertToPainterUnits( mFixedSize.height(), mFixedSizeUnit );

      QRectF boundsInPixels;
      if ( callout() && !calloutAnchor().isEmpty() )
      {
        QgsGeometry anchor = calloutAnchor();

        const double calloutOffsetWidthPixels = context.convertToPainterUnits( offsetFromCallout().width(), offsetFromCalloutUnit() );
        const double calloutOffsetHeightPixels = context.convertToPainterUnits( offsetFromCallout().height(), offsetFromCalloutUnit() );

        QPointF anchorPoint = anchor.asQPointF();
        if ( context.coordinateTransform().isValid() )
        {
          double x = anchorPoint.x();
          double y = anchorPoint.y();
          double z = 0.0;
          context.coordinateTransform().transformInPlace( x, y, z );
          anchorPoint = QPointF( x, y );
        }

        context.mapToPixel().transformInPlace( anchorPoint.rx(), anchorPoint.ry() );

        QgsRectangle textRect( anchorPoint.x() + calloutOffsetWidthPixels,
                               anchorPoint.y() + calloutOffsetHeightPixels,
                               anchorPoint.x() + calloutOffsetWidthPixels + widthPixels,
                               anchorPoint.y() + calloutOffsetHeightPixels + heightPixels );
        QgsRectangle anchorRect( anchorPoint.x(), anchorPoint.y(), anchorPoint.x(), anchorPoint.y() );
        anchorRect.combineExtentWith( textRect );

        boundsInPixels = anchorRect.toRectF();
      }
      else
      {
        QPointF center = mBounds.center().toQPointF();
        if ( context.coordinateTransform().isValid() )
        {
          double x = center.x();
          double y = center.y();
          double z = 0.0;
          context.coordinateTransform().transformInPlace( x, y, z );
          center = QPointF( x, y );
        }

        context.mapToPixel().transformInPlace( center.rx(), center.ry() );
        boundsInPixels = QRectF( center.x() - widthPixels * 0.5,
                                 center.y() - heightPixels * 0.5,
                                 widthPixels, heightPixels );
      }
      const QgsPointXY topLeft = context.mapToPixel().toMapCoordinates( boundsInPixels.left(), boundsInPixels.top() );
      const QgsPointXY topRight = context.mapToPixel().toMapCoordinates( boundsInPixels.right(), boundsInPixels.top() );
      const QgsPointXY bottomLeft = context.mapToPixel().toMapCoordinates( boundsInPixels.left(), boundsInPixels.bottom() );
      const QgsPointXY bottomRight = context.mapToPixel().toMapCoordinates( boundsInPixels.right(), boundsInPixels.bottom() );

      const QgsRectangle boundsMapUnits = QgsRectangle( topLeft.x(), bottomLeft.y(), bottomRight.x(), topRight.y() );
      QgsRectangle textRect = context.coordinateTransform().transformBoundingBox( boundsMapUnits, Qgis::TransformDirection::Reverse );
      return textRect;
    }

    case Qgis::AnnotationPlacementMode::RelativeToMapFrame:
    {
      const double widthPixels = context.convertToPainterUnits( mFixedSize.width(), mFixedSizeUnit );
      const double heightPixels = context.convertToPainterUnits( mFixedSize.height(), mFixedSizeUnit );

      QRectF boundsInPixels;

      const double centerMapX = context.mapExtent().xMinimum() + mBounds.center().x() * context.mapExtent().width();
      const double centerMapY = context.mapExtent().yMaximum() - mBounds.center().y() * context.mapExtent().height();
      QPointF center( centerMapX, centerMapY );
      if ( context.coordinateTransform().isValid() )
      {
        double x = centerMapX;
        double y = centerMapY;
        double z = 0.0;
        context.coordinateTransform().transformInPlace( x, y, z );
        center = QPointF( x, y );
      }

      context.mapToPixel().transformInPlace( center.rx(), center.ry() );
      boundsInPixels = QRectF( center.x() - widthPixels * 0.5,
                               center.y() - heightPixels * 0.5,
                               widthPixels, heightPixels );

      const QgsPointXY topLeft = context.mapToPixel().toMapCoordinates( boundsInPixels.left(), boundsInPixels.top() );
      const QgsPointXY topRight = context.mapToPixel().toMapCoordinates( boundsInPixels.right(), boundsInPixels.top() );
      const QgsPointXY bottomLeft = context.mapToPixel().toMapCoordinates( boundsInPixels.left(), boundsInPixels.bottom() );
      const QgsPointXY bottomRight = context.mapToPixel().toMapCoordinates( boundsInPixels.right(), boundsInPixels.bottom() );

      const QgsRectangle boundsMapUnits = QgsRectangle( topLeft.x(), bottomLeft.y(), bottomRight.x(), topRight.y() );
      QgsRectangle textRect = context.coordinateTransform().transformBoundingBox( boundsMapUnits, Qgis::TransformDirection::Reverse );
      return textRect;
    }
  }
  BUILTIN_UNREACHABLE
}

void QgsAnnotationRectItem::setBounds( const QgsRectangle &bounds )
{
  mBounds = bounds;
}

const QgsFillSymbol *QgsAnnotationRectItem::backgroundSymbol() const
{
  return mBackgroundSymbol.get();
}

void QgsAnnotationRectItem::setBackgroundSymbol( QgsFillSymbol *symbol )
{
  mBackgroundSymbol.reset( symbol );
}

const QgsFillSymbol *QgsAnnotationRectItem::frameSymbol() const
{
  return mFrameSymbol.get();
}

void QgsAnnotationRectItem::setFrameSymbol( QgsFillSymbol *symbol )
{
  mFrameSymbol.reset( symbol );
}

void QgsAnnotationRectItem::copyCommonProperties( const QgsAnnotationItem *other )
{
  if ( const QgsAnnotationRectItem *otherRect = dynamic_cast< const QgsAnnotationRectItem * >( other ) )
  {
    setPlacementMode( otherRect->mPlacementMode );
    setFixedSize( otherRect->mFixedSize );
    setFixedSizeUnit( otherRect->mFixedSizeUnit );

    setBackgroundEnabled( otherRect->mDrawBackground );
    if ( otherRect->mBackgroundSymbol )
      setBackgroundSymbol( otherRect->mBackgroundSymbol->clone() );

    setFrameEnabled( otherRect->mDrawFrame );
    if ( otherRect->mFrameSymbol )
      setFrameSymbol( otherRect->mFrameSymbol->clone() );
  }

  QgsAnnotationItem::copyCommonProperties( other );
}

bool QgsAnnotationRectItem::writeCommonProperties( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "xMin" ), qgsDoubleToString( mBounds.xMinimum() ) );
  element.setAttribute( QStringLiteral( "xMax" ), qgsDoubleToString( mBounds.xMaximum() ) );
  element.setAttribute( QStringLiteral( "yMin" ), qgsDoubleToString( mBounds.yMinimum() ) );
  element.setAttribute( QStringLiteral( "yMax" ), qgsDoubleToString( mBounds.yMaximum() ) );
  element.setAttribute( QStringLiteral( "sizeMode" ), qgsEnumValueToKey( mPlacementMode ) );
  element.setAttribute( QStringLiteral( "fixedWidth" ), qgsDoubleToString( mFixedSize.width() ) );
  element.setAttribute( QStringLiteral( "fixedHeight" ), qgsDoubleToString( mFixedSize.height() ) );
  element.setAttribute( QStringLiteral( "fixedSizeUnit" ), QgsUnitTypes::encodeUnit( mFixedSizeUnit ) );

  element.setAttribute( QStringLiteral( "backgroundEnabled" ), mDrawBackground ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  if ( mBackgroundSymbol )
  {
    QDomElement backgroundElement = document.createElement( QStringLiteral( "backgroundSymbol" ) );
    backgroundElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "backgroundSymbol" ), mBackgroundSymbol.get(), document, context ) );
    element.appendChild( backgroundElement );
  }

  element.setAttribute( QStringLiteral( "frameEnabled" ), mDrawFrame ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  if ( mFrameSymbol )
  {
    QDomElement frameElement = document.createElement( QStringLiteral( "frameSymbol" ) );
    frameElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "frameSymbol" ), mFrameSymbol.get(), document, context ) );
    element.appendChild( frameElement );
  }

  return QgsAnnotationItem::writeCommonProperties( element, document, context );
}

bool QgsAnnotationRectItem::readCommonProperties( const QDomElement &element, const QgsReadWriteContext &context )
{
  mBounds.setXMinimum( element.attribute( QStringLiteral( "xMin" ) ).toDouble() );
  mBounds.setXMaximum( element.attribute( QStringLiteral( "xMax" ) ).toDouble() );
  mBounds.setYMinimum( element.attribute( QStringLiteral( "yMin" ) ).toDouble() );
  mBounds.setYMaximum( element.attribute( QStringLiteral( "yMax" ) ).toDouble() );

  mPlacementMode = qgsEnumKeyToValue( element.attribute( QStringLiteral( "sizeMode" ) ), Qgis::AnnotationPlacementMode::SpatialBounds );

  mFixedSize = QSizeF(
                 element.attribute( QStringLiteral( "fixedWidth" ) ).toDouble(),
                 element.attribute( QStringLiteral( "fixedHeight" ) ).toDouble()
               );
  mFixedSizeUnit = QgsUnitTypes::decodeRenderUnit( element.attribute( QStringLiteral( "fixedSizeUnit" ) ) );

  mDrawBackground = element.attribute( QStringLiteral( "backgroundEnabled" ), QStringLiteral( "0" ) ).toInt();
  const QDomElement backgroundSymbolElem = element.firstChildElement( QStringLiteral( "backgroundSymbol" ) ).firstChildElement();
  if ( !backgroundSymbolElem.isNull() )
  {
    setBackgroundSymbol( QgsSymbolLayerUtils::loadSymbol< QgsFillSymbol >( backgroundSymbolElem, context ) );
  }

  mDrawFrame = element.attribute( QStringLiteral( "frameEnabled" ), QStringLiteral( "0" ) ).toInt();
  const QDomElement frameSymbolElem = element.firstChildElement( QStringLiteral( "frameSymbol" ) ).firstChildElement();
  if ( !frameSymbolElem.isNull() )
  {
    setFrameSymbol( QgsSymbolLayerUtils::loadSymbol< QgsFillSymbol >( frameSymbolElem, context ) );
  }

  return QgsAnnotationItem::readCommonProperties( element, context );
}

QSizeF QgsAnnotationRectItem::fixedSize() const
{
  return mFixedSize;
}

void QgsAnnotationRectItem::setFixedSize( const QSizeF &size )
{
  mFixedSize = size;
}

Qgis::RenderUnit QgsAnnotationRectItem::fixedSizeUnit() const
{
  return mFixedSizeUnit;
}

void QgsAnnotationRectItem::setFixedSizeUnit( Qgis::RenderUnit unit )
{
  mFixedSizeUnit = unit;
}

Qgis::AnnotationPlacementMode QgsAnnotationRectItem::placementMode() const
{
  return mPlacementMode;
}

void QgsAnnotationRectItem::setPlacementMode( Qgis::AnnotationPlacementMode mode )
{
  mPlacementMode = mode;
}
