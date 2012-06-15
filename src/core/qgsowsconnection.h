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
//#include "qgisgui.h"
//#include "qgscontexthelp.h"

#include <QStringList>
#include <QPushButton>

class QgisApp;
//class QgsDataProvider;
class QgsDataProvider;
/*class QButtonGroup;*/
/*class QgsNumericSortTreeWidgetItem;*/
class QDomDocument;
class QDomElement;

/*!
 * \brief   Connections management
 */
class QgsOWSConnection : public QObject
{
//    Q_OBJECT

  public:
    /**
     * Constructor
     * @param theService service name: WMS,WFS,WCS
     */
    QgsOWSConnection( const QString & theService, const QString & theConnName );
    //! Destructor
    ~QgsOWSConnection();

    static QStringList connectionList( const QString & theService );

    static void deleteConnection( const QString & theService, const QString & name );

    static QString selectedConnection( const QString & theService );
    static void setSelectedConnection( const QString & theService, const QString & name );


  public:
    //QgsDataProvider *provider();
    QString connectionInfo();
    QString mConnName;
    QString mConnectionInfo;
    QgsDataSourceURI uri();
  private:
    QgsDataSourceURI mUri;
    QString mService;
};


#endif // QGSOWSCONNECTION_H
