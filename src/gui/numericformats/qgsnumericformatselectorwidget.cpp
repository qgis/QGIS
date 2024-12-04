/***************************************************************************
    qgsnumericformatselectorwidget.cpp
    ----------------------------------
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

#include "qgsnumericformatselectorwidget.h"
#include "moc_qgsnumericformatselectorwidget.cpp"
#include "qgsapplication.h"
#include "qgsnumericformatregistry.h"
#include "qgsnumericformat.h"
#include "qgsnumericformatwidget.h"
#include "qgis.h"
#include "qgsgui.h"
#include "qgsnumericformatguiregistry.h"
#include "qgsreadwritecontext.h"
#include "qgsbasicnumericformat.h"
#include <QDialogButtonBox>
#include <QPushButton>

QgsNumericFormatSelectorWidget::QgsNumericFormatSelectorWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );

  mCurrentFormat.reset( QgsApplication::numericFormatRegistry()->fallbackFormat() );

  mPreviewFormat = std::make_unique<QgsBasicNumericFormat>();
  mPreviewFormat->setShowThousandsSeparator( false );
  mPreviewFormat->setShowPlusSign( false );
  mPreviewFormat->setShowTrailingZeros( false );
  mPreviewFormat->setNumberDecimalPlaces( 12 );

  populateTypes();
  mCategoryCombo->setCurrentIndex( mCategoryCombo->findData( mCurrentFormat->id() ) );

  connect( mCategoryCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsNumericFormatSelectorWidget::formatTypeChanged );
  updateFormatWidget();
}

QgsNumericFormatSelectorWidget::~QgsNumericFormatSelectorWidget() = default;

void QgsNumericFormatSelectorWidget::setFormat( const QgsNumericFormat *format )
{
  if ( !format )
    return;

  mCurrentFormat.reset( format->clone() );

  const QString id = mCurrentFormat->id();
  const int index = mCategoryCombo->findData( id );
  if ( index < 0 )
  {
    whileBlocking( mCategoryCombo )->setCurrentIndex( mCategoryCombo->findData( QStringLiteral( "fallback" ) ) );
  }
  else
    mCategoryCombo->setCurrentIndex( index );

  updateFormatWidget();

  emit changed();
}

QgsNumericFormat *QgsNumericFormatSelectorWidget::format() const
{
  return mCurrentFormat->clone();
}

void QgsNumericFormatSelectorWidget::registerExpressionContextGenerator( QgsExpressionContextGenerator *generator )
{
  mExpressionContextGenerator = generator;
  if ( QgsNumericFormatWidget *w = qobject_cast<QgsNumericFormatWidget *>( stackedWidget->currentWidget() ) )
    w->registerExpressionContextGenerator( mExpressionContextGenerator );
}

void QgsNumericFormatSelectorWidget::formatTypeChanged()
{
  const QString newId = mCategoryCombo->currentData().toString();
  if ( mCurrentFormat->id() == newId )
  {
    return;
  }

  // keep as much of the current format's properties as possible
  QVariantMap props = mCurrentFormat->configuration( QgsReadWriteContext() );
  mCurrentFormat.reset( QgsApplication::numericFormatRegistry()->create( newId, props, QgsReadWriteContext() ) );

  updateFormatWidget();
  updateSampleText();
  emit changed();
}

void QgsNumericFormatSelectorWidget::formatChanged()
{
  if ( QgsNumericFormatWidget *w = qobject_cast<QgsNumericFormatWidget *>( stackedWidget->currentWidget() ) )
    mCurrentFormat.reset( w->format() );

  updateSampleText();
  emit changed();
}

void QgsNumericFormatSelectorWidget::populateTypes()
{
  QStringList ids = QgsApplication::numericFormatRegistry()->formats();

  std::sort( ids.begin(), ids.end(), [=]( const QString &a, const QString &b ) -> bool {
    if ( QgsApplication::numericFormatRegistry()->sortKey( a ) < QgsApplication::numericFormatRegistry()->sortKey( b ) )
      return true;
    else if ( QgsApplication::numericFormatRegistry()->sortKey( a ) > QgsApplication::numericFormatRegistry()->sortKey( b ) )
      return false;
    else
    {
      int res = QString::localeAwareCompare( QgsApplication::numericFormatRegistry()->visibleName( a ), QgsApplication::numericFormatRegistry()->visibleName( b ) );
      if ( res < 0 )
        return true;
      else if ( res > 0 )
        return false;
    }
    return false;
  } );

  for ( const QString &id : std::as_const( ids ) )
    mCategoryCombo->addItem( QgsApplication::numericFormatRegistry()->visibleName( id ), id );
}

void QgsNumericFormatSelectorWidget::updateFormatWidget()
{
  if ( stackedWidget->currentWidget() != pageDummy )
  {
    // stop updating from the original widget
    if ( QgsNumericFormatWidget *w = qobject_cast<QgsNumericFormatWidget *>( stackedWidget->currentWidget() ) )
      disconnect( w, &QgsNumericFormatWidget::changed, this, &QgsNumericFormatSelectorWidget::formatChanged );
    stackedWidget->removeWidget( stackedWidget->currentWidget() );
  }
  if ( QgsNumericFormatWidget *w = QgsGui::numericFormatGuiRegistry()->formatConfigurationWidget( mCurrentFormat.get() ) )
  {
    w->setFormat( mCurrentFormat->clone() );
    stackedWidget->addWidget( w );
    stackedWidget->setCurrentWidget( w );
    // start receiving updates from widget
    connect( w, &QgsNumericFormatWidget::changed, this, &QgsNumericFormatSelectorWidget::formatChanged );
    w->registerExpressionContextGenerator( mExpressionContextGenerator );
  }
  else
  {
    stackedWidget->setCurrentWidget( pageDummy );
  }

  updateSampleText();
}

void QgsNumericFormatSelectorWidget::updateSampleText()
{
  const double sampleValue = mCurrentFormat->suggestSampleValue();
  mSampleLabel->setText( QStringLiteral( "%1 %2 <b>%3</b>" ).arg( mPreviewFormat->formatDouble( sampleValue, QgsNumericFormatContext() ) ).arg( QChar( 0x2192 ) ).arg( mCurrentFormat->formatDouble( sampleValue, QgsNumericFormatContext() ) ) );
}

//
// QgsNumericFormatSelectorDialog
//

QgsNumericFormatSelectorDialog::QgsNumericFormatSelectorDialog( QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setWindowTitle( tr( "Numeric Format" ) );

  mFormatWidget = new QgsNumericFormatSelectorWidget( this );
  mFormatWidget->layout()->setContentsMargins( 0, 0, 0, 0 );

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->addWidget( mFormatWidget );

  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, Qt::Horizontal, this );
  layout->addWidget( mButtonBox );

  setLayout( layout );
  QgsGui::enableAutoGeometryRestore( this );

  connect( mButtonBox->button( QDialogButtonBox::Ok ), &QAbstractButton::clicked, this, &QDialog::accept );
  connect( mButtonBox->button( QDialogButtonBox::Cancel ), &QAbstractButton::clicked, this, &QDialog::reject );
}

void QgsNumericFormatSelectorDialog::setFormat( const QgsNumericFormat *format )
{
  mFormatWidget->setFormat( format );
}

QgsNumericFormat *QgsNumericFormatSelectorDialog::format() const
{
  return mFormatWidget->format();
}

void QgsNumericFormatSelectorDialog::registerExpressionContextGenerator( QgsExpressionContextGenerator *generator )
{
  mFormatWidget->registerExpressionContextGenerator( generator );
}
