/***************************************************************************
  qgsprocessingaggregatewidgetwrapper.cpp
  ---------------------
  Date                 : June 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingaggregatewidgetwrapper.h"

#include <QBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QToolButton>
#include <QItemSelectionModel>

#include "qgspanelwidget.h"

#include "qgsprocessingcontext.h"
#include "qgsprocessingmodelalgorithm.h"

#include "qgsprocessingparameteraggregate.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfieldexpressionwidget.h"

/// @cond private


//
// QgsProcessingAggregatePanelWidget
//


QgsProcessingAggregatePanelWidget::QgsProcessingAggregatePanelWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );

  mModel = mFieldsView->model();

  mLayerCombo->setAllowEmptyLayer( true );
  mLayerCombo->setFilters( QgsMapLayerProxyModel::VectorLayer );

  connect( mResetButton, &QPushButton::clicked, this, &QgsProcessingAggregatePanelWidget::loadFieldsFromLayer );
  connect( mAddButton, &QPushButton::clicked, this, &QgsProcessingAggregatePanelWidget::addField );
  connect( mDeleteButton, &QPushButton::clicked, mFieldsView, &QgsAggregateMappingWidget::removeSelectedFields );
  connect( mUpButton, &QPushButton::clicked, mFieldsView, &QgsAggregateMappingWidget::moveSelectedFieldsUp );
  connect( mDownButton, &QPushButton::clicked, mFieldsView, &QgsAggregateMappingWidget::moveSelectedFieldsDown );
  connect( mLoadLayerFieldsButton, &QPushButton::clicked, this, &QgsProcessingAggregatePanelWidget::loadLayerFields );

  connect( mFieldsView, &QgsAggregateMappingWidget::changed, this, [ = ]
  {
    if ( !mBlockChangedSignal )
    {
      emit changed();
    }
  } );
}

void QgsProcessingAggregatePanelWidget::setLayer( QgsVectorLayer *layer )
{
  if ( layer == mLayer )
    return;

  mLayer = layer;
  mFieldsView->setSourceLayer( mLayer );
  if ( mModel->rowCount() == 0 )
  {
    loadFieldsFromLayer();
    return;
  }

  QMessageBox dlg( this );
  dlg.setText( tr( "Do you want to reset the field mapping?" ) );
  dlg.setStandardButtons(
    QMessageBox::StandardButtons( QMessageBox::Yes |
                                  QMessageBox::No ) );
  dlg.setDefaultButton( QMessageBox::No );
  if ( dlg.exec() == QMessageBox::Yes )
  {
    loadFieldsFromLayer();
  }
}

QgsVectorLayer *QgsProcessingAggregatePanelWidget::layer()
{
  return mLayer;
}

QVariant QgsProcessingAggregatePanelWidget::value() const
{
  const QList<QgsAggregateMappingModel::Aggregate> mapping = mFieldsView->mapping();

  QVariantList results;
  results.reserve( mapping.size() );
  for ( const QgsAggregateMappingModel::Aggregate &aggregate : mapping )
  {
    QVariantMap def;
    def.insert( QStringLiteral( "name" ), aggregate.field.name() );
    def.insert( QStringLiteral( "type" ), static_cast< int >( aggregate.field.type() ) );
    def.insert( QStringLiteral( "type_name" ), aggregate.field.typeName() );
    def.insert( QStringLiteral( "length" ), aggregate.field.length() );
    def.insert( QStringLiteral( "precision" ), aggregate.field.precision() );
    def.insert( QStringLiteral( "sub_type" ), static_cast< int >( aggregate.field.subType() ) );
    def.insert( QStringLiteral( "input" ), aggregate.source );
    def.insert( QStringLiteral( "aggregate" ), aggregate.aggregate );
    def.insert( QStringLiteral( "delimiter" ), aggregate.delimiter );
    results.append( def );
  }
  return results;
}

void QgsProcessingAggregatePanelWidget::setValue( const QVariant &value )
{
  if ( value.type() != QVariant::List )
    return;

  QList< QgsAggregateMappingModel::Aggregate > aggregates;

  const QVariantList fields = value.toList();
  aggregates.reserve( fields.size() );
  for ( const QVariant &field : fields )
  {
    const QVariantMap map = field.toMap();
    const QgsField f( map.value( QStringLiteral( "name" ) ).toString(),
                      static_cast< QVariant::Type >( map.value( QStringLiteral( "type" ), QVariant::Invalid ).toInt() ),
                      map.value( QStringLiteral( "type_name" ), QVariant::typeToName( static_cast< QVariant::Type >( map.value( QStringLiteral( "type" ), QVariant::Invalid ).toInt() ) ) ).toString(),
                      map.value( QStringLiteral( "length" ), 0 ).toInt(),
                      map.value( QStringLiteral( "precision" ), 0 ).toInt(),
                      QString(),
                      static_cast< QVariant::Type >( map.value( QStringLiteral( "sub_type" ), QVariant::Invalid ).toInt() ) );

    QgsAggregateMappingModel::Aggregate aggregate;
    aggregate.field = f;

    aggregate.source = map.value( QStringLiteral( "input" ) ).toString();
    aggregate.aggregate = map.value( QStringLiteral( "aggregate" ) ).toString();
    aggregate.delimiter = map.value( QStringLiteral( "delimiter" ) ).toString();

    aggregates.append( aggregate );
  }

  mBlockChangedSignal = true;

  if ( aggregates.size() > 0 )
    mFieldsView->setMapping( aggregates );

  mBlockChangedSignal = false;

  emit changed();
}

void QgsProcessingAggregatePanelWidget::registerExpressionContextGenerator( const QgsExpressionContextGenerator *generator )
{
  mFieldsView->registerExpressionContextGenerator( generator );
}

void QgsProcessingAggregatePanelWidget::loadFieldsFromLayer()
{
  if ( mLayer )
  {
    mFieldsView->setSourceFields( mLayer->fields() );
  }
}

void QgsProcessingAggregatePanelWidget::addField()
{
  const int rowCount = mModel->rowCount();
  mModel->appendField( QgsField( QStringLiteral( "new_field" ) ) );
  const QModelIndex index = mModel->index( rowCount, 0 );
  mFieldsView->selectionModel()->select(
    index,
    QItemSelectionModel::SelectionFlags(
      QItemSelectionModel::Clear |
      QItemSelectionModel::Select |
      QItemSelectionModel::Current |
      QItemSelectionModel::Rows ) );
  mFieldsView->scrollTo( index );
}

void QgsProcessingAggregatePanelWidget::loadLayerFields()
{
  if ( QgsVectorLayer *vl = qobject_cast< QgsVectorLayer * >( mLayerCombo->currentLayer() ) )
  {
    mFieldsView->setSourceFields( vl->fields() );
  }
}

//
// QgsProcessingAggregateParameterDefinitionWidget
//

QgsProcessingAggregateParameterDefinitionWidget::QgsProcessingAggregateParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Parent layer" ) ) );

  mParentLayerComboBox = new QComboBox();
  mParentLayerComboBox->addItem( tr( "None" ), QVariant() );

  QString initialParent;
  if ( const QgsProcessingParameterAggregate *aggregateParam = dynamic_cast<const QgsProcessingParameterAggregate *>( definition ) )
    initialParent = aggregateParam->parentLayerParameterName();

  if ( auto *lModel = widgetContext.model() )
  {
    // populate combo box with other model input choices
    const QMap<QString, QgsProcessingModelParameter> components = lModel->parameterComponents();
    for ( auto it = components.constBegin(); it != components.constEnd(); ++it )
    {
      if ( const QgsProcessingParameterFeatureSource *definition = dynamic_cast< const QgsProcessingParameterFeatureSource * >( lModel->parameterDefinition( it.value().parameterName() ) ) )
      {
        mParentLayerComboBox-> addItem( definition->description(), definition->name() );
        if ( !initialParent.isEmpty() && initialParent == definition->name() )
        {
          mParentLayerComboBox->setCurrentIndex( mParentLayerComboBox->count() - 1 );
        }
      }
      else if ( const QgsProcessingParameterVectorLayer *definition = dynamic_cast< const QgsProcessingParameterVectorLayer * >( lModel->parameterDefinition( it.value().parameterName() ) ) )
      {
        mParentLayerComboBox-> addItem( definition->description(), definition->name() );
        if ( !initialParent.isEmpty() && initialParent == definition->name() )
        {
          mParentLayerComboBox->setCurrentIndex( mParentLayerComboBox->count() - 1 );
        }
      }
    }
  }

  if ( mParentLayerComboBox->count() == 1 && !initialParent.isEmpty() )
  {
    // if no parent candidates found, we just add the existing one as a placeholder
    mParentLayerComboBox->addItem( initialParent, initialParent );
    mParentLayerComboBox->setCurrentIndex( mParentLayerComboBox->count() - 1 );
  }

  vlayout->addWidget( mParentLayerComboBox );
  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingAggregateParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  auto param = std::make_unique< QgsProcessingParameterAggregate >( name, description, mParentLayerComboBox->currentData().toString() );
  param->setFlags( flags );
  return param.release();
}

//
// QgsProcessingAggregateWidgetWrapper
//

QgsProcessingAggregateWidgetWrapper::QgsProcessingAggregateWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{
}

QString QgsProcessingAggregateWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterAggregate::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingAggregateWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingAggregateWidgetWrapper( parameter, type );
}

QWidget *QgsProcessingAggregateWidgetWrapper::createWidget()
{
  mPanel = new QgsProcessingAggregatePanelWidget( nullptr );
  mPanel->setToolTip( parameterDefinition()->toolTip() );
  mPanel->registerExpressionContextGenerator( this );

  connect( mPanel, &QgsProcessingAggregatePanelWidget::changed, this, [ = ]
  {
    emit widgetValueHasChanged( this );
  } );

  return mPanel;
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingAggregateWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingAggregateParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}

void QgsProcessingAggregateWidgetWrapper::postInitialize( const QList<QgsAbstractProcessingParameterWidgetWrapper *> &wrappers )
{
  QgsAbstractProcessingParameterWidgetWrapper::postInitialize( wrappers );
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
    {
      for ( const QgsAbstractProcessingParameterWidgetWrapper *wrapper : wrappers )
      {
        if ( wrapper->parameterDefinition()->name() == static_cast< const QgsProcessingParameterAggregate * >( parameterDefinition() )->parentLayerParameterName() )
        {
          setParentLayerWrapperValue( wrapper );
          connect( wrapper, &QgsAbstractProcessingParameterWidgetWrapper::widgetValueHasChanged, this, [ = ]
          {
            setParentLayerWrapperValue( wrapper );
          } );
          break;
        }
      }
      break;
    }

    case QgsProcessingGui::Modeler:
      break;
  }
}

int QgsProcessingAggregateWidgetWrapper::stretch() const
{
  return 1;
}

void QgsProcessingAggregateWidgetWrapper::setParentLayerWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *parentWrapper )
{
  // evaluate value to layer
  QgsProcessingContext *context = nullptr;
  std::unique_ptr< QgsProcessingContext > tmpContext;
  if ( mProcessingContextGenerator )
    context = mProcessingContextGenerator->processingContext();

  if ( !context )
  {
    tmpContext = std::make_unique< QgsProcessingContext >();
    context = tmpContext.get();
  }

  QgsVectorLayer *layer = QgsProcessingParameters::parameterAsVectorLayer( parentWrapper->parameterDefinition(), parentWrapper->parameterValue(), *context );
  if ( !layer )
  {
    if ( mPanel )
      mPanel->setLayer( nullptr );
    return;
  }

  // need to grab ownership of layer if required - otherwise layer may be deleted when context
  // goes out of scope
  std::unique_ptr< QgsMapLayer > ownedLayer( context->takeResultLayer( layer->id() ) );
  if ( ownedLayer && ownedLayer->type() == QgsMapLayerType::VectorLayer )
  {
    mParentLayer.reset( qobject_cast< QgsVectorLayer * >( ownedLayer.release() ) );
    layer = mParentLayer.get();
  }
  else
  {
    // don't need ownership of this layer - it wasn't owned by context (so e.g. is owned by the project)
  }

  if ( mPanel )
    mPanel->setLayer( layer );
}

void QgsProcessingAggregateWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext & )
{
  if ( mPanel )
    mPanel->setValue( value );
}

QVariant QgsProcessingAggregateWidgetWrapper::widgetValue() const
{
  return mPanel ? mPanel->value() : QVariant();
}

QStringList QgsProcessingAggregateWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterAggregate::typeName();
}

QStringList QgsProcessingAggregateWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList();
}

QString QgsProcessingAggregateWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "an array of map items, each containing a 'name', 'type', 'aggregate' and 'input' value (and optional 'length' and 'precision' values)." );
}

const QgsVectorLayer *QgsProcessingAggregateWidgetWrapper::linkedVectorLayer() const
{
  if ( mPanel && mPanel->layer() )
    return mPanel->layer();

  return QgsAbstractProcessingParameterWidgetWrapper::linkedVectorLayer();
}

/// @endcond


