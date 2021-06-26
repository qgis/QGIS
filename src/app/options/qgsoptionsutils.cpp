/***************************************************************************
    qgsoptionsutils.cpp
    -------------------------
    begin                : September 2020
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
#include "qgsoptionsutils.h"

#include "qgisapp.h"
#include "qgis.h"
#include "qgsoptionswidgetfactory.h"

QgsScopedOptionsWidgetFactory::QgsScopedOptionsWidgetFactory() = default;

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
