/***************************************************************************
    qgsnetworkloggerwidgetfactory.cpp
    -------------------------
    begin                : March 2020
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

#include "qgsnetworkloggerwidgetfactory.h"

#include "qgsapplication.h"
#include "qgsnetworkloggerpanelwidget.h"

QgsNetworkLoggerWidgetFactory::QgsNetworkLoggerWidgetFactory( QgsNetworkLogger *logger )
  : QgsDevToolWidgetFactory( QObject::tr( "Network Logger" ), QgsApplication::getThemeIcon( u"propertyicons/network_and_proxy.svg"_s ) )
  , mLogger( logger )
{
}

QgsDevToolWidget *QgsNetworkLoggerWidgetFactory::createWidget( QWidget *parent ) const
{
  return new QgsNetworkLoggerPanelWidget( mLogger, parent );
}
