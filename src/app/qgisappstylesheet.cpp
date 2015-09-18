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
#include "qgisapp.h"
#include "qgslogger.h"

#include <QFont>
#include <QSettings>
#include <QStyle>

/** @class QgisAppStyleSheet
 * @brief Adjustable stylesheet for the Qgis application
 */

QgisAppStyleSheet::QgisAppStyleSheet( QObject *parent )
    : QObject( parent )
{
  setActiveValues();
}

QgisAppStyleSheet::~QgisAppStyleSheet()
{
}

QMap<QString, QVariant> QgisAppStyleSheet::defaultOptions()
{
  QMap<QString, QVariant> opts;

  // the following default values, before insertion in opts, can be
  // configured using the platforms and window servers defined in the
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

  bool gbxCustom = ( mMacStyle ? true : false );
  opts.insert( "groupBoxCustom", settings.value( "groupBoxCustom", QVariant( gbxCustom ) ) );

  settings.endGroup(); // "qgis/stylesheet"

  opts.insert( "iconSize", settings.value( "/IconSize", QGIS_ICON_SIZE ) );

  return opts;
}

void QgisAppStyleSheet::buildStyleSheet( const QMap<QString, QVariant>& opts )
{
  QString ss;

  // QgisApp-wide font
  QString fontSize = opts.value( "fontPointSize" ).toString();
  QgsDebugMsg( QString( "fontPointSize: %1" ).arg( fontSize ) );
  if ( fontSize.isEmpty() ) { return; }

  QString fontFamily = opts.value( "fontFamily" ).toString();
  QgsDebugMsg( QString( "fontFamily: %1" ).arg( fontFamily ) );
  if ( fontFamily.isEmpty() ) { return; }

  ss += QString( "* { font: %1pt \"%2\"} " ).arg( fontSize ).arg( fontFamily );

  // QGroupBox and QgsCollapsibleGroupBox, mostly for Ubuntu and Mac
  bool gbxCustom = opts.value( "groupBoxCustom" ).toBool();
  QgsDebugMsg( QString( "groupBoxCustom: %1" ).arg( gbxCustom ) );

  ss += "QGroupBox{";
  // doesn't work for QGroupBox::title
  ss += QString( "color: rgb(%1,%1,%1);" ).arg( mMacStyle ? 25 : 60 );
  ss += "font-weight: bold;";

  if ( gbxCustom )
  {
    ss += QString( "background-color: rgba(0,0,0,%1%);" )
          .arg( mWinOS && mStyle.startsWith( "windows" ) ? 0 : 3 );
    ss += "border: 1px solid rgba(0,0,0,20%);";
    ss += "border-radius: 5px;";
    ss += "margin-top: 2.5ex;";
    ss += QString( "margin-bottom: %1ex;" ).arg( mMacStyle ? 1.5 : 1 );
  }
  ss += "} ";
  if ( gbxCustom )
  {
    ss += "QGroupBox:flat{";
    ss += "background-color: rgba(0,0,0,0);";
    ss += "border: rgba(0,0,0,0);";
    ss += "} ";

    ss += "QGroupBox::title{";
    ss += "subcontrol-origin: margin;";
    ss += "subcontrol-position: top left;";
    ss += "margin-left: 6px;";
    if ( !( mWinOS && mStyle.startsWith( "windows" ) ) && !mOxyStyle )
    {
      ss += "background-color: rgba(0,0,0,0);";
    }
    ss += "} ";
  }

  //sidebar style
  QString style = "QListWidget#mOptionsListWidget {"
                  "    background-color: rgb(69, 69, 69, 220);"
                  "    outline: 0;"
                  "}"
                  "QListWidget#mOptionsListWidget::item {"
                  "    color: white;"
                  "    padding: 3px;"
                  "}"
                  "QListWidget#mOptionsListWidget::item::selected {"
                  "    color: black;"
                  "    background-color:palette(Window);"
                  "    padding-right: 0px;"
                  "}";
  ss += style;

  // Fix selection color on loosing focus (Windows)
  const QPalette palette = qApp->palette();

  ss += QString( "QTableView {"
                 "selection-background-color: %1;"
                 "selection-color: %2;"
                 "}" )
        .arg( palette.highlight().color().name() )
        .arg( palette.highlightedText().color().name() );

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
  settings.endGroup(); // "qgis/stylesheet"
}

void QgisAppStyleSheet::setActiveValues()
{
  mStyle = qApp->style()->objectName(); // active style name (lowercase)
  QgsDebugMsg( QString( "Style name: %1" ).arg( mStyle ) );

  mMotifStyle = mStyle.contains( "motif" ) ? true : false; // motif
  mCdeStyle = mStyle.contains( "cde" ) ? true : false; // cde
  mPlastqStyle = mStyle.contains( "plastique" ) ? true : false; // plastique
  mCleanLkStyle = mStyle.contains( "cleanlooks" ) ? true : false; // cleanlooks
  mGtkStyle = mStyle.contains( "gtk" ) ? true : false; // gtk+
  mWinStyle = mStyle.contains( "windows" ) ? true : false; // windows
  mWinXpStyle = mStyle.contains( "windowsxp" ) ? true : false; // windowsxp
  mWinVistaStyle = mStyle.contains( "windowsvista" ) ? true : false; // windowsvista
  mMacStyle = mStyle.contains( "macintosh" ) ? true : false; // macintosh (aqua)
  mOxyStyle = mStyle.contains( "oxygen" ) ? true : false; // oxygen

  mDefaultFont = qApp->font(); // save before it is changed in any way

  // platforms, specific
#ifdef Q_OS_LINUX
  mLinuxOS = true;
#else
  mLinuxOS = false;
#endif
#ifdef Q_OS_WIN
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
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
  mX11WS = true;
#else
  mX11WS = false;
#endif
#ifdef Q_OS_WIN
  mWinWS = true;
#else
  mWinWS = false;
#endif
#ifdef Q_OS_MAC
  mMacWS = true;
#else
  mMacWS = false;
#endif

}
