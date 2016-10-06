/***************************************************************************
                              qgsftptransaction.cpp
                              ---------------------
  begin                : May 16, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsftptransaction.h"
#include <QApplication>
#include <QUrl>

QgsFtpTransaction::QgsFtpTransaction(): mFtp( new QFtp( nullptr ) ), mRequestFinished( false ), mErrorFlag( false )
{

}

QgsFtpTransaction::~QgsFtpTransaction()
{
  delete mFtp;
}

int QgsFtpTransaction::get( const QString& ftpUrl, QByteArray& ba )
{
  if ( !mFtp )
  {
    return 1;
  }

  QUrl completeUrl( ftpUrl );
  QString serverUrl = completeUrl.host();
  QString ftpPath = completeUrl.path();

  int lastSlashIndex = ftpPath.lastIndexOf( "/" );
  if ( lastSlashIndex < 0 )
  {
    return 2;
  }
  QString fileName = ftpPath.right( ftpPath.size() - lastSlashIndex - 1 );
  ftpPath = ftpPath.left( lastSlashIndex );

  if ( ftpPath.startsWith( "/" ) ) //remove starting slashes
  {
    ftpPath.remove( 0, 1 );
  }

  mRequestFinished = false;
  mErrorFlag = false;

  mFtp->connectToHost( serverUrl );
  mFtp->login( "anonymous", "" );
  mFtp->cd( ftpPath );

  QObject::connect( mFtp, SIGNAL( done( bool ) ), this, SLOT( setFinishedFlag( bool ) ) );
  mFtp->get( fileName );

  //make this function blocking
  while ( !mRequestFinished )
  {
    qApp->processEvents();
  }

  QObject::disconnect( mFtp, SIGNAL( done( bool ) ), this, SLOT( setFinishedFlag( bool ) ) );

  if ( mErrorFlag )
  {
    return 3;
  }



  ba = mFtp->readAll();
  mFtp->close();

  return 0;
}

void QgsFtpTransaction::setFinishedFlag( bool error )
{
  mErrorFlag = error;
  mRequestFinished = true;
}
