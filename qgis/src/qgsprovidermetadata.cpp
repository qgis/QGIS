/***************************************************************************
                    qgsprovidermetadata.cpp  -  Metadata class for
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
#include <qstring.h>
#include "qgsprovidermetadata.h"

QgsProviderMetadata::QgsProviderMetadata(QString _key, QString _description, QString _library):key_(_key), description_(_description),
library_(_library)
{
}

QString QgsProviderMetadata::key()
{
  return key_;
}

QString QgsProviderMetadata::description()
{
  return description_;
}

QString QgsProviderMetadata::library()
{
  return library_;
}
