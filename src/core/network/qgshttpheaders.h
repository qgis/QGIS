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

#include <QMap>
#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgssettingsentry.h"

class QNetworkRequest;
class QUrlQuery;
class QDomElement;

/**
 * \ingroup core
 * \brief This class implements simple http header management.
 * \since QGIS 3.24
 */
class CORE_EXPORT QgsHttpHeaders
{
  public:

#ifndef SIP_RUN

    //! Used in settings as the group name
    static const QString PATH_PREFIX;

    //! Used in settings as the referer key
    static const QString KEY_REFERER;

    //! Used in uri to pass headers as params
    static const QString PARAM_PREFIX;

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
     * \brief Constructor from QgsSettings \a settings object and root \a key
     * \param settings
     * \param key
     */
    QgsHttpHeaders( const QgsSettings &settings, const QString &key = QString() );

    /**
     * \brief Constructor from default QgsSettings object and root \a key
     * \param key
     */
    QgsHttpHeaders( const QString &key );

    /**
     * \brief Constructor from a QDomElement \a element
     * \param element
     */
    QgsHttpHeaders( const QDomElement &element );

    //! default detructor
    virtual ~QgsHttpHeaders();

    /**
     * \brief Updates the \a settings by adding all the http headers in the path "key/PATH_PREFIX/"
     *
     * KEY_REFERER value will be available at path "key/PATH_PREFIX/KEY_REFERER" and path "key/KEY_REFERER" (for backward compatibility)
     *
     * \param settings
     * \param key sub group path
     * \return TRUE if the update succeed
     */
    bool updateSettings( QgsSettings &settings, const QString &key = QString() ) const;

    /**
     * \brief Updates a \a request by adding all the HTTP headers
     * \return TRUE if the update succeed
     */
    bool updateNetworkRequest( QNetworkRequest &request ) const;

    /**
     * \brief Updates an \a uri by adding all the HTTP headers
     * \return TRUE if the update succeed
     */
    bool updateUrlQuery( QUrlQuery &uri ) const;

    /**
     * \brief Updates a \a map by adding all the HTTP headers
     *
     * KEY_REFERER value will be available at key "KEY_PREFIX+KEY_REFERER" and key "KEY_REFERER" (for backward compatibility)
     *
     * \return TRUE if the update succeed
     */
    bool updateMap( QVariantMap &map ) const;

    /**
     * \brief Updates a \a map by adding all the HTTP headers
     *
     * KEY_REFERER value will be available at attribute "KEY_PREFIX+KEY_REFERER" and attribute "KEY_REFERER" (for backward compatibility)
     *
     * \return TRUE if the update succeed
     */
    bool updateDomElement( QDomElement &el ) const;

    /**
     * \brief Loads headers from the \a settings
     *
     * key KEY_REFERER will be read at path "key/PATH_PREFIX/KEY_REFERER" and path "key/KEY_REFERER" (for backward compatibility)
     *
     * \param settings
     * \param key sub group path
     */
    void setFromSettings( const QgsSettings &settings, const QString &key = QString() );

    /**
     * \brief Loads headers from the \a uri
     * \param uri
     */
    void setFromUrlQuery( const QUrlQuery &uri );

    /**
     * \brief Loads headers from the \a map
     *
     * key KEY_REFERER will be read from key "KEY_PREFIX+KEY_REFERER" and key "KEY_REFERER" (for backward compatibility)
     *
     * \param map
     */
    void setFromMap( const QVariantMap &map );

    /**
     * \brief Loads headers from the \a element
     *
     * key KEY_REFERER will be read from attribute "KEY_PREFIX+KEY_REFERER" and attribute "KEY_REFERER" (for backward compatibility)
     *
     * \param element
     */
    void setFromDomElement( const QDomElement &element );

    /**
     * \brief Returns a cleansed \a key
     * \param key a key to be sanitized
     */
    QString sanitizeKey( const QString &key ) const;

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

    //! Returns key/value pairs as strings separated by space
    QString toSpacedString() const;

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
