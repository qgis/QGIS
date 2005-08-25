/***************************************************************************
                    qgsproviderregistry.h  -  Singleton class for
                    registering data providers.
                             -------------------
    begin                : Sat Jan 10 2004
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 /* $Id$ */
 
#ifndef QGSPROVIDERREGISTRY_H
#define QGSPROVIDERREGISTRY_H

#include <map>

#include <qstring.h>

class QgsDataProvider;
class QgsProviderMetadata;

/** canonical manager of data providers

  This is a Singleton class that manages data provider access.
*/
class QgsProviderRegistry
{
public:

    /** means of accessing canonical single instance
     */
    static QgsProviderRegistry* instance(const char *pluginPath = 0);

    QString library(QString const & providerKey) const;

    QString pluginList(bool asHtml = false) const;

    QString const & libDirectory() const;

    void setLibDirectory(QString const & path);
 
    QgsDataProvider* getProvider( QString const & providerKey, 
                                  QString const & dataSource );

private:

    /** ctor private since instance() creates it */
    QgsProviderRegistry(const char *pluginPath);

    /// pointer to canonical Singleton object
    static QgsProviderRegistry* _instance;

    /// associative container of provider metadata handles
    std::map<QString,QgsProviderMetadata*> provider;

    /// directory provider plugins are installed in
    QString libDir;

}; // class QgsProviderRegistry

#endif //QGSPROVIDERREGISTRY_H

