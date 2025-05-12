/***************************************************************************
                             qgsmodelgraphicitem.cpp
                             ----------------------------------
    Date                 : February 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodelgraphicitem.h"
#include "qgsmodelcomponentgraphicitem.h"
#include "moc_qgsmodelgraphicitem.cpp"
#include "qgsapplication.h"
#include "qgsmodelgraphicsscene.h"
#include "qgsmodelgraphicsview.h"
#include "qgsmodelviewtool.h"
#include "qgsmodelviewmouseevent.h"
#include "qgsprocessingmodelcomponent.h"
#include "qgsprocessingoutputs.h"
#include "qgsprocessingparameters.h"
#include "qgsprocessingmodelchildalgorithm.h"
#include "qgsprocessingalgorithm.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QSvgRenderer>

///@cond NOT_STABLE

QgsModelDesignerFlatButtonGraphicItem::QgsModelDesignerFlatButtonGraphicItem( QGraphicsItem *parent, const QPicture &picture, const QPointF &position, const QSizeF &size )
  : QGraphicsObject( parent )
  , mPicture( picture )
  , mPosition( position )
  , mSize( size )
{
  setAcceptHoverEvents( true );
  setFlag( QGraphicsItem::ItemIsMovable, false );
  setCacheMode( QGraphicsItem::DeviceCoordinateCache );
}

void QgsModelDesignerFlatButtonGraphicItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *, QWidget * )
{
  if ( QgsModelGraphicsScene *modelScene = qobject_cast<QgsModelGraphicsScene *>( scene() ) )
  {
    if ( modelScene->flags() & QgsModelGraphicsScene::FlagHideControls )
      return;
  }

  if ( mHoverState )
  {
    painter->setPen( QPen( Qt::transparent, 1.0 ) );
    painter->setBrush( QBrush( QColor( 55, 55, 55, 33 ), Qt::SolidPattern ) );
  }
  else
  {
    painter->setPen( QPen( Qt::transparent, 1.0 ) );
    painter->setBrush( QBrush( Qt::transparent, Qt::SolidPattern ) );
  }
  const QPointF topLeft = mPosition - QPointF( std::floor( mSize.width() / 2 ), std::floor( mSize.height() / 2 ) );
  const QRectF rect = QRectF( topLeft.x(), topLeft.y(), mSize.width(), mSize.height() );
  painter->drawRect( rect );
  painter->drawPicture( topLeft.x(), topLeft.y(), mPicture );
}

QRectF QgsModelDesignerFlatButtonGraphicItem::boundingRect() const
{
  return QRectF( mPosition.x() - std::floor( mSize.width() / 2 ), mPosition.y() - std::floor( mSize.height() / 2 ), mSize.width(), mSize.height() );
}

void QgsModelDesignerFlatButtonGraphicItem::hoverEnterEvent( QGraphicsSceneHoverEvent * )
{
  if ( view()->tool() && !view()->tool()->allowItemInteraction() )
    mHoverState = false;
  else
    mHoverState = true;
  update();
}

void QgsModelDesignerFlatButtonGraphicItem::hoverLeaveEvent( QGraphicsSceneHoverEvent * )
{
  mHoverState = false;
  update();
}

void QgsModelDesignerFlatButtonGraphicItem::mousePressEvent( QGraphicsSceneMouseEvent * )
{
  if ( view()->tool() && view()->tool()->allowItemInteraction() )
    emit clicked();
}

void QgsModelDesignerFlatButtonGraphicItem::modelHoverEnterEvent( QgsModelViewMouseEvent * )
{
  if ( view()->tool() && !view()->tool()->allowItemInteraction() )
    mHoverState = false;
  else
    mHoverState = true;
  update();
}

void QgsModelDesignerFlatButtonGraphicItem::modelHoverLeaveEvent( QgsModelViewMouseEvent * )
{
  mHoverState = false;
  update();
}

void QgsModelDesignerFlatButtonGraphicItem::modelPressEvent( QgsModelViewMouseEvent *event )
{
  if ( view()->tool() && view()->tool()->allowItemInteraction() && event->button() == Qt::LeftButton )
  {
    QMetaObject::invokeMethod( this, "clicked", Qt::QueuedConnection );
    mHoverState = false;
    update();
  }
}

void QgsModelDesignerFlatButtonGraphicItem::setPosition( const QPointF &position )
{
  mPosition = position;
  prepareGeometryChange();
  update();
}

QgsModelGraphicsView *QgsModelDesignerFlatButtonGraphicItem::view()
{
  return qobject_cast<QgsModelGraphicsView *>( scene()->views().first() );
}

void QgsModelDesignerFlatButtonGraphicItem::setPicture( const QPicture &picture )
{
  mPicture = picture;
  update();
}

//
// QgsModelDesignerFoldButtonGraphicItem
//

QgsModelDesignerFoldButtonGraphicItem::QgsModelDesignerFoldButtonGraphicItem( QGraphicsItem *parent, bool folded, const QPointF &position, const QSizeF &size )
  : QgsModelDesignerFlatButtonGraphicItem( parent, QPicture(), position, size )
  , mFolded( folded )
{
  QSvgRenderer svg( QgsApplication::iconPath( QStringLiteral( "mIconModelerExpand.svg" ) ) );
  QPainter painter( &mPlusPicture );
  svg.render( &painter );
  painter.end();

  QSvgRenderer svg2( QgsApplication::iconPath( QStringLiteral( "mIconModelerCollapse.svg" ) ) );
  painter.begin( &mMinusPicture );
  svg2.render( &painter );
  painter.end();

  setPicture( mFolded ? mPlusPicture : mMinusPicture );
}

void QgsModelDesignerFoldButtonGraphicItem::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
  mFolded = !mFolded;
  setPicture( mFolded ? mPlusPicture : mMinusPicture );
  emit folded( mFolded );
  QgsModelDesignerFlatButtonGraphicItem::mousePressEvent( event );
}

void QgsModelDesignerFoldButtonGraphicItem::modelPressEvent( QgsModelViewMouseEvent *event )
{
  mFolded = !mFolded;
  setPicture( mFolded ? mPlusPicture : mMinusPicture );
  emit folded( mFolded );
  QgsModelDesignerFlatButtonGraphicItem::modelPressEvent( event );
}


QgsModelDesignerSocketGraphicItem::QgsModelDesignerSocketGraphicItem( QgsModelComponentGraphicItem *parent, QgsProcessingModelComponent *component, int index, const QPointF &position, Qt::Edge edge, const QSizeF &size )
  : QgsModelDesignerFlatButtonGraphicItem( parent, QPicture(), position, size )
  , mComponentItem( parent )
  , mComponent( component )
  , mIndex( index )
  , mEdge( edge )
{
}

void QgsModelDesignerSocketGraphicItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *, QWidget * )
{
  QColor outlineColor = getColor();
  QColor fillColor = QColor( outlineColor );
  fillColor.setAlpha( isDefaultParamValue() ? 30 : 255 );

  // Outline style
  painter->setPen( QPen( outlineColor, mHoverState ? mSocketOutlineWidth * 2 : mSocketOutlineWidth ) );

  // Fill style
  painter->setBrush( QBrush( fillColor, Qt::SolidPattern ) );

  painter->setRenderHint( QPainter::Antialiasing );

  constexpr float DISPLAY_SIZE = 4;
  painter->drawEllipse( position(), DISPLAY_SIZE, DISPLAY_SIZE );
  /* Uncomment to display bounding box */
#if 0
  painter->save();
  painter->setPen( QPen() );
  painter->setBrush( QBrush() );
  painter->drawRect( boundingRect() );
  painter->restore();
#endif
}


QColor QgsModelDesignerSocketGraphicItem::getColor()
{
  QString dataType;

  // Possibly, the mComponentItem is an instance of QgsModelParameterGraphicItem. In this case,
  // it needs to be explicitely casted so that the relevant getLinkedParamDataType method is being called
  if ( QgsModelParameterGraphicItem *paramItem = dynamic_cast<QgsModelParameterGraphicItem *>( componentItem() ) )
  {
    dataType = paramItem->getLinkedParamDataType( mEdge, mIndex );
  }
  else
  {
    dataType = componentItem()->getLinkedParamDataType( mEdge, mIndex );
  }

  return QgsModelDesignerSocketGraphicItem::typeToColorLookup( dataType );
}


QColor QgsModelDesignerSocketGraphicItem::typeToColorLookup( QString dataType )
{
  // Numerical types
  if (
    dataType == QgsProcessingParameterMatrix::typeName() || dataType == QgsProcessingParameterNumber::typeName() || dataType == QgsProcessingParameterRange::typeName() || dataType == QgsProcessingParameterColor::typeName() || dataType == QgsProcessingOutputNumber::typeName() || dataType == QgsProcessingParameterDistance::typeName() || dataType == QgsProcessingParameterDuration::typeName() || dataType == QgsProcessingParameterScale::typeName()

  )
  {
    return QColor( 34, 157, 214 ); // blue
  }
  else

    // Boolean type
    if (
      dataType == QgsProcessingParameterBoolean::typeName() || dataType == QgsProcessingOutputBoolean::typeName()
    )
    {
      return QColor( 51, 201, 28 ); // green
    }
    else

      // Vector types
      if (
        dataType == QgsProcessingParameterPoint::typeName() || dataType == QgsProcessingParameterGeometry::typeName() || dataType == QgsProcessingParameterVectorLayer::typeName() || dataType == QgsProcessingParameterMeshLayer::typeName() || dataType == QgsProcessingParameterPointCloudLayer::typeName() || dataType == QgsProcessingOutputVectorLayer::typeName() || dataType == QgsProcessingOutputPointCloudLayer::typeName() || dataType == QgsProcessingParameterExtent::typeName() || dataType == QgsProcessingOutputVectorTileLayer::typeName() || dataType == QgsProcessingParameterPointCloudDestination::typeName() || dataType == QgsProcessingParameterVectorTileDestination::typeName() || dataType == QgsProcessingParameterVectorDestination::typeName() || dataType == QgsProcessingParameterFeatureSource::typeName()
      )
      {
        return QColor( 180, 180, 0 ); // kaki (greenish yellow)
      }
      else

        // Raster type
        if (
          dataType == QgsProcessingParameterRasterLayer::typeName() || dataType == QgsProcessingOutputRasterLayer::typeName()

        )
        {
          return QColor( 0, 180, 180 ); // turquoise
        }
        else

          // enum
          if (
            dataType == QgsProcessingParameterEnum::typeName()
          )
          {
            return QColor( 128, 68, 201 ); // purple
          }
          else

            // String and datetime types
            if (
              dataType == QgsProcessingParameterString::typeName() || dataType == QgsProcessingParameterDateTime::typeName() || dataType == QgsProcessingParameterCrs::typeName() || dataType == QgsProcessingOutputHtml::typeName() || dataType == QgsProcessingOutputString::typeName()

            )
            {
              return QColor( 100, 100, 255 ); // slate blueish
            }
            else

              // filesystem types
              if (
                dataType == QgsProcessingParameterFile::typeName() || dataType == QgsProcessingOutputFolder::typeName() || dataType == QgsProcessingOutputFile::typeName() || dataType == QgsProcessingParameterFolderDestination::typeName() || dataType == QgsProcessingParameterFeatureSink::typeName() || dataType == QgsProcessingParameterRasterDestination::typeName() || dataType == QgsProcessingParameterFileDestination::typeName()
              )
              {
                return QColor( 80, 80, 80 ); // dark gray
              }
              else

                // Expression type
                if ( dataType == QgsProcessingParameterExpression::typeName() )
                {
                  return QColor( 180, 80, 180 ); // dark pink
                }
                else

                  // Other Layer types
                  if (
                    dataType == QgsProcessingParameterMultipleLayers::typeName() || dataType == QgsProcessingParameterMapLayer::typeName() || dataType == QgsProcessingParameterAnnotationLayer::typeName() || dataType == QgsProcessingOutputMultipleLayers::typeName()
                  )
                  {
                    return QColor( 128, 128, 0 ); // Dark kaki
                  }
                  else

                  // Default color, applies for:
                  // QgsProcessingParameterField
                  // QgsProcessingParameterMapTheme
                  // QgsProcessingParameterBand
                  // QgsProcessingParameterLayout
                  // QgsProcessingParameterLayoutItem
                  // QgsProcessingParameterCoordinateOperation
                  // QgsProcessingParameterAuthConfig // config
                  // QgsProcessingParameterDatabaseSchema
                  // QgsProcessingParameterDatabaseTable
                  // QgsProcessingParameterProviderConnection
                  // QgsProcessingParameterPointCloudAttribute
                  // QgsProcessingOutputVariant
                  // QgsProcessingOutputConditionalBranch
                  {
                    return QColor( 128, 128, 128 ); // mid gray
                  }
}


bool QgsModelDesignerSocketGraphicItem::isDefaultParamValue()
{
  if ( !mComponent )
  {
    return false;
  }

  const QgsProcessingModelChildAlgorithm *child = dynamic_cast<const QgsProcessingModelChildAlgorithm *>( mComponent );

  if ( !child )
  {
    return false;
  }

  bool isDefaultValue = true;

  // We can only know if the socket should be filled if the algorithm is non null
  if ( child->algorithm() )
  {
    switch ( mEdge )
    {
      // Input params
      case Qt::TopEdge:
      {
        QgsProcessingParameterDefinitions params = child->algorithm()->parameterDefinitions();

        if ( mIndex > ( params.length() - 1 ) )
        {
          break;
        }

        const QgsProcessingParameterDefinition *param = params.at( mIndex );
        QString name = param->name();

        QgsProcessingModelChildParameterSources paramSources = child->parameterSources().value( name );
        if ( paramSources.size() == 0 )
        {
          break;
        }

        // The default value can only happen in the case of the parameter uses a static value
        if ( paramSources[0].getSourceType() != Qgis::ProcessingModelChildParameterSource::StaticValue )
        {
          isDefaultValue = false;
          break;
        }

        isDefaultValue = paramSources[0].staticValue() == param->defaultValue();
        break;
      }

      // Ouputs
      case Qt::BottomEdge:
      {
        break;
      }
      case Qt::LeftEdge:
      case Qt::RightEdge:
        break;
    }
  }

  return isDefaultValue;
}

///@endcond
