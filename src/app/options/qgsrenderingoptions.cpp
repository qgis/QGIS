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
#include "qgssettings.h"
#include "qgsapplication.h"
#include "qgssettingsregistrycore.h"
#include "qgsguiutils.h"

#include <QThread>
//
// QgsRenderingOptionsWidget
//

QgsRenderingOptionsWidget::QgsRenderingOptionsWidget( QWidget *parent )
  : QgsOptionsPageWidget( parent )
{
  setupUi( this );

  QgsSettings settings;
  chkAddedVisibility->setChecked( settings.value( QStringLiteral( "/qgis/new_layers_visible" ), true ).toBool() );

  spinMaxThreads->setRange( 0, QThread::idealThreadCount() );
  spinMaxThreads->setClearValue( 0, tr( "All Available (%1)" ).arg( QThread::idealThreadCount() ) );
  if ( QgsApplication::maxThreads() != -1 )
    spinMaxThreads->setValue( QgsApplication::maxThreads() );
  else
    spinMaxThreads->clear();

  spinMapUpdateInterval->setValue( settings.value( QStringLiteral( "/qgis/map_update_interval" ), 250 ).toInt() );
  spinMapUpdateInterval->setClearValue( 250 );

  double magnifierMin = 100 * QgsGuiUtils::CANVAS_MAGNIFICATION_MIN;
  double magnifierMax = 100 * QgsGuiUtils::CANVAS_MAGNIFICATION_MAX;
  double magnifierVal = 100 * settings.value( QStringLiteral( "/qgis/magnifier_factor_default" ), 1.0 ).toDouble();
  doubleSpinBoxMagnifierDefault->setRange( magnifierMin, magnifierMax );
  doubleSpinBoxMagnifierDefault->setSingleStep( 50 );
  doubleSpinBoxMagnifierDefault->setDecimals( 0 );
  doubleSpinBoxMagnifierDefault->setSuffix( QStringLiteral( "%" ) );
  doubleSpinBoxMagnifierDefault->setValue( magnifierVal );
  doubleSpinBoxMagnifierDefault->setClearValue( 100 );

  chkAntiAliasing->setChecked( settings.value( QStringLiteral( "/qgis/enable_anti_aliasing" ), true ).toBool() );
}

void QgsRenderingOptionsWidget::apply()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "/qgis/new_layers_visible" ), chkAddedVisibility->isChecked() );

  const int maxThreads = spinMaxThreads->value() == spinMaxThreads->clearValue() ? -1 : spinMaxThreads->value();
  QgsApplication::setMaxThreads( maxThreads );
  settings.setValue( QStringLiteral( "/qgis/max_threads" ), maxThreads );

  settings.setValue( QStringLiteral( "/qgis/map_update_interval" ), spinMapUpdateInterval->value() );

  // magnification
  settings.setValue( QStringLiteral( "/qgis/magnifier_factor_default" ), doubleSpinBoxMagnifierDefault->value() / 100 );

  settings.setValue( QStringLiteral( "/qgis/enable_anti_aliasing" ), chkAntiAliasing->isChecked() );
}


//
// QgsRenderingOptionsFactory
//
QgsRenderingOptionsFactory::QgsRenderingOptionsFactory()
  : QgsOptionsWidgetFactory( tr( "Rendering" ), QIcon() )
{
}

QIcon QgsRenderingOptionsFactory::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/rendering.svg" ) );
}

QgsOptionsPageWidget *QgsRenderingOptionsFactory::createWidget( QWidget *parent ) const
{
  return new QgsRenderingOptionsWidget( parent );
}

QString QgsRenderingOptionsFactory::pagePositionHint() const
{
  return QStringLiteral( "mOptionsPageMapCanvas" );
}
