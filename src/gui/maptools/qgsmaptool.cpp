/***************************************************************************
    qgsmaptool.cpp  -  base class for map canvas tools
    ----------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptool.h"

#include "qgsexpressionnodeimpl.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmapmouseevent.h"
#include "qgsmaptopixel.h"
#include "qgsrendercontext.h"
#include "qgssettings.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingstree.h"

#include <QAbstractButton>
#include <QAction>
#include <QString>

#include "moc_qgsmaptool.cpp"

using namespace Qt::StringLiterals;

const QgsSettingsEntryDouble *QgsMapTool::settingSearchRadiusMM
  = new QgsSettingsEntryDouble( u"search-radius-mm"_s, QgsSettingsTree::sTreeMap, Qgis::DEFAULT_SEARCH_RADIUS_MM, u"Search/identify radius in millimeters"_s );

QgsMapTool::QgsMapTool( QgsMapCanvas *canvas )
  : QObject( canvas )
  , mCanvas( canvas )
  , mCursor( Qt::CrossCursor )
{}


QgsMapTool::~QgsMapTool()
{
  if ( mCanvas )
    mCanvas->unsetMapTool( this );
}

QgsPointXY QgsMapTool::toMapCoordinates( QPoint point )
{
  return mCanvas->getCoordinateTransform()->toMapCoordinates( point );
}

QgsPoint QgsMapTool::toMapCoordinates( const QgsMapLayer *layer, const QgsPoint &point )
{
  return mCanvas->mapSettings().layerToMapCoordinates( layer, point );
}

QgsPointXY QgsMapTool::toLayerCoordinates( const QgsMapLayer *layer, QPoint point )
{
  const QgsPointXY pt = toMapCoordinates( point );
  return toLayerCoordinates( layer, pt );
}

QgsPointXY QgsMapTool::toLayerCoordinates( const QgsMapLayer *layer, const QgsPointXY &point )
{
  return mCanvas->mapSettings().mapToLayerCoordinates( layer, point );
}

QgsPoint QgsMapTool::toLayerCoordinates( const QgsMapLayer *layer, const QgsPoint &point )
{
  return mCanvas->mapSettings().mapToLayerCoordinates( layer, point );
}

QgsPointXY QgsMapTool::toMapCoordinates( const QgsMapLayer *layer, const QgsPointXY &point )
{
  return mCanvas->mapSettings().layerToMapCoordinates( layer, point );
}

QgsRectangle QgsMapTool::toLayerCoordinates( const QgsMapLayer *layer, const QgsRectangle &rect )
{
  return mCanvas->mapSettings().mapToLayerCoordinates( layer, rect );
}

QPoint QgsMapTool::toCanvasCoordinates( const QgsPointXY &point ) const
{
  qreal x = point.x(), y = point.y();
  mCanvas->getCoordinateTransform()->transformInPlace( x, y );
  return QPoint( std::round( x ), std::round( y ) );
}

QgsMapLayer *QgsMapTool::layer( const QString &id )
{
  return mCanvas->layer( id );
}

void QgsMapTool::setToolName( const QString &name )
{
  mToolName = name;
}

void QgsMapTool::activate()
{
  // make action and/or button active
  if ( mAction )
    mAction->setChecked( true );
  if ( mButton )
    mButton->setChecked( true );

  // set cursor (map tools usually set it in constructor)
  mCanvas->setCursor( mCursor );
  QgsDebugMsgLevel( u"Cursor has been set"_s, 4 );

  emit activated();
}


void QgsMapTool::deactivate()
{
  if ( mAction )
    mAction->setChecked( false );
  if ( mButton )
    mButton->setChecked( false );

  emit deactivated();
}


void QgsMapTool::reactivate()
{
  emit reactivated();
}

void QgsMapTool::clean()
{}

void QgsMapTool::setAction( QAction *action )
{
  if ( mAction )
    disconnect( mAction, &QObject::destroyed, this, &QgsMapTool::actionDestroyed );
  mAction = action;
  if ( mAction )
    connect( mAction, &QObject::destroyed, this, &QgsMapTool::actionDestroyed );
}

void QgsMapTool::actionDestroyed()
{
  if ( mAction == sender() )
    mAction = nullptr;
}

QAction *QgsMapTool::action()
{
  return mAction;
}

bool QgsMapTool::isActive() const
{
  return mCanvas && mCanvas->mapTool() == this;
}

void QgsMapTool::setButton( QAbstractButton *button )
{
  mButton = button;
}

QAbstractButton *QgsMapTool::button()
{
  return mButton;
}

void QgsMapTool::setCursor( const QCursor &cursor )
{
  mCursor = cursor;
  if ( isActive() )
    mCanvas->setCursor( mCursor );
}


void QgsMapTool::canvasMoveEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )
}

void QgsMapTool::canvasDoubleClickEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )
}

void QgsMapTool::canvasPressEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )
}

void QgsMapTool::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )
}

void QgsMapTool::wheelEvent( QWheelEvent *e )
{
  e->ignore();
}

void QgsMapTool::keyPressEvent( QKeyEvent *e )
{
  Q_UNUSED( e )
}

void QgsMapTool::keyReleaseEvent( QKeyEvent *e )
{
  Q_UNUSED( e )
}

bool QgsMapTool::gestureEvent( QGestureEvent *e )
{
  Q_UNUSED( e )
  return true;
}

bool QgsMapTool::canvasToolTipEvent( QHelpEvent *e )
{
  Q_UNUSED( e )
  return false;
}

bool QgsMapTool::shortcutEvent( QKeyEvent *e )
{
  Q_UNUSED( e )
  return false;
}

QgsMapCanvas *QgsMapTool::canvas() const
{
  return mCanvas;
}

double QgsMapTool::searchRadiusMM()
{
  const double radius = settingSearchRadiusMM->value();

  if ( radius > 0 )
  {
    return radius;
  }
  return Qgis::DEFAULT_SEARCH_RADIUS_MM;
}

double QgsMapTool::searchRadiusMU( const QgsRenderContext &context )
{
  return context.convertToMapUnits( searchRadiusMM(), Qgis::RenderUnit::Millimeters );
}

double QgsMapTool::searchRadiusMU( QgsMapCanvas *canvas )
{
  if ( !canvas )
  {
    return 0;
  }
  const QgsMapSettings mapSettings = canvas->mapSettings();
  const QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  return searchRadiusMU( context ) / mapSettings.magnificationFactor();
}


void QgsMapTool::populateContextMenu( QMenu * )
{}


bool QgsMapTool::populateContextMenuWithEvent( QMenu *, QgsMapMouseEvent * )
{
  return false;
}

QString QgsMapTool::dataDefinedColumnName( int propertyKey, const QgsPropertyCollection &properties, const QgsVectorLayer *layer, PropertyStatus &status ) const
{
  status = PropertyStatus::DoesNotExist;
  if ( !properties.isActive( propertyKey ) )
    return QString();

  const QgsProperty property = properties.property( propertyKey );

  switch ( property.propertyType() )
  {
    case Qgis::PropertyType::Invalid:
      break;

    case Qgis::PropertyType::Static:
      status = PropertyStatus::Valid;
      break;

    case Qgis::PropertyType::Field:
      status = PropertyStatus::Valid;
      return property.field();

    case Qgis::PropertyType::Expression:
    {
      status = PropertyStatus::Valid;

      // an expression based property may still be a effectively a single field reference in the map canvas context.
      // e.g. if it is a expression like '"some_field"', or 'case when @some_project_var = 'a' then "field_a" else "field_b" end'

      QgsExpressionContext context = mCanvas->createExpressionContext();
      context.appendScope( layer->createExpressionContextScope() );

      QgsExpression expression( property.expressionString() );
      if ( expression.prepare( &context ) )
      {
        // maybe the expression is effectively a single node in this context...
        const QgsExpressionNode *node = expression.rootNode()->effectiveNode();
        if ( node->nodeType() == QgsExpressionNode::ntColumnRef )
        {
          const QgsExpressionNodeColumnRef *columnRef = qgis::down_cast<const QgsExpressionNodeColumnRef *>( node );
          return columnRef->name();
        }

        // ok, it's not. But let's be super smart and helpful for users!
        // maybe it's a COALESCE("some field", 'some' || 'fallback' || 'expression') type expression, where the user wants to override
        // some labels with a value stored in a field but all others use some expression
        if ( node->nodeType() == QgsExpressionNode::ntFunction )
        {
          const QgsExpressionNodeFunction *functionNode = qgis::down_cast<const QgsExpressionNodeFunction *>( node );
          if ( const QgsExpressionFunction *function = QgsExpression::QgsExpression::Functions()[functionNode->fnIndex()] )
          {
            if ( function->name() == "coalesce"_L1 )
            {
              if ( const QgsExpressionNode *firstArg = functionNode->args()->list().value( 0 ) )
              {
                const QgsExpressionNode *firstArgNode = firstArg->effectiveNode();
                if ( firstArgNode->nodeType() == QgsExpressionNode::ntColumnRef )
                {
                  const QgsExpressionNodeColumnRef *columnRef = qgis::down_cast<const QgsExpressionNodeColumnRef *>( firstArgNode );
                  return columnRef->name();
                }
              }
            }
          }
        }
      }
      else
      {
        status = PropertyStatus::CurrentExpressionInvalid;
      }
      break;
    }
  }

  return QString();
}

int QgsMapTool::dataDefinedColumnIndex( int propertyKey, const QgsPropertyCollection &properties, const QgsVectorLayer *vlayer ) const
{
  PropertyStatus status = PropertyStatus::DoesNotExist;
  QString fieldname = dataDefinedColumnName( propertyKey, properties, vlayer, status );
  if ( !fieldname.isEmpty() )
    return vlayer->fields().lookupField( fieldname );
  return -1;
}
