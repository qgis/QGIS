/***************************************************************************
    qgsauthmethod.cpp
    ---------------------
    begin                : March 2021
    copyright            : (C) 2021
    author               : Matthias Khn
    email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauthmethod.h"

#ifdef HAVE_GUI
QWidget *QgsAuthMethod::editWidget( QWidget *parent ) const
{
  Q_UNUSED( parent )
  return nullptr;
}
#endif

QgsAuthMethod::QgsAuthMethod()
{}
