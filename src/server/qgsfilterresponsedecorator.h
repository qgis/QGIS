/***************************************************************************
                          qgsfilterresponsedecorator.h

  Define response adapter for handling filter's hooks
  -------------------
  begin                : 2017-01-05
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
#ifndef QGSFILTERRESPONSEDECORATOR_H
#define QGSFILTERRESPONSEDECORATOR_H

#include "qgsserverresponse.h"
#include "qgsserverfilter.h"

/**
 * \ingroup server
 * QgsFilterResponseDecorator
 * Class defining decorator for calling filter's hooks
 */
class QgsFilterResponseDecorator: public QgsServerResponse
{
  public:

    QgsFilterResponseDecorator( QgsServerFiltersMap filters, QgsServerResponse& response );
    ~QgsFilterResponseDecorator();

    /**
     * Call flter's requestReady() method
     */
    void start();

    // QgsServerResponse overrides

    void setHeader( const QString& key, const QString& value ) override {  mResponse.setHeader( key, value ); }

    void clearHeader( const QString& key ) override { mResponse.clearHeader( key ); }

    QString getHeader( const QString& key ) const override { return mResponse.getHeader( key ); }

    QList<QString> headerKeys() const override { return mResponse.headerKeys(); }

    bool headersWritten() const override { return mResponse.headersWritten(); }

    void setReturnCode( int code ) override { mResponse.setReturnCode( code ); }

    void sendError( int code,  const QString& message ) override { mResponse.sendError( code, message ); }

    QIODevice* io() override { return mResponse.io(); }

    void finish() override;

    void flush() override;

    void clear() override { mResponse.clear(); }



  private:
    QgsServerFiltersMap  mFilters;
    QgsServerResponse&   mResponse;
};

#endif





