/***************************************************************************
    qgsadvancedoptions.cpp
    -------------------------
    begin                : September 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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

QgsSettingsTree *QgsAdvancedSettingsWidget::settingsTree()
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
