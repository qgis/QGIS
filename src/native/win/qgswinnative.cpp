/***************************************************************************
    qgswinnative.cpp - abstracted interface to native system calls
                             -------------------
    begin                : January 2017
    copyright            : (C) 2017 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswinnative.h"
#include <QString>
#include <QDir>

void QgsWinNative::openFileExplorerAndSelectFile( const QString &path )
{
  const QString nativePath = QDir::toNativeSeparators( path );
  ITEMIDLIST *pidl = ILCreateFromPath( nativePath.toUtf8().constData() );
  if ( pidl )
  {
    SHOpenFolderAndSelectItems( pidl, 0, 0, 0 );
    ILFree( pidl );
  }
}
