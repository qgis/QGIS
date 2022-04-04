/***************************************************************************
  qgsprocessingdxflayerswidgetwrapper.cpp
  ---------------------
  Date                 : September 2020
  Copyright            : (C) 2020 by Alexander Bruy
  Email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingdxflayerswidgetwrapper.h"

#include <QBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QToolButton>

#include "qgspanelwidget.h"
#include "qgsprocessingparameters.h"
#include "qgsprocessingoutputs.h"
#include "qgsprocessingparameterdxflayers.h"

/// @cond private

//
// QgsProcessingDxfLayerDetailsWidget
//

QgsProcessingDxfLayerDetailsWidget::QgsProcessingDxfLayerDetailsWidget( const QVariant &value, QgsProject *project )
{
  setupUi( this );

  mFieldsComboBox->setAllowEmptyFieldName( true );

  mContext.setProject( project );

  const QgsDxfExport::DxfLayer layer = QgsProcessingParameterDxfLayers::variantMapAsLayer( value.toMap(), mContext );
  mLayer = layer.layer();

  if ( !mLayer )
    return;

  mFieldsComboBox->setLayer( mLayer );
  mFieldsComboBox->setCurrentIndex( layer.layerOutputAttributeIndex() );

  connect( mFieldsComboBox, &QgsFieldComboBox::fieldChanged, this, &QgsPanelWidget::widgetChanged );
}

QVariant QgsProcessingDxfLayerDetailsWidget::value() const
{
  const int index = mLayer->fields().lookupField( mFieldsComboBox->currentField() );
  const QgsDxfExport::DxfLayer layer( mLayer, index );
  return QgsProcessingParameterDxfLayers::layerAsVariantMap( layer );
}


//
// QgsProcessingDxfLayersPanelWidget
//

QgsProcessingDxfLayersPanelWidget::QgsProcessingDxfLayersPanelWidget(
  const QVariant &value,
  QgsProject *project,
  QWidget *parent )
  : QgsProcessingMultipleSelectionPanelWidget( QVariantList(), QVariantList(), parent )
  , mProject( project )
{
  connect( listView(), &QListView::doubleClicked, this, &QgsProcessingDxfLayersPanelWidget::configureLayer );

  QPushButton *configureLayerButton = new QPushButton( tr( "Configure Layer…" ) );
  connect( configureLayerButton, &QPushButton::clicked, this, &QgsProcessingDxfLayersPanelWidget::configureLayer );
  buttonBox()->addButton( configureLayerButton, QDialogButtonBox::ActionRole );

  // populate the list: first layers already selected, then layers from project not yet selected
  mContext.setProject( project );

  QSet<const QgsVectorLayer *> seenVectorLayers;
  const QVariantList valueList = value.toList();
  for ( const QVariant &v : valueList )
  {
    const QgsDxfExport::DxfLayer layer = QgsProcessingParameterDxfLayers::variantMapAsLayer( v.toMap(), mContext );
    if ( !layer.layer() )
      continue;  // skip any invalid layers

    addOption( v, titleForLayer( layer ), true );
    seenVectorLayers.insert( layer.layer() );
  }

  const QList<QgsVectorLayer *> options = QgsProcessingUtils::compatibleVectorLayers( project, QList< int >() );
  for ( const QgsVectorLayer *layer : options )
  {
    if ( seenVectorLayers.contains( layer ) )
      continue;

    QVariantMap vm;
    vm["layer"] = layer->id();
    vm["attributeIndex"] = -1;

    const QString title = layer->name();
    addOption( vm, title, false );
  }
}

void QgsProcessingDxfLayersPanelWidget::configureLayer()
{
  const QModelIndexList selection = listView()->selectionModel()->selectedIndexes();
  if ( selection.size() != 1 )
  {
    QMessageBox::warning( this, tr( "Configure Layer" ), tr( "Please select a single layer." ) );
    return;
  }

  QStandardItem *item = mModel->itemFromIndex( selection[0] );
  const QVariant value = item->data( Qt::UserRole );

  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsProcessingDxfLayerDetailsWidget *widget = new QgsProcessingDxfLayerDetailsWidget( value, mProject );
    widget->setPanelTitle( tr( "Configure Layer" ) );
    widget->buttonBox()->hide();

    connect( widget, &QgsProcessingDxfLayerDetailsWidget::widgetChanged, this, [ = ]()
    {
      setItemValue( item, widget->value() );
    } );
    panel->openPanel( widget );
  }
  else
  {
    QDialog dlg;
    dlg.setWindowTitle( tr( "Configure Layer" ) );
    QVBoxLayout *vLayout = new QVBoxLayout();
    QgsProcessingDxfLayerDetailsWidget *widget = new QgsProcessingDxfLayerDetailsWidget( value, mProject );
    vLayout->addWidget( widget );
    connect( widget->buttonBox(), &QDialogButtonBox::accepted, &dlg, &QDialog::accept );
    connect( widget->buttonBox(), &QDialogButtonBox::rejected, &dlg, &QDialog::reject );
    dlg.setLayout( vLayout );
    if ( dlg.exec() )
    {
      setItemValue( item, widget->value() );
    }
  }
}

void QgsProcessingDxfLayersPanelWidget::setItemValue( QStandardItem *item, const QVariant &value )
{
  mContext.setProject( mProject );

  const QgsDxfExport::DxfLayer layer = QgsProcessingParameterDxfLayers::variantMapAsLayer( value.toMap(), mContext );

  item->setText( titleForLayer( layer ) );
  item->setData( value, Qt::UserRole );
}

QString QgsProcessingDxfLayersPanelWidget::titleForLayer( const QgsDxfExport::DxfLayer &layer )
{
  QString title = layer.layer()->name();

  if ( layer.layerOutputAttributeIndex() != -1 )
    title += tr( " [split attribute: %1]" ).arg( layer.splitLayerAttribute() );

  return title;
}


//
// QgsProcessingDxfLayersWidget
//

QgsProcessingDxfLayersWidget::QgsProcessingDxfLayersWidget( QWidget *parent )
  : QWidget( parent )
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

  updateSummaryText();

  connect( mToolButton, &QToolButton::clicked, this, &QgsProcessingDxfLayersWidget::showDialog );
}

void QgsProcessingDxfLayersWidget::setValue( const QVariant &value )
{
  if ( value.isValid() )
    mValue = value.type() == QVariant::List ? value.toList() : QVariantList() << value;
  else
    mValue.clear();

  updateSummaryText();
  emit changed();
}

void QgsProcessingDxfLayersWidget::setProject( QgsProject *project )
{
  mProject = project;
}

void QgsProcessingDxfLayersWidget::showDialog()
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsProcessingDxfLayersPanelWidget *widget = new QgsProcessingDxfLayersPanelWidget( mValue, mProject );
    widget->setPanelTitle( tr( "Input layers" ) );
    connect( widget, &QgsProcessingMultipleSelectionPanelWidget::selectionChanged, this, [ = ]()
    {
      setValue( widget->selectedOptions() );
    } );
    connect( widget, &QgsProcessingMultipleSelectionPanelWidget::acceptClicked, widget, &QgsPanelWidget::acceptPanel );
    panel->openPanel( widget );
  }
  else
  {
    QDialog dlg;
    dlg.setWindowTitle( tr( "Input layers" ) );
    QVBoxLayout *vLayout = new QVBoxLayout();
    QgsProcessingDxfLayersPanelWidget *widget = new QgsProcessingDxfLayersPanelWidget( mValue, mProject );
    vLayout->addWidget( widget );
    widget->buttonBox()->addButton( QDialogButtonBox::Cancel );
    connect( widget->buttonBox(), &QDialogButtonBox::accepted, &dlg, &QDialog::accept );
    connect( widget->buttonBox(), &QDialogButtonBox::rejected, &dlg, &QDialog::reject );
    dlg.setLayout( vLayout );
    if ( dlg.exec() )
    {
      setValue( widget->selectedOptions() );
    }
  }
}

void QgsProcessingDxfLayersWidget::updateSummaryText()
{
  mLineEdit->setText( tr( "%n vector layer(s) selected", nullptr, mValue.count() ) );
}


//
// QgsProcessingDxfLayersWidgetWrapper
//

QgsProcessingDxfLayersWidgetWrapper::QgsProcessingDxfLayersWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{
}

QString QgsProcessingDxfLayersWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterDxfLayers::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingDxfLayersWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingDxfLayersWidgetWrapper( parameter, type );
}

QWidget *QgsProcessingDxfLayersWidgetWrapper::createWidget()
{
  mPanel = new QgsProcessingDxfLayersWidget( nullptr );
  mPanel->setProject( widgetContext().project() );
  connect( mPanel, &QgsProcessingDxfLayersWidget::changed, this, [ = ]
  {
    emit widgetValueHasChanged( this );
  } );
  return mPanel;
}

void QgsProcessingDxfLayersWidgetWrapper::setWidgetContext( const QgsProcessingParameterWidgetContext &context )
{
  QgsAbstractProcessingParameterWidgetWrapper::setWidgetContext( context );
  if ( mPanel )
  {
    mPanel->setProject( context.project() );
  }
}

void QgsProcessingDxfLayersWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  Q_UNUSED( context )
  if ( mPanel )
  {
    mPanel->setValue( value );
  }
}

QVariant QgsProcessingDxfLayersWidgetWrapper::widgetValue() const
{
  return mPanel ? mPanel->value() : QVariant();
}

QStringList QgsProcessingDxfLayersWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterMultipleLayers::typeName()
         << QgsProcessingParameterMapLayer::typeName()
         << QgsProcessingParameterVectorLayer::typeName()
         << QgsProcessingParameterFeatureSource::typeName()
         << QgsProcessingParameterFile::typeName()
         << QgsProcessingParameterString::typeName();
}

QStringList QgsProcessingDxfLayersWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList()
         << QgsProcessingOutputString::typeName()
         << QgsProcessingOutputMapLayer::typeName()
         << QgsProcessingOutputVectorLayer::typeName()
         << QgsProcessingOutputMultipleLayers::typeName()
         << QgsProcessingOutputFile::typeName();
}

/// @endcond
