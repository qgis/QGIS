/***************************************************************************
                             qgsmodelviewtoollink.cpp
                             ------------------------------------
    Date                 : January 2024
    Copyright            : (C) 2024 Valentin Buira
    Email                : valentin dot buira at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodelviewtoollink.h"
#include "moc_qgsmodelviewtoollink.cpp"
#include "qgsprocessingmodelerparameterwidget.h"
#include "qgsprocessingmodelalgorithm.h"
#include "qgsgui.h"
#include "qgsprocessingguiregistry.h"
#include "qgsprocessingmodelchildalgorithm.h"
#include "qgsmodelgraphicsscene.h"
#include "qgsmodelviewmouseevent.h"
#include "qgsmodelgraphicsview.h"
#include <QScrollBar>

QgsModelViewToolLink::QgsModelViewToolLink( QgsModelGraphicsView *view )
  : QgsModelViewTool( view, tr( "Link Tool" ) )
{
  setCursor( Qt::PointingHandCursor );
  mBezierRubberBand.reset( new QgsModelViewBezierRubberBand( view ) );

  mBezierRubberBand->setBrush( QBrush( QColor( 0, 0, 0, 63 ) ) );
  mBezierRubberBand->setPen( QPen( QBrush( QColor( 0, 0, 0, 100 ) ), 0, Qt::SolidLine ) );
}

void QgsModelViewToolLink::modelMoveEvent( QgsModelViewMouseEvent *event )
{
  mBezierRubberBand->update( event->modelPoint(), Qt::KeyboardModifiers() );

  // we need to manually pass this event down to items we want it to go to -- QGraphicsScene doesn't propagate
  QList<QGraphicsItem *> items = scene()->items( event->modelPoint() );

  QgsModelDesignerSocketGraphicItem *socket = nullptr;
  for ( QGraphicsItem *item : items )
  {
    if ( ( socket = dynamic_cast<QgsModelDesignerSocketGraphicItem *>( item ) ) )
    {
      // snap
      if ( mFromSocket != socket && mFromSocket->edge() != socket->edge() )
      {
        socket->modelHoverEnterEvent( event );
        QPointF rubberEndPos = socket->mapToScene( socket->getPosition() );
        mBezierRubberBand->update( rubberEndPos, Qt::KeyboardModifiers() );

        break;
      }
    }
  }

  if ( socket == nullptr && mLastHoveredSocket != nullptr && socket != mLastHoveredSocket )
  {
    mLastHoveredSocket->modelHoverLeaveEvent( event );
    mLastHoveredSocket = nullptr;
  }
  else
  {
    mLastHoveredSocket = socket;
  }
}

void QgsModelViewToolLink::modelReleaseEvent( QgsModelViewMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
  {
    return;
  }
  view()->setTool( mPreviousViewTool );
  mBezierRubberBand->finish( event->modelPoint() );

  // we need to manually pass this event down to items we want it to go to -- QGraphicsScene doesn't propagate
  QList<QGraphicsItem *> items = scene()->items( event->modelPoint() );

  mToSocket = nullptr;

  for ( QGraphicsItem *item : items )
  {
    if ( QgsModelDesignerSocketGraphicItem *socket = dynamic_cast<QgsModelDesignerSocketGraphicItem *>( item ) )
    {
      mToSocket = socket;
    }
  }

  // Do nothing if cursor don't land on another socket
  if ( mToSocket == nullptr )
  {
    return;
  }

  // Do nothing if from socket and to socket are both input or both output
  if ( mFromSocket->edge() == mToSocket->edge() )
  {
    return;
  }

  emit view() -> beginCommand( "Edit link" );

  QList<QgsProcessingModelChildParameterSource> sources;

  QgsProcessingModelComponent *component_from;
  QgsProcessingModelChildAlgorithm *child_to;

  // ReOrder in out socket
  // always fix on the input end receiving
  if ( !mToSocket->isInput() )
  {
    std::swap( mFromSocket, mToSocket );
  }

  component_from = mFromSocket->component();
  child_to = dynamic_cast<QgsProcessingModelChildAlgorithm *>( mToSocket->component() );


  const QgsProcessingParameterDefinition *toParam = child_to->algorithm()->parameterDefinitions().at( mToSocket->index() );

  QgsProcessingModelChildParameterSource source;
  if ( QgsProcessingModelChildAlgorithm *child_from = dynamic_cast<QgsProcessingModelChildAlgorithm *>( component_from ) )
  {
    QString outputName = child_from->algorithm()->outputDefinitions().at( mFromSocket->index() )->name();
    source = QgsProcessingModelChildParameterSource::fromChildOutput( child_from->childId(), outputName );
  }
  else if ( QgsProcessingModelParameter *param_from = dynamic_cast<QgsProcessingModelParameter *>( component_from ) )
  {
    source = QgsProcessingModelChildParameterSource::fromModelParameter( param_from->parameterName() );
  }

  QgsProcessingContext context;
  QgsProcessingModelerParameterWidget *widget = QgsGui::processingGuiRegistry()->createModelerParameterWidget( view()->modelScene()->model(), child_to->childId(), toParam, context );


  QList<QgsProcessingModelChildParameterSource> compatible_param_type = widget->availableSourcesForChild();
  delete widget;

  if ( !compatible_param_type.contains( source ) )
  {
    //Type are incomatible
    QString title = "Impossible to connect socket";
    QString message = "Impossible to connect socket either type are incompatibles or there is a circular dependency";
    scene()->showWarning( message, title, message );
    return;
  }

  sources << source;
  child_to->addParameterSources( toParam->name(), sources );


  //We need to pass the update child algorithm to the model
  scene()->model()->setChildAlgorithm( *child_to );

  emit view() -> endCommand();
  // Redraw
  emit scene() -> rebuildRequired();
}

bool QgsModelViewToolLink::allowItemInteraction()
{
  return true;
}

void QgsModelViewToolLink::activate()
{
  mPreviousViewTool = view()->tool();

  QPointF rubberStartPos = mFromSocket->mapToScene( mFromSocket->getPosition() );
  mBezierRubberBand->start( rubberStartPos, Qt::KeyboardModifiers() );

  QgsModelViewTool::activate();
}

void QgsModelViewToolLink::deactivate()
{
  mBezierRubberBand->finish();
  QgsModelViewTool::deactivate();
}

void QgsModelViewToolLink::setFromSocket( QgsModelDesignerSocketGraphicItem *socket )
{
  mFromSocket = socket;

  if ( mFromSocket->isInput() )
  {
    QgsProcessingModelChildAlgorithm *child_from = dynamic_cast<QgsProcessingModelChildAlgorithm *>( mFromSocket->component() );
    const QgsProcessingParameterDefinition *param = child_from->algorithm()->parameterDefinitions().at( mFromSocket->index() );

    auto current_sources = child_from->parameterSources().value( param->name() );

    // we need to manually pass this event down to items we want it to go to -- QGraphicsScene doesn't propagate
    QList<QGraphicsItem *> items = scene()->items();
    QgsProcessingModelChildParameterSource old_source;
    for ( const QgsProcessingModelChildParameterSource &source : std::as_const( current_sources ) )
    {
      switch ( source.source() )
      {
        case Qgis::ProcessingModelChildParameterSource::ModelParameter:
        case Qgis::ProcessingModelChildParameterSource::ChildOutput:
        {
          old_source = source;
          QgsProcessingModelChildAlgorithm *_alg;
          // This is not so nice to have the UI tangled gotta think of a better abstraction later
          // Loop trought all items to get the output socket
          for ( QGraphicsItem *item : items )
          {
            if ( QgsModelDesignerSocketGraphicItem *output_socket = dynamic_cast<QgsModelDesignerSocketGraphicItem *>( item ) )
            {
              if ( ( _alg = dynamic_cast<QgsProcessingModelChildAlgorithm *>( output_socket->component() ) ) )
              {
                if ( source.outputChildId() != _alg->childId() || output_socket->isInput() )
                {
                  continue;
                }
                if ( output_socket->index() == _alg->algorithm()->outputDefinitionIndex( source.outputName() ) )
                {
                  mFromSocket = output_socket;
                  emit view() -> beginCommand( "Edit link" );
                }
              }
              else if ( QgsProcessingModelParameter *_param = dynamic_cast<QgsProcessingModelParameter *>( output_socket->component() ) )
              {
                if ( source.parameterName() == _param->parameterName() )
                {
                  mFromSocket = output_socket;
                  emit view() -> beginCommand( "Edit link" );
                }
              }
            }
          }

          //reset to default value
          QList<QgsProcessingModelChildParameterSource> new_sources;
          new_sources << QgsProcessingModelChildParameterSource::fromStaticValue( param->defaultValue() );


          child_from->addParameterSources( param->name(), new_sources );
          //We need to pass the update child algorithm to the model
          scene()->model()->setChildAlgorithm( *child_from );
          // Redraw
          emit scene() -> rebuildRequired();

          //Get Socket from Source alg / source parameter
          QgsModelComponentGraphicItem *item = nullptr;
          int socket_index = -1;
          if ( old_source.source() == Qgis::ProcessingModelChildParameterSource::ChildOutput )
          {
            item = scene()->childAlgorithmItem( old_source.outputChildId() );
            socket_index = _alg->algorithm()->outputDefinitionIndex( source.outputName() );
          }
          else if ( old_source.source() == Qgis::ProcessingModelChildParameterSource::ModelParameter )
          {
            item = scene()->parameterItem( source.parameterName() );
            socket_index = 0;
          }

          mFromSocket = item->outSocketAt( socket_index );
        }


        break;
        default:
          continue;
      }

      // Stop on first iteration to get only one link at a time
      break;
    }
  }
};
