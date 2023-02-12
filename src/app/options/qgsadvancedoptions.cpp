/***************************************************************************
    qgsadvancedoptions.cpp
    -------------------------
    begin                : September 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsadvancedoptions.h"
#include "qgsapplication.h"
#include "qgssettings.h"
#include "qgis.h"

//
// QgsAdvancedSettingsWidget
//

QgsAdvancedSettingsWidget::QgsAdvancedSettingsWidget( QWidget *parent )
  : QgsOptionsPageWidget( parent )
{
  setupUi( this );

  layout()->setContentsMargins( 0, 0, 0, 0 );

  connect( mAdvancedSettingsEnableButton, &QPushButton::clicked, this, [ = ]
  {
    mAdvancedSettingsEditor->show();
    mAdvancedSettingsWarning->hide();
  } );
}

QgsAdvancedSettingsWidget::~QgsAdvancedSettingsWidget()
{
}

void QgsAdvancedSettingsWidget::apply()
{
// nothing to do -- mAdvancedSettingsEditor applies changes immediately
}

QgsSettingsTreeWidget *QgsAdvancedSettingsWidget::settingsTree()
{
  return mAdvancedSettingsEditor;
}

//
// QgsAdvancedSettingsOptionsFactory
//
QgsAdvancedSettingsOptionsFactory::QgsAdvancedSettingsOptionsFactory()
  : QgsOptionsWidgetFactory( QCoreApplication::translate( "QgsOptionsBase", "Advanced" ), QIcon() )
{

}

QIcon QgsAdvancedSettingsOptionsFactory::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconWarning.svg" ) );
}

QgsOptionsPageWidget *QgsAdvancedSettingsOptionsFactory::createWidget( QWidget *parent ) const
{
  return new QgsAdvancedSettingsWidget( parent );
}
