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


/**
 * Optional class factory to return a pointer to a newly created edit widget
 */
QGISEXTERN QgsAuthEsriTokenEdit *editWidget( QWidget *parent )
{
  return new QgsAuthEsriTokenEdit( parent );
}

