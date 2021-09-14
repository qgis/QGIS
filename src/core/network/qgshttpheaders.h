/***************************************************************************
                       qgshttpheaders.h
  This class implements simple http header management.

                              -------------------
          begin                : 2021-09-09
          copyright            : (C) 2021 B. De Mezzo
          email                : benoit.de.mezzo@oslandia.com

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
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsHttpHeaders
{
  public:

    /**
     * used in settings
     */
    static const QString KEY_PREFIX;

    /**
     * @brief Copy constructor
     * @param headers
     */
    QgsHttpHeaders( const QMap<QString, QVariant> &headers );
    QgsHttpHeaders();
    QgsHttpHeaders( const QgsSettings &settings, const QString &key = QString() );

    virtual ~QgsHttpHeaders();

    /**
     * @brief update the \a request by adding all the http headers
     * @param request
     * @return true if the update succeed
     */
    bool updateNetworkRequest( QNetworkRequest &request ) const;

    /**
     * @brief update the \a settings by adding all the http headers in the path "key/KEY_PREFIX/"
     * @param settings
     * @param key sub group path
     */
    void updateSettings( QgsSettings &settings, const QString &key = QString() ) const;

    /**
     * @brief loads headers from the \a settings
     * @param settings
     * @param key sub group path
     */
    void setFromSettings( const QgsSettings &settings, const QString &key = QString() );

    QVariant &operator[]( const QString &key );

    QList<QString> keys() const;

#ifndef SIP_RUN
    const QVariant operator[]( const QString &key ) const;
#endif

  private:
    QMap<QString, QVariant> mHeaders;
};

#endif // QGSHTTPHEADERS_H
