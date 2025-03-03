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

  connect( this, &QgsModelViewToolLink::requestRebuildRequired, scene(), &QgsModelGraphicsScene::rebuildRequired );
}

void QgsModelViewToolLink::modelMoveEvent( QgsModelViewMouseEvent *event )
{
  mBezierRubberBand->update( event->modelPoint(), Qt::KeyboardModifiers() );

  // we need to manually pass this event down to items we want it to go to -- QGraphicsScene doesn't propagate
  QList<QGraphicsItem *> items = scene()->items( event->modelPoint() );

  QgsModelDesignerSocketGraphicItem *socket = nullptr;
  for ( QGraphicsItem *item : items )
  {
    if ( ( socket = dynamic_cast<QgsModelDesignerSocketGraphicItem *>( item ) )
         && ( mFromSocket != socket && mFromSocket->edge() != socket->edge() ) )
    {
      // snap
      socket->modelHoverEnterEvent( event );
      QPointF rubberEndPos = socket->mapToScene( socket->getPosition() );
      mBezierRubberBand->update( rubberEndPos, Qt::KeyboardModifiers() );

      break;
    }
  }

  if ( !socket && mLastHoveredSocket && socket != mLastHoveredSocket )
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
      break;
    }
  }

  // Do nothing if cursor didn't land on another socket
  if ( !mToSocket )
  {
    return;
  }

  // Do nothing if from socket and to socket are both input or both output
  if ( mFromSocket->edge() == mToSocket->edge() )
  {
    return;
  }

  view()->beginCommand( tr( "Edit link" ) );

  QList<QgsProcessingModelChildParameterSource> sources;

  QgsProcessingModelComponent *componentFrom = nullptr;
  QgsProcessingModelChildAlgorithm *childTo = nullptr;

  /**
   * Reorder input and output socket
   * whether the user dragged :
   *    - From an input socket to an output socket
   *    - From an output socket to an input socket
   * 
   * In the code, we always come back to the first case
   */
  if ( !mToSocket->isInput() )
  {
    std::swap( mFromSocket, mToSocket );
  }

  componentFrom = mFromSocket->component();
  childTo = dynamic_cast<QgsProcessingModelChildAlgorithm *>( mToSocket->component() );


  const QgsProcessingParameterDefinition *toParam = childTo->algorithm()->parameterDefinitions().at( mToSocket->index() );

  QgsProcessingModelChildParameterSource source;
  if ( QgsProcessingModelChildAlgorithm *childFrom = dynamic_cast<QgsProcessingModelChildAlgorithm *>( componentFrom ) )
  {
    QString outputName = childFrom->algorithm()->outputDefinitions().at( mFromSocket->index() )->name();
    source = QgsProcessingModelChildParameterSource::fromChildOutput( childFrom->childId(), outputName );
  }
  else if ( QgsProcessingModelParameter *paramFrom = dynamic_cast<QgsProcessingModelParameter *>( componentFrom ) )
  {
    source = QgsProcessingModelChildParameterSource::fromModelParameter( paramFrom->parameterName() );
  }

  QgsProcessingContext context;
  QgsProcessingModelerParameterWidget *widget = QgsGui::processingGuiRegistry()->createModelerParameterWidget( view()->modelScene()->model(), childTo->childId(), toParam, context );


  QList<QgsProcessingModelChildParameterSource> compatibleParamSources = widget->availableSourcesForChild();
  delete widget;

  if ( !compatibleParamSources.contains( source ) )
  {
    //Type are incomatible
    const QString title = tr( "Sockets cannot be connected" );
    const QString message = tr( "Either the sockets are incompatible or there is a circular dependency" );
    scene()->showWarning( message, title, message );
    return;
  }

  sources << source;
  childTo->addParameterSources( toParam->name(), sources );


  //We need to pass the update child algorithm to the model
  scene()->model()->setChildAlgorithm( *childTo );

  view()->endCommand();
  // Redraw
  emit requestRebuildRequired();
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
    QgsProcessingModelChildAlgorithm *childFrom = dynamic_cast<QgsProcessingModelChildAlgorithm *>( mFromSocket->component() );
    const QgsProcessingParameterDefinition *param = childFrom->algorithm()->parameterDefinitions().at( mFromSocket->index() );

    auto currentSources = childFrom->parameterSources().value( param->name() );

    // we need to manually pass this event down to items we want it to go to -- QGraphicsScene doesn't propagate
    QList<QGraphicsItem *> items = scene()->items();
    QgsProcessingModelChildParameterSource oldSource;
    for ( const QgsProcessingModelChildParameterSource &source : std::as_const( currentSources ) )
    {
      switch ( source.source() )
      {
        case Qgis::ProcessingModelChildParameterSource::ModelParameter:
        case Qgis::ProcessingModelChildParameterSource::ChildOutput:
        {
          oldSource = source;
          QgsProcessingModelChildAlgorithm *_alg;
          // This is not so nice to have the UI tangled gotta think of a better abstraction later
          // Loop trought all items to get the output socket
          for ( QGraphicsItem *item : items )
          {
            if ( QgsModelDesignerSocketGraphicItem *outputSocket = dynamic_cast<QgsModelDesignerSocketGraphicItem *>( item ) )
            {
              if ( ( _alg = dynamic_cast<QgsProcessingModelChildAlgorithm *>( outputSocket->component() ) ) )
              {
                if ( source.outputChildId() != _alg->childId() || outputSocket->isInput() )
                {
                  continue;
                }
                int outputIndexForSourceName = QgsProcessingUtils::outputDefinitionIndex( _alg->algorithm(), source.outputName() );
                if ( outputSocket->index() == outputIndexForSourceName )
                {
                  mFromSocket = outputSocket;
                  view()->beginCommand( tr( "Edit link" ) );
                }
              }
              else if ( QgsProcessingModelParameter *_param = dynamic_cast<QgsProcessingModelParameter *>( outputSocket->component() ) )
              {
                if ( source.parameterName() == _param->parameterName() )
                {
                  mFromSocket = outputSocket;
                  view()->beginCommand( tr( "Edit link" ) );
                }
              }
            }
          }

          //reset to default value
          QList<QgsProcessingModelChildParameterSource> newSources;
          newSources << QgsProcessingModelChildParameterSource::fromStaticValue( param->defaultValue() );


          childFrom->addParameterSources( param->name(), newSources );
          //We need to pass the update child algorithm to the model
          scene()->model()->setChildAlgorithm( *childFrom );
          // Redraw
          emit requestRebuildRequired();

          //Get Socket from Source alg / source parameter
          QgsModelComponentGraphicItem *item = nullptr;
          int socket_index = -1;
          if ( oldSource.source() == Qgis::ProcessingModelChildParameterSource::ChildOutput )
          {
            item = scene()->childAlgorithmItem( oldSource.outputChildId() );
            socket_index = QgsProcessingUtils::outputDefinitionIndex( _alg->algorithm(), source.outputName() );
          }
          else if ( oldSource.source() == Qgis::ProcessingModelChildParameterSource::ModelParameter )
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
