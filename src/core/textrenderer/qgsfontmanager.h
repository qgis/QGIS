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
     *
     * \returns TRUE if installation was successful.
     */
    bool installFontsFromData( const QByteArray &data, QString &errorMessage SIP_OUT, QStringList &families SIP_OUT, QString &licenseDetails SIP_OUT );

    /**
     * Sets the \a directory to use for user fonts.
     *
     * This directory will be scanned for any TTF or OTF font files, which will automatically be added and made
     * available for use in the QGIS session. Additionally, any fonts downloaded via downloadAndInstallFont() will be
     * installed into this directory.
     */
    void setUserFontDirectory( const QString &directory );

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
    mutable QReadWriteLock mReplacementLock;
    QString mUserFontDirectory;

    void storeFamilyReplacements();
};

#endif // QGSFONTMANAGER_H
