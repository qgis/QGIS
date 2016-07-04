/***************************************************************************
    qgsowsconnection.h  -  OWS connection
                             -------------------
    begin                : 3 April 2005
    original             : (C) 2005 by Brendan Morley email  : morb at ozemail dot com dot au
    wms search           : (C) 2009 Mathias Walker <mwa at sourcepole.ch>, Sourcepole AG

    generalized          : (C) 2012 Radim Blazek, based on qgswmsconnection.h


 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOWSCONNECTION_H
#define QGSOWSCONNECTION_H

#include "qgsdatasourceuri.h"

#include <QStringList>
#include <QPushButton>

/** \ingroup core
 * \brief Connections management
 */
class CORE_EXPORT QgsOWSConnection : public QObject
{
    Q_OBJECT

  public:
    /**
     * Constructor
     * @param theService service name: WMS,WFS,WCS
     * @param theConnName connection name
     */
    QgsOWSConnection( const QString & theService, const QString & theConnName );

    //! Destructor
    ~QgsOWSConnection();

    /** Returns the list of connections for the specified service */
    static QStringList connectionList( const QString & theService );

    /** Deletes the connection for the specified service with the specified name */
    static void deleteConnection( const QString & theService, const QString & name );

    /** Retreives the selected connection for the specified service */
    static QString selectedConnection( const QString & theService );
    /** Marks the specified connection for the specified service as selected */
    static void setSelectedConnection( const QString & theService, const QString & name );

    QString mConnName;
    /** Returns the connection uri */
    QgsDataSourceURI uri() const;
    QString mConnectionInfo;

    //! @deprecated use mConnectionInfo instead
    Q_DECL_DEPRECATED QString connectionInfo();

  protected:
    QgsDataSourceURI mUri;
    QString mService;
};


#endif // QGSOWSCONNECTION_H
