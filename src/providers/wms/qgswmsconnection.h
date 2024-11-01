/***************************************************************************
    qgswmsconnection.h  -  WMS connection
                             -------------------
    begin                : 3 April 2005
    original             : (C) 2005 by Brendan Morley email  : morb at ozemail dot com dot au
    wms search           : (C) 2009 Mathias Walker <mwa at sourcepole.ch>, Sourcepole AG
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWMSCONNECTION_H
#define QGSWMSCONNECTION_H

#include "qgsowsconnection.h"
#include <QStringList>

/*!
 * \brief   Connections management
 */
class QgsWMSConnection : public QgsOwsConnection
{
    Q_OBJECT

  public:
    //! Constructor
    explicit QgsWMSConnection( const QString &connName );

    static QStringList connectionList();

    static void deleteConnection( const QString &name );

    static QString selectedConnection();
    static void setSelectedConnection( const QString &name );
};

#endif // QGSWMSCONNECTION_H
