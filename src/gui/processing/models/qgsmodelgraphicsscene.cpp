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
#include "qgsprocessingmodelgroupbox.h"
#include "qgsmessagebar.h"
#include "qgsmessagebaritem.h"
#include "qgsmessageviewer.h"
#include "qgsapplication.h"
#include <QGraphicsSceneMouseEvent>
#include <QPushButton>

///@cond NOT_STABLE

QgsModelGraphicsScene::QgsModelGraphicsScene( QObject *parent )
  : QGraphicsScene( parent )
{
  setItemIndexMethod( QGraphicsScene::NoIndex );
}

QgsProcessingModelAlgorithm *QgsModelGraphicsScene::model()
{
  return mModel;
}

void QgsModelGraphicsScene::setModel( QgsProcessingModelAlgorithm *model )
{
  mModel = model;
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

QgsModelChildAlgorithmGraphicItem *QgsModelGraphicsScene::createChildAlgGraphicItem( QgsProcessingModelAlgorithm *model, QgsProcessingModelChildAlgorithm *child ) const
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

QgsModelComponentGraphicItem *QgsModelGraphicsScene::createGroupBoxGraphicItem( QgsProcessingModelAlgorithm *model, QgsProcessingModelGroupBox *box ) const
{
  return new QgsModelGroupBoxGraphicItem( box, model, nullptr );
}

void QgsModelGraphicsScene::createItems( QgsProcessingModelAlgorithm *model, QgsProcessingContext &context )
{
  // model group boxes
  const QList<QgsProcessingModelGroupBox> boxes = model->groupBoxes();
  mGroupBoxItems.clear();
  for ( const QgsProcessingModelGroupBox &box : boxes )
  {
    QgsModelComponentGraphicItem *item = createGroupBoxGraphicItem( model, box.clone() );
    addItem( item );
    item->setPos( box.position().x(), box.position().y() );
    mGroupBoxItems.insert( box.uuid(), item );
    connect( item, &QgsModelComponentGraphicItem::requestModelRepaint, this, &QgsModelGraphicsScene::rebuildRequired );
    connect( item, &QgsModelComponentGraphicItem::changed, this, &QgsModelGraphicsScene::componentChanged );
    connect( item, &QgsModelComponentGraphicItem::aboutToChange, this, &QgsModelGraphicsScene::componentAboutToChange );
  }

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
    connect( item, &QgsModelComponentGraphicItem::aboutToChange, this, &QgsModelGraphicsScene::componentAboutToChange );

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
        std::unique_ptr< QgsModelArrowItem > arrow = std::make_unique< QgsModelArrowItem >( mParameterItems.value( otherName ), QgsModelArrowItem::Marker::Circle, mParameterItems.value( it.key() ), QgsModelArrowItem::Marker::ArrowHead );
        arrow->setPenStyle( Qt::DotLine );
        addItem( arrow.release() );
      }
    }
  }

  // child algorithms
  const QMap<QString, QgsProcessingModelChildAlgorithm> childAlgs = model->childAlgorithms();
  for ( auto it = childAlgs.constBegin(); it != childAlgs.constEnd(); ++it )
  {
    QgsModelChildAlgorithmGraphicItem *item = createChildAlgGraphicItem( model, it.value().clone() );
    addItem( item );
    item->setPos( it.value().position().x(), it.value().position().y() );
    item->setResults( mChildResults.value( it.value().childId() ).toMap() );
    item->setInputs( mChildInputs.value( it.value().childId() ).toMap() );
    mChildAlgorithmItems.insert( it.value().childId(), item );
    connect( item, &QgsModelComponentGraphicItem::requestModelRepaint, this, &QgsModelGraphicsScene::rebuildRequired );
    connect( item, &QgsModelComponentGraphicItem::changed, this, &QgsModelGraphicsScene::componentChanged );
    connect( item, &QgsModelComponentGraphicItem::aboutToChange, this, &QgsModelGraphicsScene::componentAboutToChange );

    addCommentItemForComponent( model, it.value(), item );
  }

  // arrows linking child algorithms
  for ( auto it = childAlgs.constBegin(); it != childAlgs.constEnd(); ++it )
  {
    int topIdx = 0;
    int bottomIdx = 0;
    if ( !it.value().algorithm() )
      continue;

    const QgsProcessingParameterDefinitions parameters = it.value().algorithm()->parameterDefinitions();
    for ( const QgsProcessingParameterDefinition *parameter : parameters )
    {
      if ( !( parameter->flags() & QgsProcessingParameterDefinition::FlagHidden ) )
      {
        QList< QgsProcessingModelChildParameterSource > sources;
        if ( it.value().parameterSources().contains( parameter->name() ) )
          sources = it.value().parameterSources()[parameter->name()];
        for ( const QgsProcessingModelChildParameterSource &source : sources )
        {
          const QList< LinkSource > sourceItems = linkSourcesForParameterValue( model, QVariant::fromValue( source ), it.value().childId(), context );
          for ( const LinkSource &link : sourceItems )
          {
            if ( !link.item )
              continue;
            QgsModelArrowItem *arrow = nullptr;
            if ( link.linkIndex == -1 )
              arrow = new QgsModelArrowItem( link.item, QgsModelArrowItem::Marker::Circle, mChildAlgorithmItems.value( it.value().childId() ), parameter->isDestination() ? Qt::BottomEdge : Qt::TopEdge, parameter->isDestination() ? bottomIdx : topIdx, QgsModelArrowItem::Marker::Circle );
            else
              arrow = new QgsModelArrowItem( link.item, link.edge, link.linkIndex, true, QgsModelArrowItem::Marker::Circle,
                                             mChildAlgorithmItems.value( it.value().childId() ),
                                             parameter->isDestination() ? Qt::BottomEdge : Qt::TopEdge,
                                             parameter->isDestination() ? bottomIdx : topIdx,
                                             true,
                                             QgsModelArrowItem::Marker::Circle );
            addItem( arrow );
          }
        }
        if ( parameter->isDestination() )
          bottomIdx++;
        else
          topIdx++;
      }
    }
    const QList< QgsProcessingModelChildDependency > dependencies = it.value().dependencies();
    for ( const QgsProcessingModelChildDependency &depend : dependencies )
    {
      if ( depend.conditionalBranch.isEmpty() || !model->childAlgorithm( depend.childId ).algorithm() )
      {
        addItem( new QgsModelArrowItem( mChildAlgorithmItems.value( depend.childId ), QgsModelArrowItem::Marker::Circle, mChildAlgorithmItems.value( it.value().childId() ), QgsModelArrowItem::Marker::ArrowHead ) );
      }
      else
      {
        // find branch link point
        const QgsProcessingOutputDefinitions outputs = model->childAlgorithm( depend.childId ).algorithm()->outputDefinitions();
        int i = 0;
        bool found = false;
        for ( const QgsProcessingOutputDefinition *output : outputs )
        {
          if ( output->name() == depend.conditionalBranch )
          {
            found = true;
            break;
          }
          i++;
        }
        if ( found )
          addItem( new QgsModelArrowItem( mChildAlgorithmItems.value( depend.childId ), Qt::BottomEdge, i, QgsModelArrowItem::Marker::Circle, mChildAlgorithmItems.value( it.value().childId() ), QgsModelArrowItem::Marker::ArrowHead ) );
      }
    }
  }

  // and finally the model outputs
  for ( auto it = childAlgs.constBegin(); it != childAlgs.constEnd(); ++it )
  {
    const QMap<QString, QgsProcessingModelOutput> outputs = it.value().modelOutputs();
    QMap< QString, QgsModelComponentGraphicItem * > outputItems;

    // offsets from algorithm item needed to correctly place output items
    // which does not have valid position assigned (https://github.com/qgis/QGIS/issues/48132)
    QgsProcessingModelComponent *algItem = mChildAlgorithmItems[it.value().childId()]->component();
    const double outputOffsetX = algItem->size().width();
    double outputOffsetY = 1.5 * algItem->size().height();

    for ( auto outputIt = outputs.constBegin(); outputIt != outputs.constEnd(); ++outputIt )
    {
      QgsModelComponentGraphicItem *item = createOutputGraphicItem( model, outputIt.value().clone() );
      addItem( item );
      connect( item, &QgsModelComponentGraphicItem::requestModelRepaint, this, &QgsModelGraphicsScene::rebuildRequired );
      connect( item, &QgsModelComponentGraphicItem::changed, this, &QgsModelGraphicsScene::componentChanged );
      connect( item, &QgsModelComponentGraphicItem::aboutToChange, this, &QgsModelGraphicsScene::componentAboutToChange );

      // if output added not at the same time as algorithm then it does not have
      // valid position and will be placed at (0,0). We need to calculate better position.
      // See https://github.com/qgis/QGIS/issues/48132.
      QPointF pos = outputIt.value().position();
      if ( pos.isNull() )
      {
        pos = algItem->position() + QPointF( outputOffsetX, outputOffsetY );
        outputOffsetY += 1.5 * outputIt.value().size().height();
      }
      int idx = -1;
      int i = 0;
      // find the actual index of the linked output from the child algorithm it comes from
      if ( it.value().algorithm() )
      {
        const QgsProcessingOutputDefinitions sourceChildAlgOutputs = it.value().algorithm()->outputDefinitions();
        for ( const QgsProcessingOutputDefinition *childAlgOutput : sourceChildAlgOutputs )
        {
          if ( childAlgOutput->name() == outputIt.value().childOutputName() )
          {
            idx = i;
            break;
          }
          i++;
        }
      }

      item->setPos( pos );
      outputItems.insert( outputIt.key(), item );
      addItem( new QgsModelArrowItem( mChildAlgorithmItems[it.value().childId()], Qt::BottomEdge, idx, QgsModelArrowItem::Marker::Circle, item, QgsModelArrowItem::Marker::Circle ) );

      addCommentItemForComponent( model, outputIt.value(), item );
    }
    mOutputItems.insert( it.value().childId(), outputItems );
  }
}

QList<QgsModelComponentGraphicItem *> QgsModelGraphicsScene::selectedComponentItems()
{
  QList<QgsModelComponentGraphicItem *> componentItemList;

  const QList<QGraphicsItem *> graphicsItemList = selectedItems();
  for ( QGraphicsItem *item : graphicsItemList )
  {
    if ( QgsModelComponentGraphicItem *componentItem = dynamic_cast<QgsModelComponentGraphicItem *>( item ) )
    {
      componentItemList.push_back( componentItem );
    }
  }

  return componentItemList;
}

QgsModelComponentGraphicItem *QgsModelGraphicsScene::componentItemAt( QPointF position ) const
{
  //get a list of items which intersect the specified position, in descending z order
  const QList<QGraphicsItem *> itemList = items( position, Qt::IntersectsItemShape, Qt::DescendingOrder );

  for ( QGraphicsItem *graphicsItem : itemList )
  {
    if ( QgsModelComponentGraphicItem *componentItem = dynamic_cast<QgsModelComponentGraphicItem *>( graphicsItem ) )
    {
      return componentItem;
    }
  }
  return nullptr;
}

QgsModelComponentGraphicItem *QgsModelGraphicsScene::groupBoxItem( const QString &uuid )
{
  return mGroupBoxItems.value( uuid );
}

void QgsModelGraphicsScene::selectAll()
{
  //select all items in scene
  QgsModelComponentGraphicItem *focusedItem = nullptr;
  const QList<QGraphicsItem *> itemList = items();
  for ( QGraphicsItem *graphicsItem : itemList )
  {
    if ( QgsModelComponentGraphicItem *componentItem = dynamic_cast<QgsModelComponentGraphicItem *>( graphicsItem ) )
    {
      componentItem->setSelected( true );
      if ( !focusedItem )
        focusedItem = componentItem;
    }
  }
  emit selectedItemChanged( focusedItem );
}

void QgsModelGraphicsScene::deselectAll()
{
  //we can't use QGraphicsScene::clearSelection, as that emits no signals
  //and we don't know which items are being deselected
  //instead, do the clear selection manually...
  const QList<QGraphicsItem *> selectedItemList = selectedItems();
  for ( QGraphicsItem *item : selectedItemList )
  {
    if ( QgsModelComponentGraphicItem *componentItem = dynamic_cast<QgsModelComponentGraphicItem *>( item ) )
    {
      componentItem->setSelected( false );
    }
  }
  emit selectedItemChanged( nullptr );
}

void QgsModelGraphicsScene::setSelectedItem( QgsModelComponentGraphicItem *item )
{
  whileBlocking( this )->deselectAll();
  if ( item )
  {
    item->setSelected( true );
  }
  emit selectedItemChanged( item );
}

void QgsModelGraphicsScene::setChildAlgorithmResults( const QVariantMap &results )
{
  mChildResults = results;

  for ( auto it = mChildResults.constBegin(); it != mChildResults.constEnd(); ++it )
  {
    if ( QgsModelChildAlgorithmGraphicItem *item = mChildAlgorithmItems.value( it.key() ) )
    {
      item->setResults( it.value().toMap() );
    }
  }
}

void QgsModelGraphicsScene::setChildAlgorithmInputs( const QVariantMap &inputs )
{
  mChildInputs = inputs;

  for ( auto it = mChildInputs.constBegin(); it != mChildInputs.constEnd(); ++it )
  {
    if ( QgsModelChildAlgorithmGraphicItem *item = mChildAlgorithmItems.value( it.key() ) )
    {
      item->setInputs( it.value().toMap() );
    }
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
  else if ( value.userType() == QMetaType::type( "QgsProcessingModelChildParameterSource" ) )
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
        if ( !model->childAlgorithm( source.outputChildId() ).algorithm() )
          break;

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

          // do sanity check of linked index
          if ( i >= model->childAlgorithm( source.outputChildId() ).algorithm()->outputDefinitions().length() )
          {
            QString short_message = tr( "Check output links for alg: %1" ).arg( model->childAlgorithm( source.outputChildId() ).algorithm()->name() );
            QString long_message = tr( "Cannot link output for alg: %1" ).arg( model->childAlgorithm( source.outputChildId() ).algorithm()->name() );
            QString title( tr( "Algorithm link error" ) );
            if ( messageBar() )
              showWarning( const_cast<QString &>( short_message ), const_cast<QString &>( title ), const_cast<QString &>( long_message ) );
            else
              QgsMessageLog::logMessage( long_message, "QgsModelGraphicsScene", Qgis::MessageLevel::Warning, true );
            break;
          }

          l.linkIndex = i;
          res.append( l );
        }

        break;
      }

      case QgsProcessingModelChildParameterSource::Expression:
      {
        const QMap<QString, QgsProcessingModelAlgorithm::VariableDefinition> variables = model->variablesForChildAlgorithm( childId, context );
        const QgsExpression exp( source.expression() );
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
      case QgsProcessingModelChildParameterSource::ModelOutput:
        break;
    }
  }
  return res;
}

void QgsModelGraphicsScene::addCommentItemForComponent( QgsProcessingModelAlgorithm *model, const QgsProcessingModelComponent &component, QgsModelComponentGraphicItem *parentItem )
{
  if ( mFlags & FlagHideComments || !component.comment() || component.comment()->description().isEmpty() )
    return;

  QgsModelComponentGraphicItem *commentItem = createCommentGraphicItem( model, component.comment()->clone(), parentItem );
  commentItem->setPos( component.comment()->position().x(), component.comment()->position().y() );
  addItem( commentItem );
  connect( commentItem, &QgsModelComponentGraphicItem::requestModelRepaint, this, &QgsModelGraphicsScene::rebuildRequired );
  connect( commentItem, &QgsModelComponentGraphicItem::changed, this, &QgsModelGraphicsScene::componentChanged );
  connect( commentItem, &QgsModelComponentGraphicItem::aboutToChange, this, &QgsModelGraphicsScene::componentAboutToChange );

  std::unique_ptr< QgsModelArrowItem > arrow = std::make_unique< QgsModelArrowItem >( parentItem, QgsModelArrowItem::Circle, commentItem, QgsModelArrowItem::Circle );
  arrow->setPenStyle( Qt::DotLine );
  addItem( arrow.release() );
}

QgsMessageBar *QgsModelGraphicsScene::messageBar() const
{
  return mMessageBar;
}

void QgsModelGraphicsScene::setMessageBar( QgsMessageBar *messageBar )
{
  mMessageBar = messageBar;
}

void QgsModelGraphicsScene::showWarning( const QString &shortMessage, const QString &title, const QString &longMessage, Qgis::MessageLevel level ) const
{
  QgsMessageBarItem *messageWidget = QgsMessageBar::createMessage( QString(), shortMessage );
  QPushButton *detailsButton = new QPushButton( tr( "Details" ) );
  connect( detailsButton, &QPushButton::clicked, detailsButton, [ = ]
  {
    QgsMessageViewer *dialog = new QgsMessageViewer( detailsButton );
    dialog->setTitle( title );
    dialog->setMessage( longMessage, QgsMessageOutput::MessageHtml );
    dialog->showMessage();
  } );
  messageWidget->layout()->addWidget( detailsButton );
  mMessageBar->clearWidgets();
  mMessageBar->pushWidget( messageWidget, level, 0 );
}

///@endcond
