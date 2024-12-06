/***************************************************************************
   qgsscalerangewidget.cpp
    --------------------------------------
   Date                 : 25.04.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgsscalerangewidget.h"
#include "moc_qgsscalerangewidget.cpp"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgsscalewidget.h"
#include "qgsguiutils.h"
#include "qgsprojectviewsettings.h"

QgsScaleRangeWidget::QgsScaleRangeWidget( QWidget *parent )
  : QWidget( parent )

{
  mLayout = new QGridLayout( this );
  mLayout->setContentsMargins( 0, 0, 0, 0 );

  QLabel *minLbl = new QLabel( tr( "Minimum (exclusive)" ), this );
  minLbl->setWordWrap( true );
  minLbl->setAlignment( Qt::AlignTop );
  minLbl->setToolTip( tr( "Minimum scale, i.e. most \"zoomed out\". "
                          "This limit is exclusive, that means the layer will not be displayed on this scale." ) );
  QLabel *maxLbl = new QLabel( tr( "Maximum (inclusive)" ), this );
  maxLbl->setWordWrap( true );
  maxLbl->setAlignment( Qt::AlignTop );
  maxLbl->setToolTip( tr( "Maximum scale, i.e. most \"zoomed in\". "
                          "This limit is inclusive, that means the layer will be displayed on this scale." ) );

  const int iconSize = QgsGuiUtils::scaleIconSize( 24 );
  mMinimumScaleIconLabel = new QLabel( this );
  mMinimumScaleIconLabel->setPixmap( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomOut.svg" ) ).pixmap( QSize( iconSize, iconSize ) ) );
  mMaximumScaleIconLabel = new QLabel( this );
  mMaximumScaleIconLabel->setPixmap( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomIn.svg" ) ).pixmap( QSize( iconSize, iconSize ) ) );

  mMinimumScaleWidget = new QgsScaleWidget( this );
  mMaximumScaleWidget = new QgsScaleWidget( this );
  connect( mMinimumScaleWidget, &QgsScaleWidget::scaleChanged, mMaximumScaleWidget, &QgsScaleWidget::setMinScale );
  mMinimumScaleWidget->setShowCurrentScaleButton( true );
  mMaximumScaleWidget->setShowCurrentScaleButton( true );
  reloadProjectScales();
  // add start, add comprehension of scales by settings fake ordered values
  mMinimumScaleWidget->setScale( 100000 );
  mMaximumScaleWidget->setScale( 1000 );

  mLayout->addWidget( minLbl, 0, 0, 1, 2 );
  mLayout->addWidget( mMinimumScaleIconLabel, 1, 0 );
  mLayout->addWidget( mMinimumScaleWidget, 1, 1 );
  mLayout->addWidget( maxLbl, 0, 2, 1, 2 );
  mLayout->addWidget( mMaximumScaleIconLabel, 1, 2 );
  mLayout->addWidget( mMaximumScaleWidget, 1, 3 );

  mLayout->setColumnStretch( 0, 0 );
  mLayout->setColumnStretch( 1, 3 );
  mLayout->setColumnStretch( 2, 0 );
  mLayout->setColumnStretch( 3, 3 );

  connect( mMinimumScaleWidget, &QgsScaleWidget::scaleChanged, this, &QgsScaleRangeWidget::emitRangeChanged );
  connect( mMaximumScaleWidget, &QgsScaleWidget::scaleChanged, this, &QgsScaleRangeWidget::emitRangeChanged );
}

void QgsScaleRangeWidget::reloadProjectScales()
{
  if ( QgsProject::instance()->viewSettings()->useProjectScales() )
  {
    const QVector<double> projectScales = QgsProject::instance()->viewSettings()->mapScales();
    mMinimumScaleWidget->setPredefinedScales( projectScales );
    mMaximumScaleWidget->setPredefinedScales( projectScales );
  }
}

void QgsScaleRangeWidget::setMapCanvas( QgsMapCanvas *mapCanvas )
{
  mMinimumScaleWidget->setMapCanvas( mapCanvas );
  mMaximumScaleWidget->setMapCanvas( mapCanvas );
}

void QgsScaleRangeWidget::setMinimumScale( double scale )
{
  mMinimumScaleWidget->setScale( scale );
}

double QgsScaleRangeWidget::minimumScale() const
{
  return mMinimumScaleWidget->scale();
}

void QgsScaleRangeWidget::setMaximumScale( double scale )
{
  mMaximumScaleWidget->setScale( scale );
}

double QgsScaleRangeWidget::maximumScale() const
{
  return mMaximumScaleWidget->scale();
}

void QgsScaleRangeWidget::setScaleRange( double min, double max )
{
  setMinimumScale( min );
  setMaximumScale( max );
}

void QgsScaleRangeWidget::emitRangeChanged()
{
  emit rangeChanged( minimumScale(), maximumScale() );
}
