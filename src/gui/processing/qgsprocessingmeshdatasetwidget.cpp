/***************************************************************************
  qgsprocessingmeshdatasetgroupswidget.h
  ---------------------
  Date                 : October 2020
  Copyright            : (C) 2020 by Vincent Cloarec
  Email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingmeshdatasetwidget.h"
#include "qgsdatetimeedit.h"
#include "qgsprocessingmultipleselectiondialog.h"
#include "qgsprocessingoutputs.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayerutils.h"
#include "qgsmeshlayertemporalproperties.h"
#include "qgspanelwidget.h"
#include "qgsmapcanvas.h"

#include <QLineEdit>
#include <QLabel>
#include <QMenu>
#include <QToolButton>
#include <QVBoxLayout>

/// @cond PRIVATE

QgsProcessingMeshDatasetGroupsWidget::QgsProcessingMeshDatasetGroupsWidget( QWidget *parent, const QgsProcessingParameterMeshDatasetGroups *param )
  : QWidget( parent ),
    mParam( param )
{
  QHBoxLayout *hl = new QHBoxLayout();
  hl->setContentsMargins( 0, 0, 0, 0 );

  mLineEdit = new QLineEdit();
  mLineEdit->setEnabled( false );
  hl->addWidget( mLineEdit, 1 );

  mToolButton = new QToolButton();
  mToolButton->setText( QString( QChar( 0x2026 ) ) );
  hl->addWidget( mToolButton );

  setLayout( hl );

  mLineEdit->setText( tr( "%1 dataset groups selected" ).arg( 0 ) );

  mToolButton->setPopupMode( QToolButton::InstantPopup );
  QMenu *toolButtonMenu = new QMenu( this );
  mActionCurrentActiveDatasetGroups = toolButtonMenu->addAction( tr( "Current Active Dataset Group" ) );
  connect( mActionCurrentActiveDatasetGroups,
           &QAction::triggered, this, &QgsProcessingMeshDatasetGroupsWidget::selectCurrentActiveDatasetGroup );

  mActionAvailableDatasetGroups = toolButtonMenu->addAction( tr( "Select in Available Dataset Groups" ) );
  connect( mActionAvailableDatasetGroups, &QAction::triggered, this, &QgsProcessingMeshDatasetGroupsWidget::showDialog );

  mToolButton->setMenu( toolButtonMenu );
}

void QgsProcessingMeshDatasetGroupsWidget::setMeshLayer( QgsMeshLayer *layer, bool layerFromProject )
{
  mActionCurrentActiveDatasetGroups->setEnabled( layer && layerFromProject );
  mActionAvailableDatasetGroups->setEnabled( layer );

  if ( mMeshLayer == layer )
    return;

  mDatasetGroupsNames.clear();

  if ( layerFromProject )
    mMeshLayer = layer;
  else
  {
    mMeshLayer = nullptr;
    if ( layer )
    {
      QList<int> datasetGroupsIndexes = layer->datasetGroupsIndexes();
      for ( int i : datasetGroupsIndexes )
      {
        QgsMeshDatasetGroupMetadata meta = layer->datasetGroupMetadata( i );
        if ( mParam->isDataTypeSupported( meta.dataType() ) )
        {
          mDatasetGroupsNames[i] = meta.name();
        }
      }
    }
  }
  mValue.clear();
  updateSummaryText();
  emit changed();
}

void QgsProcessingMeshDatasetGroupsWidget::setValue( const QVariant &value )
{
  if ( value.isValid() )
    mValue = value.type() == QVariant::List ? value.toList() : QVariantList() << value;
  else
    mValue.clear();

  updateSummaryText();
  emit changed();
}

QVariant QgsProcessingMeshDatasetGroupsWidget::value() const
{
  return mValue;
}

void QgsProcessingMeshDatasetGroupsWidget::showDialog()
{
  QList<int> datasetGroupsIndexes;
  QStringList options;
  QVariantList availableOptions;
  if ( mMeshLayer )
  {
    datasetGroupsIndexes = mMeshLayer->datasetGroupsIndexes();
    for ( int i : std::as_const( datasetGroupsIndexes ) )
    {
      QgsMeshDatasetGroupMetadata meta = mMeshLayer->datasetGroupMetadata( i );
      if ( mParam->isDataTypeSupported( meta.dataType() ) )
      {
        availableOptions.append( i );
        options.append( meta.name() );
      }

    }
  }
  else
  {
    for ( int i : mDatasetGroupsNames.keys() )
    {
      availableOptions.append( i );
      options.append( mDatasetGroupsNames.value( i ) );
    }
  }

  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsProcessingMultipleSelectionPanelWidget *widget = new QgsProcessingMultipleSelectionPanelWidget( availableOptions, mValue );
    widget->setPanelTitle( tr( "Dataset Groups Available" ) );

    widget->setValueFormatter( [availableOptions, options]( const QVariant & v ) -> QString
    {
      const int index = v.toInt();
      const int pos = availableOptions.indexOf( index );
      return ( pos >= 0 && pos < options.size() ) ? options.at( pos ) : QString();
    } );

    connect( widget, &QgsProcessingMultipleSelectionPanelWidget::selectionChanged, this, [ = ]()
    {
      setValue( widget->selectedOptions() );
    } );
    connect( widget, &QgsProcessingMultipleSelectionPanelWidget::acceptClicked, widget, &QgsPanelWidget::acceptPanel );
    panel->openPanel( widget );
  }
  else
  {
    QgsProcessingMultipleSelectionDialog dlg( availableOptions, mValue, this, Qt::WindowFlags() );

    dlg.setValueFormatter( [datasetGroupsIndexes, options]( const QVariant & v ) -> QString
    {
      const int index = v.toInt();
      const int pos = datasetGroupsIndexes.indexOf( index );
      return ( pos >= 0 && pos < options.size() ) ? options.at( pos ) : QString();
    } );
    if ( dlg.exec() )
    {
      setValue( dlg.selectedOptions() );
    }
  }
}

void QgsProcessingMeshDatasetGroupsWidget::selectCurrentActiveDatasetGroup()
{
  QVariantList options;
  if ( mMeshLayer && mParam )
  {
    int scalarDatasetGroup = mMeshLayer->rendererSettings().activeScalarDatasetGroup();
    int vectorDatasetGroup = mMeshLayer->rendererSettings().activeVectorDatasetGroup();

    if ( scalarDatasetGroup >= 0 && mParam->isDataTypeSupported( mMeshLayer->datasetGroupMetadata( scalarDatasetGroup ).dataType() ) )
      options.append( scalarDatasetGroup );

    if ( vectorDatasetGroup >= 0
         && mParam->isDataTypeSupported( mMeshLayer->datasetGroupMetadata( vectorDatasetGroup ).dataType() )
         && vectorDatasetGroup != scalarDatasetGroup )
      options.append( vectorDatasetGroup );
  }

  setValue( options );
}

QgsProcessingMeshDatasetGroupsWidgetWrapper::QgsProcessingMeshDatasetGroupsWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent ):
  QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{}

QString QgsProcessingMeshDatasetGroupsWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterMeshDatasetGroups::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingMeshDatasetGroupsWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingMeshDatasetGroupsWidgetWrapper( parameter, type );
}

void QgsProcessingMeshDatasetGroupsWidgetWrapper::postInitialize( const QList<QgsAbstractProcessingParameterWidgetWrapper *> &wrappers )
{
  QgsAbstractProcessingParameterWidgetWrapper::postInitialize( wrappers ); //necessary here?
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
    {
      for ( const QgsAbstractProcessingParameterWidgetWrapper *wrapper : wrappers )
      {
        if ( wrapper->parameterDefinition()->name() == static_cast< const QgsProcessingParameterMeshDatasetGroups * >( parameterDefinition() )->meshLayerParameterName() )
        {
          setMeshLayerWrapperValue( wrapper );
          connect( wrapper, &QgsAbstractProcessingParameterWidgetWrapper::widgetValueHasChanged, this, [ = ]
          {
            setMeshLayerWrapperValue( wrapper );
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

void QgsProcessingMeshDatasetGroupsWidgetWrapper::setMeshLayerWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *wrapper )
{
  if ( ! mWidget )
    return;

  // evaluate value to layer
  QgsProcessingContext *context = nullptr;
  if ( mProcessingContextGenerator )
    context = mProcessingContextGenerator->processingContext();

  bool layerFromProject;
  QgsMeshLayer *meshLayer;
  if ( !context )
  {
    QgsProcessingContext dummyContext;
    meshLayer = QgsProcessingParameters::parameterAsMeshLayer( wrapper->parameterDefinition(), wrapper->parameterValue(), dummyContext );
    layerFromProject = false;
  }
  else
  {
    meshLayer = QgsProcessingParameters::parameterAsMeshLayer( wrapper->parameterDefinition(), wrapper->parameterValue(), *context );
    layerFromProject = context->project() && context->project()->layerStore()->layers<QgsMeshLayer *>().contains( meshLayer );
  }

  if ( mWidget )
    mWidget->setMeshLayer( meshLayer, layerFromProject );
}

QStringList QgsProcessingMeshDatasetGroupsWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList() << QgsProcessingParameterMeshDatasetGroups::typeName()
         << QgsProcessingParameterString::typeName()
         << QgsProcessingParameterNumber::typeName();
}

QStringList QgsProcessingMeshDatasetGroupsWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList() << QgsProcessingOutputString::typeName()
         << QgsProcessingOutputNumber::typeName();
}

QWidget *QgsProcessingMeshDatasetGroupsWidgetWrapper::createWidget() SIP_FACTORY
{
  mWidget = new QgsProcessingMeshDatasetGroupsWidget( nullptr, static_cast<const QgsProcessingParameterMeshDatasetGroups *>( parameterDefinition() ) );
  connect( mWidget, &QgsProcessingMeshDatasetGroupsWidget::changed, this, [ = ]
  {
    emit widgetValueHasChanged( this );
  } );

  return mWidget;
}

void QgsProcessingMeshDatasetGroupsWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  if ( !mWidget )
    return;

  QList<int> datasetGroupIndexes;
  if ( value.type() == QVariant::List )
  {
    //here we can't use  QgsProcessingParameters::parameterAsInts() because this method return empry list when first value is 0...
    datasetGroupIndexes = QgsProcessingParameters::parameterAsInts( parameterDefinition(), value, context );
  }
  else
    datasetGroupIndexes.append( QgsProcessingParameters::parameterAsInt( parameterDefinition(), value, context ) );

  QVariantList varList;
  for ( const int index : std::as_const( datasetGroupIndexes ) )
    varList.append( index );

  mWidget->setValue( varList );
}

QVariant QgsProcessingMeshDatasetGroupsWidgetWrapper::widgetValue() const
{
  if ( mWidget )
    return mWidget->value();
  return QVariant();
}


void QgsProcessingMeshDatasetGroupsWidget::updateSummaryText()
{
  mLineEdit->setText( tr( "%n option(s) selected", nullptr, mValue.count() ) );
}

QgsProcessingMeshDatasetTimeWidgetWrapper::QgsProcessingMeshDatasetTimeWidgetWrapper( const QgsProcessingParameterDefinition *parameter,
    QgsProcessingGui::WidgetType type,
    QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

QString QgsProcessingMeshDatasetTimeWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterMeshDatasetTime::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingMeshDatasetTimeWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingMeshDatasetTimeWidgetWrapper( parameter, type );
}

void QgsProcessingMeshDatasetTimeWidgetWrapper::postInitialize( const QList<QgsAbstractProcessingParameterWidgetWrapper *> &wrappers )
{
  QgsAbstractProcessingParameterWidgetWrapper::postInitialize( wrappers ); //necessary here?
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
    {
      const QgsAbstractProcessingParameterWidgetWrapper *layerParameterWrapper = nullptr;
      const QgsAbstractProcessingParameterWidgetWrapper *datasetGroupsParameterWrapper = nullptr;
      for ( const QgsAbstractProcessingParameterWidgetWrapper *wrapper : wrappers )
      {
        if ( wrapper->parameterDefinition()->name() == static_cast< const QgsProcessingParameterMeshDatasetTime * >( parameterDefinition() )->meshLayerParameterName() )
          layerParameterWrapper = wrapper;

        if ( wrapper->parameterDefinition()->name() == static_cast< const QgsProcessingParameterMeshDatasetTime * >( parameterDefinition() )->datasetGroupParameterName() )
          datasetGroupsParameterWrapper = wrapper;
      }
      setMeshLayerWrapperValue( layerParameterWrapper );
      setDatasetGroupIndexesWrapperValue( datasetGroupsParameterWrapper );
      connect( datasetGroupsParameterWrapper, &QgsAbstractProcessingParameterWidgetWrapper::widgetValueHasChanged, this, [ = ]
      {
        setMeshLayerWrapperValue( layerParameterWrapper );
        setDatasetGroupIndexesWrapperValue( datasetGroupsParameterWrapper );
      } );

      break;
    }

    case QgsProcessingGui::Modeler:
      break;
  }
}

void QgsProcessingMeshDatasetTimeWidgetWrapper::setMeshLayerWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *wrapper )
{
  if ( !mWidget || !wrapper )
    return;

  // evaluate value to layer
  QgsProcessingContext *context = nullptr;
  if ( mProcessingContextGenerator )
    context = mProcessingContextGenerator->processingContext();

  bool layerFromProject;
  QgsMeshLayer *meshLayer;
  if ( !context )
  {
    QgsProcessingContext dummyContext;
    meshLayer = QgsProcessingParameters::parameterAsMeshLayer( wrapper->parameterDefinition(), wrapper->parameterValue(), dummyContext );
    layerFromProject = false;
  }
  else
  {
    meshLayer = QgsProcessingParameters::parameterAsMeshLayer( wrapper->parameterDefinition(), wrapper->parameterValue(), *context );
    layerFromProject = context->project() && context->project()->layerStore()->layers<QgsMeshLayer *>().contains( meshLayer );
  }

  mWidget->setMeshLayer( meshLayer, layerFromProject );
}

void QgsProcessingMeshDatasetTimeWidgetWrapper::setDatasetGroupIndexesWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *wrapper )
{
  if ( !mWidget || !wrapper )
    return;

  QVariant datasetGroupsVariant = wrapper->parameterValue();

  if ( !datasetGroupsVariant.isValid() || datasetGroupsVariant.type() != QVariant::List )
    mWidget->setDatasetGroupIndexes( QList<int>() );

  QVariantList datasetGroupsListVariant = datasetGroupsVariant.toList();

  QList<int> datasetGroupsIndexes;
  for ( const QVariant &variantIndex : datasetGroupsListVariant )
    datasetGroupsIndexes << variantIndex.toInt();

  mWidget->setDatasetGroupIndexes( datasetGroupsIndexes );

}

QStringList QgsProcessingMeshDatasetTimeWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterMeshDatasetTime::typeName()
         << QgsProcessingParameterString::typeName()
         << QgsProcessingParameterDateTime::typeName();
}

QStringList QgsProcessingMeshDatasetTimeWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName();
}

QWidget *QgsProcessingMeshDatasetTimeWidgetWrapper::createWidget()
{
  mWidget = new QgsProcessingMeshDatasetTimeWidget( nullptr, static_cast<const QgsProcessingParameterMeshDatasetTime *>( parameterDefinition() ), widgetContext() );

  QgsMapCanvas *canvas = widgetContext().mapCanvas();
  if ( canvas )
  {
    connect( canvas, &QgsMapCanvas::temporalRangeChanged, mWidget, &QgsProcessingMeshDatasetTimeWidget::updateValue );
  }
  connect( mWidget, &QgsProcessingMeshDatasetTimeWidget::changed, this, [ = ]
  {
    emit widgetValueHasChanged( this );
  } );

  return mWidget;
}

void QgsProcessingMeshDatasetTimeWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  Q_UNUSED( context );
  if ( mWidget )
    mWidget->setValue( value );
}

QVariant QgsProcessingMeshDatasetTimeWidgetWrapper::widgetValue() const
{
  if ( mWidget )
    return mWidget->value();
  return QVariant();
}

QgsProcessingMeshDatasetTimeWidget::QgsProcessingMeshDatasetTimeWidget( QWidget *parent,
    const QgsProcessingParameterMeshDatasetTime *param,
    const QgsProcessingParameterWidgetContext &context ):
  QWidget( parent ),
  mParam( param )
{
  setupUi( this );

  mValue.insert( QStringLiteral( "type" ), QStringLiteral( "static" ) );

  dateTimeEdit->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );

  mCanvas = context.mapCanvas();

  connect( radioButtonCurrentCanvasTime, &QRadioButton::toggled, [this]( bool isChecked ) {if ( isChecked ) this->updateValue();} );
  connect( radioButtonDefinedDateTime, &QRadioButton::toggled, [this]( bool isChecked ) {if ( isChecked ) this->updateValue();} );
  connect( radioButtonDatasetGroupTimeStep, &QRadioButton::toggled, [this]( bool isChecked ) {if ( isChecked ) this->updateValue();} );
  connect( dateTimeEdit, &QgsDateTimeEdit::dateTimeChanged, this, &QgsProcessingMeshDatasetTimeWidget::updateValue );
  connect( comboBoxDatasetTimeStep, qOverload<int>( &QComboBox::currentIndexChanged ),
           this, &QgsProcessingMeshDatasetTimeWidget::updateValue );

  updateWidget();
}

void QgsProcessingMeshDatasetTimeWidget::setMeshLayer( QgsMeshLayer *layer, bool layerFromProject )
{
  if ( mMeshLayer == layer )
    return;

  mReferenceTime = QDateTime();

  if ( layerFromProject )
  {
    mMeshLayer = layer;
    mReferenceTime = static_cast<QgsMeshLayerTemporalProperties *>( layer->temporalProperties() )->referenceTime();
  }
  else
  {
    mMeshLayer = nullptr;
    if ( layer )
      mReferenceTime = layer->dataProvider()->temporalCapabilities()->referenceTime();
    storeTimeStepsFromLayer( layer );
  }

  if ( mReferenceTime.isValid() )
    whileBlocking( dateTimeEdit )->setDateTime( mReferenceTime );

  updateValue();
}

void QgsProcessingMeshDatasetTimeWidget::setDatasetGroupIndexes( const QList<int> datasetGroupIndexes )
{
  if ( datasetGroupIndexes == mDatasetGroupIndexes )
    return;
  mDatasetGroupIndexes = datasetGroupIndexes;
  populateTimeSteps();
  updateValue();
}

void QgsProcessingMeshDatasetTimeWidget::setValue( const QVariant &value )
{
  if ( !value.isValid() || ( value.type() != QVariant::Map && !value.toDateTime().isValid() ) )
    return;

  mValue.clear();
  if ( value.toDateTime().isValid() )
  {
    QDateTime dateTime = value.toDateTime();
    dateTime.setTimeSpec( Qt::UTC );
    mValue.insert( QStringLiteral( "type" ), QStringLiteral( "defined-date-time" ) );
    mValue.insert( QStringLiteral( "value" ), dateTime );
  }
  else
    mValue = value.toMap();

  if ( !mValue.contains( QStringLiteral( "type" ) ) || !mValue.contains( QStringLiteral( "value" ) ) )
    return;

  QString type = mValue.value( QStringLiteral( "type" ) ).toString();

  setEnabled( true );
  if ( type == QLatin1String( "static" ) )
  {
    setEnabled( false );
  }
  else if ( type == QLatin1String( "dataset-time-step" ) )
  {
    QVariantList dataset = mValue.value( QStringLiteral( "value" ) ).toList();
    whileBlocking( comboBoxDatasetTimeStep )->setCurrentIndex( comboBoxDatasetTimeStep->findData( dataset ) );
    whileBlocking( radioButtonDatasetGroupTimeStep )->setChecked( true );
  }
  else if ( type == QLatin1String( "defined-date-time" ) )
  {
    whileBlocking( dateTimeEdit )->setDateTime( mValue.value( QStringLiteral( "value" ) ).toDateTime() );
    whileBlocking( radioButtonDefinedDateTime )->setChecked( true );
  }
  else if ( type == QLatin1String( "current-context-time" ) )
  {
    whileBlocking( radioButtonCurrentCanvasTime )->setChecked( true );
  }

  emit changed();
  updateWidget();
}

QVariant QgsProcessingMeshDatasetTimeWidget::value() const
{
  return mValue;
}

void QgsProcessingMeshDatasetTimeWidget::updateWidget()
{
  bool isStatic = !hasTemporalDataset();
  setEnabled( !isStatic );

  if ( mCanvas != nullptr  && mCanvas->mapSettings().isTemporal() )
  {
    whileBlocking( radioButtonCurrentCanvasTime )->setEnabled( true && mReferenceTime.isValid() );
    labelCurrentCanvasTime->setText( mCanvas->mapSettings().temporalRange().begin().toString( "yyyy-MM-dd HH:mm:ss" ) );
  }
  else
  {
    whileBlocking( radioButtonCurrentCanvasTime )->setEnabled( false );
    if ( radioButtonCurrentCanvasTime->isChecked() )
      whileBlocking( radioButtonDefinedDateTime )->setChecked( true );
  }

  if ( ! mReferenceTime.isValid() )
    whileBlocking( radioButtonDatasetGroupTimeStep )->setChecked( true );

  whileBlocking( radioButtonDefinedDateTime )->setEnabled( mReferenceTime.isValid() );

  dateTimeEdit->setVisible( radioButtonDefinedDateTime->isChecked() && !isStatic );
  labelCurrentCanvasTime->setVisible( radioButtonCurrentCanvasTime->isChecked() && !isStatic );
  comboBoxDatasetTimeStep->setVisible( radioButtonDatasetGroupTimeStep->isChecked() && !isStatic );
}

bool QgsProcessingMeshDatasetTimeWidget::hasTemporalDataset() const
{
  for ( int index : mDatasetGroupIndexes )
  {
    if ( mMeshLayer && mMeshLayer->datasetGroupMetadata( index ).isTemporal() )
      return true;
    else if ( mDatasetTimeSteps.contains( index ) )
      return true;
  }

  return false;
}


void QgsProcessingMeshDatasetTimeWidget::populateTimeSteps()
{
  if ( mMeshLayer )
  {
    populateTimeStepsFromLayer();
    return;
  }

  QMap<quint64, QgsMeshDatasetIndex> timeStep;
  for ( int groupIndex : mDatasetGroupIndexes )
  {
    if ( !mDatasetTimeSteps.contains( groupIndex ) )
      continue;
    const QList<qint64> relativeTimeSteps = mDatasetTimeSteps.value( groupIndex );
    for ( int index = 0; index < relativeTimeSteps.count(); ++index )
    {
      QgsMeshDatasetIndex datasetIndex( groupIndex, index );
      if ( timeStep.contains( relativeTimeSteps.at( index ) ) )
        continue;
      timeStep[relativeTimeSteps.at( index )] = datasetIndex;
    }
  }

  for ( qint64 key : timeStep.keys() )
  {
    QString stringTime = QgsMeshLayerUtils::formatTime( key / 1000 / 3600, mReferenceTime, QgsMeshTimeSettings() );
    QVariantList data;
    const QgsMeshDatasetIndex &index = timeStep.value( key );
    data << index.group() << index.dataset();
    whileBlocking( comboBoxDatasetTimeStep )->addItem( stringTime, data );
  }

}

void QgsProcessingMeshDatasetTimeWidget::populateTimeStepsFromLayer()
{
  whileBlocking( comboBoxDatasetTimeStep )->clear();

  if ( !mMeshLayer )
    return;

  QMap<quint64, QgsMeshDatasetIndex> timeStep;
  for ( int groupIndex : std::as_const( mDatasetGroupIndexes ) )
  {
    QgsMeshDatasetGroupMetadata meta = mMeshLayer->datasetGroupMetadata( groupIndex );
    if ( !meta.isTemporal() )
      continue;
    int datasetCount = mMeshLayer->datasetCount( groupIndex );

    for ( int index = 0; index < datasetCount; ++index )
    {
      QgsMeshDatasetIndex datasetIndex( groupIndex, index );
      qint64 relativeTime = mMeshLayer->datasetRelativeTimeInMilliseconds( datasetIndex );
      if ( timeStep.contains( relativeTime ) )
        continue;
      timeStep[relativeTime] = datasetIndex;
    }
  }

  for ( qint64 key : timeStep.keys() )
  {
    QString stringTime = mMeshLayer->formatTime( key / 1000.0 / 3600.0 );
    QVariantList data;
    const QgsMeshDatasetIndex &index = timeStep.value( key );
    data << index.group() << index.dataset();
    whileBlocking( comboBoxDatasetTimeStep )->addItem( stringTime, data );
  }
}

void QgsProcessingMeshDatasetTimeWidget::storeTimeStepsFromLayer( QgsMeshLayer *layer )
{
  mDatasetTimeSteps.clear();
  if ( !layer )
    return;
  QList<int> datasetGroupsList = layer->datasetGroupsIndexes();
  for ( int groupIndex : datasetGroupsList )
  {
    QgsMeshDatasetGroupMetadata meta = layer->datasetGroupMetadata( groupIndex );
    if ( !meta.isTemporal() )
      continue;
    int datasetCount = layer->datasetCount( groupIndex );
    QList<qint64> relativeTimeSteps;
    relativeTimeSteps.reserve( datasetCount );
    for ( int index = 0; index < datasetCount; ++index )
      relativeTimeSteps.append( layer->datasetRelativeTimeInMilliseconds( QgsMeshDatasetIndex( groupIndex, index ) ) );
    mDatasetTimeSteps[groupIndex] = relativeTimeSteps;
  }
}

void QgsProcessingMeshDatasetTimeWidget::buildValue()
{
  mValue.clear();

  if ( !isEnabled() )
  {
    mValue[QStringLiteral( "type" )] = QStringLiteral( "static" );
  }
  else if ( radioButtonDatasetGroupTimeStep->isChecked() )
  {
    mValue[QStringLiteral( "type" )] = QStringLiteral( "dataset-time-step" );
    mValue[QStringLiteral( "value" )] = comboBoxDatasetTimeStep->currentData();
  }
  else if ( radioButtonDefinedDateTime->isChecked() )
  {
    mValue[QStringLiteral( "type" )] = QStringLiteral( "defined-date-time" );
    mValue[QStringLiteral( "value" )] = dateTimeEdit->dateTime();
  }
  else if ( radioButtonCurrentCanvasTime->isChecked() && mCanvas )
  {
    mValue[QStringLiteral( "type" )] = QStringLiteral( "current-context-time" );
  }

  emit changed();
}

void QgsProcessingMeshDatasetTimeWidget::updateValue()
{
  updateWidget();
  buildValue();
}

///@endcond
