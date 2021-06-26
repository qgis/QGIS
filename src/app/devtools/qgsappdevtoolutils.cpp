/***************************************************************************
    qgsappdevtoolutils.cpp
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
