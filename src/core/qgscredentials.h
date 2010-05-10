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
/* $Id$ */


#ifndef QGSCREDENTIALS_H
#define QGSCREDENTIALS_H

#include <QString>
#include <QObject>
#include <QPair>
#include <QMap>

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

    bool get( QString realm, QString &username, QString &password, QString message = QString::null );
    void put( QString realm, QString username, QString password );

    //! retrieves instance
    static QgsCredentials *instance();

  protected:
    //! request a password
    virtual bool request( QString realm, QString &username, QString &password, QString message = QString::null ) = 0;

    //! register instance
    void setInstance( QgsCredentials *theInstance );

  private:
    //! cache for already requested credentials in this session
    QMap< QString, QPair<QString, QString> > mCredentialCache;

    //! Pointer to the credential instance
    static QgsCredentials *smInstance;
};


/**
\brief Default implementation of credentials interface

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
    virtual bool request( QString realm, QString &username, QString &password, QString message = QString::null );
};

#endif
