/***************************************************************************
      qgsnetworkreplyparser.h - Multipart QNetworkReply parser
                             -------------------
    begin                : 4 January, 2013
    copyright            : (C) 2013 by Radim Blazek
    email                : radim dot blazek at gmail.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNETWORKREPLYPARSER_H
#define QGSNETWORKREPLYPARSER_H

#define SIP_NO_FILE

#include <QNetworkReply>

#include "qgis_core.h"

/**
 * \ingroup core
  \brief Multipart QNetworkReply parser.

  It seams that Qt does not have currently support for multipart reply
  and it is not even possible to create QNetworkReply from raw data
  so we need a class for multipart QNetworkReply parsing.
  \note not available in Python bindings

*/

class CORE_EXPORT QgsNetworkReplyParser : public QObject
{
    Q_OBJECT

  public:
    typedef QMap<QByteArray, QByteArray> RawHeaderMap;

    /**
     * Constructor
      * \param reply */
    QgsNetworkReplyParser( QNetworkReply *reply );

    /**
     * Indicates if successfully parsed
      * \returns TRUE if successfully parsed */
    bool isValid() const { return mValid; }

    /**
     * Gets number of parts
      * \returns number of parts */
    int parts() const { return mHeaders.size(); }

    /**
     * Gets part header
      * \param part part index
      * \param headerName header name
      * \returns raw header */
    QByteArray rawHeader( int part, const QByteArray &headerName ) const { return mHeaders.value( part ).value( headerName ); }

    //! Gets headers
    QList< RawHeaderMap > headers() const { return mHeaders; }

    /**
     * Gets part part body
      * \param part part index
      * \returns part body */
    QByteArray body( int part ) const { return mBodies.value( part ); }

    //! Gets bodies
    QList<QByteArray> bodies() const { return mBodies; }

    //! Parsing error
    QString error() const { return mError; }

    /**
     * Test if reply is multipart.
      * \returns TRUE if reply is multipart */
    static bool isMultipart( QNetworkReply *reply );

  private:
    QNetworkReply *mReply = nullptr;

    bool mValid;

    QString mError;

    /* List of header maps */
    QList< RawHeaderMap > mHeaders;

    /* List of part bodies */
    QList<QByteArray> mBodies;
};

#endif
