/***************************************************************************
  qgswmsnewconnection.cpp - QgsWmsNewConnection

 ---------------------
 begin                : 16.10.2025
 copyright            : (C) 2025 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgswmsnewconnection.h"

#include <memory>

#include "qgsguiutils.h"
#include "qgsowsconnection.h"
#include "qgsproject.h"
#include "qgswmsprovider.h"

#include <QMessageBox>

#include "moc_qgswmsnewconnection.cpp"

QgsWmsNewConnection::QgsWmsNewConnection( QWidget *parent, const QString &connName )
  : QgsNewHttpConnection( parent, QgsNewHttpConnection::ConnectionWms, u"WMS"_s, connName )
{
  connect( wmsFormatDetectButton(), &QPushButton::clicked, this, &QgsWmsNewConnection::detectFormat );
  initWmsSpecificSettings();
}


void QgsWmsNewConnection::detectFormat()
{
  const QString preparedUrl { QgsWmsProvider::prepareUri( url() ) };
  QgsWmsCapabilitiesDownload capDownload( preparedUrl, authorizationSettings(), true );

  const QgsTemporaryCursorOverride busyCursor { Qt::WaitCursor };
  bool res = capDownload.downloadCapabilities();

  if ( !res )
  {
    QMessageBox::warning(
      this,
      tr( "WMS Provider" ),
      capDownload.lastError()
    );
    return;
  }

  const QgsWmsParserSettings wmsParserSettings { ignoreAxisOrientation(), invertAxisOrientation() };
  mCapabilities = std::make_unique<QgsWmsCapabilities>( QgsProject::instance()->transformContext(), preparedUrl );
  if ( !mCapabilities->parseResponse( capDownload.response(), wmsParserSettings ) )
  {
    QMessageBox msgBox( QMessageBox::Warning, tr( "WMS Provider" ), tr( "The server you are trying to connect to does not seem to be a WMS server. Please check the URL." ), QMessageBox::Ok, this );
    msgBox.setDetailedText( tr( "Instead of the capabilities string that was expected, the following response has been received:\n\n%1" ).arg( mCapabilities->lastError() ) );
    msgBox.exec();
    return;
  }

  // Store current format value
  QString currentFormat { wmsPreferredFormatCombo()->currentData().toString() };
  if ( currentFormat.isEmpty() )
  {
    currentFormat = QgsSettings().value( u"qgis/lastWmsImageEncoding"_s, "image/png" ).toString();
  }

  wmsPreferredFormatCombo()->clear();

  // Get the list of provider supported formats
  const QStringList supportedFormats { mCapabilities->supportedImageEncodings() };
  const QVector<QgsWmsSupportedFormat> providerSupportedFormats { QgsWmsProvider::supportedFormats() };
  int itemCount { 0 };
  for ( const QgsWmsSupportedFormat &format : std::as_const( providerSupportedFormats ) )
  {
    if ( supportedFormats.contains( format.format ) )
    {
      wmsPreferredFormatCombo()->addItem( format.label, format.format );
      if ( format.format == currentFormat )
      {
        wmsPreferredFormatCombo()->setCurrentIndex( itemCount );
      }
      itemCount++;
    }
  }
}

void QgsWmsNewConnection::initWmsSpecificSettings()
{
  QStringList detailsParameters = { u"wms"_s, originalConnectionName() };
  QString imageFormat = QgsOwsConnection::settingsDefaultImageFormat->value( detailsParameters );

  wmsPreferredFormatCombo()->clear();

  if ( imageFormat.isEmpty() )
  {
    // Read from global default setting
    imageFormat = QgsSettings().value( u"qgis/lastWmsImageEncoding"_s, u"image/png"_s ).toString();
  }


  // Check the settings for available formats
  const QStringList availableFormats = QgsOwsConnection::settingsAvailableImageFormats->value( detailsParameters );
  const QVector<QgsWmsSupportedFormat> supportedFormats { QgsWmsProvider::supportedFormats() };

  // Find the label for the default image format
  QString imageFormatLabel;
  auto it = std::find_if( supportedFormats.cbegin(), supportedFormats.cend(), [imageFormat]( const QgsWmsSupportedFormat &fmt ) { return fmt.format == imageFormat; } );
  if ( it != supportedFormats.cend() )
  {
    imageFormatLabel = it->label;
  }

  QSet<QString> labelsAdded; // deduplication

  // Add the full mime as user data and the base type as label
  for ( const QgsWmsSupportedFormat &fmt : std::as_const( supportedFormats ) )
  {
    if ( !labelsAdded.contains( fmt.label ) && ( availableFormats.isEmpty() || availableFormats.contains( fmt.format ) ) )
    {
      if ( fmt.label == imageFormatLabel )
      {
        wmsPreferredFormatCombo()->addItem( imageFormatLabel, imageFormat );
        wmsPreferredFormatCombo()->setCurrentIndex( wmsPreferredFormatCombo()->count() - 1 );
      }
      else
      {
        wmsPreferredFormatCombo()->addItem( fmt.label, fmt.format );
      }
      labelsAdded.insert( fmt.label );
    }
  }
}
