/***************************************************************************
    qgsqueryloggerwidgetfactory.cpp
    -------------------------
    begin                : October 2021
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

#include "qgsqueryloggerwidgetfactory.h"
#include "qgsqueryloggerpanelwidget.h"
#include "qgsapplication.h"

QgsDatabaseQueryLoggerWidgetFactory::QgsDatabaseQueryLoggerWidgetFactory( QgsAppQueryLogger *logger )
  : QgsDevToolWidgetFactory( QObject::tr( "Query Logger" ), QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/database.svg" ) ) )
  , mLogger( logger )
{
}

QgsDevToolWidget *QgsDatabaseQueryLoggerWidgetFactory::createWidget( QWidget *parent ) const
{
  return new QgsDatabaseQueryLoggerPanelWidget( mLogger, parent );
}
