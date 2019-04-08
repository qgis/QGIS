/***************************************************************************
    qgscredentials.h  -  interface for requesting credentials
    ----------------------
    begin                : February 2010
    copyright            : (C) 2010 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSCREDENTIALS_H
#define QGSCREDENTIALS_H

#include <QMap>
#include <QMutex>
#include <QObject>
#include <QPair>
#include <QString>

#include "qgis_core.h"
#include "qgis_sip.h"

/**
 * \ingroup core
 * Interface for requesting credentials in QGIS in GUI independent way.
 * This class provides abstraction of a dialog for requesting credentials to the user.
 * By default QgsCredentials will be used if not overridden with other
 * credential creator function.

 * QGIS application uses QgsCredentialDialog class for displaying a dialog to the user.

 * Caller can use the mutex to synchronize authentications to avoid requesting
 * credentials for the same resource several times.

 * Object deletes itself when it's not needed anymore. Children should use
 * signal destroyed() to be notified of the deletion
*/
class CORE_EXPORT QgsCredentials
{
  public:

    /**
     * Destructor.
     */
    virtual ~QgsCredentials() = default;

    /**
     * Requests credentials for the specified \a realm.
     *
     * If existing credentials exist for the given \a realm, these will be returned. Otherwise the credential
     * handler will prompt for the correct username and password.
     *
     * The retrieved or user-entered details will be stored in \a username and \a password.
     *
     * Optionally, a specific \a message can be used to advise users of the context for the credentials request.
     *
     * \note This method will not automatically store the newly obtained credentials. Callers must
     * manually call put() after verifying that the obtained credentials are correct.
     *
     * \see put()
     */
    bool get( const QString &realm, QString &username SIP_INOUT, QString &password SIP_INOUT, const QString &message = QString() );

    /**
     * Stores the correct \a username and \a password for the specified \a realm.
     *
     * These values will be used for all future calls to get() for the same \a realm, without requesting
     * users to re-enter them. It is the caller's responsibility to ensure that only valid \a username and \a password
     * combinations are used with this method.
     *
     * \see get()
     */
    void put( const QString &realm, const QString &username, const QString &password );

    bool getMasterPassword( QString &password SIP_INOUT, bool stored = false );

    //! retrieves instance
    static QgsCredentials *instance();

    /**
     * Lock the instance against access from multiple threads. This does not really lock access to get/put methds,
     * it will just prevent other threads to lock the instance and continue the execution. When the class is used
     * from non-GUI threads, they should call lock() before the get/put calls to avoid race conditions.
     * \since QGIS 2.4
     */
    void lock();

    /**
     * Unlock the instance after being locked.
     * \since QGIS 2.4
     */
    void unlock();

    /**
     * Returns pointer to mutex
     * \since QGIS 2.4
     */
    QMutex *mutex() { return &mAuthMutex; }

  protected:

    /**
     * Constructor for QgsCredentials.
     */
    QgsCredentials() = default;

    //! request a password
    virtual bool request( const QString &realm, QString &username SIP_INOUT, QString &password SIP_INOUT, const QString &message = QString() ) = 0;

    //! request a master password
    virtual bool requestMasterPassword( QString &password SIP_INOUT, bool stored = false ) = 0;

    //! register instance
    void setInstance( QgsCredentials *instance );

  private:
    Q_DISABLE_COPY( QgsCredentials )

#ifdef SIP_RUN
    QgsCredentials( const QgsCredentials & );
#endif

    //! cache for already requested credentials in this session
    QMap< QString, QPair<QString, QString> > mCredentialCache;

    //! Pointer to the credential instance
    static QgsCredentials *sInstance;

    //! Mutex to synchronize authentications
    QMutex mAuthMutex;

    //! Mutex to guard the cache
    QMutex mCacheMutex;
};


/**
 * \ingroup core
\brief Default implementation of credentials interface

This class doesn't prompt or return credentials
*/
class CORE_EXPORT QgsCredentialsNone : public QObject, public QgsCredentials
{
    Q_OBJECT

  public:
    QgsCredentialsNone();

  signals:
    //! signals that object will be destroyed and shouldn't be used anymore
    void destroyed();

  protected:
    bool request( const QString &realm, QString &username SIP_INOUT, QString &password SIP_INOUT, const QString &message = QString() ) override;
    bool requestMasterPassword( QString &password SIP_INOUT, bool stored = false ) override;
};


/**
 * \ingroup core
\brief Implementation of credentials interface for the console

This class outputs message to the standard output and retrieves input from
standard input. Therefore it won't be the right choice for apps without
GUI.
*/
class CORE_EXPORT QgsCredentialsConsole : public QObject, public QgsCredentials
{
    Q_OBJECT

  public:
    QgsCredentialsConsole();

  signals:
    //! signals that object will be destroyed and shouldn't be used anymore
    void destroyed();

  protected:
    bool request( const QString &realm, QString &username SIP_INOUT, QString &password SIP_INOUT, const QString &message = QString() ) override;
    bool requestMasterPassword( QString &password SIP_INOUT, bool stored = false ) override;
};

#endif
