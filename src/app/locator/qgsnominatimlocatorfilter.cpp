/***************************************************************************
                        qgsnominatimlocatorfilters.cpp
                        ----------------------------
   begin                : May 2017
   copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsnominatimlocatorfilter.h"
#include "moc_qgsnominatimlocatorfilter.cpp"
#include "qgsgeocoder.h"
#include "qgslocatorfilter.h"
#include "qgssettings.h"
#include "qgsmessagebaritem.h"
#include "qgsmessagebar.h"
#include "qgisapp.h"

#include <QDesktopServices>
#include <QPushButton>


QgsNominatimLocatorFilter::QgsNominatimLocatorFilter( QgsGeocoderInterface *geocoder, QgsMapCanvas *canvas )
  : QgsGeocoderLocatorFilter( QStringLiteral( "nominatimgeocoder" ), tr( "Nominatim Geocoder" ), QStringLiteral( ">" ), geocoder, canvas )
{
  setFetchResultsDelay( 1000 );
  setUseWithoutPrefix( false );
}

QgsNominatimLocatorFilter *QgsNominatimLocatorFilter::clone() const
{
  auto filter = std::make_unique< QgsNominatimLocatorFilter >( geocoder(), mCanvas );
  filter->setFetchResultsDelay( fetchResultsDelay() );
  return filter.release();
}

void QgsNominatimLocatorFilter::triggerResult( const QgsLocatorResult &result )
{
  QgsSettings settings;
  if ( !settings.value( "locator_filters/nominatim_geocoder/attribution_shown", false, QgsSettings::App ).toBool() )
  {
    settings.setValue( "locator_filters/nominatim_geocoder/attribution_shown", true, QgsSettings::App );

    QgsMessageBarItem *messageWidget = QgsMessageBar::createMessage( tr( "The Nominatim geocoder data is made available by OpenStreetMap Foundation and contributors." ) );
    QPushButton *learnMoreButton = new QPushButton( tr( "Learn more" ) );
    connect( learnMoreButton, &QPushButton::clicked, learnMoreButton, [=] {
      QDesktopServices::openUrl( QStringLiteral( "https://nominatim.org/" ) );
    } );
    messageWidget->layout()->addWidget( learnMoreButton );
    QgisApp::instance()->messageBar()->pushWidget( messageWidget, Qgis::MessageLevel::Info );
  }
  QgsGeocoderLocatorFilter::triggerResult( result );
}

void QgsNominatimLocatorFilter::openConfigWidget( QWidget *parent )
{
  auto dlg = std::make_unique<QDialog>( parent );
  dlg->setWindowTitle( "Nominatim Geocoder Country Codes" );

  QGridLayout *formLayout = new QGridLayout;
  QLineEdit *countryCodesEdit = new QLineEdit( dlg.get() );

  // Load existing settings
  QgsSettings settings;
  countryCodesEdit->setText( settings.value( "locator_filters/nominatim_geocoder/country_codes", "", QgsSettings::App ).toString() );

  formLayout->addRow( tr( "Two letter Country Codes (comma-separated)" ), countryCodesEdit );
  QDialogButtonBox *buttonbBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dlg.get() );
  formLayout->addRow( buttonbBox );
  dlg->setLayout( formLayout );

  // Save settings when dialog accepted
  connect( buttonbBox, &QDialogButtonBox::accepted, dlg.get(), [&]() {
    settings.setValue( "locator_filters/nominatim_geocoder/country_codes", countryCodesEdit->text().trimmed(), QgsSettings::App );
    dlg->accept();
  } );

  connect( buttonbBox, &QDialogButtonBox::rejected, dlg.get(), &QDialog::reject );
  dlg->exec();
}
