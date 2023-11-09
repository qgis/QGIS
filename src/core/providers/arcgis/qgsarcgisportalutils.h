/***************************************************************************
    qgsarcgisportalutils.h
    --------------------
    begin                : December 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSARCGISPORTALUTILS_H
#define QGSARCGISPORTALUTILS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgshttpheaders.h"
#include "qgis.h"

#include <QVariantMap>
#include <QString>

class QgsFeedback;

/**
 * \ingroup core
 * \brief Utility functions for working with ArcGIS REST services.
 *
 * \see QgsArcGisRestUtils
 *
 * \since QGIS 3.18
 */

class CORE_EXPORT QgsArcGisPortalUtils
{
  public:

    /**
     * Retrieves JSON user info for the specified user name.
     *
     * If \a user is blank then the user associated with the current logon details will be retrieved
     *
     * \param communityUrl should be set to the Portal's community URL, e.g. https://mysite.com/portal/sharing/rest/community/
     * \param user username to query, or an empty string to query the current user
     * \param authcfg authentication configuration ID
     * \param errorTitle title summary of any encountered errors
     * \param errorText error text of any encountered errors
     * \param requestHeaders optional additional request headers
     * \param feedback optional feedback argument for cancellation support
     *
     * \returns JSON user info
     * \since QGIS 3.24
     */
    static QVariantMap retrieveUserInfo( const QString &communityUrl, const QString &user, const QString &authcfg, QString &errorTitle SIP_OUT, QString &errorText SIP_OUT, const QgsHttpHeaders &requestHeaders = QgsHttpHeaders(), QgsFeedback *feedback = nullptr );

    /**
     * Retrieves JSON user info for the specified user name. Only to avoid API break.
     *
     * If \a user is blank then the user associated with the current logon details will be retrieved
     *
     * \param communityUrl should be set to the Portal's community URL, e.g. https://mysite.com/portal/sharing/rest/community/
     * \param user username to query, or an empty string to query the current user
     * \param authcfg authentication configuration ID
     * \param errorTitle title summary of any encountered errors
     * \param errorText error text of any encountered errors
     * \param requestHeaders optional additional request headers
     * \param feedback optional feedback argument for cancellation support
     *
     * \returns JSON user info
     * \deprecated since QGIS 3.24, use the version with QgsHttpHeaders instead
     */
    Q_DECL_DEPRECATED static QVariantMap retrieveUserInfo( const QString &communityUrl, const QString &user, const QString &authcfg, QString &errorTitle SIP_OUT, QString &errorText SIP_OUT, const QMap< QString, QVariant > &requestHeaders, QgsFeedback *feedback = nullptr ) SIP_DEPRECATED;

    /**
     * Retrieves JSON definitions for all groups which the specified user name is a member of.
     *
     * If \a user is blank then the user associated with the current logon details will be retrieved
     *
     * \param communityUrl should be set to the Portal's community URL, e.g. https://mysite.com/portal/sharing/rest/community/
     * \param user username to query, or an empty string to query the current user
     * \param authcfg authentication configuration ID
     * \param errorTitle title summary of any encountered errors
     * \param errorText error text of any encountered errors
     * \param requestHeaders optional additional request headers
     * \param feedback optional feedback argument for cancellation support
     *
     * \returns a list of JSON group info
     * \since QGIS 3.24
     */
    static QVariantList retrieveUserGroups( const QString &communityUrl, const QString &user, const QString &authcfg, QString &errorTitle SIP_OUT, QString &errorText SIP_OUT, const QgsHttpHeaders &requestHeaders = QgsHttpHeaders(), QgsFeedback *feedback = nullptr );

    /**
     * Retrieves JSON definitions for all groups which the specified user name is a member of. Only to avoid API break.
     *
     * If \a user is blank then the user associated with the current logon details will be retrieved
     *
     * \param communityUrl should be set to the Portal's community URL, e.g. https://mysite.com/portal/sharing/rest/community/
     * \param user username to query, or an empty string to query the current user
     * \param authcfg authentication configuration ID
     * \param errorTitle title summary of any encountered errors
     * \param errorText error text of any encountered errors
     * \param requestHeaders optional additional request headers
     * \param feedback optional feedback argument for cancellation support
     *
     * \returns a list of JSON group info
     * \deprecated since QGIS 3.24, use the version with QgsHttpHeaders instead
     */
    Q_DECL_DEPRECATED static QVariantList retrieveUserGroups( const QString &communityUrl, const QString &user, const QString &authcfg, QString &errorTitle SIP_OUT, QString &errorText SIP_OUT, const QMap< QString, QVariant > &requestHeaders, QgsFeedback *feedback = nullptr ) SIP_DEPRECATED;

    /**
     * Retrieves JSON definitions for all items which belong the the specified \a groupId.
     *
     * \param contentUrl should be set to the Portal's content URL, e.g. https://mysite.com/portal/sharing/rest/content/
     * \param groupId ID of group to query
     * \param authcfg authentication configuration ID
     * \param errorTitle title summary of any encountered errors
     * \param errorText error text of any encountered errors
     * \param requestHeaders optional additional request headers
     * \param feedback optional feedback argument for cancellation support
     * \param pageSize number of results to retrieve for each request. Maximum value is 100.
     *
     * \returns a list of JSON item info for all items within the group
     * \since QGIS 3.24
     */
    static QVariantList retrieveGroupContent( const QString &contentUrl, const QString &groupId, const QString &authcfg, QString &errorTitle SIP_OUT, QString &errorText SIP_OUT, const QgsHttpHeaders &requestHeaders = QgsHttpHeaders(), QgsFeedback *feedback = nullptr, int pageSize = 100 );

    /**
     * Retrieves JSON definitions for all items which belong the the specified \a groupId. Only to avoid API break.
     *
     * \param contentUrl should be set to the Portal's content URL, e.g. https://mysite.com/portal/sharing/rest/content/
     * \param groupId ID of group to query
     * \param authcfg authentication configuration ID
     * \param errorTitle title summary of any encountered errors
     * \param errorText error text of any encountered errors
     * \param requestHeaders optional additional request headers
     * \param feedback optional feedback argument for cancellation support
     * \param pageSize number of results to retrieve for each request. Maximum value is 100.
     *
     * \returns a list of JSON item info for all items within the group
     * \deprecated since QGIS 3.24, use the version with QgsHttpHeaders instead
     */
    Q_DECL_DEPRECATED static QVariantList retrieveGroupContent( const QString &contentUrl, const QString &groupId, const QString &authcfg, QString &errorTitle SIP_OUT, QString &errorText SIP_OUT, const QMap< QString, QVariant > &requestHeaders, QgsFeedback *feedback = nullptr, int pageSize = 100 ) SIP_DEPRECATED;

    /**
     * Retrieves JSON definitions for all items which belong the the specified \a groupId.
     *
     * \param contentUrl should be set to the Portal's content URL, e.g. https://mysite.com/portal/sharing/rest/content/
     * \param groupId ID of group to query
     * \param authcfg authentication configuration ID
     * \param itemTypes list of desired item types (using Qgis.ArcGisRestServiceType values)
     * \param errorTitle title summary of any encountered errors
     * \param errorText error text of any encountered errors
     * \param requestHeaders optional additional request headers
     * \param feedback optional feedback argument for cancellation support
     * \param pageSize number of results to retrieve for each request. Maximum value is 100.
     *
     * \returns a list of JSON item info for all items within the group
     * \since QGIS 3.24
     */
    static QVariantList retrieveGroupItemsOfType( const QString &contentUrl, const QString &groupId, const QString &authcfg,
        const QList< int > &itemTypes,
        QString &errorTitle SIP_OUT, QString &errorText SIP_OUT, const QgsHttpHeaders &requestHeaders = QgsHttpHeaders(), QgsFeedback *feedback = nullptr, int pageSize = 100 );

    /**
     * Retrieves JSON definitions for all items which belong the the specified \a groupId. Only to avoid API break.
     *
     * \param contentUrl should be set to the Portal's content URL, e.g. https://mysite.com/portal/sharing/rest/content/
     * \param groupId ID of group to query
     * \param authcfg authentication configuration ID
     * \param itemTypes list of desired item types (using Qgis.ArcGisRestServiceType values)
     * \param errorTitle title summary of any encountered errors
     * \param errorText error text of any encountered errors
     * \param requestHeaders optional additional request headers
     * \param feedback optional feedback argument for cancellation support
     * \param pageSize number of results to retrieve for each request. Maximum value is 100.
     *
     * \returns a list of JSON item info for all items within the group
     * \deprecated since QGIS 3.24, use the version with QgsHttpHeaders instead
     */
    Q_DECL_DEPRECATED static QVariantList retrieveGroupItemsOfType( const QString &contentUrl, const QString &groupId, const QString &authcfg,
        const QList< int > &itemTypes,
        QString &errorTitle SIP_OUT, QString &errorText SIP_OUT, const QMap< QString, QVariant > &requestHeaders, QgsFeedback *feedback = nullptr, int pageSize = 100 ) SIP_DEPRECATED;

  private:

    static QString typeToString( Qgis::ArcGisRestServiceType type );

};

#endif // QGSARCGISPORTALUTILS_H
