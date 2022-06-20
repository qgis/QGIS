/***************************************************************************
    qgsfontmanager.h
    ------------------
    Date                 : June 2022
    Copyright            : (C) 2022 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFONTMANAGER_H
#define QGSFONTMANAGER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgssettingsentryimpl.h"
#include <QObject>
#include <QMap>
#include <QReadWriteLock>
#include <QSet>

/**
 * \ingroup core
 * \class QgsFontManager
 *
 * \brief Manages available fonts and font installation for a QGIS instance.
 *
 * \note QgsFontManager is not usually directly created, but rather accessed through QgsApplication::fontManager().
 *
 * \since QGIS 3.28
 */
class CORE_EXPORT QgsFontManager : public QObject
{
    Q_OBJECT

  public:

#ifndef SIP_RUN
    //! Settings entry for font family replacements
    static const inline QgsSettingsEntryStringList settingsFontFamilyReplacements = QgsSettingsEntryStringList( QStringLiteral( "fontFamilyReplacements" ), QgsSettings::Prefix::FONTS, QStringList(), QStringLiteral( "Automatic font family replacements" ) );

    //! Settings entry for font family replacements
    static const inline QgsSettingsEntryBool settingsDownloadMissingFonts = QgsSettingsEntryBool( QStringLiteral( "downloadMissingFonts" ), QgsSettings::Prefix::FONTS, true, QStringLiteral( "Automatically download missing fonts whenever possible" ) );
#endif

    /**
     * Constructor for QgsFontManager, with the specified \a parent object.
     *
     * \note QgsFontManager is not usually directly created, but rather accessed through QgsApplication::fontManager().
     */
    explicit QgsFontManager( QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the map of automatic font family replacements.
     *
     * This map is used to transparently map an original font family to an alternative
     * font family, e.g. to permit graceful handling of opening projects which reference
     * fonts which are not available on the system.
     *
     * The map keys are the original font family names, and the values are the alternative
     * replacement family to use for the font.
     *
     * \note This method is thread safe.
     *
     * \see addFontFamilyReplacement()
     * \see setFontFamilyReplacements()
     */
    QMap< QString, QString > fontFamilyReplacements() const;

    /**
     * Adds a new font replacement from the \a original font family to a \a replacement font family.
     *
     * This is used to transparently map an original font family to an alternative
     * font family, e.g. to permit graceful handling of opening projects which reference
     * fonts which are not available on the system.
     *
     * The replacement map is stored locally and persists across QGIS sessions.
     *
     * If \a replacement is an empty string then any existing mapping for the \a original
     * family will be removed.
     *
     * \note This method is thread safe.
     *
     * \see fontFamilyReplacements()
     * \see setFontFamilyReplacements()
     */
    void addFontFamilyReplacement( const QString &original, const QString &replacement );

    /**
     * Sets the map of automatic font family \a replacements.
     *
     * This map is used to transparently map an original font family to an alternative
     * font family, e.g. to permit graceful handling of opening projects which reference
     * fonts which are not available on the system.
     *
     * The map keys are the original font family names, and the values are the alternative
     * replacement family to use for the font.
     *
     * The replacement map is stored locally and persists across QGIS sessions.
     *
     * \note This method is thread safe.
     *
     * \see fontFamilyReplacements()
     * \see addFontFamilyReplacement()
     */
    void setFontFamilyReplacements( const QMap< QString, QString> &replacements );

    /**
     * Processes a font family \a name, applying any matching fontFamilyReplacements()
     * to the name.
     *
     * \note This method is thread safe.
     */
    QString processFontFamilyName( const QString &name ) const;

    /**
     * Installs user fonts from the profile/fonts directory as application fonts.
     *
     * \note Not available in Python bindings
     */
    void installUserFonts() SIP_SKIP;

    /**
     * Tries to download and install the specified font \a family.
     *
     * This method will attempt to download missing fonts, if the font download URL
     * is known and the font is freely licensed.
     *
     * Returns TRUE if a download link for the family is known and the
     * download has commenced, or FALSE if the family is not known and cannot be
     * automatically downloaded.
     *
     * The actual download operation occurs in a background task, and this method
     * returns immediately. Connect to fontDownloaded() in order to respond when the
     * font is installed and available for use.
     *
     * \warning Before calling this method a QgsApplication must be fully initialized
     * and a call to enableFontDownloadsForSession() made.
     *
     * \param family input font family name to try to match to known fonts
     * \param matchedFamily will be set to found font family if a match was successful
     * \returns TRUE if match was successful and the download will occur
     */
    bool tryToDownloadFontFamily( const QString &family, QString &matchedFamily SIP_OUT );

    /**
     * Enables font downloads the the current QGIS session.
     *
     * \warning Ensure that the QgsApplication is fully initialized before calling this method.
     */
    void enableFontDownloadsForSession();

    /**
     * Returns the URL at which the font \a family can be downloaded.
     *
     * This method relies on a hardcoded list of available freely licensed fonts, and will
     * return an empty string for any font families not present in this list.
     *
     * \param family input font family name to try to match to known fonts
     * \param matchedFamily will be set to found font family if a match was successful
     * \returns URL to download font, or an empty string if no URL is available
     */
    QString urlForFontDownload( const QString &family, QString &matchedFamily SIP_OUT ) const;

    /**
     * Downloads a font and installs in the user's profile/fonts directory as an application font.
     *
     * The download will proceed in a background task.
     *
     * The optional \a identifier string can be used to specify a user-friendly name for the download
     * tasks, e.g. the font family name if known.
     *
     * \see fontDownloaded()
     * \see fontDownloadErrorOccurred()
     */
    void downloadAndInstallFont( const QUrl &url, const QString &identifier = QString() );

    /**
     * Installs local user fonts from the specified raw \a data.
     *
     * The \a data array may correspond to the contents of a TTF or OTF font file,
     * or a zipped archive of font files.
     *
     * \param data raw font data or zipped font data
     * \param errorMessage will be set to a descriptive error message if the installation fails
     * \param families will be populated with a list of font families installed from the data
     * \param licenseDetails will be populated with font license details, if found
     * \param filename filename hint for destination file. Will be ignored for archived content (e.g. zip file data)
     *
     * \returns TRUE if installation was successful.
     */
    bool installFontsFromData( const QByteArray &data, QString &errorMessage SIP_OUT, QStringList &families SIP_OUT, QString &licenseDetails SIP_OUT, const QString &filename = QString() );

    /**
     * Adds a \a directory to use for user fonts.
     *
     * This directory will be scanned for any TTF or OTF font files, which will automatically be added and made
     * available for use in the QGIS session.
     *
     * Additionally, if this is the first user font directory added, any fonts downloaded via downloadAndInstallFont() will be
     * installed into this directory.
     */
    void addUserFontDirectory( const QString &directory );

    /**
     * Returns the mapping of installed user fonts to font families.
     *
     * The map keys are the file names, the values are a list of families provided by the file.
     */
    QMap< QString, QStringList > userFontToFamilyMap() const;

    /**
     * Removes the user font at the specified \a path.
     */
    bool removeUserFont( const QString &path );

  signals:

    /**
     * Emitted when a font has downloaded and been locally loaded.
     *
     * The \a families list specifies the font families contained in the downloaded font.
     *
     * If found, the \a licenseDetails string will be populated with corresponding font license details.
     *
     * \see downloadAndInstallFont()
     * \see fontDownloadErrorOccurred()
     */
    void fontDownloaded( const QStringList &families, const QString &licenseDetails );

    /**
     * Emitted when an error occurs during font downloading.
     *
     * \see downloadAndInstallFont()
     * \see fontDownloaded()
     */
    void fontDownloadErrorOccurred( const QUrl &url, const QString &identifier, const QString &error );

  private:

    QMap< QString, QString > mFamilyReplacements;
    QMap< QString, QString > mLowerCaseFamilyReplacements;
    QMap< QString, QStringList > mUserFontToFamilyMap;
    QMap< QString, int > mUserFontToIdMap;
    mutable QReadWriteLock mReplacementLock;
    QStringList mUserFontDirectories;

    bool mEnableFontDownloads = false;
    QMap< QString, QString > mPendingFontDownloads;
    QMap< QString, QString > mDeferredFontDownloads;

    void storeFamilyReplacements();
    void installFontsFromDirectory( const QString &dir );
};

#endif // QGSFONTMANAGER_H
