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
#include "qgsmeshlayer.h"
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
  connect( toolButtonMenu->addAction( tr( "Current Active Dataset Group" ) ),
           &QAction::triggered, this, &QgsProcessingMeshDatasetGroupsWidget::selectCurrentActiveDatasetGroup );
  connect( toolButtonMenu->addAction( tr( "Select in Available Dataset Groups" ) ),
           &QAction::triggered, this, &QgsProcessingMeshDatasetGroupsWidget::showDialog );

  mToolButton->setMenu( toolButtonMenu );
}

void QgsProcessingMeshDatasetGroupsWidget::setMeshLayer( QgsMeshLayer *layer )
{
  if ( mMeshLayer == layer )
    return;
  mMeshLayer = layer;
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
    for ( int i : datasetGroupsIndexes )
    {
      QgsMeshDatasetGroupMetadata meta = mMeshLayer->datasetGroupMetadata( i );
      if ( meta.dataType() == mParam->dataType() )
      {
        availableOptions.append( i );
        options.append( meta.name() );
      }

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

    if ( scalarDatasetGroup >= 0 && mMeshLayer->datasetGroupMetadata( scalarDatasetGroup ).dataType() == mParam->dataType() )
      options.append( scalarDatasetGroup );

    if ( vectorDatasetGroup >= 0
         && mMeshLayer->datasetGroupMetadata( vectorDatasetGroup ).dataType() == mParam->dataType()
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
  // evaluate value to layer
  QgsProcessingContext *context = nullptr;
  std::unique_ptr< QgsProcessingContext > tmpContext;
  if ( mProcessingContextGenerator )
    context = mProcessingContextGenerator->processingContext();

  bool needLayerOwnership = !context;
  if ( !context )
  {
    tmpContext = qgis::make_unique< QgsProcessingContext >();
    context = tmpContext.get();
  }

  QgsMeshLayer *meshLayer = QgsProcessingParameters::parameterAsMeshLayer( wrapper->parameterDefinition(), wrapper->parameterValue(), *context );
  if ( needLayerOwnership )
  {
    mTemporarytMeshLayer.reset( qobject_cast< QgsMeshLayer * >( context->takeResultLayer( meshLayer->id() ) ) );
    meshLayer = mTemporarytMeshLayer.get();
  }
  else
  {
    // don't need ownership of this layer - it wasn't owned by temporary context (so e.g. is owned by the project or cotext in context generator)
  }

  if ( mWidget )
    mWidget->setMeshLayer( meshLayer );
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
  Q_UNUSED( context );
  if ( mWidget )
    mWidget->setValue( value );
}

QVariant QgsProcessingMeshDatasetGroupsWidgetWrapper::widgetValue() const
{
  if ( mWidget )
    return mWidget->value();
  return QVariant();
}


void QgsProcessingMeshDatasetGroupsWidget::updateSummaryText()
{
  mLineEdit->setText( tr( "%1 options selected" ).arg( mValue.count() ) );
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
  if ( !mWidget )
    return;

  // evaluate value to layer
  QgsProcessingContext *context = nullptr;
  std::unique_ptr< QgsProcessingContext > tmpContext;
  if ( mProcessingContextGenerator )
    context = mProcessingContextGenerator->processingContext();

  bool needLayerOwnership = !context;
  if ( !context )
  {
    tmpContext = qgis::make_unique< QgsProcessingContext >();
    context = tmpContext.get();
  }

  QgsMeshLayer *meshLayer = QgsProcessingParameters::parameterAsMeshLayer( wrapper->parameterDefinition(), wrapper->parameterValue(), *context );
  if ( needLayerOwnership )
  {
    mTemporarytMeshLayer.reset( qobject_cast< QgsMeshLayer * >( context->takeResultLayer( meshLayer->id() ) ) );
    meshLayer = mTemporarytMeshLayer.get();
  }
  else
  {
    // don't need ownership of this layer - it wasn't owned by temporary context (so e.g. is owned by the project or cotext in context generator)
  }

  if ( mWidget )
    mWidget->setMeshLayer( meshLayer );

}

void QgsProcessingMeshDatasetTimeWidgetWrapper::setDatasetGroupIndexesWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *wrapper )
{
  if ( !mWidget )
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

QWidget *QgsProcessingMeshDatasetTimeWidgetWrapper::createWidget()
{
  mWidget = new QgsProcessingMeshDatasetTimeWidget( nullptr, static_cast<const QgsProcessingParameterMeshDatasetTime *>( parameterDefinition() ), widgetContext() );

  QgsMapCanvas *canvas = widgetContext().mapCanvas();
  if ( canvas )
  {
    connect( canvas, &QgsMapCanvas::temporalRangeChanged, mWidget, &QgsProcessingMeshDatasetTimeWidget::buildValue );
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

  dateTimeEdit->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );

  mCanvas = context.mapCanvas();

  connect( radioButtonCurrentCanvasTime, &QRadioButton::toggled, [this]( bool isChecked ) {if ( isChecked ) this->buildValue();} );
  connect( radioButtonDefinedDateTime, &QRadioButton::toggled, [this]( bool isChecked ) {if ( isChecked ) this->buildValue();} );
  connect( radioButtonDatasetGroupTimeStep, &QRadioButton::toggled, [this]( bool isChecked ) {if ( isChecked ) this->buildValue();} );
  connect( dateTimeEdit, &QgsDateTimeEdit::dateTimeChanged, this, &QgsProcessingMeshDatasetTimeWidget::buildValue );
  connect( comboBoxDatasetTimeStep, qgis::overload<int>::of( &QComboBox::currentIndexChanged ),
           this, &QgsProcessingMeshDatasetTimeWidget::buildValue );

  updateWidget();
}

void QgsProcessingMeshDatasetTimeWidget::setMeshLayer( QgsMeshLayer *layer )
{
  if ( mMeshLayer == layer )
    return;
  mMeshLayer = layer;
  if ( mMeshLayer )
    whileBlocking( dateTimeEdit )->setDateTime( static_cast<QgsMeshLayerTemporalProperties *>( mMeshLayer->temporalProperties() )->referenceTime() );

  buildValue();
  updateWidget();
}

void QgsProcessingMeshDatasetTimeWidget::setDatasetGroupIndexes( const QList<int> datasetGroupIndexes )
{
  if ( datasetGroupIndexes == mDatasetGroupIndexes )
    return;
  mDatasetGroupIndexes = datasetGroupIndexes;
  populateTimeStep();
  updateWidget();
  buildValue();
}

void QgsProcessingMeshDatasetTimeWidget::setValue( const QVariant &value )
{
  if ( !value.isValid() || value.type() != QVariant::Map )
    return;

  mValue = value.toMap();

  if ( !mValue.contains( QStringLiteral( "type" ) ) || !mValue.contains( QStringLiteral( "value" ) ) )
    return;

  QString type = mValue.value( QStringLiteral( "type" ) ).toString();

  setEnabled( true );
  if ( type == QStringLiteral( "static" ) )
  {
    setEnabled( false );
  }
  else if ( type == QStringLiteral( "dataset-time-step" ) )
  {
    whileBlocking( radioButtonDatasetGroupTimeStep )->setChecked( true );
    whileBlocking( comboBoxDatasetTimeStep )->setCurrentIndex( comboBoxDatasetTimeStep->findData( mValue.value( QStringLiteral( "value" ) ) ) );
  }
  else if ( type == QStringLiteral( "dataset-time-step" ) )
  {
    radioButtonDefinedDateTime->setChecked( true );
    whileBlocking( dateTimeEdit )->setDate( mValue.value( QStringLiteral( "value" ) ).toDate() );
  }
  else if ( type == QStringLiteral( "current-context-time" ) )
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
    whileBlocking( radioButtonCurrentCanvasTime )->setEnabled( true );
    labelCurrentCanvasTime->setText( mCanvas->mapSettings().temporalRange().begin().toString( "yyyy-MM-dd HH:mm:ss" ) );
  }
  else
  {
    whileBlocking( radioButtonCurrentCanvasTime )->setEnabled( false );
    if ( radioButtonCurrentCanvasTime->isChecked() )
      whileBlocking( radioButtonDefinedDateTime )->setChecked( true );
  }

  dateTimeEdit->setVisible( radioButtonDefinedDateTime->isChecked() && !isStatic );
  labelCurrentCanvasTime->setVisible( radioButtonCurrentCanvasTime->isChecked() && !isStatic );
  comboBoxDatasetTimeStep->setVisible( radioButtonDatasetGroupTimeStep->isChecked() && !isStatic );
}

bool QgsProcessingMeshDatasetTimeWidget::hasTemporalDataset() const
{
  if ( !mMeshLayer )
    return false;

  for ( int index : mDatasetGroupIndexes )
  {
    if ( mMeshLayer->datasetGroupMetadata( index ).isTemporal() )
      return true;
  }

  return false;
}

void QgsProcessingMeshDatasetTimeWidget::populateTimeStep()
{
  whileBlocking( comboBoxDatasetTimeStep )->clear();

  if ( !mMeshLayer )
    return;

  QMap<quint64, QgsMeshDatasetIndex> timeStep;
  for ( int groupIndex : mDatasetGroupIndexes )
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
    QString stringTime = mMeshLayer->formatTime( key / 1000 / 3600 );
    QVariantList data;
    const QgsMeshDatasetIndex &index = timeStep.value( key );
    data << index.group() << index.dataset();
    whileBlocking( comboBoxDatasetTimeStep )->addItem( stringTime, data );
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

  updateWidget();
}

///@endcond
