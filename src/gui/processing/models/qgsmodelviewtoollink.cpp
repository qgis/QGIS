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
  // qDebug() << "QgsModelViewToolLink::modelMoveEvent";

  
  mBezierRubberBand->update( event->modelPoint(), Qt::KeyboardModifiers() );

  // we need to manually pass this event down to items we want it to go to -- QGraphicsScene doesn't propagate 
  QList<QGraphicsItem *> items = scene()->items( event->modelPoint() );
  for ( QGraphicsItem *item : items )
  {
    if ( QgsModelDesignerSocketGraphicItem *socket = dynamic_cast<QgsModelDesignerSocketGraphicItem *>( item ) )
    {
      socket->modelHoverEnterEvent( event );
      // snap 
      if ( mFrom != socket && mFrom->edge() != socket->edge()){
        QPointF rubberEndPos = socket->mapToScene(socket->getPosition());
        mBezierRubberBand->update( rubberEndPos, Qt::KeyboardModifiers() );
      }
      qDebug() << "should trigger socket->modelHoverEnterEvent( event );";
    }
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

  mTo = nullptr;

  for ( QGraphicsItem *item : items )
  {
    if ( QgsModelDesignerSocketGraphicItem *socket = dynamic_cast<QgsModelDesignerSocketGraphicItem *>( item ) )
    {
      mTo = socket;
    }
  }

  // Do nothing if cursor don't land on another socket
  if (mTo == nullptr){
    return;
  }

  // Do nothing if from socket and to socket are both input or both output
  if ( mFrom->edge() == mTo->edge()){
    return;
  }

  emit view()->beginCommand("Edit link");

  QList<QgsProcessingModelChildParameterSource> sources;
  

  QgsProcessingModelComponent *component_from;
  QgsProcessingModelChildAlgorithm *child_to;

  // ReOrder in out socket
  // always fix on the input end receiving
  if( mTo->edge() == Qt::TopEdge ) {
    component_from = mFrom->component();
    child_to = dynamic_cast<QgsProcessingModelChildAlgorithm *>( mTo->component() );
  }
  else{
    component_from = mTo->component();
    child_to = dynamic_cast<QgsProcessingModelChildAlgorithm *>( mFrom->component() );
  }

  

  // QString inputName = "INPUT";
  const QgsProcessingParameterDefinition* toParam = child_to->algorithm()->parameterDefinitions().at(mTo->index());
  
  

  QgsProcessingModelChildParameterSource source;
  if ( QgsProcessingModelChildAlgorithm *child_from = dynamic_cast<QgsProcessingModelChildAlgorithm *>( component_from ) )
  {
    QString outputName = child_from->algorithm()->outputDefinitions().at(mFrom->index())->name();
    source = QgsProcessingModelChildParameterSource::fromChildOutput( child_from->childId(),  outputName );
    qDebug() << "child_from->childId: " << child_from->childId();

  }
  else if ( QgsProcessingModelParameter *param_from = dynamic_cast<QgsProcessingModelParameter *>( component_from) ) {
    source = QgsProcessingModelChildParameterSource::fromModelParameter(param_from->parameterName());
  }
  
  // QgsProcessingModelChildParameterSource source =  QgsProcessingModelChildParameterSource::fromExpression( QStringLiteral( "@c2_CONCATENATION || 'x'" ) ) ;
  qDebug() << "OUTPUTT:" << source.outputName();
  qDebug() << "OUTPUT  child id" << source.outputChildId();

  



  // setChildAlgorithm
  
  QgsProcessingContext context;
  QgsProcessingModelerParameterWidget *widget = QgsGui::processingGuiRegistry()->createModelerParameterWidget(view()->modelScene()->model(),
                                                               child_to->childId(),
                                                               toParam,
                                                               context
                                                               );


  QList<QgsProcessingModelChildParameterSource> compatible_param_type = widget->availableSourcesForChild();
  delete widget;

  if ( !compatible_param_type.contains(source) ) {
    //Type are incomatible
    QString title = "Impossible to connect socket";
    QString message = "Impossible to connect socket either type are incompatibles or theres is a circular dependency";
    scene()->showWarning(message, title, message);
    return;
  }

  sources << source;
  child_to->addParameterSources(toParam->name(), sources);


  
  //We need to pass the update child algorithm to the model
  scene()->model()->setChildAlgorithm(*child_to);

  emit view()->endCommand();
  // Redraw
  emit scene()->rebuildRequired();
 

    // // we need to manually pass this event down to items we want it to go to -- QGraphicsScene doesn't propagate events
    // // to multiple items
    // QList<QGraphicsItem *> items = scene()->items( event->modelPoint() );
    // qDebug() << "Click on an item";
    // for ( QGraphicsItem *item : items )
    // {
    //   if ( QgsModelDesignerSocketGraphicItem *socket = dynamic_cast<QgsModelDesignerSocketGraphicItem *>( item ) ){
    //     // Start link tool"
    //     // qDebug() << "Start link tool";
    //     // mLinkTool->setFromSocket(socket);
    //     // view()->setTool( mLinkTool.get() );
    //   }
    // }


}

bool QgsModelViewToolLink::allowItemInteraction()
{
  return false;
}

void QgsModelViewToolLink::activate()
{
  qDebug() << "activate link tool";
  mPreviousViewTool = view()->tool();

  QPointF rubberStartPos = mFrom->mapToScene(mFrom->getPosition());
  mBezierRubberBand->start( rubberStartPos, Qt::KeyboardModifiers() );

  // if mFrom 

  QgsModelViewTool::activate();
}

void QgsModelViewToolLink::deactivate()
{
  qDebug() << "deactivate link tool";
  mBezierRubberBand->finish( );
  QgsModelViewTool::deactivate();
}

void QgsModelViewToolLink::setFromSocket(QgsModelDesignerSocketGraphicItem *socket) { 
  mFrom = socket;

  if (mFrom->edge() == Qt::TopEdge)
  {
    QgsProcessingModelChildAlgorithm *child_from = dynamic_cast<QgsProcessingModelChildAlgorithm *>( mFrom->component() );
    const QgsProcessingParameterDefinition* param = child_from->algorithm()->parameterDefinitions().at(mFrom->index());

    auto current_sources = child_from->parameterSources().value(param->name());
    qDebug() << "SOURCES :" << current_sources.size();

    // we need to manually pass this event down to items we want it to go to -- QGraphicsScene doesn't propagate 
    QList<QGraphicsItem *> items = scene()->items();

    for ( const QgsProcessingModelChildParameterSource &source : std::as_const( current_sources ) )
    {
      switch ( source.source() )
      {
        case Qgis::ProcessingModelChildParameterSource::ModelParameter:
        case Qgis::ProcessingModelChildParameterSource::ChildOutput:
        {
          // source.outputName();
          // source.outputChildId();

          QgsProcessingModelChildAlgorithm source_alg = scene()->model()->childAlgorithm(source.outputChildId());
          
          // This is not so nice to have the UI tangled gotta think of a better abstraction later
          // Loop trought all items to get the output socket 
          for ( QGraphicsItem *item : items )
          {
            if ( QgsModelDesignerSocketGraphicItem *output_socket = dynamic_cast<QgsModelDesignerSocketGraphicItem *>( item ) )
            {
              if ( QgsProcessingModelChildAlgorithm *_alg = dynamic_cast<QgsProcessingModelChildAlgorithm *>( output_socket->component() ) ){
                if (source.outputChildId() != _alg->childId() || 
                    output_socket->edge() == Qt::TopEdge ){
                  continue;
                }
                if (output_socket->index() == _alg->algorithm()->outputDefinitionIndex(source.outputName()) ){
                  mFrom = output_socket;
                  emit view()->beginCommand("Edit link");
                }
              }
              else if ( QgsProcessingModelParameter *_param = dynamic_cast<QgsProcessingModelParameter *>( output_socket->component() ) ){
                if ( source.parameterName() == _param->parameterName()) {
                  mFrom = output_socket;
                  emit view()->beginCommand("Edit link");
                }
              }
            }
          }


          qDebug() << "new source";
          // child_alg
              //reset to default value
          QList<QgsProcessingModelChildParameterSource> new_sources;
          new_sources << QgsProcessingModelChildParameterSource::fromStaticValue(param->defaultValue());


          child_from->addParameterSources(param->name(), new_sources);
          //We need to pass the update child algorithm to the model
          scene()->model()->setChildAlgorithm(*child_from);
          // Redraw
          emit scene()->rebuildRequired();


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
