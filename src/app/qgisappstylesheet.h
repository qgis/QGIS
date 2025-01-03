/***************************************************************************
                         qgisappstylesheet.h
                         ----------------------
    begin                : Jan 18, 2013
    copyright            : (C) 2013 by Larry Shaffer
    email                : larrys at dakotacarto dot com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGISAPPSTYLESHEET_H
#define QGISAPPSTYLESHEET_H

#include <QObject>
#include <QFont>
#include <QMap>
#include "qgis_app.h"

/**
 * \class QgisAppStyleSheet
 * \brief Adjustable stylesheet for the QGIS application
 */
class APP_EXPORT QgisAppStyleSheet : public QObject
{
    Q_OBJECT

  public:
    QgisAppStyleSheet( QObject *parent = nullptr );

    //! Returns changeable options built from settings and/or defaults
    QMap<QString, QVariant> defaultOptions();

    /**
     * Build and apply a stylesheet.
     *
     * \param opts generated default option values, or a changed copy of them
     * \note on success emits appStyleSheetChanged
     */
    void applyStyleSheet( const QMap<QString, QVariant> &opts );

    /**
     * Applies an updated stylesheet using current user settings.
     */
    void updateStyleSheet();

    /**
     * Sets a new user font \a size. Set to -1 to force the default Qt font size to be used.
     */
    void setUserFontSize( double size );

    /**
     * Sets a new user font \a family. Set to an empty string to force the default Qt font family to be used.
     */
    void setUserFontFamily( const QString &family );

    //! Save changed default option keys/values to user settings
    void saveToSettings( const QMap<QString, QVariant> &opts );

    //! Gets reference font for initial qApp
    QFont defaultFont() { return mDefaultFont; }

    /**
     * Returns the user set font size override value, or -1 if not set and the Qt default font size should be used.
     */
    double userFontSize() const { return mUserFontSize; }

    /**
     * Returns the user set font family override value, or an empty if not set and the Qt default font family should be used.
     */
    QString userFontFamily() const { return mUserFontFamily; }

    /**
     * Returns the application font size to use. This will match userFontSize() if the user
     * has set a custom font size, or the defaultFont() font size otherwise.
     */
    double fontSize() const { return mUserFontSize > 0 ? mUserFontSize : mDefaultFont.pointSizeF(); }

    /**
     * Returns the application font family. This will match userFontFamily() if the user
     * has set a custom font, or the defaultFont() family otherwise.
     */
    QString fontFamily() const { return !mUserFontFamily.isEmpty() ? mUserFontFamily : mDefaultFont.family(); }

  signals:

    /**
     * Signal the successful stylesheet build results
     * \note connect to (app|widget)->setStyleSheet or similar custom slot
     */
    void appStyleSheetChanged( const QString &appStyleSheet );

  private:
    //! Sets active configuration values
    void setActiveValues();

    // qt styles
    QString mStyle;         // active style name (lowercase)
    bool mMacStyle = false; // macintosh (aqua)
    bool mOxyStyle = false; // oxygen

    // default font saved for reference
    QFont mDefaultFont;

    double mUserFontSize = -1;
    QString mUserFontFamily;

    // platforms, specific
    bool mWinOS = false;
    bool mAndroidOS = false;

    static bool sIsFirstRun;
};

#endif //QGISAPPSTYLESHEET_H
