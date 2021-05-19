/***************************************************************************
    qgsauthpkipathsmethodgui.cpp
    ---------------------
    begin                : September 1, 2015
    copyright            : (C) 2015 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauthpkipathsedit.h"


/**
 * Optional class factory to return a pointer to a newly created edit widget
 */
QGISEXTERN QgsAuthPkiPathsEdit *editWidget( QWidget *parent )
{
  return new QgsAuthPkiPathsEdit( parent );
}

