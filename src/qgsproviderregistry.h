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
class QgsProviderMetadata;
class QString;

class QgsProviderRegistry
{
public:
 static QgsProviderRegistry* instance();
 QString library(QString providerKey);
 QString pluginList(bool asHtml=false);
protected:
 QgsProviderRegistry();
private:
 static QgsProviderRegistry* _instance;
 std::map<QString,QgsProviderMetadata*> provider;
};
#endif //QGSPROVIDERREGISTRY_H

