/***************************************************************************
                          qgsnewconnection.h  -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
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
#ifndef QGSNEWCONNECTION_H
#define QGSNEWCONNECTION_H
#include "qgsnewconnectionbase.h"
/*! \class QgsNewConnection
 * \brief Dialog to allow the user to configure and save connection
 * information for a PostgresQl database
 */
class QgsNewConnection : public QgsNewConnectionBase 
{
  Q_OBJECT
 public:
    //! Constructor
    QgsNewConnection(QString connName= QString::null);
    //! Destructor
    ~QgsNewConnection();
    //! Tests the connection using the parameters supplied
    void testConnection();
    //! Saves the connection to ~/.qt/qgisrc
    void saveConnection();
};

#endif //  QGSNEWCONNECTIONBASE_H
