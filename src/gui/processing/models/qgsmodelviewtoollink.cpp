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
#include "qgsprocessingguiregistry.h"
#include "qgsprocessingmodelchildalgorithm.h"
#include "qgsmodelgraphicsscene.h"
#include "qgsmodelviewmouseevent.h"
#include "qgsmodelviewtoolselect.h"
#include "qgsmodelgraphicsview.h"
#include "qgsmodelviewrubberband.h"
#include "qgsmodelgraphicitem.h"


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
  const QList<QGraphicsItem *> items = scene()->items( event->modelPoint() );

  QgsModelDesignerSocketGraphicItem *socket = nullptr;
  for ( QGraphicsItem *item : items )
  {
    socket = dynamic_cast<QgsModelDesignerSocketGraphicItem *>( item );
    if ( !socket || mFromSocket == socket || mFromSocket->edge() == socket->edge() || mFromSocket->component() == socket->component() )
      continue;

    // snap
    socket->modelHoverEnterEvent( event );
    const QPointF rubberEndPos = socket->mapToScene( socket->position() );
    mBezierRubberBand->update( rubberEndPos, Qt::KeyboardModifiers() );
    break;
  }

  if ( mLastHoveredSocket && socket != mLastHoveredSocket )
  {
    mLastHoveredSocket->modelHoverLeaveEvent( event );
    mLastHoveredSocket = nullptr;
  }

  if ( socket && socket != mLastHoveredSocket )
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
  mBezierRubberBand->finish( event->modelPoint() );
  if ( mLastHoveredSocket )
  {
    mLastHoveredSocket->modelHoverLeaveEvent( nullptr );
    mLastHoveredSocket = nullptr;
  }

  view()->setTool( mPreviousViewTool );

  // we need to manually pass this event down to items we want it to go to -- QGraphicsScene doesn't propagate
  const QList<QGraphicsItem *> items = scene()->items( event->modelPoint() );

  mToSocket = nullptr;

  for ( QGraphicsItem *item : items )
  {
    if ( QgsModelDesignerSocketGraphicItem *socket = dynamic_cast<QgsModelDesignerSocketGraphicItem *>( item ) )
    {
      // Skip if sockets are both input or both output or both from the same algorithm
      if ( mFromSocket->edge() == socket->edge() || mFromSocket->component() == socket->component() )
        continue;

      mToSocket = socket;
      break;
    }
  }

  // Do nothing if cursor didn't land on another socket
  if ( !mToSocket )
  {
    // but it might have been an unlink, so we properly end the command
    view()->endCommand();
    return;
  }
  // and we abort any pending unlink command to not litter the undo buffer
  view()->abortCommand();

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

  QgsProcessingModelComponent *outputComponent = mFromSocket->component();
  QgsProcessingModelChildAlgorithm *inputChildAlgorithm = dynamic_cast<QgsProcessingModelChildAlgorithm *>( mToSocket->component() );
  if ( !inputChildAlgorithm )
  {
    // Should not happen, but checking is cheap!
    QgsDebugError( QStringLiteral( "Input is not a QgsProcessingModelChildAlgorithm" ) );
    return;
  }

  QgsProcessingModelChildParameterSource newInputParamSource;
  QString outParamDescription;
  if ( const QgsProcessingModelChildAlgorithm *outputChildAlgorithm = dynamic_cast<QgsProcessingModelChildAlgorithm *>( outputComponent ) )
  {
    const QString outParamName = outputChildAlgorithm->algorithm()->outputDefinitions().at( mFromSocket->index() )->name();
    newInputParamSource = QgsProcessingModelChildParameterSource::fromChildOutput( outputChildAlgorithm->childId(), outParamName );
    outParamDescription = outputChildAlgorithm->algorithm()->outputDefinitions().at( mFromSocket->index() )->description();
  }
  else if ( const QgsProcessingModelParameter *paramFrom = dynamic_cast<QgsProcessingModelParameter *>( outputComponent ) )
  {
    newInputParamSource = QgsProcessingModelChildParameterSource::fromModelParameter( paramFrom->parameterName() );
    outParamDescription = paramFrom->description();
  }

  const QgsProcessingParameterDefinition *inputParam = inputChildAlgorithm->algorithm()->parameterDefinitions().at( mToSocket->index() );
  const QList<QgsProcessingModelChildParameterSource> compatibleInputParamSources = scene()->model()->availableSourcesForChild( inputChildAlgorithm->childId(), inputParam );
  if ( !compatibleInputParamSources.contains( newInputParamSource ) )
  {
    // Types are incompatible
    const QString title = tr( "Sockets cannot be connected" );
    const QString message = tr( "Either the sockets are incompatible or there is a circular dependency" );
    scene()->showWarning( message, title, message );
    return;
  }

  view()->beginCommand( tr( "Link %1: %2 to %3: %4" ).arg( outputComponent->description(), outParamDescription, inputChildAlgorithm->description(), inputParam->description() ) );

  inputChildAlgorithm->addParameterSources( inputParam->name(), { newInputParamSource } );

  //We need to pass the update child algorithm to the model
  scene()->model()->setChildAlgorithm( *inputChildAlgorithm );

  view()->endCommand();
  // Redraw
  scene()->requestRebuildRequired();
}

bool QgsModelViewToolLink::allowItemInteraction()
{
  return true;
}

void QgsModelViewToolLink::activate()
{
  QgsModelViewTool *tool = view()->tool();
  // Make sure we always return to the select tool and not a temporary tool.
  if ( dynamic_cast<QgsModelViewToolSelect *>( tool ) )
  {
    mPreviousViewTool = tool;
  }

  const QPointF rubberStartPos = mFromSocket->mapToScene( mFromSocket->position() );
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

  // If it's an input socket and it's already connected, we want 'From' to be the output at the other end of the connection
  if ( mFromSocket->isInput() )
  {
    QgsProcessingModelChildAlgorithm *childFrom = dynamic_cast<QgsProcessingModelChildAlgorithm *>( mFromSocket->component() );
    const QgsProcessingParameterDefinition *param = childFrom->algorithm()->parameterDefinitions().at( mFromSocket->index() );
    const QList<QgsProcessingModelChildParameterSource> currentSources = childFrom->parameterSources().value( param->name() );

    for ( const QgsProcessingModelChildParameterSource &source : currentSources )
    {
      // Was not connected, nothing to do
      if ( source.outputChildId().isEmpty() )
        break;

      switch ( source.source() )
      {
        case Qgis::ProcessingModelChildParameterSource::ModelParameter:
        case Qgis::ProcessingModelChildParameterSource::ChildOutput:
        {
          view()->beginCommand( tr( "Unlink %1: %2", "Unlink Algorithm: Input" ).arg( childFrom->description(), param->description() ) );

          // reset to default value. Layers/feature sources default to an empty model parameter
          QList<QgsProcessingModelChildParameterSource> newSources;
          if ( param->type() == QgsProcessingParameterFeatureSource::typeName() || param->type() == QgsProcessingParameterMapLayer::typeName() || param->type() == QgsProcessingParameterMeshLayer::typeName() || param->type() == QgsProcessingParameterPointCloudLayer::typeName() || param->type() == QgsProcessingParameterRasterLayer::typeName() || param->type() == QgsProcessingParameterVectorLayer::typeName() )
          {
            newSources << QgsProcessingModelChildParameterSource::fromModelParameter( QString() );
          }
          else
          {
            // Other parameters default to static value
            newSources << QgsProcessingModelChildParameterSource::fromStaticValue( param->defaultValue() );
          }

          childFrom->addParameterSources( param->name(), newSources );
          //We need to pass the update child algorithm to the model
          scene()->model()->setChildAlgorithm( *childFrom );
          // Redraw
          scene()->requestRebuildRequired();

          //Get socket from initial source alg / source parameter
          QgsModelComponentGraphicItem *item = nullptr;
          int socketIndex = -1;
          if ( source.source() == Qgis::ProcessingModelChildParameterSource::ChildOutput )
          {
            item = scene()->childAlgorithmItem( source.outputChildId() );
            auto algSource = dynamic_cast<QgsProcessingModelChildAlgorithm *>( item->component() );
            if ( !algSource )
            {
              QgsDebugError( QStringLiteral( "algSource not set, aborting!" ) );
              return;
            }
            socketIndex = QgsProcessingUtils::outputDefinitionIndex( algSource->algorithm(), source.outputName() );
          }
          else if ( source.source() == Qgis::ProcessingModelChildParameterSource::ModelParameter )
          {
            item = scene()->parameterItem( source.parameterName() );
            socketIndex = 0;
          }

          if ( !item )
          {
            QgsDebugError( QStringLiteral( "item not set, aborting!" ) );
            return;
          }

          mFromSocket = item->outSocketAt( socketIndex );
        }
        break;

        case Qgis::ProcessingModelChildParameterSource::StaticValue:
        case Qgis::ProcessingModelChildParameterSource::Expression:
        case Qgis::ProcessingModelChildParameterSource::ExpressionText:
        case Qgis::ProcessingModelChildParameterSource::ModelOutput:
          continue;
      }

      // Stop on first iteration to get only one link at a time
      break;
    }
  }
};
