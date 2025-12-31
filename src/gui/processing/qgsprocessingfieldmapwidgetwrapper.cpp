/***************************************************************************
  qgsprocessingfieldmapwidgetwrapper.cpp
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

#include "qgsprocessingfieldmapwidgetwrapper.h"

#include "qgspanelwidget.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingmodelalgorithm.h"
#include "qgsprocessingparameterfieldmap.h"

#include <QBoxLayout>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QToolButton>

#include "moc_qgsprocessingfieldmapwidgetwrapper.cpp"

/// @cond private

//
// QgsProcessingFieldMapPanelWidget
//


QgsProcessingFieldMapPanelWidget::QgsProcessingFieldMapPanelWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );

  mModel = mFieldsView->model();
  mFieldsView->setDestinationEditable( true );

  mLayerCombo->setAllowEmptyLayer( true );
  mLayerCombo->setFilters( Qgis::LayerFilter::VectorLayer );

  connect( mResetButton, &QPushButton::clicked, this, &QgsProcessingFieldMapPanelWidget::loadFieldsFromLayer );
  connect( mAddButton, &QPushButton::clicked, this, &QgsProcessingFieldMapPanelWidget::addField );
  connect( mDeleteButton, &QPushButton::clicked, mFieldsView, &QgsFieldMappingWidget::removeSelectedFields );
  connect( mUpButton, &QPushButton::clicked, mFieldsView, &QgsFieldMappingWidget::moveSelectedFieldsUp );
  connect( mDownButton, &QPushButton::clicked, mFieldsView, &QgsFieldMappingWidget::moveSelectedFieldsDown );
  connect( mInvertSelectionButton, &QPushButton::clicked, mFieldsView, &QgsFieldMappingWidget::invertSelection );
  connect( mLoadLayerFieldsButton, &QPushButton::clicked, this, &QgsProcessingFieldMapPanelWidget::loadLayerFields );


  connect( mFieldsView, &QgsFieldMappingWidget::changed, this, [this] {
    if ( !mBlockChangedSignal )
    {
      emit changed();
    }
  } );
}

void QgsProcessingFieldMapPanelWidget::setLayer( QgsVectorLayer *layer )
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

  if ( mSkipConfirmDialog )
  {
    return;
  }

  QMessageBox dlg( this );
  dlg.setText( tr( "Do you want to reset the field mapping?" ) );
  dlg.setStandardButtons(
    QMessageBox::StandardButtons( QMessageBox::Yes | QMessageBox::No )
  );
  dlg.setDefaultButton( QMessageBox::No );
  if ( dlg.exec() == QMessageBox::Yes )
  {
    loadFieldsFromLayer();
  }
}

QgsVectorLayer *QgsProcessingFieldMapPanelWidget::layer()
{
  return mLayer;
}

QVariant QgsProcessingFieldMapPanelWidget::value() const
{
  const QList<QgsFieldMappingModel::Field> mapping = mFieldsView->mapping();

  QVariantList results;
  results.reserve( mapping.size() );
  for ( const QgsFieldMappingModel::Field &field : mapping )
  {
    QVariantMap def;
    def.insert( u"name"_s, field.field.name() );
    def.insert( u"type"_s, static_cast<int>( field.field.type() ) );
    def.insert( u"type_name"_s, field.field.typeName() );
    def.insert( u"length"_s, field.field.length() );
    def.insert( u"precision"_s, field.field.precision() );
    def.insert( u"sub_type"_s, static_cast<int>( field.field.subType() ) );
    def.insert( u"expression"_s, field.expression );
    def.insert( u"alias"_s, field.field.alias() );
    def.insert( u"comment"_s, field.field.comment() );
    results.append( def );
  }
  return results;
}

void QgsProcessingFieldMapPanelWidget::setValue( const QVariant &value )
{
  if ( value.userType() != QMetaType::Type::QVariantList )
    return;

  QgsFields destinationFields;
  QMap<QString, QString> expressions;

  const QgsFields layerFields = mLayer ? mLayer->fields() : QgsFields();
  const QVariantList fields = value.toList();
  for ( const QVariant &field : fields )
  {
    const QVariantMap map = field.toMap();
    QgsField f( map.value( u"name"_s ).toString(), static_cast<QMetaType::Type>( map.value( u"type"_s, static_cast<int>( QMetaType::Type::UnknownType ) ).toInt() ), map.value( u"type_name"_s, QVariant::typeToName( static_cast<QMetaType::Type>( map.value( u"type"_s, static_cast<int>( QMetaType::Type::UnknownType ) ).toInt() ) ) ).toString(), map.value( u"length"_s, 0 ).toInt(), map.value( u"precision"_s, 0 ).toInt(), QString(), static_cast<QMetaType::Type>( map.value( u"sub_type"_s, QgsVariantUtils::createNullVariant( QMetaType::Type::UnknownType ) ).toInt() ) );
    f.setAlias( map.value( u"alias"_s ).toString() );
    f.setComment( map.value( u"comment"_s ).toString() );

    if ( map.contains( u"constraints"_s ) )
    {
      const QgsFieldConstraints::Constraints constraints = static_cast<QgsFieldConstraints::Constraints>( map.value( u"constraints"_s, 0 ).toInt() );
      QgsFieldConstraints fieldConstraints;

      if ( constraints & QgsFieldConstraints::ConstraintNotNull )
        fieldConstraints.setConstraint( QgsFieldConstraints::ConstraintNotNull );
      if ( constraints & QgsFieldConstraints::ConstraintUnique )
        fieldConstraints.setConstraint( QgsFieldConstraints::ConstraintUnique );
      if ( constraints & QgsFieldConstraints::ConstraintExpression )
        fieldConstraints.setConstraint( QgsFieldConstraints::ConstraintExpression );

      f.setConstraints( fieldConstraints );
    }

    if ( !map.value( u"expression"_s ).toString().isEmpty() )
    {
      expressions.insert( f.name(), map.value( u"expression"_s ).toString() );
    }

    destinationFields.append( f );
  }

  mBlockChangedSignal = true;
  if ( destinationFields.size() > 0 )
    mFieldsView->setDestinationFields( destinationFields, expressions );
  mBlockChangedSignal = false;

  emit changed();
}

void QgsProcessingFieldMapPanelWidget::registerExpressionContextGenerator( const QgsExpressionContextGenerator *generator )
{
  mFieldsView->registerExpressionContextGenerator( generator );
}

void QgsProcessingFieldMapPanelWidget::loadFieldsFromLayer()
{
  if ( mLayer )
  {
    mFieldsView->setSourceFields( mLayer->fields() );
    mFieldsView->setDestinationFields( mLayer->fields() );
  }
}

void QgsProcessingFieldMapPanelWidget::addField()
{
  const int rowCount = mModel->rowCount();
  mModel->appendField( QgsField( u"new_field"_s ) );
  const QModelIndex index = mModel->index( rowCount, 0 );
  mFieldsView->selectionModel()->select(
    index,
    QItemSelectionModel::SelectionFlags(
      QItemSelectionModel::Clear | QItemSelectionModel::Select | QItemSelectionModel::Current | QItemSelectionModel::Rows
    )
  );
  mFieldsView->scrollTo( index );
}

void QgsProcessingFieldMapPanelWidget::loadLayerFields()
{
  if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( mLayerCombo->currentLayer() ) )
  {
    mFieldsView->setDestinationFields( vl->fields() );
  }
}

//
// QgsProcessingFieldMapParameterDefinitionWidget
//

QgsProcessingFieldMapParameterDefinitionWidget::QgsProcessingFieldMapParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Parent layer" ) ) );

  mParentLayerComboBox = new QComboBox();
  mParentLayerComboBox->addItem( tr( "None" ), QVariant() );

  QString initialParent;
  if ( const QgsProcessingParameterFieldMapping *mapParam = dynamic_cast<const QgsProcessingParameterFieldMapping *>( definition ) )
    initialParent = mapParam->parentLayerParameterName();

  if ( auto *lModel = widgetContext.model() )
  {
    // populate combo box with other model input choices
    const QMap<QString, QgsProcessingModelParameter> components = lModel->parameterComponents();
    for ( auto it = components.constBegin(); it != components.constEnd(); ++it )
    {
      if ( const QgsProcessingParameterFeatureSource *definition = dynamic_cast<const QgsProcessingParameterFeatureSource *>( lModel->parameterDefinition( it.value().parameterName() ) ) )
      {
        mParentLayerComboBox->addItem( definition->description(), definition->name() );
        if ( !initialParent.isEmpty() && initialParent == definition->name() )
        {
          mParentLayerComboBox->setCurrentIndex( mParentLayerComboBox->count() - 1 );
        }
      }
      else if ( const QgsProcessingParameterVectorLayer *definition = dynamic_cast<const QgsProcessingParameterVectorLayer *>( lModel->parameterDefinition( it.value().parameterName() ) ) )
      {
        mParentLayerComboBox->addItem( definition->description(), definition->name() );
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

QgsProcessingParameterDefinition *QgsProcessingFieldMapParameterDefinitionWidget::createParameter( const QString &name, const QString &description, Qgis::ProcessingParameterFlags flags ) const
{
  auto param = std::make_unique<QgsProcessingParameterFieldMapping>( name, description, mParentLayerComboBox->currentData().toString() );
  param->setFlags( flags );
  return param.release();
}

//
// QgsProcessingFieldMapWidgetWrapper
//

QgsProcessingFieldMapWidgetWrapper::QgsProcessingFieldMapWidgetWrapper( const QgsProcessingParameterDefinition *parameter, Qgis::ProcessingMode type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{
}

QString QgsProcessingFieldMapWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterFieldMapping::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingFieldMapWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, Qgis::ProcessingMode type )
{
  return new QgsProcessingFieldMapWidgetWrapper( parameter, type );
}

QWidget *QgsProcessingFieldMapWidgetWrapper::createWidget()
{
  mPanel = new QgsProcessingFieldMapPanelWidget( nullptr );
  mPanel->setToolTip( parameterDefinition()->toolTip() );
  mPanel->registerExpressionContextGenerator( this );

  connect( mPanel, &QgsProcessingFieldMapPanelWidget::changed, this, [this] {
    emit widgetValueHasChanged( this );
  } );

  return mPanel;
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingFieldMapWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingFieldMapParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}

void QgsProcessingFieldMapWidgetWrapper::postInitialize( const QList<QgsAbstractProcessingParameterWidgetWrapper *> &wrappers )
{
  QgsAbstractProcessingParameterWidgetWrapper::postInitialize( wrappers );
  switch ( type() )
  {
    case Qgis::ProcessingMode::Standard:
    case Qgis::ProcessingMode::Batch:
    {
      for ( const QgsAbstractProcessingParameterWidgetWrapper *wrapper : wrappers )
      {
        if ( wrapper->parameterDefinition()->name() == static_cast<const QgsProcessingParameterFieldMapping *>( parameterDefinition() )->parentLayerParameterName() )
        {
          setParentLayerWrapperValue( wrapper );
          connect( wrapper, &QgsAbstractProcessingParameterWidgetWrapper::widgetValueHasChanged, this, [this, wrapper] {
            setParentLayerWrapperValue( wrapper );
          } );
          break;
        }
      }
      break;
    }

    case Qgis::ProcessingMode::Modeler:
      break;
  }
}

int QgsProcessingFieldMapWidgetWrapper::stretch() const
{
  return 1;
}

void QgsProcessingFieldMapWidgetWrapper::setParentLayerWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *parentWrapper )
{
  // evaluate value to layer
  QgsProcessingContext *context = nullptr;
  std::unique_ptr<QgsProcessingContext> tmpContext;
  if ( mProcessingContextGenerator )
    context = mProcessingContextGenerator->processingContext();

  if ( !context )
  {
    tmpContext = std::make_unique<QgsProcessingContext>();
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
  std::unique_ptr<QgsMapLayer> ownedLayer( context->takeResultLayer( layer->id() ) );
  if ( ownedLayer && ownedLayer->type() == Qgis::LayerType::Vector )
  {
    mParentLayer.reset( qobject_cast<QgsVectorLayer *>( ownedLayer.release() ) );
    layer = mParentLayer.get();
  }
  else
  {
    // don't need ownership of this layer - it wasn't owned by context (so e.g. is owned by the project)
  }

  if ( mPanel )
    mPanel->setLayer( layer );
}

void QgsProcessingFieldMapWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext & )
{
  if ( mPanel )
    mPanel->setValue( value );
}

QVariant QgsProcessingFieldMapWidgetWrapper::widgetValue() const
{
  return mPanel ? mPanel->value() : QVariant();
}

QString QgsProcessingFieldMapWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "an array of map items, each containing a 'name', 'type' and 'expression' values (and optional 'length' and 'precision' values)." );
}

const QgsVectorLayer *QgsProcessingFieldMapWidgetWrapper::linkedVectorLayer() const
{
  if ( mPanel && mPanel->layer() )
    return mPanel->layer();

  return QgsAbstractProcessingParameterWidgetWrapper::linkedVectorLayer();
}

/// @endcond
