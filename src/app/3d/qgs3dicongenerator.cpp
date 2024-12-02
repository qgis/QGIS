/***************************************************************************
    qgs3dicongenerator.cpp
    ---------------
    begin                : July 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dicongenerator.h"
#include "moc_qgs3dicongenerator.cpp"
#include "qgsapplication.h"
#include <QDir>

Qgs3DIconGenerator::Qgs3DIconGenerator( QObject *parent )
  : QgsAbstractStyleEntityIconGenerator( parent )
{
}

void Qgs3DIconGenerator::generateIcon( QgsStyle *, QgsStyle::StyleEntity type, const QString &name )
{
  QIcon icon;
  const QList<QSize> sizes = iconSizes();
  if ( sizes.isEmpty() )
    icon.addFile( QgsApplication::defaultThemePath() + QDir::separator() + QStringLiteral( "3d.svg" ), QSize( 24, 24 ) );
  for ( const QSize &s : sizes )
  {
    icon.addFile( QgsApplication::defaultThemePath() + QDir::separator() + QStringLiteral( "3d.svg" ), s );
  }

  emit iconGenerated( type, name, icon );
}
