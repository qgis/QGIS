/***************************************************************************
  qgsfirstrundialog.cpp - qgsfirstrundialog

 ---------------------
 begin                : 11.12.2017
 copyright            : (C) 2017 by Nathan Woodrow
 email                : woodrow.nathan@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsfirstrundialog.h"
#include "qgis.h"

QgsFirstRunDialog::QgsFirstRunDialog( QWidget *parent ) : QDialog( parent )
{
  setupUi( this );
  mWelcomeDevLabel->hide();
  mWelcomeLabel->setText( tr( "Welcome to QGIS %1" ).arg( Qgis::version() ) );
  if ( Qgis::version().endsWith( QLatin1String( "Master" ) ) )
  {
    mWelcomeDevLabel->show();
  }
  const QStringList versionParts = Qgis::version().split( '.' );
  QString major = versionParts.at( 0 );
  const QString minor = versionParts.at( 1 );
  if ( minor.toInt() % 2 == 1 )
  {
    // Development version doesn't show the link to the changelog
    mWelcomeProdLabel->hide();
  }
  else
  {
    // Production version shows link.
    mWelcomeProdLabel->setText( mWelcomeProdLabel->text().replace( QLatin1String( "VERSION_TOKEN" ), major.append( minor ) ) );
    mWelcomeDevLabel->hide();
  }

}

bool QgsFirstRunDialog::migrateSettings()
{
  return ( mImportSettingsYes->isChecked() );
}

