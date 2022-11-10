/***************************************************************************
  qgsgpslogger.h
   -------------------
  begin                : November 2022
  copyright            : (C) 2022 by Nyall Dawson
  email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGPSLOGGER_H
#define QGSGPSLOGGER_H

#include "qgis_core.h"
#include "qgis.h"
#include <QObject>
#include <QPointer>

class QgsGpsConnection;


/**
 * \ingroup core
 * \class QgsGpsLogger
 * \brief Handles logging of incoming GPS data to a vector layer.
 *
 * QgsBabelFormatRegistry is not usually directly created, but rather accessed through
 * QgsApplication::gpsBabelFormatRegistry().
 *
 * \since QGIS 3.30
*/
class CORE_EXPORT QgsGpsLogger : QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsGpsLogger with the specified \a parent object.
     *
     * The logger will automatically record GPS information from the specified \a connection.
     */
    QgsGpsLogger( QgsGpsConnection *connection, QObject *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsGpsLogger() override;

    /**
     * Returns the associated GPS connection.
     */
    QgsGpsConnection *connection();

  private:

    QPointer< QgsGpsConnection > mConnection;
};


#endif // QGSGPSLOGGER_H
