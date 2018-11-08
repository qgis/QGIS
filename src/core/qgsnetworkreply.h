/***************************************************************************
    qgsnetworkreply.h
    -----------------
    begin                : November 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSNETWORKREPLY_H
#define QGSNETWORKREPLY_H

#include "qgis_core.h"

#include <QNetworkReply>

/**
 * Encapsulates a network reply within a container which is inexpensive to copy and safe to pass between threads.
 * \ingroup core
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsNetworkReplyContent
{
  public:

    /**
     * Default constructor for an empty reply.
     */
    QgsNetworkReplyContent() = default;

    /**
     * Constructor for QgsNetworkReplyContent, populated from the specified \a reply.
     */
    explicit QgsNetworkReplyContent( QNetworkReply *reply );

    /**
     * Clears the reply, resetting it back to a default, empty reply.
     */
    void clear();

    /**
     * Returns the raw reply content.
     */
    QByteArray content() const
    {
      return mContent;
    }

    /**
     * Returns the reply's error message, or QNetworkReply::NoError if no
     * error was encountered.
     *
     * \see errorString()
     */
    QNetworkReply::NetworkError error() const
    {
      return mError;
    }

    /**
     * Returns the error text for the reply, or an empty string if no
     * error was encountered.
     *
     * \see error()
     */
    QString errorString() const
    {
      return mErrorString;
    }

#ifndef SIP_RUN
    typedef QPair<QByteArray, QByteArray> RawHeaderPair;

    /**
     * Returns the list of raw header pairs in the reply.
     * \see hasRawHeader()
     * \see rawHeaderList()
     * \see rawHeader()
     * \note Not available in Python bindings
     */
    const QList<RawHeaderPair> &rawHeaderPairs() const
    {
      return mRawHeaderPairs;
    }
#endif

    /**
     * Returns true if the reply contains a header with the specified \a headerName.
     * \see rawHeaderPairs()
     * \see rawHeaderList()
     * \see rawHeader()
     */
    bool hasRawHeader( const QByteArray &headerName ) const;

    /**
     * Returns a list of raw header names contained within the reply.
     * \see rawHeaderPairs()
     * \see hasRawHeader()
     * \see rawHeader()
     */
    QList<QByteArray> rawHeaderList() const;

    /**
     * Returns the content of the header with the specified \a headerName, or an
     * empty QByteArray if the specified header was not found in the reply.
     * \see rawHeaderPairs()
     * \see hasRawHeader()
     * \see rawHeaderList()
     */
    QByteArray rawHeader( const QByteArray &headerName ) const;

  private:

    QByteArray mContent;
    QNetworkReply::NetworkError mError = QNetworkReply::NoError;
    QString mErrorString;
    QList<RawHeaderPair> mRawHeaderPairs;
};

#endif // QGSNETWORKREPLY_H
