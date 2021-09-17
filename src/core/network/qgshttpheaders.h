/***************************************************************************
                       qgshttpheaders.h
  This class implements simple http header management.

                              -------------------
          begin                : 2021-09-09
          copyright            : (C) 2021 B. De Mezzo
          email                : benoit dot de dot mezzo at oslandia dot com

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSHTTPHEADERS_H
#define QGSHTTPHEADERS_H

#include <QNetworkRequest>
#include <QMap>
#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgssettingsentry.h"

/**
 * \ingroup core
 * \brief This class implements simple http header management.
 * \since QGIS 3.24
 */
class CORE_EXPORT QgsHttpHeaders
{
  public:

#ifndef SIP_RUN

    /**
     * Used in settings
     */
    static const QString KEY_PREFIX;
#endif

    /**
     * \brief Constructor from map
     * \param headers
     */
    QgsHttpHeaders( const QMap<QString, QVariant> &headers );

    /**
     * \brief default constructor
     */
    QgsHttpHeaders();

    /**
     * \brief Constructor from QgsSettings \a settings object
     * \param settings
     * \param key
     */
    QgsHttpHeaders( const QgsSettings &settings, const QString &key = QString() );

    virtual ~QgsHttpHeaders();

    /**
     * \brief Updates a \a request by adding all the HTTP headers
     * \return TRUE if the update succeed
     */
    bool updateNetworkRequest( QNetworkRequest &request ) const;

    /**
     * \brief Updates the \a settings by adding all the http headers in the path "key/KEY_PREFIX/"
     * \param settings
     * \param key sub group path
     */
    void updateSettings( QgsSettings &settings, const QString &key = QString() ) const;

    /**
     * \brief Loads headers from the \a settings
     * \param settings
     * \param key sub group path
     */
    void setFromSettings( const QgsSettings &settings, const QString &key = QString() );

    /**
     * \param key http header key name
     * \return http header value
     */
    QVariant &operator[]( const QString &key );

    QgsHttpHeaders &operator = ( const QMap<QString, QVariant> &headers ) SIP_SKIP;

    /**
     * \return the list of all http header keys
     */
    QList<QString> keys() const;

#ifndef SIP_RUN

    /**
     * \param key http header key name
     * \return http header value
     */
    const QVariant operator[]( const QString &key ) const;
#endif

  private:
    QMap<QString, QVariant> mHeaders;
};

#endif // QGSHTTPHEADERS_H
