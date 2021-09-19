/***************************************************************************
                         qgscolorramplegendnodewidget.h
                         -----------------------
    begin                : December 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscolorramplegendnodewidget.h"
#include "qgscolorramplegendnode.h"
#include "qgshelp.h"
#include "qgsnumericformatselectorwidget.h"
#include "qgsnumericformat.h"
#include <QDialogButtonBox>

QgsColorRampLegendNodeWidget::QgsColorRampLegendNodeWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );

  mDirectionComboBox->addItem( tr( "Maximum on Top" ), QgsColorRampLegendNodeSettings::MinimumToMaximum );
  mDirectionComboBox->addItem( tr( "Minimum on Top" ), QgsColorRampLegendNodeSettings::MaximumToMinimum );

  mOrientationComboBox->addItem( tr( "Vertical" ), Qt::Vertical );
  mOrientationComboBox->addItem( tr( "Horizontal" ), Qt::Horizontal );

  mMinLabelLineEdit->setPlaceholderText( tr( "Default" ) );
  mMaxLabelLineEdit->setPlaceholderText( tr( "Default" ) );

  mFontButton->setShowNullFormat( true );
  mFontButton->setNoFormatString( tr( "Default" ) );

  connect( mUseContinuousLegendCheckBox, &QCheckBox::stateChanged, this, [ = ]( bool checked )
  {
    mLayoutGroup->setEnabled( checked );
    mLabelsGroup->setEnabled( checked );
    onChanged();
  } );

  connect( mMinLabelLineEdit, &QLineEdit::textChanged, this, &QgsColorRampLegendNodeWidget::onChanged );
  connect( mMaxLabelLineEdit, &QLineEdit::textChanged, this, &QgsColorRampLegendNodeWidget::onChanged );
  connect( mPrefixLineEdit, &QLineEdit::textChanged, this, &QgsColorRampLegendNodeWidget::onChanged );
  connect( mSuffixLineEdit, &QLineEdit::textChanged, this, &QgsColorRampLegendNodeWidget::onChanged );
  connect( mDirectionComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsColorRampLegendNodeWidget::onChanged );
  connect( mOrientationComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsColorRampLegendNodeWidget::onOrientationChanged );
  connect( mNumberFormatPushButton, &QPushButton::clicked, this, &QgsColorRampLegendNodeWidget::changeNumberFormat );
  connect( mFontButton, &QgsFontButton::changed, this, &QgsColorRampLegendNodeWidget::onChanged );
}

QgsColorRampLegendNodeSettings QgsColorRampLegendNodeWidget::settings() const
{
  QgsColorRampLegendNodeSettings settings;
  settings.setUseContinuousLegend( mUseContinuousLegendCheckBox->isChecked() );
  settings.setDirection( static_cast< QgsColorRampLegendNodeSettings::Direction >( mDirectionComboBox->currentData().toInt() ) );
  settings.setOrientation( static_cast< Qt::Orientation >( mOrientationComboBox->currentData().toInt() ) );
  settings.setMinimumLabel( mMinLabelLineEdit->text() );
  settings.setMaximumLabel( mMaxLabelLineEdit->text() );
  settings.setPrefix( mPrefixLineEdit->text() );
  settings.setSuffix( mSuffixLineEdit->text() );
  settings.setNumericFormat( mSettings.numericFormat()->clone() );
  settings.setTextFormat( mFontButton->textFormat() );
  return settings;
}

void QgsColorRampLegendNodeWidget::setSettings( const QgsColorRampLegendNodeSettings &settings )
{
  mBlockSignals = true;

  mSettings = settings;
  mUseContinuousLegendCheckBox->setChecked( settings.useContinuousLegend() );
  mMinLabelLineEdit->setText( settings.minimumLabel() );
  mMaxLabelLineEdit->setText( settings.maximumLabel() );
  mPrefixLineEdit->setText( settings.prefix() );
  mSuffixLineEdit->setText( settings.suffix() );
  mDirectionComboBox->setCurrentIndex( mDirectionComboBox->findData( settings.direction() ) );
  mOrientationComboBox->setCurrentIndex( mOrientationComboBox->findData( settings.orientation() ) );
  mFontButton->setTextFormat( settings.textFormat() );
  onOrientationChanged();
  mBlockSignals = false;
}

void QgsColorRampLegendNodeWidget::setUseContinuousRampCheckBoxVisibility( bool visible )
{
  mUseContinuousLegendCheckBox->setVisible( visible );
}

void QgsColorRampLegendNodeWidget::changeNumberFormat()
{
  QgsNumericFormatSelectorWidget *widget = new QgsNumericFormatSelectorWidget( this );
  widget->setPanelTitle( tr( "Number Format" ) );
  widget->setFormat( mSettings.numericFormat() );
  connect( widget, &QgsNumericFormatSelectorWidget::changed, this, [ = ]
  {
    mSettings.setNumericFormat( widget->format() );
    onChanged();
  } );
  openPanel( widget );
  return;
}

void QgsColorRampLegendNodeWidget::onOrientationChanged()
{
  switch ( static_cast< Qt::Orientation >( mOrientationComboBox->currentData().toInt() ) )
  {
    case Qt::Vertical:
      mDirectionComboBox->setItemText( 0, tr( "Maximum on Top" ) );
      mDirectionComboBox->setItemText( 1, tr( "Minimum on Top" ) );
      break;

    case Qt::Horizontal:
      mDirectionComboBox->setItemText( 0, tr( "Maximum on Right" ) );
      mDirectionComboBox->setItemText( 1, tr( "Minimum on Right" ) );
      break;
  }

  onChanged();
}

void QgsColorRampLegendNodeWidget::onChanged()
{
  if ( mBlockSignals )
    return;

  emit widgetChanged();
}

//
// QgsColorRampLegendNodeDialog
//

QgsColorRampLegendNodeDialog::QgsColorRampLegendNodeDialog( const QgsColorRampLegendNodeSettings &settings, QWidget *parent )
  : QDialog( parent )
{
  QVBoxLayout *vLayout = new QVBoxLayout();
  mWidget = new QgsColorRampLegendNodeWidget( nullptr );
  vLayout->addWidget( mWidget );
  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Help | QDialogButtonBox::Ok, Qt::Horizontal );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, [ = ]
  {
    QgsHelp::openHelp( QStringLiteral( "working_with_raster/raster_properties.html#raster-legend-settings" ) );
  } );
  connect( mWidget, &QgsPanelWidget::panelAccepted, this, &QDialog::reject );
  vLayout->addWidget( mButtonBox );
  setLayout( vLayout );
  setWindowTitle( tr( "Legend Node Settings" ) );

  mWidget->setSettings( settings );
}

QgsColorRampLegendNodeSettings QgsColorRampLegendNodeDialog::settings() const
{
  return mWidget->settings();
}

QDialogButtonBox *QgsColorRampLegendNodeDialog::buttonBox() const
{
  return mButtonBox;
}

void QgsColorRampLegendNodeDialog::setUseContinuousRampCheckBoxVisibility( bool visible )
{
  mWidget->setUseContinuousRampCheckBoxVisibility( visible );
}
