/***************************************************************************
                          qgsogrhelperfunctions.h
    helper functions to create ogr uris for database and protocol drivers
                             -------------------
    begin                : Mon Jan 2 2009
    copyright            : (C) 2009 by Godofredo Contreras Nava
    email                : frdcn at hotmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QString>

/* Create database uri from connection parameters */
QString createDatabaseURI( const QString& connectionType, const QString& host, const QString& database, QString port, const QString& user, const QString& password );

/* Create protocol uri from connection parameters */
QString createProtocolURI( const QString& type, const QString& url );
