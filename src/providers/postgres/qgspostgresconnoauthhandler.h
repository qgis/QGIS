/***************************************************************************
  qgspostgresconnoauthhandler.h

 ---------------------
 begin                : 27.12.2025
 copyright            : (C) 2025 by Jan Dalheimer
 email                : jan@dalheimer.de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#pragma once

#include <QCoreApplication>
#include <QString>

/**
 * Allows passing information about the authcfg to be used for a new database
 * connection to the handler called by libpq.
 *
 * This is mostly a workaround since the PQconnect* functions do not allow
 * passing any form of user data (which could otherwise have been used to pass
 * the authcfg).
 *
 * It works by storing the current authcfg in a thread_local variable, which
 * can then be retrieved by the handler called by libpq. The code flow from
 * PQconnectdb to the handler is synchronous, so by using a thread_local
 * variable this workaround should be fully safe against race conditions.
 *
 * An alternative approach would have been to use PQconnectStart in
 * QgsPostgresConn, then storing information about the authcfg for the returned
 * PGconn* somewhere where the handler (which is called after PQconnectStart
 * has returned) could retrieve it. However, that would require replicating
 * the functionality of pqConnectDBComplete which is a non-public function of
 * libpq, so unless QgsPostgresConn is rewritten to use a non-blocking approach
 * we can use this workaround.
 *
 * This is a RAII-style class, the only public interface is creation of an
 * instance of this class. During the life time of that instance the given
 * authcfg is used by the OAuth handler.
 */
class QgsPostgresConnOAuthHandler
{
    Q_DECLARE_TR_FUNCTIONS( QgsPostgresConnOAuthHandler );

  public:
    explicit QgsPostgresConnOAuthHandler( const QString &authcfg );
    ~QgsPostgresConnOAuthHandler();

    static QString currentAuthCfg() { return sAuthCfg; }
    static bool currentlyInScope() { return sInScope; }

  private:
    thread_local static QString sAuthCfg;
    thread_local static bool sInScope;
};
