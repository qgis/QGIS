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

#include <QFont>
#include <QStyle>

#include "qgisappstylesheet.h"
#include "qgsapplication.h"
#include "qgisapp.h"
#include "qgsproxystyle.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsguiutils.h"

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
  const QVariant oldFontPointSize = settings.value( QStringLiteral( "fontPointSize" ) );
  const QVariant oldFontFamily = settings.value( QStringLiteral( "fontFamily" ) );

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
  QgsDebugMsgLevel( QStringLiteral( "fontPointSize: %1" ).arg( fontSize ), 2 );
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
    const QFont tempFont( fontFamily );
    if ( tempFont.family() != fontFamily )
    {
      // missing from system, drop back to default
      fontFamily = mDefaultFont.family();
    }
  }
  QgsDebugMsgLevel( QStringLiteral( "fontFamily: %1" ).arg( fontFamily ), 2 );
  opts.insert( QStringLiteral( "fontFamily" ), QVariant( fontFamily ) );

  opts.insert( QStringLiteral( "toolbarSpacing" ), settings.value( QStringLiteral( "toolbarSpacing" ), QString() ) );

  settings.endGroup(); // "qgis/stylesheet"

  opts.insert( QStringLiteral( "iconSize" ), settings.value( QStringLiteral( "/qgis/iconSize" ), QGIS_ICON_SIZE ) );

  return opts;
}

void QgisAppStyleSheet::buildStyleSheet( const QMap<QString, QVariant> &opts )
{
  const QgsSettings settings;
  QString ss;

  // QgisApp-wide font
  const QString fontSize = opts.value( QStringLiteral( "fontPointSize" ) ).toString();
  QgsDebugMsgLevel( QStringLiteral( "fontPointSize: %1" ).arg( fontSize ), 2 );
  if ( fontSize.isEmpty() ) { return; }

  const QString fontFamily = opts.value( QStringLiteral( "fontFamily" ) ).toString();
  QgsDebugMsgLevel( QStringLiteral( "fontFamily: %1" ).arg( fontFamily ), 2 );
  if ( fontFamily.isEmpty() ) { return; }

  const QString defaultSize = QString::number( mDefaultFont.pointSize() );
  const QString defaultFamily = mDefaultFont.family();
  if ( fontSize != defaultSize || fontFamily != defaultFamily )
    ss += QStringLiteral( "* { font: %1pt \"%2\"} " ).arg( fontSize, fontFamily );

#if QT_VERSION < QT_VERSION_CHECK(5, 12, 2)
  // Fix for macOS Qt 5.9+, where close boxes do not show on document mode tab bar tabs
  // See: https://bugreports.qt.io/browse/QTBUG-61092 => fixed in 5.12.2 / 5.14
  //      https://bugreports.qt.io/browse/QTBUG-61742 => fixed in 5.9.2
  // Setting any stylesheet makes the default close button disappear.
  // Specifically setting a custom close button temporarily works around issue.
  if ( mMacStyle )
  {
    ss += QLatin1String( "QTabBar::close-button{ image: url(:/images/themes/default/mIconCloseTab.svg); }" );
    ss += QLatin1String( "QTabBar::close-button:hover{ image: url(:/images/themes/default/mIconCloseTabHover.svg); }" );
  }
#endif
  if ( mMacStyle )
  {
    ss += QLatin1String( "QWidget#QgsTextFormatWidgetBase QTabWidget#mOptionsTab QTabBar::tab," );
    ss += QLatin1String( "QWidget#QgsRendererMeshPropsWidgetBase QTabWidget#mStyleOptionsTab" );
    ss += QLatin1String( "QTabBar::tab { width: 1.2em; }" );
  }

  ss += QLatin1String( "QGroupBox{ font-weight: 600; }" );

  const QString themeName = settings.value( QStringLiteral( "UI/UITheme" ), "default" ).toString();
  if ( themeName == QLatin1String( "default" ) || !QgsApplication::uiThemes().contains( themeName ) )
  {
    //sidebar style
    const int frameMargin = QgsGuiUtils::scaleIconSize( 3 );

    QString style = QStringLiteral( "QListWidget#mOptionsListWidget {"
                                    "    background-color: rgba(69, 69, 69, 0);"
                                    "    outline: 0;"
                                    "}"
                                    "QFrame#mOptionsListFrame {"
                                    "    background-color: rgba(69, 69, 69, 220);"
                                    "}"
                                    "QListWidget#mOptionsListWidget::item {"
                                    "    color: white;"
                                    "    padding: %1px;"
                                    "}"
                                    "QListWidget#mOptionsListWidget::item::selected {"
                                    "    color: palette(window-text);"
                                    "    background-color:palette(window);"
                                    "    padding-right: 0px;"
                                    "}" ).arg( frameMargin );

    style += QStringLiteral( "QTreeView#mOptionsTreeView {"
                             "    background-color: rgba(69, 69, 69, 0);"
                             "    outline: 0;"
                             "}"
                             "QFrame#mOptionsListFrame {"
                             "    background-color: rgba(69, 69, 69, 220);"
                             "}"
                             "QTreeView#mOptionsTreeView::item {"
                             "    color: white;"
                             "    padding: %1px;"
                             "}"
                             "QTreeView#mOptionsTreeView::item::selected, QTreeView#mOptionsTreeView::branch::selected {"
                             "    color: palette(window-text);"
                             "    background-color:palette(window);"
                             "    padding-right: 0px;"
                             "}" ).arg( frameMargin );

    const QString toolbarSpacing = opts.value( QStringLiteral( "toolbarSpacing" ), QString() ).toString();
    if ( !toolbarSpacing.isEmpty() )
    {
      bool ok = false;
      const int toolbarSpacingInt = toolbarSpacing.toInt( &ok );
      if ( ok )
      {
        style += QStringLiteral( "QToolBar > QToolButton { padding: %1px; } " ).arg( toolbarSpacingInt );
      }
    }
    ss += style;

    // Fix selection color on losing focus (Windows)
    const QPalette palette = qApp->palette();

    ss += QString( "QTableView {"
                   "selection-background-color: %1;"
                   "selection-color: %2;"
                   "}" )
          .arg( palette.highlight().color().name(),
                palette.highlightedText().color().name() );

    ss += QLatin1String( "QgsPropertyOverrideButton { background: none; border: 1px solid rgba(0, 0, 0, 0%); } QgsPropertyOverrideButton:focus { border: 1px solid palette(highlight); }" );
#ifdef Q_OS_MACX
    ss += QLatin1String( "QgsPropertyOverrideButton::menu-indicator { width: 5px; }" );
#endif
  }

  QgsDebugMsgLevel( QStringLiteral( "Stylesheet built: %1" ).arg( ss ), 2 );

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
  QgsAppStyle *style = dynamic_cast<QgsAppStyle *>( qApp->style() );
  mStyle = style ? style->baseStyle() : qApp->style()->objectName(); // active style name (lowercase)
  QgsDebugMsgLevel( QStringLiteral( "Style name: %1" ).arg( mStyle ), 2 );

  mMacStyle = mStyle.contains( QLatin1String( "macintosh" ) ); // macintosh (aqua)
  mOxyStyle = mStyle.contains( QLatin1String( "oxygen" ) ); // oxygen

  mDefaultFont = qApp->font(); // save before it is changed in any way

  // platforms, specific
#ifdef Q_OS_WIN
  mWinOS = true;
#else
  mWinOS = false;
#endif
#ifdef ANDROID
  mAndroidOS = true;
#else
  mAndroidOS = false;
#endif

}
