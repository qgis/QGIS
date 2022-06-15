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
     * \see fontFamilyReplacements()
     * \see addFontFamilyReplacement()
     */
    void setFontFamilyReplacements( const QMap< QString, QString> &replacements );

  private:

    QMap< QString, QString > mFamilyReplacements;

    void storeFamilyReplacements();

};

#endif // QGSFONTMANAGER_H
