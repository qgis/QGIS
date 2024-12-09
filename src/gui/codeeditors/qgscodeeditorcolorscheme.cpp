/***************************************************************************
    qgscodeeditorcolorscheme.cpp
     --------------------------------------
    Date                 : October 2020
    Copyright            : (C) 2020 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgscodeeditorcolorscheme.h"


QgsCodeEditorColorScheme::QgsCodeEditorColorScheme( const QString &id, const QString &name )
  : mId( id )
  , mThemeName( name )
{
}

QColor QgsCodeEditorColorScheme::color( QgsCodeEditorColorScheme::ColorRole role ) const
{
  return mColors.value( role );
}

void QgsCodeEditorColorScheme::setColor( QgsCodeEditorColorScheme::ColorRole role, const QColor &color )
{
  mColors[role] = color;
}

void QgsCodeEditorColorScheme::setColors( const QMap<QgsCodeEditorColorScheme::ColorRole, QColor> &colors )
{
  mColors = colors;
}
