/***************************************************************************
    qgsappdevtoolutils.cpp
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
#include "qgsappdevtoolutils.h"

#include "qgisapp.h"
#include "qgis.h"
#include "qgsdevtoolwidgetfactory.h"

QgsScopedDevToolWidgetFactory::QgsScopedDevToolWidgetFactory() = default;

QgsScopedDevToolWidgetFactory::~QgsScopedDevToolWidgetFactory()
{
  if ( mFactory )
    QgisApp::instance()->unregisterDevToolFactory( mFactory.get() );
}

void QgsScopedDevToolWidgetFactory::reset( std::unique_ptr<QgsDevToolWidgetFactory> factory )
{
  if ( mFactory )
    QgisApp::instance()->unregisterDevToolFactory( mFactory.get() );
  mFactory = std::move( factory );
  if ( mFactory )
    QgisApp::instance()->registerDevToolFactory( mFactory.get() );
}
