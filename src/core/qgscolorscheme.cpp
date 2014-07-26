/***************************************************************************
                             qgscolorscheme.cpp
                             -------------------
    begin                : July 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscolorscheme.h"

#include <QSettings>


QgsColorScheme::QgsColorScheme()
{

}

QgsColorScheme::~QgsColorScheme()
{

}


//
// QgsRecentColorScheme
//

QgsRecentColorScheme::QgsRecentColorScheme() : QgsColorScheme()
{

}

QgsRecentColorScheme::~QgsRecentColorScheme()
{

}

QList< QPair< QColor, QString > > QgsRecentColorScheme::fetchColors( const QString context,
    const QColor baseColor )
{
  Q_UNUSED( context );
  Q_UNUSED( baseColor );

  //fetch recent colors
  QSettings settings;
  QList< QVariant > recentColorVariants = settings.value( QString( "/colors/recent" ) ).toList();

  //generate list from recent colors
  QList< QPair< QColor, QString > > colorList;
  foreach ( QVariant color, recentColorVariants )
  {
    colorList.append( qMakePair( color.value<QColor>(), QString() ) );
  }
  return colorList;
}

QgsColorScheme *QgsRecentColorScheme::clone() const
{
  return new QgsRecentColorScheme();
}
