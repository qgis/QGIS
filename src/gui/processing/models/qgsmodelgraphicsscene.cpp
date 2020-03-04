/***************************************************************************
                             qgsmodelgraphicsscene.cpp
                             ----------------------------------
    Date                 : March 2020
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

#include "qgsmodelgraphicsscene.h"
#include "qgsprocessingmodelchildparametersource.h"
#include "qgsprocessingmodelalgorithm.h"
#include "qgsmodelcomponentgraphicitem.h"
#include "qgsmodelarrowitem.h"
#include <QGraphicsSceneMouseEvent>

///@cond NOT_STABLE

QgsModelGraphicsScene::QgsModelGraphicsScene( QObject *parent )
  : QGraphicsScene( parent )
{
  setItemIndexMethod( QGraphicsScene::NoIndex );
}

void QgsModelGraphicsScene::setFlag( QgsModelGraphicsScene::Flag flag, bool on )
{
  if ( on )
    mFlags |= flag;
  else
    mFlags &= ~flag;
}

void QgsModelGraphicsScene::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
    return;
  QGraphicsScene::mousePressEvent( event );
}

QgsModelComponentGraphicItem *QgsModelGraphicsScene::createParameterGraphicItem( QgsProcessingModelAlgorithm *model, QgsProcessingModelParameter *param ) const
{
  return new QgsModelParameterGraphicItem( param, model, nullptr );
}

QgsModelComponentGraphicItem *QgsModelGraphicsScene::createChildAlgGraphicItem( QgsProcessingModelAlgorithm *model, QgsProcessingModelChildAlgorithm *child ) const
{
  return new QgsModelChildAlgorithmGraphicItem( child, model, nullptr );
}

QgsModelComponentGraphicItem *QgsModelGraphicsScene::createOutputGraphicItem( QgsProcessingModelAlgorithm *model, QgsProcessingModelOutput *output ) const
{
  return new QgsModelOutputGraphicItem( output, model, nullptr );
}

QgsModelComponentGraphicItem *QgsModelGraphicsScene::createCommentGraphicItem( QgsProcessingModelAlgorithm *model, QgsProcessingModelComment *comment, QgsModelComponentGraphicItem *parentItem ) const
{
  return new QgsModelCommentGraphicItem( comment, parentItem, model, nullptr );
}

void QgsModelGraphicsScene::createItems( QgsProcessingModelAlgorithm *model, QgsProcessingContext &context )
{
  // model input parameters
  const QMap<QString, QgsProcessingModelParameter> params = model->parameterComponents();
  for ( auto it = params.constBegin(); it != params.constEnd(); ++it )
  {
    QgsModelComponentGraphicItem *item = createParameterGraphicItem( model, it.value().clone() );
    addItem( item );
    item->setPos( it.value().position().x(), it.value().position().y() );
    mParameterItems.insert( it.value().parameterName(), item );
    connect( item, &QgsModelComponentGraphicItem::requestModelRepaint, this, &QgsModelGraphicsScene::rebuildRequired );
    connect( item, &QgsModelComponentGraphicItem::changed, this, &QgsModelGraphicsScene::componentChanged );

    addCommentItemForComponent( model, it.value(), item );
  }

  // input dependency arrows
  for ( auto it = params.constBegin(); it != params.constEnd(); ++it )
  {
    const QgsProcessingParameterDefinition *parameterDef = model->parameterDefinition( it.key() );
    const QStringList parameterLinks = parameterDef->dependsOnOtherParameters();
    for ( const QString &otherName : parameterLinks )
    {
      if ( mParameterItems.contains( it.key() ) && mParameterItems.contains( otherName ) )
      {
        std::unique_ptr< QgsModelArrowItem > arrow = qgis::make_unique< QgsModelArrowItem >( mParameterItems.value( otherName ), mParameterItems.value( it.key() ) );
        arrow->setPenStyle( Qt::DotLine );
        addItem( arrow.release() );
      }
    }
  }

  // child algorithms
  const QMap<QString, QgsProcessingModelChildAlgorithm> childAlgs = model->childAlgorithms();
  for ( auto it = childAlgs.constBegin(); it != childAlgs.constEnd(); ++it )
  {
    QgsModelComponentGraphicItem *item = createChildAlgGraphicItem( model, it.value().clone() );
    addItem( item );
    item->setPos( it.value().position().x(), it.value().position().y() );
    mChildAlgorithmItems.insert( it.value().childId(), item );
    connect( item, &QgsModelComponentGraphicItem::requestModelRepaint, this, &QgsModelGraphicsScene::rebuildRequired );
    connect( item, &QgsModelComponentGraphicItem::changed, this, &QgsModelGraphicsScene::componentChanged );

    addCommentItemForComponent( model, it.value(), item );
  }

  // arrows linking child algorithms
  for ( auto it = childAlgs.constBegin(); it != childAlgs.constEnd(); ++it )
  {
    int idx = 0;
    const QgsProcessingParameterDefinitions parameters = it.value().algorithm()->parameterDefinitions();
    for ( const QgsProcessingParameterDefinition *parameter : parameters )
    {
      if ( !parameter->isDestination() && !( parameter->flags() & QgsProcessingParameterDefinition::FlagHidden ) )
      {
        QList< QgsProcessingModelChildParameterSource > sources;
        if ( it.value().parameterSources().contains( parameter->name() ) )
          sources = it.value().parameterSources()[parameter->name()];
        for ( const QgsProcessingModelChildParameterSource &source : sources )
        {
          const QList< LinkSource > sourceItems = linkSourcesForParameterValue( model, QVariant::fromValue( source ), it.value().childId(), context );
          for ( const LinkSource &link : sourceItems )
          {
            QgsModelArrowItem *arrow = nullptr;
            if ( link.linkIndex == -1 )
              arrow = new QgsModelArrowItem( link.item, mChildAlgorithmItems.value( it.value().childId() ), Qt::TopEdge, idx );
            else
              arrow = new QgsModelArrowItem( link.item, link.edge, link.linkIndex, mChildAlgorithmItems.value( it.value().childId() ), Qt::TopEdge, idx );
            addItem( arrow );
          }
          idx += 1;
        }
      }
    }
    const QStringList dependencies = it.value().dependencies();
    for ( const QString &depend : dependencies )
    {
      addItem( new QgsModelArrowItem( mChildAlgorithmItems.value( depend ), mChildAlgorithmItems.value( it.value().childId() ) ) );
    }
  }

  // and finally the model outputs
  for ( auto it = childAlgs.constBegin(); it != childAlgs.constEnd(); ++it )
  {
    const QMap<QString, QgsProcessingModelOutput> outputs = it.value().modelOutputs();
    QMap< QString, QgsModelComponentGraphicItem * > outputItems;

    for ( auto outputIt = outputs.constBegin(); outputIt != outputs.constEnd(); ++outputIt )
    {
      QgsModelComponentGraphicItem *item = createOutputGraphicItem( model, outputIt.value().clone() );
      addItem( item );
      connect( item, &QgsModelComponentGraphicItem::requestModelRepaint, this, &QgsModelGraphicsScene::rebuildRequired );
      connect( item, &QgsModelComponentGraphicItem::changed, this, &QgsModelGraphicsScene::componentChanged );

      addCommentItemForComponent( model, outputIt.value(), item );

      QPointF pos = outputIt.value().position();

      // find the actual index of the linked output from the child algorithm it comes from
      const QgsProcessingOutputDefinitions sourceChildAlgOutputs = it.value().algorithm()->outputDefinitions();
      int idx = -1;
      int i = 0;
      for ( const QgsProcessingOutputDefinition *childAlgOutput : sourceChildAlgOutputs )
      {
        if ( childAlgOutput->name() == outputIt.value().childOutputName() )
        {
          idx = i;
          break;
        }
        i++;
      }

      item->setPos( pos );
      outputItems.insert( outputIt.key(), item );
      addItem( new QgsModelArrowItem( mChildAlgorithmItems[it.value().childId()], Qt::BottomEdge, idx, item ) );
    }
    mOutputItems.insert( it.value().childId(), outputItems );
  }
}

QList<QgsModelGraphicsScene::LinkSource> QgsModelGraphicsScene::linkSourcesForParameterValue( QgsProcessingModelAlgorithm *model, const QVariant &value, const QString &childId, QgsProcessingContext &context ) const
{
  QList<QgsModelGraphicsScene::LinkSource> res;
  if ( value.type() == QVariant::List )
  {
    const QVariantList list = value.toList();
    for ( const QVariant &v : list )
      res.append( linkSourcesForParameterValue( model, v, childId, context ) );
  }
  else if ( value.type() == QVariant::StringList )
  {
    const QStringList list = value.toStringList();
    for ( const QString &v : list )
      res.append( linkSourcesForParameterValue( model, v, childId, context ) );
  }
  else if ( value.canConvert< QgsProcessingModelChildParameterSource >() )
  {
    const QgsProcessingModelChildParameterSource source = value.value< QgsProcessingModelChildParameterSource >();
    switch ( source.source() )
    {
      case QgsProcessingModelChildParameterSource::ModelParameter:
      {
        LinkSource l;
        l.item = mParameterItems.value( source.parameterName() );
        res.append( l );
        break;
      }
      case QgsProcessingModelChildParameterSource::ChildOutput:
      {
        const QgsProcessingOutputDefinitions outputs = model->childAlgorithm( source.outputChildId() ).algorithm()->outputDefinitions();
        int i = 0;
        for ( const QgsProcessingOutputDefinition *output : outputs )
        {
          if ( output->name() == source.outputName() )
            break;
          i++;
        }
        if ( mChildAlgorithmItems.contains( source.outputChildId() ) )
        {
          LinkSource l;
          l.item = mChildAlgorithmItems.value( source.outputChildId() );
          l.edge = Qt::BottomEdge;
          l.linkIndex = i;
          res.append( l );
        }

        break;
      }

      case QgsProcessingModelChildParameterSource::Expression:
      {
        const QMap<QString, QgsProcessingModelAlgorithm::VariableDefinition> variables = model->variablesForChildAlgorithm( childId, context );
        QgsExpression exp( source.expression() );
        const QSet<QString> vars = exp.referencedVariables();
        for ( const QString &v : vars )
        {
          if ( variables.contains( v ) )
          {
            res.append( linkSourcesForParameterValue( model, QVariant::fromValue( variables.value( v ).source ), childId, context ) );
          }
        }
        break;
      }

      case QgsProcessingModelChildParameterSource::StaticValue:
      case QgsProcessingModelChildParameterSource::ExpressionText:
        break;
    }
  }
  return res;
}

void QgsModelGraphicsScene::addCommentItemForComponent( QgsProcessingModelAlgorithm *model, const QgsProcessingModelComponent &component, QgsModelComponentGraphicItem *parentItem )
{
  if ( !component.comment() || component.comment()->description().isEmpty() )
    return;

  QgsModelComponentGraphicItem *commentItem = createCommentGraphicItem( model, component.comment()->clone(), parentItem );
  commentItem->setPos( component.comment()->position().x(), component.comment()->position().y() );
  addItem( commentItem );
  connect( commentItem, &QgsModelComponentGraphicItem::requestModelRepaint, this, &QgsModelGraphicsScene::rebuildRequired );
  connect( commentItem, &QgsModelComponentGraphicItem::changed, this, &QgsModelGraphicsScene::componentChanged );

  std::unique_ptr< QgsModelArrowItem > arrow = qgis::make_unique< QgsModelArrowItem >( parentItem, commentItem );
  arrow->setPenStyle( Qt::DotLine );
  addItem( arrow.release() );
}

///@endcond

