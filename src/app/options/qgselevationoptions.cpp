/***************************************************************************
    qgselevationoptions.h
    -------------------------
    begin                : September 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgselevationoptions.h"
#include "moc_qgselevationoptions.cpp"

#include "qgssettings.h"
#include "qgsapplication.h"
#include "elevation/qgselevationprofilewidget.h"
#include <QDir>

//
// QgsElevationOptionsWidget
//

QgsElevationOptionsWidget::QgsElevationOptionsWidget( QWidget *parent )
  : QgsOptionsPageWidget( parent )
{
  setupUi( this );

  mButtonBackgroundColor->setShowNull( true, tr( "Use Default" ) );
  mButtonBackgroundColor->setColorDialogTitle( tr( "Chart Background Color" ) );

  const QColor backgroundColor = QgsElevationProfileWidget::settingBackgroundColor->value();
  if ( backgroundColor.isValid() )
    mButtonBackgroundColor->setColor( backgroundColor );
  else
    mButtonBackgroundColor->setToNull();
}

QString QgsElevationOptionsWidget::helpKey() const
{
  return QStringLiteral( "introduction/qgis_configuration.html#elevation-options" );
}

void QgsElevationOptionsWidget::apply()
{
  QgsElevationProfileWidget::settingBackgroundColor->setValue(
    mButtonBackgroundColor->isNull() ? QColor() : mButtonBackgroundColor->color()
  );
}

//
// QgsElevationOptionsFactory
//
QgsElevationOptionsFactory::QgsElevationOptionsFactory()
  : QgsOptionsWidgetFactory( tr( "Elevation" ), QIcon(), QStringLiteral( "elevation" ) )
{
}

QIcon QgsElevationOptionsFactory::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/elevationscale.svg" ) );
}

QgsOptionsPageWidget *QgsElevationOptionsFactory::createWidget( QWidget *parent ) const
{
  return new QgsElevationOptionsWidget( parent );
}

QString QgsElevationOptionsFactory::pagePositionHint() const
{
  return QStringLiteral( "mOptionsPageColors" );
}
