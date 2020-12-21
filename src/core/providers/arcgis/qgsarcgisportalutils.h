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

#include <QVariantMap>
#include <QString>

class QgsFeedback;

/**
 * \ingroup core
 * Utility functions for working with ArcGIS REST services.
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
     * \param authcfg authentification configuration ID
     * \param errorTitle title summary of any encountered errrors
     * \param errorText error text of any encountered errors
     * \param requestHeaders optional additional request headers
     * \param feedback optional feedback argument for cancelation support
     *
     * \returns JSON user info
     */
    static QVariantMap retrieveUserInfo( const QString &communityUrl, const QString &user, const QString &authcfg, QString &errorTitle SIP_OUT, QString &errorText SIP_OUT, const QMap< QString, QString > &requestHeaders = QMap< QString, QString >(), QgsFeedback *feedback = nullptr );

};

#endif // QGSARCGISPORTALUTILS_H
