/***************************************************************************
  qgsauthenticationwidget.cpp - QgsAuthenticationWidget

 ---------------------
 begin                : 28.9.2017
 copyright            : (C) 2017 by Alessandro Pasotti
 email                : apasotti at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsauthenticationwidget.h"

QgsAuthenticationWidget::QgsAuthenticationWidget( QWidget *parent, const QString &dataprovider )
  : QWidget( parent )
{
  setupUi( new QgsAuthConfigSelect( this, dataprovider ) );
}
