/***************************************************************************
    qgsgpsdeviceoptions.cpp
    -------------------------
    begin                : July 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrenderingoptions.h"

#include "qgsapplication.h"
#include "qgsguiutils.h"
#include "qgssettings.h"
#include "qgssettingsregistrycore.h"
#include "qgssettingsregistrygui.h"

#include <QString>
#include <QThread>

#include "moc_qgsrenderingoptions.cpp"

using namespace Qt::StringLiterals;

//
// QgsRenderingOptionsWidget
//

QgsRenderingOptionsWidget::QgsRenderingOptionsWidget( QWidget *parent )
  : QgsOptionsPageWidget( parent )
{
  setupUi( this );

  QgsSettings settings;
  chkAddedVisibility->setChecked( QgsSettingsRegistryGui::settingsNewLayersVisible->value() );

  spinMaxThreads->setRange( 1, QThread::idealThreadCount() );
  spinMaxThreads->setClearValue( 1, tr( "All Available (%1)" ).arg( QThread::idealThreadCount() ) );
  if ( QgsApplication::maxThreads() != -1 )
    spinMaxThreads->setValue( QgsApplication::maxThreads() );
  else
    spinMaxThreads->clear();

  spinMapUpdateInterval->setValue( QgsSettingsRegistryGui::settingsMapUpdateInterval->value() );
  spinMapUpdateInterval->setClearValue( 250 );

  double magnifierMin = 100 * QgsGuiUtils::CANVAS_MAGNIFICATION_MIN;
  double magnifierMax = 100 * QgsGuiUtils::CANVAS_MAGNIFICATION_MAX;
  double magnifierVal = 100 * QgsSettingsRegistryGui::settingsMagnifierFactorDefault->value();
  doubleSpinBoxMagnifierDefault->setRange( magnifierMin, magnifierMax );
  doubleSpinBoxMagnifierDefault->setSingleStep( 50 );
  doubleSpinBoxMagnifierDefault->setDecimals( 0 );
  doubleSpinBoxMagnifierDefault->setSuffix( u"%"_s );
  doubleSpinBoxMagnifierDefault->setValue( magnifierVal );
  doubleSpinBoxMagnifierDefault->setClearValue( 100 );

  chkAntiAliasing->setChecked( QgsSettingsRegistryGui::settingsEnableAntiAliasing->value() );
}

QString QgsRenderingOptionsWidget::helpKey() const
{
  return u"introduction/qgis_configuration.html#rendering-options"_s;
}

void QgsRenderingOptionsWidget::apply()
{
  QgsSettings settings;
  QgsSettingsRegistryGui::settingsNewLayersVisible->setValue( chkAddedVisibility->isChecked() );

  const int maxThreads = spinMaxThreads->value() == spinMaxThreads->clearValue() ? -1 : spinMaxThreads->value();
  QgsApplication::setMaxThreads( maxThreads );
  settings.setValue( u"/qgis/max_threads"_s, maxThreads );

  QgsSettingsRegistryGui::settingsMapUpdateInterval->setValue( spinMapUpdateInterval->value() );

  // magnification
  QgsSettingsRegistryGui::settingsMagnifierFactorDefault->setValue( doubleSpinBoxMagnifierDefault->value() / 100 );

  QgsSettingsRegistryGui::settingsEnableAntiAliasing->setValue( chkAntiAliasing->isChecked() );
}


//
// QgsRenderingOptionsFactory
//
QgsRenderingOptionsFactory::QgsRenderingOptionsFactory()
  : QgsOptionsWidgetFactory( tr( "Rendering" ), QIcon(), u"rendering"_s )
{}

QIcon QgsRenderingOptionsFactory::icon() const
{
  return QgsApplication::getThemeIcon( u"propertyicons/rendering.svg"_s );
}

QgsOptionsPageWidget *QgsRenderingOptionsFactory::createWidget( QWidget *parent ) const
{
  return new QgsRenderingOptionsWidget( parent );
}

QString QgsRenderingOptionsFactory::pagePositionHint() const
{
  return u"mOptionsPageMapCanvas"_s;
}
