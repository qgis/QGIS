/***************************************************************************
                        qgssettingslocatorfilters.cpp
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

#include "qgssettingslocatorfilter.h"
#include "qgisapp.h"


QgsSettingsLocatorFilter::QgsSettingsLocatorFilter( QObject *parent )
  : QgsLocatorFilter( parent )
{}

QgsSettingsLocatorFilter *QgsSettingsLocatorFilter::clone() const
{
  return new QgsSettingsLocatorFilter();
}

void QgsSettingsLocatorFilter::fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback * )
{
  QMap<QString, QMap<QString, QString>> matchingSettingsPagesMap;

  const QMap<QString, QString > optionsPagesMap = QgisApp::instance()->optionsPagesMap();
  for ( auto optionsPagesIterator = optionsPagesMap.constBegin(); optionsPagesIterator != optionsPagesMap.constEnd(); ++optionsPagesIterator )
  {
    const QString title = optionsPagesIterator.key();
    matchingSettingsPagesMap.insert( title + " (" + tr( "Options" ) + ")", settingsPage( QStringLiteral( "optionpage" ), optionsPagesIterator.value() ) );
  }

  const QMap<QString, QString> projectPropertyPagesMap = QgisApp::instance()->projectPropertiesPagesMap();
  for ( auto projectPropertyPagesIterator = projectPropertyPagesMap.constBegin(); projectPropertyPagesIterator != projectPropertyPagesMap.constEnd(); ++projectPropertyPagesIterator )
  {
    const QString title = projectPropertyPagesIterator.key();
    matchingSettingsPagesMap.insert( title + " (" + tr( "Project Properties" ) + ")", settingsPage( QStringLiteral( "projectpropertypage" ), projectPropertyPagesIterator.value() ) );
  }

  const QMap<QString, QString> settingPagesMap = QgisApp::instance()->settingPagesMap();
  for ( auto settingPagesIterator = settingPagesMap.constBegin(); settingPagesIterator != settingPagesMap.constEnd(); ++settingPagesIterator )
  {
    const QString title = settingPagesIterator.key();
    matchingSettingsPagesMap.insert( title, settingsPage( QStringLiteral( "settingspage" ), settingPagesIterator.value() ) );
  }

  for ( auto matchingSettingsPagesIterator = matchingSettingsPagesMap.constBegin(); matchingSettingsPagesIterator != matchingSettingsPagesMap.constEnd(); ++matchingSettingsPagesIterator )
  {
    const QString title = matchingSettingsPagesIterator.key();
    const QMap<QString, QString> settingsPage = matchingSettingsPagesIterator.value();
    QgsLocatorResult result;
    result.filter = this;
    result.displayString = title;
    result.userData().setValue( settingsPage );

    if ( context.usingPrefix && string.isEmpty() )
    {
      emit resultFetched( result );
      continue;
    }

    result.score = fuzzyScore( result.displayString, string );

    if ( result.score > 0 )
      emit resultFetched( result );
  }
}

QMap<QString, QString> QgsSettingsLocatorFilter::settingsPage( const QString &type,  const QString &page )
{
  QMap<QString, QString> returnPage;
  returnPage.insert( QStringLiteral( "type" ), type );
  returnPage.insert( QStringLiteral( "page" ), page );
  return returnPage;
}

void QgsSettingsLocatorFilter::triggerResult( const QgsLocatorResult &result )
{

  const QMap<QString, QString> settingsPage = qvariant_cast<QMap<QString, QString>>( result.userData() );
  const QString type = settingsPage.value( QStringLiteral( "type" ) );
  const QString page = settingsPage.value( QStringLiteral( "page" ) );

  if ( type == QLatin1String( "optionpage" ) )
  {
    QgisApp::instance()->showOptionsDialog( QgisApp::instance(), page );
  }
  else if ( type == QLatin1String( "projectpropertypage" ) )
  {
    QgisApp::instance()->showProjectProperties( page );
  }
  else if ( type == QLatin1String( "settingspage" ) )
  {
    QgisApp::instance()->showSettings( page );
  }
}
