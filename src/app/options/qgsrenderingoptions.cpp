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

#include <QThread>

#include "moc_qgsrenderingoptions.cpp"

//
// QgsRenderingOptionsWidget
//

QgsRenderingOptionsWidget::QgsRenderingOptionsWidget( QWidget *parent )
  : QgsOptionsPageWidget( parent )
{
  setupUi( this );

  QgsSettings settings;
  chkAddedVisibility->setChecked( settings.value( u"/qgis/new_layers_visible"_s, true ).toBool() );

  spinMaxThreads->setRange( 1, QThread::idealThreadCount() );
  spinMaxThreads->setClearValue( 1, tr( "All Available (%1)" ).arg( QThread::idealThreadCount() ) );
  if ( QgsApplication::maxThreads() != -1 )
    spinMaxThreads->setValue( QgsApplication::maxThreads() );
  else
    spinMaxThreads->clear();

  spinMapUpdateInterval->setValue( settings.value( u"/qgis/map_update_interval"_s, 250 ).toInt() );
  spinMapUpdateInterval->setClearValue( 250 );

  double magnifierMin = 100 * QgsGuiUtils::CANVAS_MAGNIFICATION_MIN;
  double magnifierMax = 100 * QgsGuiUtils::CANVAS_MAGNIFICATION_MAX;
  double magnifierVal = 100 * settings.value( u"/qgis/magnifier_factor_default"_s, 1.0 ).toDouble();
  doubleSpinBoxMagnifierDefault->setRange( magnifierMin, magnifierMax );
  doubleSpinBoxMagnifierDefault->setSingleStep( 50 );
  doubleSpinBoxMagnifierDefault->setDecimals( 0 );
  doubleSpinBoxMagnifierDefault->setSuffix( u"%"_s );
  doubleSpinBoxMagnifierDefault->setValue( magnifierVal );
  doubleSpinBoxMagnifierDefault->setClearValue( 100 );

  chkAntiAliasing->setChecked( settings.value( u"/qgis/enable_anti_aliasing"_s, true ).toBool() );
}

QString QgsRenderingOptionsWidget::helpKey() const
{
  return u"introduction/qgis_configuration.html#rendering-options"_s;
}

void QgsRenderingOptionsWidget::apply()
{
  QgsSettings settings;
  settings.setValue( u"/qgis/new_layers_visible"_s, chkAddedVisibility->isChecked() );

  const int maxThreads = spinMaxThreads->value() == spinMaxThreads->clearValue() ? -1 : spinMaxThreads->value();
  QgsApplication::setMaxThreads( maxThreads );
  settings.setValue( u"/qgis/max_threads"_s, maxThreads );

  settings.setValue( u"/qgis/map_update_interval"_s, spinMapUpdateInterval->value() );

  // magnification
  settings.setValue( u"/qgis/magnifier_factor_default"_s, doubleSpinBoxMagnifierDefault->value() / 100 );

  settings.setValue( u"/qgis/enable_anti_aliasing"_s, chkAntiAliasing->isChecked() );
}


//
// QgsRenderingOptionsFactory
//
QgsRenderingOptionsFactory::QgsRenderingOptionsFactory()
  : QgsOptionsWidgetFactory( tr( "Rendering" ), QIcon(), u"rendering"_s )
{
}

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
