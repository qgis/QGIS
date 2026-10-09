/***************************************************************************
   qgsscalevisibilitydialog.cpp
    --------------------------------------
   Date                 : 20.05.2014
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

#include "qgsgroupwmsdatadialog.h"

#include "qgsgui.h"
#include "qgsmaplayerserverproperties.h"

#include <QComboBox>
#include <QRegularExpressionValidator>
#include <QString>

#include "moc_qgsgroupwmsdatadialog.cpp"

using namespace Qt::StringLiterals;

QgsGroupWmsDataDialog::QgsGroupWmsDataDialog( QWidget *parent, Qt::WindowFlags fl )
  : QgsGroupWmsDataDialog( QgsMapLayerServerProperties(), parent, fl )
{}

QgsGroupWmsDataDialog::QgsGroupWmsDataDialog( const QgsMapLayerServerProperties &serverProperties, QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mServerProperties( std::make_unique<QgsMapLayerServerProperties>() )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  //init widgets
  mGroupRequestModeCombo->addItem( tr( "Normal" ), QVariant::fromValue( Qgis::WmsGroupRequestMode::Normal ) );
  mGroupRequestModeCombo->addItem( tr( "Opaque" ), QVariant::fromValue( Qgis::WmsGroupRequestMode::Opaque ) );

  QMap<Qgis::WmsDimensionDefaultDisplay, QString> defaultDisplayDescriptions = QgsMapLayerServerProperties::wmsDimensionDefaultDisplayDescriptions();
  mDefaultDisplayComboBox->clear();
  for ( auto it = defaultDisplayDescriptions.constBegin(); it != defaultDisplayDescriptions.constEnd(); it++ )
  {
    mDefaultDisplayComboBox->addItem( it.value(), QVariant( static_cast<int>( it.key() ) ) );
  }

  connect( mDefaultDisplayComboBox, &QComboBox::currentIndexChanged, this, [this]( int index ) {
    mReferenceValueDateTimeEdit->setEnabled( index == static_cast<int>( Qgis::WmsDimensionDefaultDisplay::ReferenceValue ) );
  } );

  serverProperties.copyTo( mServerProperties.get() );

  mMapLayerServerPropertiesWidget->setHasWfsTitle( false );
  mMapLayerServerPropertiesWidget->setServerProperties( mServerProperties.get() );

  auto it = std::find_if( mServerProperties->wmsDimensions().constBegin(), mServerProperties->wmsDimensions().constEnd(), []( const QgsMapLayerServerProperties::WmsDimensionInfo &dim ) {
    return dim.name == QgsServerWmsDimensionProperties::TIME_DIMENSION_NAME;
  } );

  mComputeTimeDimension->setChecked( it != mServerProperties->wmsDimensions().constEnd() );
  if ( it != mServerProperties->wmsDimensions().constEnd() )
  {
    setTimeDimensionDefaultDisplay( it->defaultDisplayType );
    setTimeDimensionReferenceValue( it->referenceValue().toDateTime() );
  }

  connect( mComputeTimeDimension, &QGroupBox::toggled, this, &QgsGroupWmsDataDialog::updateServerProperties );
  connect( mDefaultDisplayComboBox, &QComboBox::currentIndexChanged, this, &QgsGroupWmsDataDialog::updateServerProperties );
  connect( mReferenceValueDateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &QgsGroupWmsDataDialog::updateServerProperties );
}

QString QgsGroupWmsDataDialog::groupShortName() const
{
  mMapLayerServerPropertiesWidget->save();
  return mServerProperties->shortName();
}

void QgsGroupWmsDataDialog::setGroupShortName( const QString &shortName )
{
  mServerProperties->setShortName( shortName );
  mMapLayerServerPropertiesWidget->sync();
}

QString QgsGroupWmsDataDialog::groupTitle() const
{
  mMapLayerServerPropertiesWidget->save();
  return mServerProperties->title();
}

void QgsGroupWmsDataDialog::setGroupTitle( const QString &title )
{
  mServerProperties->setTitle( title );
  mMapLayerServerPropertiesWidget->sync();
}

QString QgsGroupWmsDataDialog::groupAbstract() const
{
  mMapLayerServerPropertiesWidget->save();
  return mServerProperties->abstract();
}

void QgsGroupWmsDataDialog::setGroupAbstract( const QString &abstract )
{
  mServerProperties->setAbstract( abstract );
  mMapLayerServerPropertiesWidget->sync();
}

QgsMapLayerServerProperties *QgsGroupWmsDataDialog::serverProperties()
{
  return mServerProperties.get();
}

const QgsMapLayerServerProperties *QgsGroupWmsDataDialog::serverProperties() const
{
  return mServerProperties.get();
}

void QgsGroupWmsDataDialog::updateServerProperties() const
{
  if ( !hasTimeDimension() )
  {
    mServerProperties->removeWmsDimension( QgsServerWmsDimensionProperties::TIME_DIMENSION_NAME );
    return;
  }

  QList<QgsServerWmsDimensionProperties::WmsDimensionInfo> wmsDimensions = mServerProperties->wmsDimensions();
  auto it = std::find_if( wmsDimensions.begin(), wmsDimensions.end(), []( const QgsMapLayerServerProperties::WmsDimensionInfo &dim ) {
    return dim.name == QgsServerWmsDimensionProperties::TIME_DIMENSION_NAME;
  } );

  if ( it == wmsDimensions.end() )
  {
    wmsDimensions.append( QgsServerWmsDimensionProperties::WmsDimensionInfo( QgsServerWmsDimensionProperties::TIME_DIMENSION_NAME, timeDimensionDefaultDisplay(), timeDimensionReferenceValue() ) );
  }
  else
  {
    it->defaultDisplayType = timeDimensionDefaultDisplay();
    it->setReferenceValue( timeDimensionReferenceValue() );
  }

  mServerProperties->setWmsDimensions( wmsDimensions );
}


void QgsGroupWmsDataDialog::accept()
{
  mMapLayerServerPropertiesWidget->save();
  QDialog::accept();
}

bool QgsGroupWmsDataDialog::hasTimeDimension() const
{
  return mComputeTimeDimension->isChecked();
}

void QgsGroupWmsDataDialog::setHasTimeDimension( bool hasTimeDimension )
{
  mComputeTimeDimension->setChecked( hasTimeDimension );
}

Qgis::WmsGroupRequestMode QgsGroupWmsDataDialog::groupRequestMode() const
{
  return mGroupRequestModeCombo->currentData().value<Qgis::WmsGroupRequestMode>();
}

void QgsGroupWmsDataDialog::setGroupRequestMode( Qgis::WmsGroupRequestMode groupRequestMode )
{
  mGroupRequestModeCombo->setCurrentIndex( mGroupRequestModeCombo->findData( QVariant::fromValue( groupRequestMode ) ) );
}

Qgis::WmsDimensionDefaultDisplay QgsGroupWmsDataDialog::timeDimensionDefaultDisplay() const
{
  return static_cast<Qgis::WmsDimensionDefaultDisplay>( mDefaultDisplayComboBox->currentData().toInt() );
}

void QgsGroupWmsDataDialog::setTimeDimensionDefaultDisplay( Qgis::WmsDimensionDefaultDisplay timeDimensionDefaultDisplay )
{
  mDefaultDisplayComboBox->setCurrentIndex( mDefaultDisplayComboBox->findData( QVariant( static_cast<int>( timeDimensionDefaultDisplay ) ) ) );
  mReferenceValueDateTimeEdit->setEnabled( timeDimensionDefaultDisplay == Qgis::WmsDimensionDefaultDisplay::ReferenceValue );
}

QDateTime QgsGroupWmsDataDialog::timeDimensionReferenceValue() const
{
  return mReferenceValueDateTimeEdit->dateTime();
}

void QgsGroupWmsDataDialog::setTimeDimensionReferenceValue( const QDateTime &timeDimensionReferenceValue )
{
  mReferenceValueDateTimeEdit->setDateTime( timeDimensionReferenceValue );
}
