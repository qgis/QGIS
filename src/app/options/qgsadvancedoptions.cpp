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
#include "moc_qgsadvancedoptions.cpp"
#include "qgssettingstreewidget.h"
#include "qgssettingstreewidgetold.h"
#include "qgsapplication.h"
#include "qgis.h"

//
// QgsAdvancedSettingsWidget
//
const QgsSettingsEntryBool *QgsAdvancedSettingsWidget::settingsUseNewTreeWidget = new QgsSettingsEntryBool( QStringLiteral( "use-new-widget" ), sTreeSettings, true, QStringLiteral( "Use new settings widget" ) );
const QgsSettingsEntryBool *QgsAdvancedSettingsWidget::settingsShowWarning = new QgsSettingsEntryBool( QStringLiteral( "show-warning" ), sTreeSettings, true, QStringLiteral( "Show warning before opening the settings tree" ) );


QgsAdvancedSettingsWidget::QgsAdvancedSettingsWidget( QWidget *parent )
  : QgsOptionsPageWidget( parent )
{
  setupUi( this );

  mUseNewSettingsTree->setChecked( settingsUseNewTreeWidget->value() );

  layout()->setContentsMargins( 0, 0, 0, 0 );

  if ( !settingsShowWarning->value() )
  {
    mAdvancedSettingsWarning->hide();
    bool newTree = settingsUseNewTreeWidget->value();
    createSettingsTreeWidget( newTree, !newTree, false );
  }
  else
  {
    createSettingsTreeWidget( true, true, true );

    connect( mAdvancedSettingsEnableButton, &QPushButton::clicked, this, [=] {
      settingsUseNewTreeWidget->setValue( mUseNewSettingsTree->isChecked() );
      mAdvancedSettingsWarning->hide();
      if ( settingsUseNewTreeWidget->value() )
        mTreeWidget->show();
      else
        mTreeWidgetOld->show();
    } );
  }
}

QgsAdvancedSettingsWidget::~QgsAdvancedSettingsWidget()
{
}

QString QgsAdvancedSettingsWidget::helpKey() const
{
  return QStringLiteral( "introduction/qgis_configuration.html#optionsadvanced" );
}

void QgsAdvancedSettingsWidget::apply()
{
  // the old settings editor applies changes immediately
  // new settings tree is performing changes on apply
  if ( mTreeWidget )
    mTreeWidget->applyChanges();
}

void QgsAdvancedSettingsWidget::createSettingsTreeWidget( bool newWidget, bool oldWidget, bool hide )
{
  if ( newWidget )
  {
    mTreeWidget = new QgsSettingsTreeWidget( this );
    mGroupBox->layout()->addWidget( mTreeWidget );
    if ( hide )
      mTreeWidget->hide();
  }

  if ( oldWidget )
  {
    mTreeWidgetOld = new QgsSettingsTreeWidgetOld( this );
    mGroupBox->layout()->addWidget( mTreeWidgetOld );
    if ( hide )
      mTreeWidgetOld->hide();
  }
}

//
// QgsAdvancedSettingsOptionsFactory
//
QgsAdvancedSettingsOptionsFactory::QgsAdvancedSettingsOptionsFactory()
  : QgsOptionsWidgetFactory( QCoreApplication::translate( "QgsOptionsBase", "Advanced" ), QIcon(), QStringLiteral( "advanced" ) )
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
