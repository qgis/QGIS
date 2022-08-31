/***************************************************************************
  qgscplerrorhandler.h - QgsCplErrorHandler

 ---------------------
 begin                : Oct 29, 2003
 copyright            : (C) 2003 by Gary E.Sherman
 email                : sherman at mrcc.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCPLERRORHANDLER_H
#define QGSCPLERRORHANDLER_H

#include "cpl_error.h"
#include "qgsmessagelog.h"

/**
 * \ingroup core
 * \class QgsCPLErrorHandler
 */
class QgsCPLErrorHandler
{
    static void CPL_STDCALL showError( CPLErr errClass, int errNo, const char *msg )
    {
      if ( errClass != CE_None )
      {
        const QString *sourceName = static_cast< QString * >( CPLGetErrorHandlerUserData() );
        const QString identifier = sourceName ? *sourceName : QObject::tr( "OGR" );
        QgsMessageLog::logMessage( QObject::tr( "%1[%2] error %3: %4" ).arg( identifier ).arg( errClass ).arg( errNo ).arg( msg ), identifier );
      }
    }

  public:
    QgsCPLErrorHandler( const QString &sourceName = QObject::tr( "OGR" ) )
      : mSourceName( sourceName )
    {
      CPLPushErrorHandlerEx( showError, &mSourceName );
    }

    ~QgsCPLErrorHandler()
    {
      CPLPopErrorHandler();
    }

    QgsCPLErrorHandler( const QgsCPLErrorHandler &other ) = delete;
    QgsCPLErrorHandler &operator=( const QgsCPLErrorHandler &other ) = delete;

  private:

    QString mSourceName;

};

/**
 * \ingroup core
 * \class QgsCPLErrorCollectorHandler
 *
 * A GDAL error handler which collects errors for later processing.
 *
 * \since QGIS 3.28
 */
class QgsCPLErrorCollectorHandler
{
    static void CPL_STDCALL showError( CPLErr errClass, int, const char *msg )
    {
      if ( errClass != CE_None )
      {
        QStringList *errors = static_cast< QStringList * >( CPLGetErrorHandlerUserData() );
        if ( errors )
        {
          errors->append( QString( msg ) );
        }
      }
    }

  public:
    QgsCPLErrorCollectorHandler()
    {
      CPLPushErrorHandlerEx( showError, &mErrors );
    }

    ~QgsCPLErrorCollectorHandler()
    {
      CPLPopErrorHandler();
    }

    /**
     * Takes all collected errors.
     */
    QStringList popErrors()
    {
      const QStringList errors = mErrors;
      mErrors.clear();
      return errors;
    }

    QgsCPLErrorCollectorHandler( const QgsCPLErrorCollectorHandler &other ) = delete;
    QgsCPLErrorCollectorHandler &operator=( const QgsCPLErrorCollectorHandler &other ) = delete;

  private:

    QStringList mErrors;

};



#endif // QGSCPLERRORHANDLER_H
