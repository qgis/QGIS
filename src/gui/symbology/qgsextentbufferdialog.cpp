/***************************************************************************
    qgsextentbufferdialog.cpp
    ---------------------
    begin                : December 2024
    copyright            : (C) 2024 by Juho Ervasti
    email                : juho dot ervasti at gispo dot fi
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsextentbufferdialog.h"
#include "moc_qgsextentbufferdialog.cpp"
#include "qdialogbuttonbox.h"
#include "qgsexpressioncontext.h"
#include "qgsexpressioncontextutils.h"
#include "qgshelp.h"
#include "qgsmapcanvas.h"
#include "qgspanelwidget.h"
#include "qgsproject.h"
#include "qgssymbol.h"
#include "qgssymbolwidgetcontext.h"
#include "qgsunittypes.h"
#include "qgsvectorlayer.h"

QgsExtentBufferWidget::QgsExtentBufferWidget( QgsSymbol *symbol, QgsVectorLayer *layer, QWidget *parent )
  : QgsPanelWidget( parent ), mSymbol( symbol ), mLayer( layer )
{
  setupUi( this );

  mExtentBufferSpinBox->setValue( mSymbol->extentBuffer() );

  mExtentBufferUnitSelectionWidget->setShowMapScaleButton( false );
  mExtentBufferUnitSelectionWidget->setUnits( { Qgis::RenderUnit::Millimeters, Qgis::RenderUnit::MetersInMapUnits, Qgis::RenderUnit::MapUnits, Qgis::RenderUnit::Pixels, Qgis::RenderUnit::Points, Qgis::RenderUnit::Inches } );
  mExtentBufferUnitSelectionWidget->setUnit( mSymbol->extentBufferSizeUnit() );

  connect( mExtentBufferSpinBox, static_cast<void ( QgsDoubleSpinBox::* )( double )>( &QgsDoubleSpinBox::valueChanged ), this, [=]() {
    emit widgetChanged();
  } );

  connect( mExtentBufferUnitSelectionWidget, &QgsUnitSelectionWidget::changed, this, [=]() {
    emit widgetChanged();
  } );

  registerDataDefinedButton( mExtentBufferDDButton, QgsSymbol::Property::ExtentBuffer );
}

QgsSymbolWidgetContext QgsExtentBufferWidget::context() const
{
  return mContext;
}

void QgsExtentBufferWidget::setContext( const QgsSymbolWidgetContext &context )
{
  mContext = context;
}

void QgsExtentBufferWidget::registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsSymbol::Property key )
{
  // pass in nullptr to avoid id, feature and geometry variables being added
  // since the buffer is not evaluated per-feature
  button->init( static_cast<int>( key ), mSymbol->dataDefinedProperties(), QgsSymbol::propertyDefinitions(), nullptr );
  connect( button, &QgsPropertyOverrideButton::changed, this, [=]() {
    emit widgetChanged();
  } );

  button->registerExpressionContextGenerator( this );
}

QgsExpressionContext QgsExtentBufferWidget::createExpressionContext() const
{
  if ( QgsExpressionContext *lExpressionContext = mContext.expressionContext() )
    return *lExpressionContext;

  QgsExpressionContext expContext;

  if ( mContext.mapCanvas() )
  {
    expContext = mContext.mapCanvas()->createExpressionContext();
  }
  else
  {
    expContext << QgsExpressionContextUtils::globalScope()
               << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
               << QgsExpressionContextUtils::atlasScope( nullptr )
               << QgsExpressionContextUtils::mapSettingsScope( QgsMapSettings() );
  }

  if ( mLayer )
    expContext << QgsExpressionContextUtils::layerScope( mLayer );

  expContext.setOriginalValueVariable( mExtentBufferSpinBox->value() );
  expContext.setHighlightedVariables( QStringList() << QgsExpressionContext::EXPR_ORIGINAL_VALUE );

  return expContext;
}

double QgsExtentBufferWidget::extentBuffer() const
{
  return mExtentBufferSpinBox->value();
}

QgsProperty QgsExtentBufferWidget::dataDefinedProperty() const
{
  return mExtentBufferDDButton->toProperty();
}


/// QgsExtentBufferDialog

QgsExtentBufferDialog::QgsExtentBufferDialog( QgsSymbol *symbol, QgsVectorLayer *layer, QWidget *parent )
  : QDialog( parent )
{
  QVBoxLayout *vLayout = new QVBoxLayout();
  mWidget = new QgsExtentBufferWidget( symbol, layer );
  vLayout->addWidget( mWidget );

  QDialogButtonBox *bbox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Help | QDialogButtonBox::Ok, Qt::Horizontal );
  connect( bbox, &QDialogButtonBox::accepted, this, &QgsExtentBufferDialog::accept );
  connect( bbox, &QDialogButtonBox::rejected, this, &QgsExtentBufferDialog::reject );
  connect( bbox, &QDialogButtonBox::helpRequested, this, &QgsExtentBufferDialog::showHelp );

  vLayout->addWidget( bbox );
  setLayout( vLayout );

  setWindowTitle( tr( "Extent Buffer" ) );
}

double QgsExtentBufferDialog::extentBuffer() const
{
  if ( !mWidget )
    return 0;

  return mWidget->extentBuffer();
}

Qgis::RenderUnit QgsExtentBufferWidget::sizeUnit() const
{
  return mExtentBufferUnitSelectionWidget->unit();
}

Qgis::RenderUnit QgsExtentBufferDialog::sizeUnit() const
{
  if ( !mWidget )
    return Qgis::RenderUnit::MapUnits;

  return mWidget->sizeUnit();
}

QgsProperty QgsExtentBufferDialog::dataDefinedProperty() const
{
  if ( !mWidget )
    return QgsProperty();

  return mWidget->dataDefinedProperty();
}

QgsExtentBufferWidget *QgsExtentBufferDialog::widget() const
{
  return mWidget;
}

void QgsExtentBufferDialog::setContext( const QgsSymbolWidgetContext &context )
{
  mWidget->setContext( context );
}


void QgsExtentBufferDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_vector/vector_properties.html#extent-buffer" ) );
}
