/***************************************************************************
                          qgshtmlrendersettings.h
                             -------------------
    begin                : November 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSHTMLRENDERSETTINGS_H
#define QGSHTMLRENDERSETTINGS_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include <QUrl>

/**
 * \ingroup core
 * \brief Encapsulates settings for rendering HTML content.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsHtmlRenderSettings
{
  public:
    /**
   * Returns TRUE no settings are set.
   */
    bool isEmpty() const;

    /**
   * Returns the base URL used when resolving relative links.
   *
   * \see setBaseUrl()
   */
    QUrl baseUrl() const { return mBaseUrl; }

    /**
     * Sets the base \a url to use when resolving relative links.
     *
     * \see baseUrl()
     */
    void setBaseUrl( const QUrl &url ) { mBaseUrl = url; }

    /**
     * Sets the user \a stylesheet CSS rules to use while rendering the HTML content.
     *
     * These allow for overriding the styles specified within the HTML source.
     *
     * \see userStylesheet()
     */
    void setUserStylesheet( const QString &stylesheet ) { mUserStylesheet = stylesheet; }

    /**
     * Returns the user stylesheet CSS rules used while rendering the HTML content.
     *
     * These overriding the styles specified within the HTML source.
     *
     * \see setUserStylesheet()
     */
    QString userStylesheet() const { return mUserStylesheet; }

    /**
     * Sets the feature \a json.
     *
     * When the HTML has been loaded, an attempt will be made to call a "setFeature" function,
     * passing this custom json object.
     *
     * \see featureJson()
     */
    void setFeatureJson( const QVariantMap &json ) { mFeatureJson = json; }

    /**
     * Returns the feature json.
     *
     * When the HTML has been loaded, an attempt will be made to call a "setFeature" function,
     * passing this custom json object.
     *
     * \see setFeatureJson()
     */
    QVariantMap featureJson() const { return mFeatureJson; }

    /**
     * Returns the settings encapsulated in a variantmap, for passing to the qgis_html_to_pdf utility.
     */
    QVariantMap asMap() const;

  private:
    QUrl mBaseUrl;
    QString mUserStylesheet;
    QVariantMap mFeatureJson;
};

#endif // QGSHTMLRENDERSETTINGS_H
