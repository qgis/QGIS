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
     * Returns the translated external storage name, which should be used for any
     * user-visible display of the external storage name.
     */
    virtual QString displayName() const = 0;

    /**
     * Stores file \a filePath to the \a url for this project external storage.
     * Storing process is run in background.
     * Returns a QgsExternalStorageStoredContent to follow the status of the stored resource.
     *
     * \a storingMode defines if the download will start immediately or shall be manually triggered
     * calling QgsExternalStorageStoredContent::store(). User should use
     * Qgis::ActionStart::Deferred if he needs to connect the stored() signal.
     *
     * After using this method, user wait for QgsExternalStorageStoredContent::stored(),
     * QgsExternalStorageStoredContent::errorOccurred() or QgsExternalStorageStoredContent::canceled() signals.
     *
     * It's possible to give \a authCfg authentication configuration id in case its needed.
     *
     * Caller takes ownership of the returned symbol.
     */
    QgsExternalStorageStoredContent *store( const QString &filePath, const QString &url, const QString &authCfg = QString(), Qgis::ActionStart storingMode = Qgis::ActionStart::Deferred ) const SIP_FACTORY;

    /**
     * Fetches file from \a url for this project external storage.
     * Fetching process is run in background.
     * Returns a QgsExternalStorageFetchedContent to follow the status of the fetched resource.
     *
     * \a fetchingMode defines if the download will start immediately or shall be manually triggered
     * calling QgsExternalStorageFetchedContent::fetch(). User should use
     * Qgis::ActionStart::Deferred if he needs to connect the fetched() signal.
     *
     * After using this method, user should wait for QgsExternalStorageStoredContent::fetched(),
     * QgsExternalStorageStoredContent::errorOccurred() or QgsExternalStorageStoredContent::canceled() signals.
     *
     * It's possible to give \a authCfg authentication configuration id in case its needed.
     */
    QgsExternalStorageFetchedContent *fetch( const QString &url, const QString &authCfg = QString(), Qgis::ActionStart fetchingMode = Qgis::ActionStart::Deferred ) const SIP_FACTORY;

  protected:

    /**
     * Stores file \a filePath to the \a url using \a authCfg authentication for this project external storage.
     * \see QgsExternalStorage::store()
     */
    virtual QgsExternalStorageStoredContent *doStore( const QString &filePath, const QString &url, const QString &authCfg = QString() ) const = 0 SIP_FACTORY;

    /**
     * Fetches file from \a url using \a authCfg for this project external storage.
     * \see QgsExternalStorage::fetch()
     */
    virtual QgsExternalStorageFetchedContent *doFetch( const QString &url, const QString &authCfg = QString() ) const = 0 SIP_FACTORY;
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
     * Returns error textual description if an error occurred and status() returns Failed
     */
    const QString &errorString() const;

  public slots:

    /**
     * Cancels content fetching/storing
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
     * Returns fetched resource file path
     */
    virtual QString filePath() const = 0;

    /**
     * Starts fetching
     */
    virtual void fetch() = 0;

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
     * Returns stored resource URL
     */
    virtual QString url() const = 0;

    /**
     * Starts storing
     */
    virtual void store() = 0;

  signals:

    /**
     * The signal is emitted when the resource has successfully been stored
     */
    void stored();
};

#endif // QGSEXTERNALSTORAGE_H
