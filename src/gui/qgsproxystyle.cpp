/***************************************************************************
  qgsproxystyle.cpp
  -----------------
  Date                 : March 2018
  Copyright            : (C) 2018 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsproxystyle.h"
#include <QStyleFactory>
#include <QStyle>
#include <QApplication>

QgsProxyStyle::QgsProxyStyle( QWidget *parent )
  : QProxyStyle( nullptr ) // no base style yet - it transfer ownership, so we need a NEW QStyle object for the base style
{
  // get application style
  QString appStyle = QApplication::style()->objectName();
  if ( !appStyle.isEmpty() )
  {
    if ( QStyle *style = QStyleFactory::create( appStyle ) )
      setBaseStyle( style );
  }

  // set lifetime to match parent widget's
  setParent( parent );
}
