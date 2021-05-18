/***************************************************************************
    qgsauthoauth2methodgui.cpp
    ---------------------
    begin                : July 13, 2016
    copyright            : (C) 2016 by Monsanto Company, USA
    author               : Larry Shaffer, Boundless Spatial
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauthoauth2edit.h"

static const QString AUTH_METHOD_KEY = QStringLiteral( "OAuth2" );


/**
 * Optional class factory to return a pointer to a newly created edit widget
 */
QGISEXTERN QgsAuthOAuth2Edit *editWidget( QWidget *parent )
{
  return new QgsAuthOAuth2Edit( parent );
}

/**
 * Required isAuthMethodGui function. Used to determine if this shared library
 * is an authentication method plugin
 */
QGISEXTERN bool isAuthMethodGui()
{
  return true;
}

/**
 * Required key function (used to map the plugin to a data store type)
 */
QGISEXTERN QString authMethodKey()
{
  return AUTH_METHOD_KEY;
}
