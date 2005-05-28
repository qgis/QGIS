/***************************************************************************
    qgsnewhttpconnection.cpp -  selector for a new HTTP server for WMS, etc.
                             -------------------
    begin                : 3 April 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
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
#ifndef QGSNEWHTTPCONNECTION_H
#define QGSNEWHTTPCONNECTION_H
#ifdef WIN32
#include "qgsnewhttpconnectionbase.h"
#else
#include "qgsnewhttpconnectionbase.uic.h"
#endif
/*! 
 * \brief Dialog to allow the user to configure and save connection
 * information for an HTTP Server for WMS, etc.
 */
class QgsNewHttpConnection : public QgsNewHttpConnectionBase 
{
  Q_OBJECT
 public:
    //! Constructor
    QgsNewHttpConnection(QString connName = QString::null);
    //! Destructor
    ~QgsNewHttpConnection();
    //! Tests the connection using the parameters supplied
    void testConnection();
    //! Saves the connection to ~/.qt/qgisrc
    void saveConnection();
};

#endif //  QGSNEWHTTPCONNECTION_H
