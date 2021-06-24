/***************************************************************************
  qgsexternalstorage.h
  --------------------------------------
  Date                 : March 2021
  Copyright            : (C) 2021 by Julien Cabieces
  Email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXTERNALSTORAGE_H
#define QGSEXTERNALSTORAGE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"

#include <QObject>
#include <QString>

class QgsExternalStorageFetchedContent;
class QgsExternalStorageStoredContent;

/**
 * \ingroup core
 * \brief Abstract interface for external storage - to be implemented by various backends
 * and registered in QgsExternalStorageRegistry.
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsExternalStorage
{
  public:

    /**
     * Destructor
     */
    virtual ~QgsExternalStorage() = default;

    /**
     * Unique identifier of the external storage type.
     */
    virtual QString type() const = 0;

    /**
     * Store file \a filePath to the \a url for this project external storage.
     * Storing process is run in background.
     * Returns a QgsExternalStorageStoredContent to follow the status of the stored resource.
     *
     * After using this method, user should check if the returned content is not already finished
     * (storing could be instantaneous, if file has already been stored for instance) and then wait
     * for QgsExternalStorageStoredContent::stored(), QgsExternalStorageStoredContent::errorOccurred() or
     * QgsExternalStorageStoredContent::canceled() signals.
     *
     * It's possible to give \a authcfg authentification configuration id in case its needed.
     *
     * Caller takes ownership of the returned symbol.
     */
    virtual QgsExternalStorageStoredContent *store( const QString &filePath, const QString &url, const QString &authcfg = QString() ) const = 0 SIP_FACTORY;

    /**
     * Fetch file from \a url for this project external storage.
     * Fetching process is run in background.
     * Returns a QgsExternalStorageFetchedContent to follow the status of the fetched resource.
     *
     * After using this method, user should check if the returned content is not already finished
     * (fetching could be instantaneous, if file has already been fetched and cached for instance)
     * and then wait for QgsExternalStorageStoredContent::fetched(), QgsExternalStorageStoredContent::errorOccurred() or
     * QgsExternalStorageStoredContent::canceled() signals.
     *
     * It's possible to give \a authcfg authentification configuration id in case its needed.
     */
    virtual QgsExternalStorageFetchedContent *fetch( const QString &url, const QString &authcfg = QString() ) const = 0 SIP_FACTORY;
};

/**
 * \ingroup core
 * \brief Base class for QgsExternalStorage stored and fetched content
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsExternalStorageContent : public QObject
{
    Q_OBJECT

  public:

    /**
     * Returns content status
     */
    Qgis::ContentStatus status() const;

    /**
     * Returns error textual description if an error occured and status() returns Failed
     */
    const QString &errorString() const;

  public slots:

    /**
     * Cancel content fetching/storing
     */
    virtual void cancel() {};

  signals:

    /**
     * The signal is emitted when an error occurred. \a errorString is a textual description of the error
     */
    void errorOccurred( const QString &errorString );

    /**
     * The signal is emitted whenever content fetching/storing estimated progression value \a progress has changed.
     * \a progress value is between 0 and 100.
     */
    void progressChanged( double progress );

    /**
     * The signal is emitted when content fetching/storing has been canceled
     */
    void canceled();

  protected:

    /**
     * Update content according to given \a errorMsg error message
     * Inherited classes should call this method whenever they meet an error.
     */
    void reportError( const QString &errorMsg );

    Qgis::ContentStatus mStatus = Qgis::ContentStatus::NotStarted;
    QString mErrorString;
};

/**
 * \ingroup core
 * \brief Class for QgsExternalStorage fetched content
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsExternalStorageFetchedContent : public QgsExternalStorageContent
{
    Q_OBJECT

  public:

    /**
     * Return fetched resource file path
     */
    virtual QString filePath() const = 0;

  signals:

    /**
     * The signal is emitted when the resource has successfully been fetched
     */
    void fetched();
};

/**
 * \ingroup core
 * \brief Class for QgsExternalStorage stored content
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsExternalStorageStoredContent : public QgsExternalStorageContent
{
    Q_OBJECT

  public:

    /**
     * Return stored resource URL
     */
    virtual QString url() const = 0;

  signals:

    /**
     * The signal is emitted when the resource has successfully been stored
     */
    void stored();
};

#endif // QGSEXTERNALSTORAGE_H
