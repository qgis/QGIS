/***************************************************************************
                    qgsprovidermetadata.h  -  Metadata class for
                    describing a data provider.
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
 
#ifndef QGSPROVIDERMETADATA_H
#define QGSPROVIDERMETADATA_H
#include <map>
class QgsProviderMetadata
{
public:
 QgsProviderMetadata(QString _key, QString _description, QString _library);
 QString key();
 QString description();
 QString library();
private:
 QString key_;
 QString description_;
 QString library_;
};
#endif //QGSPROVIDERMETADATA_H

