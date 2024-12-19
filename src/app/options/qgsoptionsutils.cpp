/***************************************************************************
    qgsoptionsutils.cpp
    -------------------------
    begin                : September 2020
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
#include "qgsoptionsutils.h"

#include "qgisapp.h"
#include "qgis.h"
#include "qgsoptionswidgetfactory.h"

QgsScopedOptionsWidgetFactory::QgsScopedOptionsWidgetFactory( std::unique_ptr< QgsOptionsWidgetFactory > &&factory )
{
  reset( std::move( factory ) );
}

QgsScopedOptionsWidgetFactory::QgsScopedOptionsWidgetFactory( QgsScopedOptionsWidgetFactory &&other )
  : mFactory( std::move( other.mFactory ) )
{

}

QgsScopedOptionsWidgetFactory::~QgsScopedOptionsWidgetFactory()
{
  if ( mFactory )
    QgisApp::instance()->unregisterOptionsWidgetFactory( mFactory.get() );
}

void QgsScopedOptionsWidgetFactory::reset( std::unique_ptr<QgsOptionsWidgetFactory> factory )
{
  if ( mFactory )
    QgisApp::instance()->unregisterOptionsWidgetFactory( mFactory.get() );
  mFactory = std::move( factory );
  if ( mFactory )
    QgisApp::instance()->registerOptionsWidgetFactory( mFactory.get() );
}
