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

#include "gdal.h"
#include "qgsmessagelog.h"

/**
 * \ingroup core
 * \class QgsCPLErrorHandler
 */
class QgsCPLErrorHandler
{
    static void CPL_STDCALL showError( CPLErr errClass, int errNo, const char *msg )
    {
      if ( errNo != OGRERR_NONE )
        QgsMessageLog::logMessage( QObject::tr( "OGR[%1] error %2: %3" ).arg( errClass ).arg( errNo ).arg( msg ), QObject::tr( "OGR" ) );
    }

  public:
    QgsCPLErrorHandler()
    {
      CPLPushErrorHandler( showError );
    }

    ~QgsCPLErrorHandler()
    {
      CPLPopErrorHandler();
    }

    QgsCPLErrorHandler( const QgsCPLErrorHandler &other ) = delete;
    QgsCPLErrorHandler &operator=( const QgsCPLErrorHandler &other ) = delete;

};


#endif // QGSCPLERRORHANDLER_H
