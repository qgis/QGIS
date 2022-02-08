/***************************************************************************
  qgsprocessingvectortilewriterlayerswidgetwrapper.cpp
  ---------------------
  Date                 : April 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingvectortilewriterlayerswidgetwrapper.h"

#include <QBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QToolButton>

#include "qgspanelwidget.h"

#include "qgsvectortilewriter.h"

#include "qgsprocessingparametervectortilewriterlayers.h"

/// @cond private

//
// QgsProcessingVectorTileWriteLayerDetailsWidget
//

QgsProcessingVectorTileWriteLayerDetailsWidget::QgsProcessingVectorTileWriteLayerDetailsWidget( const QVariant &value, QgsProject *project )
{
  setupUi( this );

  mContext.setProject( project );

  const QgsVectorTileWriter::Layer layer = QgsProcessingParameterVectorTileWriterLayers::variantMapAsLayer( value.toMap(), mContext );
  mLayer = layer.layer();

  if ( !mLayer )
    return;

  mSpinMinZoom->setClearValue( -1, tr( "Not set" ) );
  mSpinMaxZoom->setClearValue( -1, tr( "Not set" ) );
  mEditFilterExpression->setMultiLine( true );
  mEditFilterExpression->setLayer( mLayer );

  mSpinMinZoom->setValue( layer.minZoom() );
  mSpinMaxZoom->setValue( layer.maxZoom() );
  mEditLayerName->setText( layer.layerName() );
  mEditLayerName->setPlaceholderText( mLayer->name() );
  mEditFilterExpression->setExpression( layer.filterExpression() );

  connect( mSpinMinZoom, qOverload<int>( &QSpinBox::valueChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( mSpinMaxZoom, qOverload<int>( &QSpinBox::valueChanged ), this, &QgsPanelWidget::widgetChanged );
  connect( mEditLayerName, &QLineEdit::textChanged, this, &QgsPanelWidget::widgetChanged );
  connect( mEditFilterExpression, &QgsExpressionLineEdit::expressionChanged, this, &QgsPanelWidget::widgetChanged );
}

QVariant QgsProcessingVectorTileWriteLayerDetailsWidget::value() const
{
  QgsVectorTileWriter::Layer layer( mLayer );
  layer.setMinZoom( mSpinMinZoom->value() );
  layer.setMaxZoom( mSpinMaxZoom->value() );
  layer.setLayerName( mEditLayerName->text() );
  layer.setFilterExpression( mEditFilterExpression->expression() );
  return QgsProcessingParameterVectorTileWriterLayers::layerAsVariantMap( layer );
}

//
// QgsProcessingVectorTileWriterLayersPanelWidget
//


QgsProcessingVectorTileWriterLayersPanelWidget::QgsProcessingVectorTileWriterLayersPanelWidget(
  const QVariant &value,
  QgsProject *project,
  QWidget *parent )
  : QgsProcessingMultipleSelectionPanelWidget( QVariantList(), QVariantList(), parent )
  , mProject( project )
{

  connect( listView(), &QListView::doubleClicked, this, &QgsProcessingVectorTileWriterLayersPanelWidget::configureLayer );

  QPushButton *configureLayerButton = new QPushButton( tr( "Configure Layer…" ) );
  connect( configureLayerButton, &QPushButton::clicked, this, &QgsProcessingVectorTileWriterLayersPanelWidget::configureLayer );
  buttonBox()->addButton( configureLayerButton, QDialogButtonBox::ActionRole );

  QPushButton *copyLayerButton = new QPushButton( tr( "Copy Layer" ) );
  connect( copyLayerButton, &QPushButton::clicked, this, &QgsProcessingVectorTileWriterLayersPanelWidget::copyLayer );
  buttonBox()->addButton( copyLayerButton, QDialogButtonBox::ActionRole );

  // populate the list: first layers already selected, then layers from project not yet selected
  mContext.setProject( project );

  QSet<const QgsVectorLayer *> seenVectorLayers;
  const QVariantList valueList = value.toList();
  for ( const QVariant &v : valueList )
  {
    const QgsVectorTileWriter::Layer layer = QgsProcessingParameterVectorTileWriterLayers::variantMapAsLayer( v.toMap(), mContext );
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

    const QString title = layer->name();

    addOption( vm, title, false );
  }
}

void QgsProcessingVectorTileWriterLayersPanelWidget::configureLayer()
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
    QgsProcessingVectorTileWriteLayerDetailsWidget *widget = new QgsProcessingVectorTileWriteLayerDetailsWidget( value, mProject );
    widget->setPanelTitle( tr( "Configure Layer" ) );
    widget->buttonBox()->hide();

    connect( widget, &QgsProcessingVectorTileWriteLayerDetailsWidget::widgetChanged, this, [ = ]()
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
    QgsProcessingVectorTileWriteLayerDetailsWidget *widget = new QgsProcessingVectorTileWriteLayerDetailsWidget( value, mProject );
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

void QgsProcessingVectorTileWriterLayersPanelWidget::copyLayer()
{
  const QModelIndexList selection = listView()->selectionModel()->selectedIndexes();
  if ( selection.size() != 1 )
  {
    QMessageBox::warning( this, tr( "Copy Layer" ), tr( "Please select a single layer." ) );
    return;
  }

  QStandardItem *item = mModel->itemFromIndex( selection[0] );
  mModel->insertRow( selection[0].row() + 1, item->clone() );
}

void QgsProcessingVectorTileWriterLayersPanelWidget::setItemValue( QStandardItem *item, const QVariant &value )
{
  mContext.setProject( mProject );

  const QgsVectorTileWriter::Layer layer = QgsProcessingParameterVectorTileWriterLayers::variantMapAsLayer( value.toMap(), mContext );

  item->setText( titleForLayer( layer ) );
  item->setData( value, Qt::UserRole );
}

QString QgsProcessingVectorTileWriterLayersPanelWidget::titleForLayer( const QgsVectorTileWriter::Layer &layer )
{
  QString title = layer.layer()->name();

  // add more details
  if ( layer.minZoom() >= 0 && layer.maxZoom() >= 0 )
    title += tr( " [zoom %1...%2]" ).arg( layer.minZoom() ).arg( layer.maxZoom() );
  else if ( layer.minZoom() >= 0 )
    title += tr( " [zoom >= %1]" ).arg( layer.minZoom() );
  else if ( layer.maxZoom() >= 0 )
    title += tr( " [zoom <= %1]" ).arg( layer.maxZoom() );

  if ( !layer.layerName().isEmpty() )
    title += tr( " [name: %1]" ).arg( layer.layerName() );
  if ( !layer.filterExpression().isEmpty() )
    title += tr( " [with filter]" );

  return title;
}


//
// QgsProcessingVectorTileWriterLayersWidget
//


QgsProcessingVectorTileWriterLayersWidget::QgsProcessingVectorTileWriterLayersWidget( QWidget *parent )
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

  connect( mToolButton, &QToolButton::clicked, this, &QgsProcessingVectorTileWriterLayersWidget::showDialog );
}

void QgsProcessingVectorTileWriterLayersWidget::setValue( const QVariant &value )
{
  if ( value.isValid() )
    mValue = value.type() == QVariant::List ? value.toList() : QVariantList() << value;
  else
    mValue.clear();

  updateSummaryText();
  emit changed();
}

void QgsProcessingVectorTileWriterLayersWidget::setProject( QgsProject *project )
{
  mProject = project;
}

void QgsProcessingVectorTileWriterLayersWidget::showDialog()
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsProcessingVectorTileWriterLayersPanelWidget *widget = new QgsProcessingVectorTileWriterLayersPanelWidget( mValue, mProject );
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
    QgsProcessingVectorTileWriterLayersPanelWidget *widget = new QgsProcessingVectorTileWriterLayersPanelWidget( mValue, mProject );
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

void QgsProcessingVectorTileWriterLayersWidget::updateSummaryText()
{
  mLineEdit->setText( tr( "%n vector layer(s) selected", nullptr, mValue.count() ) );
}

//
// QgsProcessingVectorTileWriterLayersWidgetWrapper
//

QgsProcessingVectorTileWriterLayersWidgetWrapper::QgsProcessingVectorTileWriterLayersWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{
}

QString QgsProcessingVectorTileWriterLayersWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterVectorTileWriterLayers::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingVectorTileWriterLayersWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingVectorTileWriterLayersWidgetWrapper( parameter, type );
}

QWidget *QgsProcessingVectorTileWriterLayersWidgetWrapper::createWidget()
{
  mPanel = new QgsProcessingVectorTileWriterLayersWidget( nullptr );
  mPanel->setProject( widgetContext().project() );
  connect( mPanel, &QgsProcessingVectorTileWriterLayersWidget::changed, this, [ = ]
  {
    emit widgetValueHasChanged( this );
  } );
  return mPanel;
}

void QgsProcessingVectorTileWriterLayersWidgetWrapper::setWidgetContext( const QgsProcessingParameterWidgetContext &context )
{
  QgsAbstractProcessingParameterWidgetWrapper::setWidgetContext( context );
  if ( mPanel )
  {
    mPanel->setProject( context.project() );
  }
}

void QgsProcessingVectorTileWriterLayersWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext &context )
{
  Q_UNUSED( context )
  if ( mPanel )
  {
    mPanel->setValue( value );
  }
}

QVariant QgsProcessingVectorTileWriterLayersWidgetWrapper::widgetValue() const
{
  return mPanel ? mPanel->value() : QVariant();
}

QStringList QgsProcessingVectorTileWriterLayersWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList();
}

QStringList QgsProcessingVectorTileWriterLayersWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList();
}

/// @endcond
