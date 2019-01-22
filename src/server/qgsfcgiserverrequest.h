/***************************************************************************
                          qgsfcgiserverrequest.h

  Define response wrapper for fcgi request
  -------------------
  begin                : 2017-01-03
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
#ifndef QGSFCGISERVERREQUEST_H
#define QGSFCGISERVERREQUEST_H


#include "qgsserverrequest.h"


/**
 * \ingroup server
 * \class QgsFcgiServerRequest
 * \brief Class defining fcgi request
 * \since QGIS 3.0
 */
class SERVER_EXPORT QgsFcgiServerRequest: public QgsServerRequest
{
  public:
    QgsFcgiServerRequest();

    QByteArray data() const override;

    /**
     * Returns true if an error occurred during initialization
     */
    bool hasError() const { return mHasError; }

  private:
    void readData();

    // Log request info: print debug infos
    // about the request
    void printRequestInfos();


    QByteArray mData;
    bool       mHasError = false;
};

#endif
