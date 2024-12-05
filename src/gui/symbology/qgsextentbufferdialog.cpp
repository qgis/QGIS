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
#include "qgshelp.h"
#include "qgspanelwidget.h"
#include "qgssymbol.h"
#include "qgssymbolwidgetcontext.h"
#include "qgsvectorlayer.h"

QgsExtentBufferWidget::QgsExtentBufferWidget( QgsSymbol *symbol, QgsVectorLayer *layer, QWidget *parent )
  : QgsPanelWidget( parent ), mSymbol( symbol ), mLayer( layer )
{
  setupUi( this );

  mExtentBufferSpinBox->setValue( mSymbol-> extentBuffer() );

  connect( mExtentBufferSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, [ = ]()
  {
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
  button->init( static_cast< int >( key ), mSymbol->dataDefinedProperties(), QgsSymbol::propertyDefinitions(), nullptr );
  connect( button, &QgsPropertyOverrideButton::changed, this, [ = ]()
  {
    emit widgetChanged();
  } );

  button->registerExpressionContextGenerator( this );
}

QgsExpressionContext QgsExtentBufferWidget::createExpressionContext() const
{
  QList<QgsExpressionContextScope *> scopes = mContext.globalProjectAtlasMapLayerScopes( mLayer );
  QgsExpressionContext expContext( scopes );

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

  setWindowTitle( tr( "Extent buffer" ) );
}

double QgsExtentBufferDialog::extentBuffer() const
{
  if ( !mWidget )
    return 0;

  return mWidget->extentBuffer();
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

void QgsExtentBufferDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_vector/vector_properties.html#extent-buffer" ) );
}

