/***************************************************************************
    qgsnewsfeedparser.h
    -------------------
    begin                : July 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSNEWSFEEDPARSER_H
#define QGSNEWSFEEDPARSER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgssettingstree.h"

#include <QDateTime>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <QUrl>

using namespace Qt::StringLiterals;

class QgsNetworkContentFetcher;
class QgsSettingsEntryBool;
class QgsSettingsEntryInteger64;
class QgsSettingsEntryString;
class QgsSettingsEntryDouble;
class QgsSettingsEntryVariant;

/**
 * \ingroup core
 * \brief Parser for published QGIS news feeds.
 *
 * This class is designed to work with the specialized QGIS news feed API. See
 * https://github.com/elpaso/qgis-feed.
 *
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsNewsFeedParser : public QObject
{
    Q_OBJECT

    Q_PROPERTY( bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged );
    Q_PROPERTY( bool isFetching READ isFetching NOTIFY isFetchingChanged );

  public:

#ifndef SIP_RUN
    static inline QgsSettingsTreeNamedListNode *sTreeNewsFeed = QgsSettingsTree::sTreeApp->createNamedListNode( u"news-feed"_s );
    static const QgsSettingsEntryInteger64 *settingsFeedLastFetchTime;
    static const QgsSettingsEntryString *settingsFeedLanguage;
    static const QgsSettingsEntryDouble *settingsFeedLatitude;
    static const QgsSettingsEntryDouble *settingsFeedLongitude;

    static inline QgsSettingsTreeNamedListNode *sTreeNewsFeedEntries = sTreeNewsFeed->createNamedListNode( u"entries"_s );
    static const QgsSettingsEntryString *settingsFeedEntryTitle;
    static const QgsSettingsEntryString *settingsFeedEntryImageUrl;
    static const QgsSettingsEntryString *settingsFeedEntryContent;
    static const QgsSettingsEntryString *settingsFeedEntryLink;
    static const QgsSettingsEntryBool *settingsFeedEntrySticky;
    static const QgsSettingsEntryVariant *settingsFeedEntryExpiry;
#endif

    /**
     * \brief Represents a single entry from a news feed.
     * \ingroup core
     * \since QGIS 3.10
     */
    class Entry
    {
      public:

        //! Unique entry identifier
        int key = 0;

        //! Entry title
        QString title;

        //! Optional URL for image associated with entry
        QString imageUrl;

        //! Optional image data
        QPixmap image;

        //! HTML content of news entry
        QString content;

        //! Optional URL link for entry
        QUrl link;

        //! TRUE if entry is "sticky" and should always be shown at the top
        bool sticky = false;

        //! Optional auto-expiry time for entry
        QDateTime expiry;
    };

    /**
     * Constructor for QgsNewsFeedParser, parsing the specified \a feedUrl.
     *
     * The optional \a authcfg argument can be used to specify an authentication
     * configuration to use when connecting to the feed.
     */
    QgsNewsFeedParser( const QUrl &feedUrl, const QString &authcfg = QString(), QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns TRUE if the feed URL associated with the news parser is enabled.
     *
     * \since QGIS 4.0
     */
    bool enabled() const SIP_SKIP { return mEnabled; }

    /**
     * Sets whether the feed URL associated with the news parser is enabled.
     *
     * \since QGIS 4.0
     */
    void setEnabled( bool enabled ) SIP_SKIP;

    /**
     * Returns TRUE if the news parser is fetching items.
     *
     * \since QGIS 4.0
     */
    bool isFetching() const { return mIsFetching; }

    /**
     * Returns a list of existing entries in the feed.
     */
    QList< QgsNewsFeedParser::Entry > entries() const;

    /**
     * Dismisses an entry with matching \a key.
     *
     * This removes the entry from the local store, ensuring it will never be present again.
     *
     * \see dismissAll()
     */
    Q_INVOKABLE void dismissEntry( int key );

    /**
     * Dismisses all current news items.
     * \see dismissEntry()
     */
    Q_INVOKABLE void dismissAll();

    /**
     * Returns the authentication configuration for the parser.
     */
    QString authcfg() const;

    /**
     * Returns the settings key used for a feed with the given \a baseUrl.
     */
    static QString keyForFeed( const QString &baseUrl );

  public slots:

    /**
     * Fetches new entries from the feed's URL.
     * \see fetched()
     */
    void fetch();

  signals:

    /**
     * Emitted when \a entries have been fetched from the feed.
     *
     * \see fetch()
     */
    void fetched( const QList< QgsNewsFeedParser::Entry > &entries );

    /**
     * Emitted whenever a new \a entry is available from the feed (as a result
     * of a call to fetch()).
     *
     * \see fetch()
     */
    void entryAdded( const QgsNewsFeedParser::Entry &entry );

    /**
     * Emitted whenever an existing \a entry is available from the feed (as a result
     * of a call to fetch()).
     *
     * \see fetch()
     *
     * \since QGIS 3.36
     */
    void entryUpdated( const QgsNewsFeedParser::Entry &entry );

    /**
     * Emitted whenever an \a entry is dismissed (as a result of a call
     * to dismissEntry()).
     *
     * \see dismissEntry()
     */
    void entryDismissed( const QgsNewsFeedParser::Entry &entry );

    /**
     * Emitted when the image attached to the entry with the specified \a key has been fetched
     * and is now available.
     */
    void imageFetched( int key, const QPixmap &pixmap );

    /**
     * Emitted when the enabled/disabled state of the feed URL associated to the news parser changes.
     * \since QGIS 4.0
     */
    void enabledChanged();

    /**
     * Emitted when the news parser's fetching state changes.
     * \since QGIS 4.0
     */
    void isFetchingChanged();

  private slots:

    void onFetch( const QString &content );

  private:

    QString mBaseUrl;
    QUrl mFeedUrl;
    QString mAuthCfg;
    qint64 mFetchStartTime = 0;
    QString mFeedKey;

    QList< Entry > mEntries;
    bool mBlockSignals = false;

    bool mEnabled = true;
    bool mIsFetching = false;

    void readStoredEntries();
    Entry readEntryFromSettings( int key );
    void storeEntryInSettings( const Entry &entry );
    void fetchImageForEntry( const Entry &entry );

    friend class TestQgsNewsFeedParser;

};

#endif // QGSNEWSFEEDPARSER_H
