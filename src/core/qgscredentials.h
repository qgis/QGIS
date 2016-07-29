/***************************************************************************
    qgscredentials.h  -  interface for requesting credentials
    ----------------------
    begin                : Feburary 2010
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

#include <QString>
#include <QObject>
#include <QPair>
#include <QMap>
#include <QMutex>

/** \ingroup core
 * Interface for requesting credentials in QGIS in GUI independent way.
 * This class provides abstraction of a dialog for requesting credentials to the user.
 * By default QgsCredentials will be used if not overridden with other
 * credential creator function.

 * QGIS application uses QgsCredentialDialog class for displaying a dialog to the user.

 * Object deletes itself when it's not needed anymore. Children should use
 * signal destroyed() to be notified of the deletion
*/
class CORE_EXPORT QgsCredentials
{
  public:
    //! virtual destructor
    virtual ~QgsCredentials();

    bool get( const QString& realm, QString &username, QString &password, const QString& message = QString::null );
    void put( const QString& realm, const QString& username, const QString& password );

    bool getMasterPassword( QString &password, bool stored = false );

    //! retrieves instance
    static QgsCredentials *instance();

    /**
     * Lock the instance against access from multiple threads. This does not really lock access to get/put methds,
     * it will just prevent other threads to lock the instance and continue the execution. When the class is used
     * from non-GUI threads, they should call lock() before the get/put calls to avoid race conditions.
     * @note added in 2.4
     */
    void lock();

    /**
     * Unlock the instance after being locked.
     * @note added in 2.4
     */
    void unlock();

    /**
     * Return pointer to mutex
     * @note added in 2.4
     */
    QMutex *mutex() { return &mMutex; }

  protected:
    QgsCredentials();

    //! request a password
    virtual bool request( const QString& realm, QString &username, QString &password, const QString& message = QString::null ) = 0;

    //! request a master password
    virtual bool requestMasterPassword( QString &password, bool stored = false ) = 0;

    //! register instance
    void setInstance( QgsCredentials *theInstance );

  private:
    Q_DISABLE_COPY( QgsCredentials )

    //! cache for already requested credentials in this session
    QMap< QString, QPair<QString, QString> > mCredentialCache;

    //! Pointer to the credential instance
    static QgsCredentials *smInstance;

    QMutex mMutex;
};


/** \ingroup core
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
    virtual bool request( const QString& realm, QString &username, QString &password, const QString& message = QString::null ) override;
    virtual bool requestMasterPassword( QString &password, bool stored = false ) override;
};


/** \ingroup core
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
    virtual bool request( const QString& realm, QString &username, QString &password, const QString& message = QString::null ) override;
    virtual bool requestMasterPassword( QString &password, bool stored = false ) override;
};

#endif
