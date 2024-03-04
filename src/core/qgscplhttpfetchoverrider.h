/***************************************************************************
    qgscplhttpfetchoverrider.h
    --------------------------
    begin                : September 2020
    copyright            : (C) 2020 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCPLHTTPFETCHOVERRIDER_H
#define QGSCPLHTTPFETCHOVERRIDER_H

#define SIP_NO_FILE

#include <QNetworkRequest>
#include <QString>
#include <QPointer>

#include "qgis_core.h"
#include "cpl_http.h"
#include "gdal.h"

class QgsFeedback;

/**
 * \ingroup core
 * \class QgsCPLHTTPFetchOverrider
 * \brief Utility class to redirect GDAL's CPL HTTP calls through QgsBlockingNetworkRequest
 *
 * The implementation is a no-op before GDAL 3.2
 *
 * \note not available in Python bindings
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsCPLHTTPFetchOverrider
{
  public:
    //! Installs the redirection for the current thread
    explicit QgsCPLHTTPFetchOverrider( const QString &authCfg = QString(), QgsFeedback *feedback = nullptr );

    //! Destructor
    ~QgsCPLHTTPFetchOverrider();

    //! Define attribute that must be forwarded to the actual QNetworkRequest
    void setAttribute( QNetworkRequest::Attribute code, const QVariant &value );

    /**
     * Sets the \a feedback cancellation object for the redirection.
     *
     * \since QGIS 3.32
     */
    void setFeedback( QgsFeedback *feedback );

    /**
     * Returns the thread associated with the overrider.
     *
     * \since QGIS 3.32
     */
    QThread *thread() const;

  private:

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,2,0)
    static CPLHTTPResult *callback( const char *pszURL,
                                    CSLConstList papszOptions,
                                    GDALProgressFunc pfnProgress,
                                    void *pProgressArg,
                                    CPLHTTPFetchWriteFunc pfnWrite,
                                    void *pWriteArg,
                                    void *pUserData );
#endif

    QString mAuthCfg;

    QgsFeedback *mFeedback = nullptr;

    QPointer< QThread > mThread;

    std::map<QNetworkRequest::Attribute, QVariant> mAttributes;
};

#endif // QGSCPLHTTPFETCHOVERRIDER_H
