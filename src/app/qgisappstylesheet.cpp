/***************************************************************************
                         qgisappstylesheet.cpp
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

#include "qgisappstylesheet.h"
#include "qgsapplication.h"
#include "qgslogger.h"

#include <QFont>
#include <QSettings>

/** @class QgisAppStyleSheet
 * @brief Adjustable stylesheet for the Qgis application
 */

QgisAppStyleSheet::QgisAppStyleSheet( QObject *parent )
    : QObject( parent )
{
  // platforms, specific
#ifdef Q_OS_LINUX
  mLinuxOS = true;
#else
  mLinuxOS = false;
#endif
#ifdef Q_OS_WIN32
  mWinOS = true;
#else
  mWinOS = false;
#endif
#ifdef Q_OS_MAC
  mMacOS = true;
#else
  mMacOS = false;
#endif
#ifdef ANDROID
  mAndroidOS = true;
#else
  mAndroidOS = false;
#endif

  // platforms, general
#ifdef Q_OS_UNIX
  mUnix = true;
#else
  mUnix = false;
#endif

  // window servers
#ifdef Q_WS_X11
  mX11WS = true;
#else
  mX11WS = false;
#endif
#ifdef Q_WS_WIN
  mWinWS = true;
#else
  mWinWS = false;
#endif
#ifdef Q_WS_MAC
  mMacWS = true;
#else
  mMacWS = false;
#endif
}

QgisAppStyleSheet::~QgisAppStyleSheet()
{
}

QMap<QString, QVariant> QgisAppStyleSheet::defaultOptions()
{
  QMap<QString, QVariant> opts;
  mDefaultFont = qApp->font(); // save before it is changed in any way

  // the following default values, before insertion in opts, can be
  // configured using the platform(s) and window server(s) defined in the
  // constructor to set reasonable non-Qt defaults for the app stylesheet
  QSettings settings;
  // handle move from old QSettings group (/) to new (/qgis/stylesheet)
  // NOTE: don't delete old QSettings keys, in case user is also running older QGIS
  QVariant oldFontPointSize = settings.value( "/fontPointSize" );
  QVariant oldFontFamily = settings.value( "/fontFamily" );

  settings.beginGroup( "qgis/stylesheet" );

  int fontSize = mDefaultFont.pointSize();
  if ( mAndroidOS )
  {
    // TODO: find a better default fontsize maybe using DPI detection or so (from Marco Bernasocchi commit)
    fontSize = 8;
  }
  if ( oldFontPointSize.isValid() && !settings.value( "fontPointSize" ).isValid() )
  {
    fontSize = oldFontPointSize.toInt();
  }
  QgsDebugMsg( QString( "fontPointSize: %1" ).arg( fontSize ) );
  opts.insert( "fontPointSize", settings.value( "fontPointSize", QVariant( fontSize ) ) );

  QString fontFamily = mDefaultFont.family();
  if ( oldFontFamily.isValid() && !settings.value( "fontFamily" ).isValid() )
  {
    fontFamily = oldFontFamily.toString();
  }
  fontFamily = settings.value( "fontFamily", QVariant( fontFamily ) ).toString();
  // make sure family exists on system
  if ( fontFamily != mDefaultFont.family() )
  {
    QFont *tempFont = new QFont( fontFamily );
    if ( tempFont->family() != fontFamily )
    {
      // missing from system, drop back to default
      fontFamily = mDefaultFont.family();
    }
    delete tempFont;
  }
  QgsDebugMsg( QString( "fontFamily: %1" ).arg( fontFamily ) );
  opts.insert( "fontFamily", QVariant( fontFamily ) );

  settings.endGroup();

  return opts;
}

void QgisAppStyleSheet::buildStyleSheet( const QMap<QString, QVariant>& opts )
{
  QString ss = QString( "" );

  QString fontSize = opts.value( "fontPointSize" ).toString();
  QgsDebugMsg( QString( "fontPointSize: %1" ).arg( fontSize ) );
  if ( fontSize.isEmpty() ) { return; }

  QString fontFamily = opts.value( "fontFamily" ).toString();
  QgsDebugMsg( QString( "fontFamily: %1" ).arg( fontFamily ) );
  if ( fontFamily.isEmpty() ) { return; }

  ss += QString( "* { font: %1pt \"%2\"} " ).arg( fontSize ).arg( fontFamily );

  QgsDebugMsg( QString( "Stylesheet built: %1" ).arg( ss ) );

  emit appStyleSheetChanged( ss );
}

void QgisAppStyleSheet::saveToSettings( const QMap<QString, QVariant>& opts )
{
  QSettings settings;
  settings.beginGroup( "qgis/stylesheet" );

  QMap<QString, QVariant>::const_iterator opt = opts.constBegin();
  while ( opt != opts.constEnd() )
  {
    settings.setValue( QString( opt.key() ), opt.value() );
    ++opt;
  }
  settings.endGroup();
}
