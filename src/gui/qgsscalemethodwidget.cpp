/***************************************************************************
                              qgsscalemethodwidget.cpp
                              ------------------------
  begin                : March 2025
  copyright            : (C) 2025 by Nyall Dawson
  email                : nyall.dawson@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsscalemethodwidget.h"

#include "qgsapplication.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>

#include "moc_qgsscalemethodwidget.cpp"

QgsScaleMethodWidget::QgsScaleMethodWidget( QWidget *parent )
  : QWidget( parent )
{
  mCombo = new QComboBox();
  mCombo->setSizeAdjustPolicy( QComboBox::AdjustToMinimumContentsLengthWithIcon );

  mCombo->addItem( tr( "Average Top, Middle and Bottom Scales" ), QVariant::fromValue( Qgis::ScaleCalculationMethod::HorizontalAverage ) );
  mCombo->addItem( tr( "Calculate along Top of Map" ), QVariant::fromValue( Qgis::ScaleCalculationMethod::HorizontalTop ) );
  mCombo->addItem( tr( "Calculate along Middle of Map" ), QVariant::fromValue( Qgis::ScaleCalculationMethod::HorizontalMiddle ) );
  mCombo->addItem( tr( "Calculate along Bottom of Map" ), QVariant::fromValue( Qgis::ScaleCalculationMethod::HorizontalBottom ) );
  mCombo->addItem( tr( "Always Calculate at Equator" ), QVariant::fromValue( Qgis::ScaleCalculationMethod::AtEquator ) );

  QHBoxLayout *hLayout = new QHBoxLayout();
  hLayout->setContentsMargins( 0, 0, 0, 0 );
  hLayout->addWidget( mCombo, 1 );

  // bit of fiddlyness here -- we want the initial spacing to only be visible
  // when the warning label is shown, so it's embedded inside mWarningLabel
  // instead of outside it
  mWarningLabelContainer = new QWidget();
  QHBoxLayout *warningLayout = new QHBoxLayout();
  warningLayout->setContentsMargins( 0, 0, 0, 0 );
  mWarningLabel = new QLabel();
  const QIcon icon = QgsApplication::getThemeIcon( u"mIconWarning.svg"_s );
  const int size = static_cast<int>( std::max( 24.0, mCombo->minimumSize().height() * 0.5 ) );
  mWarningLabel->setPixmap( icon.pixmap( icon.actualSize( QSize( size, size ) ) ) );
  const int labelMargin = static_cast<int>( std::round( mCombo->fontMetrics().horizontalAdvance( 'X' ) ) );
  warningLayout->insertSpacing( 0, labelMargin / 2 );
  warningLayout->insertWidget( 1, mWarningLabel );
  mWarningLabelContainer->setLayout( warningLayout );
  hLayout->addWidget( mWarningLabelContainer );
  mWarningLabelContainer->hide();

  setLayout( hLayout );

  setFocusPolicy( Qt::FocusPolicy::StrongFocus );
  setFocusProxy( mCombo );

  connect( mCombo, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsScaleMethodWidget::methodChanged );
  connect( mCombo, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsScaleMethodWidget::updateWarning );
}

Qgis::ScaleCalculationMethod QgsScaleMethodWidget::scaleMethod() const
{
  return mCombo->currentData().value< Qgis::ScaleCalculationMethod >();
}

void QgsScaleMethodWidget::setScaleMethod( Qgis::ScaleCalculationMethod method )
{
  mCombo->setCurrentIndex( mCombo->findData( QVariant::fromValue( method ) ) );
  updateWarning();
}

void QgsScaleMethodWidget::updateWarning()
{
  switch ( scaleMethod() )
  {
    case Qgis::ScaleCalculationMethod::HorizontalTop:
    case Qgis::ScaleCalculationMethod::HorizontalMiddle:
    case Qgis::ScaleCalculationMethod::HorizontalBottom:
    case Qgis::ScaleCalculationMethod::HorizontalAverage:
      mWarningLabelContainer->hide();
      break;

    case Qgis::ScaleCalculationMethod::AtEquator:
    {
      mWarningLabelContainer->show();
      const QString warning = u"<p>%1</p><p>%2</p>"_s.arg( tr( "This method will calculate misleading scales when the map extent is not close to the "
                                                               "equator, however it ensures that the scale remains constant and does not "
                                                               "change as the map is panned." ),
                                                           tr( "This setting is valid for maps in a geographic (latitude/longitude) CRS only." ) );
      mWarningLabel->setToolTip( warning );

      break;
    }
  }
}
