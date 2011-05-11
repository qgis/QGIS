/***************************************************************************
                              qgscapabilitiescache.h
                              ----------------------
  begin                : May 11th, 2011
  copyright            : (C) 2011 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCAPABILITIESCACHE_H
#define QGSCAPABILITIESCACHE_H

#include <QDomDocument>
#include <QHash>

/**A cache for capabilities xml documents (by configuration file path)*/
class QgsCapabilitiesCache
{
  public:
    QgsCapabilitiesCache();
    ~QgsCapabilitiesCache();

    /**Returns cached capabilities document (or 0 if document for configuration file not in cache)*/
    const QDomDocument* searchCapabilitiesDocument( const QString& configFilePath ) const;
    /**Inserts new capabilities document (creates a copy of the document, does not take ownership)*/
    void insertCapabilitiesDocument( const QString& configFilePath, const QDomDocument* doc );

  private:
    QHash< QString, QDomDocument > mCachedCapabilities;
};

#endif // QGSCAPABILITIESCACHE_H
