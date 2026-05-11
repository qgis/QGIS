/***************************************************************************
                         qgsscopedconnection.h
                         ----------------------
    begin                : May 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSCOPEDCONNECTION_H
#define QGSSCOPEDCONNECTION_H

#include <QMetaObject>

#define SIP_NO_FILE

/**
 * Keeps a reference to a Qt connection (a QMetaObject::Connection) and disconnects it whenever this object is deleted.
 *
 * \ingroup core
 * \since QGIS 4.2
 */
class QgsScopedConnection
{
  public:
    /**
     * Default constructor (holds an invalid connection).
     */
    QgsScopedConnection() = default;

    /**
     * Constructor for QgsScopedConnection, managing the specified \a connection.
     */
    explicit QgsScopedConnection( QMetaObject::Connection connection )
      : mConnection( std::move( connection ) )
    {}

    /**
     * Destructor for QgsScopedConnection.
     *
     * The managed connection will be automatically disconnected.
     */
    ~QgsScopedConnection() { disconnect(); }

    // No copies permitted
    QgsScopedConnection( const QgsScopedConnection & ) = delete;
    QgsScopedConnection &operator=( const QgsScopedConnection & ) = delete;

    // Moves permitted
    QgsScopedConnection( QgsScopedConnection &&other ) noexcept
      : mConnection( std::move( other.mConnection ) )
    {
      other.mConnection = QMetaObject::Connection();
    }

    QgsScopedConnection &operator=( QgsScopedConnection &&other ) noexcept
    {
      if ( this != &other )
      {
        disconnect();
        mConnection = std::move( other.mConnection );
        other.mConnection = QMetaObject::Connection();
      }
      return *this;
    }

    QgsScopedConnection &operator=( QMetaObject::Connection connection )
    {
      disconnect();
      mConnection = std::move( connection );
      return *this;
    }

    /**
   * Manually disconnects the managed connection.
   */
    void disconnect()
    {
      if ( mConnection )
      {
        QObject::disconnect( mConnection );
        mConnection = QMetaObject::Connection();
      }
    }

  private:
    QMetaObject::Connection mConnection;
};

#endif // QGSSCOPEDCONNECTION_H
