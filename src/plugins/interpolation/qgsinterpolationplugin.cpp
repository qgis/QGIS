/***************************************************************************
                              qgsinterpolationplugin.cpp
                              --------------------------
  begin                : Marco 10, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgsinterpolationplugin.h"
#include "qgisinterface.h"
#include "qgsinterpolationdialog.h"

#include <QFile>

static const QString name_ = QObject::tr( "Interpolation plugin" );
static const QString description_ = QObject::tr( "A plugin for interpolation based on vertices of a vector layer" );
static const QString category_ = QObject::tr( "Raster" );
static const QString version_ = QObject::tr( "Version 0.001" );
static const QString icon_ = ":/raster-interpolate.png";

QgsInterpolationPlugin::QgsInterpolationPlugin( QgisInterface* iface ): mIface( iface ), mInterpolationAction( 0 )
{

}

QgsInterpolationPlugin::~QgsInterpolationPlugin()
{

}

void QgsInterpolationPlugin::initGui()
{
  if ( mIface )
  {
    mInterpolationAction = new QAction( QIcon( ":/raster-interpolate.png" ), tr( "&Interpolation" ), 0 );
    //~ setCurrentTheme( "" );
    QObject::connect( mInterpolationAction, SIGNAL( triggered() ), this, SLOT( showInterpolationDialog() ) );
    mIface->addRasterToolBarIcon( mInterpolationAction );
    mIface->addPluginToRasterMenu( tr( "&Interpolation" ), mInterpolationAction );
    // this is called when the icon theme is changed
    connect( mIface, SIGNAL( currentThemeChanged( QString ) ), this, SLOT( setCurrentTheme( QString ) ) );
  }
}

void QgsInterpolationPlugin::unload()
{
  mIface->removePluginRasterMenu( tr( "&Interpolation" ), mInterpolationAction );
  mIface->removeRasterToolBarIcon( mInterpolationAction );
  delete mInterpolationAction;
}

void QgsInterpolationPlugin::showInterpolationDialog()
{
  QgsInterpolationDialog dialog( mIface->mainWindow(), mIface );
  dialog.exec();
}

//~ //! Set icons to the current theme
//~ void QgsInterpolationPlugin::setCurrentTheme( QString theThemeName )
//~ {
  //~ Q_UNUSED( theThemeName );
  //~ QString myCurThemePath = QgsApplication::activeThemePath() + "/plugins/interpolation.png";
  //~ QString myDefThemePath = QgsApplication::defaultThemePath() + "/plugins/interpolation.png";
  //~ QString myQrcPath = ":/interpolation.png";
  //~ if ( QFile::exists( myCurThemePath ) )
  //~ {
    //~ mInterpolationAction->setIcon( QIcon( myCurThemePath ) );
  //~ }
  //~ else if ( QFile::exists( myDefThemePath ) )
  //~ {
    //~ mInterpolationAction->setIcon( QIcon( myDefThemePath ) );
  //~ }
  //~ else if ( QFile::exists( myQrcPath ) )
  //~ {
    //~ mInterpolationAction->setIcon( QIcon( myQrcPath ) );
  //~ }
  //~ else
  //~ {
    //~ mInterpolationAction->setIcon( QIcon() );
  //~ }
//~ }

QGISEXTERN QgisPlugin * classFactory( QgisInterface * theQgisInterfacePointer )
{
  return new QgsInterpolationPlugin( theQgisInterfacePointer );
}

QGISEXTERN QString name()
{
  return name_;
}

QGISEXTERN QString description()
{
  return description_;
}

QGISEXTERN QString category()
{
  return category_;
}

QGISEXTERN QString version()
{
  return version_;
}

QGISEXTERN QString icon()
{
  return icon_;
}

QGISEXTERN int type()
{
  return QgisPlugin::UI;
}

QGISEXTERN void unload( QgisPlugin* theInterpolationPluginPointer )
{
  delete theInterpolationPluginPointer;
}
