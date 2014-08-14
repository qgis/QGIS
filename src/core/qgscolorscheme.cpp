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

QgsNamedColorList QgsRecentColorScheme::fetchColors( const QString context,
    const QColor baseColor )
{
  Q_UNUSED( context );
  Q_UNUSED( baseColor );

  //fetch recent colors
  QSettings settings;
  QList< QVariant > recentColorVariants = settings.value( QString( "/colors/recent" ) ).toList();

  //generate list from recent colors
  QgsNamedColorList colorList;
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


QgsCustomColorScheme::QgsCustomColorScheme() : QgsColorScheme()
{

}

QgsCustomColorScheme::~QgsCustomColorScheme()
{

}

QgsNamedColorList QgsCustomColorScheme::fetchColors( const QString context, const QColor baseColor )
{
  Q_UNUSED( context );
  Q_UNUSED( baseColor );

  //fetch predefined custom colors
  QSettings settings;

  QList< QVariant > customColorVariants = settings.value( QString( "/colors/palettecolors" ) ).toList();
  QList< QVariant > customColorLabels = settings.value( QString( "/colors/palettelabels" ) ).toList();

  //generate list from custom colors
  QgsNamedColorList colorList;
  int colorIndex = 0;
  for ( QList< QVariant >::iterator it = customColorVariants.begin();
        it != customColorVariants.end(); ++it )
  {
    QColor color = ( *it ).value<QColor>();
    QString label;
    if ( customColorLabels.length() > colorIndex )
    {
      label = customColorLabels.at( colorIndex ).toString();
    }

    colorList.append( qMakePair( color, label ) );
    colorIndex++;
  }

  return colorList;
}

QgsColorScheme *QgsCustomColorScheme::clone() const
{
  return new QgsCustomColorScheme();
}
