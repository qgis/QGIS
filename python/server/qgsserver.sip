/***************************************************************************
                          qgsserver.sip
 Qgis Mapserver
                          -------------------
  begin                : 2015-05-21
  copyright            : (C) 2015 by Alessandro Pasotti
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



class QgsServer
{
%TypeHeaderCode
#include "qgsserver.h"
%End

  public:
    QgsServer();
    ~QgsServer();
    // Original init for the fcgi application:
    //void init( int argc, char* argv[] );
    // init for python bindings:
    void init( );
    QByteArray handleRequest( const QString queryString = QString( ) );
    // TODO: if HAVE_SERVER_PYTHON
    QByteArray handleRequest( const QString queryString,
                              const bool returnHeaders,
                              const bool returnBody );
    QByteArray handleRequestGetBody( const QString queryString = QString( ) );
    QByteArray handleRequestGetHeaders( const QString queryString = QString( ) );
    //QgsServerContext& serverContext ( ) /KeepReference/;
%If (HAVE_SERVER_PYTHON_PLUGINS)
     QgsServerInterface* serverInterface( );
     // Needs %MethodCode
     //QMultiMap<int, QgsServerFilter*> pluginFilters( );
%End

    // The following is needed because otherwise SIP fails trying to create copy
  // ctor
  private:
    QgsServer( const QgsServer& );
    QgsServer & operator=( const QgsServer& );
};
