/***************************************************************************
    qgsnetworkloggerwidgetfactory.cpp
    -------------------------
    begin                : March 2020
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

#include "qgsnetworkloggerwidgetfactory.h"
#include "qgsnetworkloggerpanelwidget.h"
#include "qgsapplication.h"

QgsNetworkLoggerWidgetFactory::QgsNetworkLoggerWidgetFactory( QgsNetworkLogger *logger )
  : QgsDevToolWidgetFactory( QObject::tr( "Network Logger" ), QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/network_and_proxy.svg" ) ) )
  , mLogger( logger )
{
}

QgsDevToolWidget *QgsNetworkLoggerWidgetFactory::createWidget( QWidget *parent ) const
{
  return new QgsNetworkLoggerPanelWidget( mLogger, parent );
}
