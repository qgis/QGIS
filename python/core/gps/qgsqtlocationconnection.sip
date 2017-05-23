/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/gps/qgsqtlocationconnection.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






QTM_USE_NAMESPACE
%Docstring
*************************************************************************
                                                                         *
   This program is free software; you can redistribute it and/or modify  *
   it under the terms of the GNU General Public License as published by  *
   the Free Software Foundation; either version 2 of the License, or     *
   (at your option) any later version.                                   *
                                                                         *
**************************************************************************
%End

class QgsQtLocationConnection: QgsGPSConnection
{
%Docstring
.. note::

   may not be available in Python bindings on all platforms
%End

%TypeHeaderCode
#include "qgsqtlocationconnection.h"
%End
  public:
    QgsQtLocationConnection();
    ~QgsQtLocationConnection();

  protected slots:
    void broadcastConnectionAvailable();
%Docstring
Needed to make QtLocation detected
%End

    void parseData();
%Docstring
Parse available data source content
%End

    void positionUpdated( const QGeoPositionInfo &info );
%Docstring
 Called when the position updated.
.. note::

   not available in Python binding
%End



};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/gps/qgsqtlocationconnection.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
