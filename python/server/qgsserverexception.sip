/***************************************************************************
                              qgsserverexception.sip
                              ------------------------
  begin                : January 11 2017
  copyright            : (C) 2017 by David Marteau
  email                : david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


/** \ingroup server
 * \class  QgsServerException
 * \brief server Exception base class.
 */

class QgsServerException
{
%TypeHeaderCode
#include <qgsserverexception.h>
%End
  public:
    QgsServerException( const QString& message, int responseCode = 500 );

    int responseCode() const;

    virtual QByteArray formatResponse( QString& responseFormat ) const;
};


class QgsOgcServiceException
{
%TypeHeaderCode
#include <qgsserverexception.h>
%End
  public:
    QgsOgcServiceException( const QString& code, const QString& message, const QString& locator = QString(),
                            int responseCode = 200, const QString& version = "1.3.0" );

    QString message() const;
    QString code()    const;
    QString locator() const;
    QString version() const;

    virtual QByteArray formatResponse( QString& responseFormat / Out / ) const;
};




