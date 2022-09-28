/***************************************************************************
    qgsgpsoptions.cpp
    -----------------
    begin                : July 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgpsoptions.h"
#include "qgssettings.h"
#include "qgsapplication.h"
#include "qgsgpsmarker.h"
#include "qgsmarkersymbol.h"
#include "qgssymbollayerutils.h"

//
// QgsGpsOptionsWidget
//

QgsGpsOptionsWidget::QgsGpsOptionsWidget( QWidget *parent )
  : QgsOptionsPageWidget( parent )
{
  setupUi( this );

  mGpsMarkerSymbolButton->setSymbolType( Qgis::SymbolType::Marker );

  const QString defaultSymbol = QgsGpsMarker::settingLocationMarkerSymbol.value();
  QDomDocument symbolDoc;
  symbolDoc.setContent( defaultSymbol );
  const QDomElement markerElement = symbolDoc.documentElement();
  std::unique_ptr< QgsMarkerSymbol > gpsMarkerSymbol( QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( markerElement, QgsReadWriteContext() ) );
  if ( gpsMarkerSymbol )
    mGpsMarkerSymbolButton->setSymbol( gpsMarkerSymbol.release() );

  mCheckRotateLocationMarker->setChecked( QgsGpsMarker::settingRotateLocationMarker.value() );
}

void QgsGpsOptionsWidget::apply()
{
  if ( QgsSymbol *markerSymbol = mGpsMarkerSymbolButton->symbol() )
  {
    QDomDocument doc;
    QDomElement elem = QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "location-marker-symbol" ), markerSymbol, doc, QgsReadWriteContext() );
    doc.appendChild( elem );
    QgsGpsMarker::settingLocationMarkerSymbol.setValue( doc.toString( 0 ) );
  }
  QgsGpsMarker::settingRotateLocationMarker.setValue( mCheckRotateLocationMarker->isChecked() );
}


//
// QgsGpsOptionsFactory
//
QgsGpsOptionsFactory::QgsGpsOptionsFactory()
  : QgsOptionsWidgetFactory( tr( "GPS" ), QIcon() )
{

}

QIcon QgsGpsOptionsFactory::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconGps.svg" ) );
}

QgsOptionsPageWidget *QgsGpsOptionsFactory::createWidget( QWidget *parent ) const
{
  return new QgsGpsOptionsWidget( parent );
}

QString QgsGpsOptionsFactory::pagePositionHint() const
{
  return QStringLiteral( "mOptionsLocatorSettings" );
}
