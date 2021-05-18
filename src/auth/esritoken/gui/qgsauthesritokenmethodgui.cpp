/***************************************************************************
    qgsauthesritokenmethodgui.cpp
    --------------------------
    begin                : October 2018
    copyright            : (C) 2018 by Nyall Dawson
    author               : Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauthesritokenedit.h"

static const QString AUTH_METHOD_KEY = QStringLiteral( "EsriToken" );


/**
 * Optional class factory to return a pointer to a newly created edit widget
 */
QGISEXTERN QgsAuthEsriTokenEdit *editWidget( QWidget *parent )
{
  return new QgsAuthEsriTokenEdit( parent );
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
