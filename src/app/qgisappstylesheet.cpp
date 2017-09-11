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
#include "qgssettings.h"

#include <QFont>
#include <QStyle>

QgisAppStyleSheet::QgisAppStyleSheet( QObject *parent )
  : QObject( parent )
{
  setActiveValues();
}

QMap<QString, QVariant> QgisAppStyleSheet::defaultOptions()
{
  QMap<QString, QVariant> opts;

  // the following default values, before insertion in opts, can be
  // configured using the platforms and window servers defined in the
  // constructor to set reasonable non-Qt defaults for the app stylesheet
  QgsSettings settings;
  // handle move from old QgsSettings group (/) to new (/qgis/stylesheet)
  // NOTE: don't delete old QgsSettings keys, in case user is also running older QGIS
  QVariant oldFontPointSize = settings.value( QStringLiteral( "fontPointSize" ) );
  QVariant oldFontFamily = settings.value( QStringLiteral( "fontFamily" ) );

  settings.beginGroup( QStringLiteral( "qgis/stylesheet" ) );

  int fontSize = mDefaultFont.pointSize();
  if ( mAndroidOS )
  {
    // TODO: find a better default fontsize maybe using DPI detection or so (from Marco Bernasocchi commit)
    fontSize = 8;
  }
  if ( oldFontPointSize.isValid() && !settings.value( QStringLiteral( "fontPointSize" ) ).isValid() )
  {
    fontSize = oldFontPointSize.toInt();
  }
  QgsDebugMsg( QString( "fontPointSize: %1" ).arg( fontSize ) );
  opts.insert( QStringLiteral( "fontPointSize" ), settings.value( QStringLiteral( "fontPointSize" ), QVariant( fontSize ) ) );

  QString fontFamily = mDefaultFont.family();
  if ( oldFontFamily.isValid() && !settings.value( QStringLiteral( "fontFamily" ) ).isValid() )
  {
    fontFamily = oldFontFamily.toString();
  }
  fontFamily = settings.value( QStringLiteral( "fontFamily" ), QVariant( fontFamily ) ).toString();
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
  opts.insert( QStringLiteral( "fontFamily" ), QVariant( fontFamily ) );

  bool gbxCustom = ( mMacStyle );
  opts.insert( QStringLiteral( "groupBoxCustom" ), settings.value( QStringLiteral( "groupBoxCustom" ), QVariant( gbxCustom ) ) );

  settings.endGroup(); // "qgis/stylesheet"

  opts.insert( QStringLiteral( "iconSize" ), settings.value( QStringLiteral( "IconSize" ), QGIS_ICON_SIZE ) );

  return opts;
}

void QgisAppStyleSheet::buildStyleSheet( const QMap<QString, QVariant> &opts )
{
  QString ss;

  // QgisApp-wide font
  QString fontSize = opts.value( QStringLiteral( "fontPointSize" ) ).toString();
  QgsDebugMsg( QString( "fontPointSize: %1" ).arg( fontSize ) );
  if ( fontSize.isEmpty() ) { return; }

  QString fontFamily = opts.value( QStringLiteral( "fontFamily" ) ).toString();
  QgsDebugMsg( QString( "fontFamily: %1" ).arg( fontFamily ) );
  if ( fontFamily.isEmpty() ) { return; }

  ss += QStringLiteral( "* { font: %1pt \"%2\"} " ).arg( fontSize, fontFamily );

  // QGroupBox and QgsCollapsibleGroupBox, mostly for Ubuntu and Mac
  bool gbxCustom = opts.value( QStringLiteral( "groupBoxCustom" ) ).toBool();
  QgsDebugMsg( QString( "groupBoxCustom: %1" ).arg( gbxCustom ) );

  ss += QLatin1String( "QGroupBox{" );
  // doesn't work for QGroupBox::title
  ss += QStringLiteral( "color: rgb(%1,%1,%1);" ).arg( mMacStyle ? 25 : 60 );
  ss += QLatin1String( "font-weight: bold;" );

  if ( gbxCustom )
  {
    ss += QStringLiteral( "background-color: rgba(0,0,0,%1%);" )
          .arg( mWinOS && mStyle.startsWith( QLatin1String( "windows" ) ) ? 0 : 3 );
    ss += QLatin1String( "border: 1px solid rgba(0,0,0,20%);" );
    ss += QLatin1String( "border-radius: 5px;" );
    ss += QLatin1String( "margin-top: 2.5ex;" );
    ss += QStringLiteral( "margin-bottom: %1ex;" ).arg( mMacStyle ? 1.5 : 1 );
  }
  ss += QLatin1String( "} " );
  if ( gbxCustom )
  {
    ss += QLatin1String( "QGroupBox:flat{" );
    ss += QLatin1String( "background-color: rgba(0,0,0,0);" );
    ss += QLatin1String( "border: rgba(0,0,0,0);" );
    ss += QLatin1String( "} " );

    ss += QLatin1String( "QGroupBox::title{" );
    ss += QLatin1String( "subcontrol-origin: margin;" );
    ss += QLatin1String( "subcontrol-position: top left;" );
    ss += QLatin1String( "margin-left: 6px;" );
    if ( !( mWinOS && mStyle.startsWith( QLatin1String( "windows" ) ) ) && !mOxyStyle )
    {
      ss += QLatin1String( "background-color: rgba(0,0,0,0);" );
    }
    ss += QLatin1String( "} " );
  }

  //sidebar style
  QString style = "QListWidget#mOptionsListWidget {"
                  "    background-color: rgb(69, 69, 69, 0);"
                  "    outline: 0;"
                  "}"
                  "QFrame#mOptionsListFrame {"
                  "    background-color: rgb(69, 69, 69, 220);"
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

  // Fix selection color on losing focus (Windows)
  const QPalette palette = qApp->palette();

  ss += QString( "QTableView {"
                 "selection-background-color: %1;"
                 "selection-color: %2;"
                 "}" )
        .arg( palette.highlight().color().name(),
              palette.highlightedText().color().name() );

  QgsDebugMsg( QString( "Stylesheet built: %1" ).arg( ss ) );

  emit appStyleSheetChanged( ss );
}

void QgisAppStyleSheet::saveToSettings( const QMap<QString, QVariant> &opts )
{
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "qgis/stylesheet" ) );

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

  mMotifStyle = mStyle.contains( QLatin1String( "motif" ) ); // motif
  mCdeStyle = mStyle.contains( QLatin1String( "cde" ) ); // cde
  mPlastqStyle = mStyle.contains( QLatin1String( "plastique" ) ); // plastique
  mCleanLkStyle = mStyle.contains( QLatin1String( "cleanlooks" ) ); // cleanlooks
  mGtkStyle = mStyle.contains( QLatin1String( "gtk" ) ); // gtk+
  mWinStyle = mStyle.contains( QLatin1String( "windows" ) ); // windows
  mWinXpStyle = mStyle.contains( QLatin1String( "windowsxp" ) ); // windowsxp
  mWinVistaStyle = mStyle.contains( QLatin1String( "windowsvista" ) ); // windowsvista
  mMacStyle = mStyle.contains( QLatin1String( "macintosh" ) ); // macintosh (aqua)
  mOxyStyle = mStyle.contains( QLatin1String( "oxygen" ) ); // oxygen

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
