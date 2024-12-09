/***************************************************************************
    qgstableeditorformattingwidget.cpp
    ------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstableeditorformattingwidget.h"
#include "moc_qgstableeditorformattingwidget.cpp"
#include "qgsnumericformatselectorwidget.h"
#include "qgsnumericformat.h"
#include "qgis.h"
#include "qgsproperty.h"
#include <QPointer>

QgsTableEditorFormattingWidget::QgsTableEditorFormattingWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );
  setPanelTitle( tr( "Cell Contents" ) );

  mFormatNumbersCheckBox->setTristate( false );

  mFontButton->setShowNullFormat( true );
  mFontButton->setNoFormatString( tr( "Clear Formatting" ) );

  mBackgroundColorButton->setAllowOpacity( true );
  mBackgroundColorButton->setColorDialogTitle( tr( "Background Color" ) );
  mBackgroundColorButton->setDefaultColor( QColor( 255, 255, 255 ) );
  mBackgroundColorButton->setShowNull( true );

  mHorizontalAlignComboBox->setAvailableAlignments( Qt::AlignLeft | Qt::AlignHCenter | Qt::AlignRight | Qt::AlignJustify );
  mVerticalAlignComboBox->setAvailableAlignments( Qt::AlignTop | Qt::AlignVCenter | Qt::AlignBottom );

  mRowHeightSpinBox->setClearValue( 0, tr( "Automatic" ) );
  mColumnWidthSpinBox->setClearValue( 0, tr( "Automatic" ) );

  connect( mBackgroundColorButton, &QgsColorButton::colorChanged, this, [=] {
    if ( !mBlockSignals )
      emit backgroundColorChanged( mBackgroundColorButton->color() );
  } );
  connect( mBackgroundColorButton, &QgsColorButton::cleared, this, [=] {
    if ( !mBlockSignals )
      emit backgroundColorChanged( QColor() );
  } );

  connect( mFormatNumbersCheckBox, &QCheckBox::stateChanged, this, [=]( int state ) {
    mCustomizeFormatButton->setEnabled( state == Qt::Checked );
    if ( state != Qt::PartiallyChecked )
      mFormatNumbersCheckBox->setTristate( false );
    if ( !mBlockSignals )
      emit numberFormatChanged();
  } );

  connect( mFontButton, &QgsFontButton::changed, this, [=] {
    if ( !mBlockSignals )
      emit textFormatChanged();
  } );

  mCustomizeFormatButton->setEnabled( false );
  connect( mCustomizeFormatButton, &QPushButton::clicked, this, [=] {
    QgsNumericFormatSelectorWidget *widget = new QgsNumericFormatSelectorWidget( this );
    widget->setFormat( mNumericFormat.get() );
    widget->setPanelTitle( tr( "Number Format" ) );
    connect( widget, &QgsNumericFormatSelectorWidget::changed, this, [=] {
      mNumericFormat.reset( widget->format() );
      emit numberFormatChanged();
    } );
    openPanel( widget );
  } );

  connect( mRowHeightSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [=]( double height ) {
    if ( !mBlockSignals )
    {
      emit rowHeightChanged( height );

      mBlockSignals++;
      mRowHeightSpinBox->setClearValue( 0, tr( "Automatic" ) );
      mBlockSignals--;
    }
  } );
  connect( mColumnWidthSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [=]( double width ) {
    if ( !mBlockSignals )
    {
      emit columnWidthChanged( width );

      mBlockSignals++;
      mColumnWidthSpinBox->setClearValue( 0, tr( "Automatic" ) );
      mBlockSignals--;
    }
  } );

  connect( mHorizontalAlignComboBox, &QgsAlignmentComboBox::changed, this, [=] {
    if ( !mBlockSignals )
    {
      emit horizontalAlignmentChanged( mHorizontalAlignComboBox->currentAlignment() );
    }
  } );

  connect( mVerticalAlignComboBox, &QgsAlignmentComboBox::changed, this, [=] {
    if ( !mBlockSignals )
    {
      emit verticalAlignmentChanged( mVerticalAlignComboBox->currentAlignment() );
    }
  } );

  connect( mExpressionEdit, qOverload<const QString &>( &QgsFieldExpressionWidget::fieldChanged ), this, [=]( const QString &expression ) {
    if ( !mBlockSignals )
    {
      emit cellPropertyChanged( expression.isEmpty() ? QgsProperty() : QgsProperty::fromExpression( expression ) );
    }
  } );

  mExpressionEdit->setAllowEmptyFieldName( true );

  mExpressionEdit->registerExpressionContextGenerator( this );
  mFontButton->registerExpressionContextGenerator( this );
}

QgsNumericFormat *QgsTableEditorFormattingWidget::numericFormat()
{
  if ( !mNumericFormat || mFormatNumbersCheckBox->checkState() != Qt::Checked )
    return nullptr;

  return mNumericFormat->clone();
}

QgsTextFormat QgsTableEditorFormattingWidget::textFormat() const
{
  return mFontButton->textFormat();
}

void QgsTableEditorFormattingWidget::setBackgroundColor( const QColor &color )
{
  mBlockSignals++;
  mBackgroundColorButton->setColor( color );
  mBlockSignals--;
}

void QgsTableEditorFormattingWidget::setNumericFormat( QgsNumericFormat *format, bool isMixedFormat )
{
  mNumericFormat.reset( format ? format->clone() : nullptr );
  mBlockSignals++;
  mFormatNumbersCheckBox->setTristate( isMixedFormat );
  mFormatNumbersCheckBox->setCheckState( isMixedFormat ? Qt::PartiallyChecked : ( mNumericFormat.get() ? Qt::Checked : Qt::Unchecked ) );
  mBlockSignals--;
}

void QgsTableEditorFormattingWidget::setTextFormat( const QgsTextFormat &format )
{
  mBlockSignals++;
  mFontButton->setTextFormat( format );
  mBlockSignals--;
}

void QgsTableEditorFormattingWidget::setRowHeight( double height )
{
  mBlockSignals++;
  if ( height < 0 )
    mRowHeightSpinBox->setClearValue( 0, tr( "Mixed" ) );
  else
    mRowHeightSpinBox->setClearValue( 0, tr( "Automatic" ) );
  mRowHeightSpinBox->setValue( height < 0 ? 0 : height );
  mBlockSignals--;
}

void QgsTableEditorFormattingWidget::setColumnWidth( double width )
{
  mBlockSignals++;
  if ( width < 0 )
    mColumnWidthSpinBox->setClearValue( 0, tr( "Mixed" ) );
  else
    mColumnWidthSpinBox->setClearValue( 0, tr( "Automatic" ) );
  mColumnWidthSpinBox->setValue( width < 0 ? 0 : width );
  mBlockSignals--;
}

void QgsTableEditorFormattingWidget::setHorizontalAlignment( Qt::Alignment alignment )
{
  mBlockSignals++;
  if ( alignment & Qt::AlignHorizontal_Mask && alignment & Qt::AlignVertical_Mask )
    mHorizontalAlignComboBox->setCurrentIndex( -1 );
  else
    mHorizontalAlignComboBox->setCurrentAlignment( alignment );
  mBlockSignals--;
}

void QgsTableEditorFormattingWidget::setVerticalAlignment( Qt::Alignment alignment )
{
  mBlockSignals++;
  if ( alignment & Qt::AlignHorizontal_Mask && alignment & Qt::AlignVertical_Mask )
    mVerticalAlignComboBox->setCurrentIndex( -1 );
  else
    mVerticalAlignComboBox->setCurrentAlignment( alignment );
  mBlockSignals--;
}

void QgsTableEditorFormattingWidget::setCellProperty( const QgsProperty &property )
{
  mBlockSignals++;
  if ( !property.isActive() )
    mExpressionEdit->setExpression( QString() );
  else
    mExpressionEdit->setExpression( property.asExpression() );
  mBlockSignals--;
}

void QgsTableEditorFormattingWidget::setLayer( QgsMapLayer *layer )
{
  mExpressionEdit->setLayer( layer );
}

void QgsTableEditorFormattingWidget::registerExpressionContextGenerator( QgsExpressionContextGenerator *generator )
{
  mContextGenerator = generator;
}

QgsExpressionContext QgsTableEditorFormattingWidget::createExpressionContext() const
{
  QgsExpressionContext context;
  if ( mContextGenerator )
    context = mContextGenerator->createExpressionContext();

  QgsExpressionContextScope *cellScope = new QgsExpressionContextScope();
  // TODO -- could set real row/column numbers here, in certain circumstances...
  cellScope->setVariable( QStringLiteral( "row_number" ), 0 );
  cellScope->setVariable( QStringLiteral( "column_number" ), 0 );
  context.appendScope( cellScope );

  context.setHighlightedVariables( QStringList() << QStringLiteral( "row_number" ) << QStringLiteral( "column_number" ) );
  return context;
}

QgsTableEditorFormattingWidget::~QgsTableEditorFormattingWidget() = default;
