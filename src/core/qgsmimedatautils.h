/***************************************************************************
    qgsmimedatautils.h
    ---------------------
    begin                : November 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMIMEDATAUTILS_H
#define QGSMIMEDATAUTILS_H

#include <QMimeData>

class QgsLayerItem;

class CORE_EXPORT QgsMimeDataUtils
{
  public:

    struct CORE_EXPORT Uri
    {
      Uri( QgsLayerItem* layer );
      Uri( QString& encData );

      QString data() const;

      QString layerType;
      QString providerKey;
      QString name;
      QString uri;
    };
    typedef QList<Uri> UriList;

    static QMimeData* encodeUriList( UriList layers );

    static bool isUriList( const QMimeData* data );

    static UriList decodeUriList( const QMimeData* data );

};

#endif // QGSMIMEDATAUTILS_H
