/***************************************************************************
    qgsauthguiutils.h
    ---------------------
    begin                : October 24, 2014
    copyright            : (C) 2014 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAUTHGUIUTILS_H
#define QGSAUTHGUIUTILS_H

#include <QColor>

class QWidget;
class QgsMessageBar;


/** \ingroup gui
 * \brief Utility functions for use by authentication GUI widgets or standalone apps
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsAuthGuiUtils
{
  public:

    /** Green color representing valid, trusted, etc. certificate */
    static QColor greenColor();

    /** Orange color representing loaded component, but not stored in database */
    static QColor orangeColor();

    /** Red color representing invalid, untrusted, etc. certificate */
    static QColor redColor();

    /** Yellow color representing caution regarding action */
    static QColor yellowColor();

    /** Green text stylesheet representing valid, trusted, etc. certificate */
    static QString greenTextStyleSheet( const QString& selector = "*" );

    /** Orange text stylesheet representing loaded component, but not stored in database */
    static QString orangeTextStyleSheet( const QString& selector = "*" );

    /** Red text stylesheet representing invalid, untrusted, etc. certificate */
    static QString redTextStyleSheet( const QString& selector = "*" );


    /** Verify the authentication system is active, else notify user */
    static bool isDisabled( QgsMessageBar *msgbar, int timeout = 0 );

    /** Sets the cached master password (and verifies it if its hash is in authentication database) */
    static void setMasterPassword( QgsMessageBar *msgbar, int timeout = 0 );

    /** Clear the currently cached master password (not its hash in database) */
    static void clearCachedMasterPassword( QgsMessageBar *msgbar, int timeout = 0 );

    /** Reset the cached master password, updating its hash in authentication database and reseting all existing configs to use it */
    static void resetMasterPassword( QgsMessageBar *msgbar, int timeout = 0, QWidget *parent = nullptr );

    /** Clear all cached authentication configs for session */
    static void clearCachedAuthenticationConfigs( QgsMessageBar *msgbar, int timeout = 0 );

    /** Remove all authentication configs */
    static void removeAuthenticationConfigs( QgsMessageBar *msgbar, int timeout = 0, QWidget *parent = nullptr );

    /** Completely clear out the authentication database (configs and master password) */
    static void eraseAuthenticationDatabase( QgsMessageBar *msgbar, int timeout = 0, QWidget *parent = nullptr );

    /** Color a widget via a stylesheet if a file path is found or not */
    static void fileFound( bool found, QWidget * widget );

    /** Open file dialog for auth associated widgets */
    static QString getOpenFileName( QWidget *parent, const QString& title, const QString& extfilter );
};

#endif // QGSAUTHGUIUTILS_H
