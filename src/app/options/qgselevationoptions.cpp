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

#include "elevation/qgselevationprofilewidget.h"
#include "qgsapplication.h"
#include "qgssettings.h"

#include <QDir>

#include "moc_qgselevationoptions.cpp"

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
  return u"introduction/qgis_configuration.html#elevation-options"_s;
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
  : QgsOptionsWidgetFactory( tr( "Elevation" ), QIcon(), u"elevation"_s )
{
}

QIcon QgsElevationOptionsFactory::icon() const
{
  return QgsApplication::getThemeIcon( u"propertyicons/elevationscale.svg"_s );
}

QgsOptionsPageWidget *QgsElevationOptionsFactory::createWidget( QWidget *parent ) const
{
  return new QgsElevationOptionsWidget( parent );
}

QString QgsElevationOptionsFactory::pagePositionHint() const
{
  return u"mOptionsPageColors"_s;
}
