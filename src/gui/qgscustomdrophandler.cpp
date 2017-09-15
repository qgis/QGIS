/***************************************************************************
    qgscustomdrophandler.cpp
    ---------------------
    begin                : August 2016
    copyright            : (C) 2016 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscustomdrophandler.h"

QString QgsCustomDropHandler::customUriProviderKey() const
{
  return QString();
}

void QgsCustomDropHandler::handleCustomUriDrop( const QgsMimeDataUtils::Uri &uri ) const
{
  Q_UNUSED( uri );
}

void QgsCustomDropHandler::handleMimeData( const QMimeData *data )
{
  Q_UNUSED( data );
}

bool QgsCustomDropHandler::handleFileDrop( const QString &file )
{
  Q_UNUSED( file );
  return false;
}
