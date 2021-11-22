/***************************************************************************
    qgsowssourcewidget.cpp
     --------------------------------------
    Date                 : November 2021
    Copyright            : (C) 2021 by Samweli Mwakisambwe
    Email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsowssourcewidget.h"
#include "qgsproviderregistry.h"
#include "qgsmapcanvas.h"

#include <QNetworkRequest>


QgsOWSSourceWidget::QgsOWSSourceWidget( const QString &providerKey, QWidget *parent )
  : QgsProviderSourceWidget( parent )
  , mProviderKey( providerKey )
{
  setupUi( this );

  connect( mChangeCRSButton, &QPushButton::clicked, this, &QgsOWSSourceWidget::mChangeCRSButton_clicked );

  mTileWidthLineEdit->setValidator( new QIntValidator( 0, 9999, this ) );
  mTileHeightLineEdit->setValidator( new QIntValidator( 0, 9999, this ) );
  mFeatureCountLineEdit->setValidator( new QIntValidator( 0, 9999, this ) );

  mCacheComboBox->addItem( tr( "Always Cache" ), QNetworkRequest::AlwaysCache );
  mCacheComboBox->addItem( tr( "Prefer Cache" ), QNetworkRequest::PreferCache );
  mCacheComboBox->addItem( tr( "Prefer Network" ), QNetworkRequest::PreferNetwork );
  mCacheComboBox->addItem( tr( "Always Network" ), QNetworkRequest::AlwaysNetwork );

  // 'Prefer network' is the default noted in the combobox's tool tip
  mCacheComboBox->setCurrentIndex( mCacheComboBox->findData( QNetworkRequest::PreferNetwork ) );

  if ( providerKey == QStringLiteral( "wcs" ) )
    mWMSGroupBox->hide();

}

void QgsOWSSourceWidget::clearCrs()
{
  mCRSLabel->setText( tr( "Coordinate Reference System" ) + ':' );
  mSelectedCRSLabel->clear();
  mChangeCRSButton->setEnabled( false );
}

void QgsOWSSourceWidget::clearFormats()
{
  mFormatComboBox->clear();
  mFormatComboBox->setEnabled( false );
}

void QgsOWSSourceWidget::clearTimes()
{
  mTimeComboBox->clear();
  mTimeComboBox->setEnabled( false );
}

void QgsOWSSourceWidget::populateTimes()
{
  mTimeComboBox->clear();
  mTimeComboBox->insertItems( 0, times() );
  mTimeComboBox->setEnabled( !times().isEmpty() );
}

QString QgsOWSSourceWidget::selectedTimeText()
{
  return mTimeComboBox->currentText();
}

void QgsOWSSourceWidget::hideInputWidgets()
{
  mTimeWidget->hide();
  mFormatWidget->hide();
  mCRSWidget->hide();
  mCacheWidget->hide();
}

void QgsOWSSourceWidget::insertItemFormat( int index, const QString &label )
{
  mFormatComboBox->insertItem( index, label );
}

void QgsOWSSourceWidget::setFormatCurrentIndex( int index )
{
  mFormatComboBox->setCurrentIndex( index );
}

int QgsOWSSourceWidget::formatCurrentIndex()
{
  return mFormatComboBox->currentIndex();
}

void QgsOWSSourceWidget::setFormatEnabled( bool enabled )
{
  mFormatComboBox->setEnabled( enabled );
}

void QgsOWSSourceWidget::mChangeCRSButton_clicked()
{
  emit changeCRSButtonClicked();
}

void QgsOWSSourceWidget::setSelectedCRSLabel( const QString &label )
{
  mCRSLabel->setText( label );
}

void QgsOWSSourceWidget::setCRSLabel( const QString &label )
{
  mSelectedCRSLabel->setText( label );
}

void QgsOWSSourceWidget::setChangeCRSButtonEnabled( bool enabled )
{
  mChangeCRSButton->setEnabled( enabled );
}

QVariant QgsOWSSourceWidget::cacheData()
{
  return mCacheComboBox->currentData();
}

void QgsOWSSourceWidget::prepareExtent( QgsMapCanvas *mapCanvas )
{
  QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) );
  mSpatialExtentBox->setOutputCrs( crs );
  if ( !mapCanvas )
    return;
  QgsCoordinateReferenceSystem destinationCrs = mapCanvas->mapSettings().destinationCrs();
  mSpatialExtentBox->setCurrentExtent( destinationCrs.bounds(), destinationCrs );
  mSpatialExtentBox->setOutputExtentFromCurrent();
  mSpatialExtentBox->setMapCanvas( mapCanvas );
}

bool QgsOWSSourceWidget::extentChecked()
{
  return mSpatialExtentBox->isChecked();
}

QgsRectangle QgsOWSSourceWidget::outputExtent()
{
  return mSpatialExtentBox->outputExtent();
}

void QgsOWSSourceWidget::setSourceUri( const QString &uri )
{
  mSourceParts = QgsProviderRegistry::instance()->decodeUri( mProviderKey, uri );
  prepareExtent( QgsProviderSourceWidget::mapCanvas() );

}

QString QgsOWSSourceWidget::sourceUri() const
{
  return QgsProviderRegistry::instance()->encodeUri( mProviderKey, mSourceParts );
}

void QgsOWSSourceWidget::setTimes( const QStringList &times )
{
  mTimes = times;
}

QStringList QgsOWSSourceWidget::times() const
{
  return mTimes;
}
