/***************************************************************************
                              qgscapabilitiescache.sip
                              -------------------
  begin                : 2014-09-10
  copyright            : (C) 2014 by Alessandro Pasotti
  email                : a dot pasotti at itopen dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**
* \class QgsCapabilitiesCache
* \brief A cache for capabilities xml documents (by configuration file path)
*/
class QgsCapabilitiesCache: QObject
{
%TypeHeaderCode
#include "qgscapabilitiescache.h"
%End
  public:

    /**Returns cached capabilities document (or 0 if document for configuration file not in cache)*/
    const QDomDocument* searchCapabilitiesDocument( QString configFilePath, QString version );
    /**Inserts new capabilities document (creates a copy of the document, does not take ownership)*/
    void insertCapabilitiesDocument( QString configFilePath, QString version, const QDomDocument* doc );

};


