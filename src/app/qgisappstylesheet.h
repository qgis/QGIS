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

/** @class QgisAppStyleSheet
 * @brief Adjustable stylesheet for the Qgis application
 */
class APP_EXPORT QgisAppStyleSheet: public QObject
{
    Q_OBJECT

  public:
    QgisAppStyleSheet( QObject * parent = 0 );
    ~QgisAppStyleSheet();

    /** Return changeable options built from settings and/or defaults */
    QMap<QString, QVariant> defaultOptions();

    /** Generate stylesheet
     * @param opts generated default option values, or a changed copy of them
     * @note on success emits appStyleSheetChanged
     */
    void buildStyleSheet( const QMap<QString, QVariant>& opts );

    /** Save changed default option keys/values to user settings */
    void saveToSettings( const QMap<QString, QVariant>& opts );

    /** Get reference font for initial qApp */
    QFont defaultFont() { return mDefaultFont; }

  signals:
    /** Signal the successful stylesheet build results
     * @note connect to (app|widget)->setStyleSheet or similar custom slot
     */
    void appStyleSheetChanged( const QString& appStyleSheet );

  private:
    /** Set active configuration values */
    void setActiveValues();

    // qt styles
    QString mStyle; // active style name (lowercase)
    bool mMotifStyle; // motif
    bool mCdeStyle; // cde
    bool mPlastqStyle; // plastique
    bool mCleanLkStyle; // cleanlooks
    bool mGtkStyle; // gtk+
    bool mWinStyle; // windows
    bool mWinXpStyle; // windowsxp
    bool mWinVistaStyle; // windowsvista
    bool mMacStyle; // macintosh (aqua)
    bool mOxyStyle; // oxygen

    // default font saved for reference
    QFont mDefaultFont;

    // platforms, specific
    bool mLinuxOS;
    bool mWinOS;
    bool mMacOS;
    bool mAndroidOS;

    // platforms, general
    bool mUnix;

    // window servers
    bool mX11WS;
    bool mWinWS;
    bool mMacWS;
};

#endif //QGISAPPSTYLESHEET_H
