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
 
/* $Id$ */

#ifndef QGSHTTPTRANSACTION_H
#define QGSHTTPTRANSACTION_H

#include <qhttp.h>
#include <qstring.h>

/**

  \brief  HTTP request/response manager that is redirect-aware.
 
  This class extends the Qt QHttp concept by being able to recognise
  and respond to redirection responses (e.g. HTTP code 302)
  
  
  TODO: Make it work
  
*/

class QgsHttpTransaction : public QObject
{
  
  Q_OBJECT

public:
  /**
  * Constructor.
  */
  QgsHttpTransaction( QString uri );

  //! Destructor
  virtual ~QgsHttpTransaction();
  
  void getAsynchronously();
  
  QByteArray getSynchronously(int redirections = 0);

  QString responseContentType();
  
  
public slots:

  void dataStarted( int id );
  
  void dataHeaderReceived( const QHttpResponseHeader& resp );
  
  void dataReceived( const QHttpResponseHeader& resp );

  void dataProgress( int done, int total );

  void dataFinished( int id, bool error );

  void dataStateChanged( int state );

  
signals:

    /** \brief emit a signal to notify of a progress event */
    void setProgress(int theProgress, int theTotalSteps);

    /** \brief emit a signal to be caught by qgisapp and display a msg on status bar */
    void setStatus(QString theStatusQString);


private:
  
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
  
  
};

#endif
