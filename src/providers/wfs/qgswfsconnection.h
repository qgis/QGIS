/***************************************************************************
    qgswfsconnection.h
    ---------------------
    begin                : February 2016
    copyright            : (C) 2016 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWFSCONNECTION_H
#define QGSWFSCONNECTION_H

#include "qgsowsconnection.h"

class QgsWfsConnection : public QgsOwsConnection
{
    Q_OBJECT

  public:
    /**
     * Constructor
     * \param connName connection name
     */
    explicit QgsWfsConnection( const QString &connName );

    static QStringList connectionList();

    static void deleteConnection( const QString &name );

    static QString selectedConnection();
    static void setSelectedConnection( const QString &name );
};

#endif
