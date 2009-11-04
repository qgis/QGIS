/***************************************************************************
  qgshttptransaction.h  -  Tracks a HTTP request with its response,
                           with particular attention to tracking
                           HTTP redirect responses
                             -------------------
    begin                : 17 Mar, 2005
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

/* $Id: qgshttptransaction.h 5697 2006-08-15 10:29:46Z morb_au $ */

#ifndef QGSHTTPTRANSACTION_H
#define QGSHTTPTRANSACTION_H

#include <QHttp>
#include <QNetworkProxy>
#include <QString>

class QTimer;

/** \ingroup core
 * HTTP request/response manager that is redirect-aware.
 * This class extends the Qt QHttp concept by being able to recognise
 *  and respond to redirection responses (e.g. HTTP code 302)
*/

class CORE_EXPORT QgsHttpTransaction : public QObject
{

    Q_OBJECT

  public:
    /**
    * Constructor.
    * \note userName and password added in 1.1
    */
    QgsHttpTransaction( QString uri,
                        QString proxyHost = QString(),
                        int     proxyPort = 80,
                        QString proxyUser = QString(),
                        QString proxyPass = QString(),
                        QNetworkProxy::ProxyType proxyType = QNetworkProxy::NoProxy,
                        QString userName = QString(),
                        QString password = QString() );

    //! Destructor
    virtual ~QgsHttpTransaction();

    void getAsynchronously();

    //! Gets the response synchronously.  Note that signals will still be emitted
    //! while in this function.

    /*!
        The function returns FALSE if there is an error while getting the response.
        @param[out] respondedContent is replaced with the new content.

        @param[in]  redirections     is used to measure how many http redirections we've been through.
        Clients typically don't need to set this.

        @param postData data to send with the http message. This is only used for HTTP POST. If
        0 then the request is done with HTTP GET.

        @return true in case of success
     */
    bool getSynchronously( QByteArray &respondedContent, int redirections = 0, const QByteArray* postData = 0 );

    QString responseContentType();

    /**
     * If an operation returns 0 (e.g. getSynchronously()), this function
     * returns the text of the error associated with the failure.
     * Interactive users of this provider can then, for example,
     * call a QMessageBox to display the contents.
     */
    QString errorString();

    /**Apply proxy settings from QSettings to a http object
    @param return true if proxy settings was applied, false else*/
    static bool applyProxySettings( QHttp& http, const QString& url );

    /**
     * Set the credentials (username and password)
     * \note added in 1.1
     */

    void setCredentials( const QString& username, const QString &password );

    /**Returns the network timeout in msec*/
    int networkTimeout() const { return mNetworkTimeoutMsec;}
    /**Sets the network timeout in milliseconds*/
    void setNetworkTimeout( int msec ) { mNetworkTimeoutMsec = msec;}


  public slots:

    void dataStarted( int id );

    void dataHeaderReceived( const QHttpResponseHeader& resp );

    void dataReceived( const QHttpResponseHeader& resp );

    void dataProgress( int done, int total );

    void dataFinished( int id, bool error );

    void transactionFinished( bool error );

    void dataStateChanged( int state );

    void networkTimedOut();

    /**Aborts the current transaction*/
    void abort();

  signals:

    /**legacy code. This signal is currently not emitted and only kept for API compatibility*/
    void setProgress( int done, int total );

    /**Signal for progress update */
    void dataReadProgress( int theProgress );
    /**Signal for adjusted number of steps*/
    void totalSteps( int theTotalSteps );

    /** \brief emit a signal to be caught by qgisapp and display a msg on status bar */
    void statusChanged( QString theStatusQString );


  private:

    /**Default constructor is forbidden*/
    QgsHttpTransaction();

    /**
     * Indicates the associated QHttp object
     *
     * \note  We tried to use this as a plain QHttp object
     *        but strange things were happening with the signals -
     *        therefore we use the "pointer to" instead.
     */
    QHttp* http;

    /**
     * Indicates the QHttp ID
     */
    int httpid;

    /**
     * Indicates if the transaction is in progress
     */
    bool httpactive;

    /*
     * Indicates the response from the QHttp
     */
    QByteArray httpresponse;

    /*
     * Indicates the content type of the response from the QHttp
     */
    QString    httpresponsecontenttype;

    /**
     * The original URL requested for this transaction
     */
    QString httpurl;

    /**
     * The host being used for this transaction
     */
    QString httphost;

    /**
     * If not empty, indicates that the QHttp is a redirect
     * to the contents of this variable
     */
    QString httpredirecturl;

    /**
     * Number of http redirections this transaction has been
     * subjected to.
     *
     * TODO: Use this as part of a redirection loop detector
     *
     */
    int httpredirections;

    /**
     * Indicates the associated QTimer object - used to detect network timeouts
     */
    QTimer * mWatchdogTimer;

    /**
     * The error message associated with the last HTTP error.
     */
    QString mError;

    /**
     * User name
     */
    QString mUserName;

    /**
     * Password
     */
    QString mPassword;

    /**Network timeout in milliseconds*/
    int mNetworkTimeoutMsec;
};

#endif

// ENDS
