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

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsguiutils.h"
#include "qgslogger.h"
#include "qgsproxystyle.h"
#include "qgssettings.h"

#include <QFont>
#include <QStyle>

#include "moc_qgisappstylesheet.cpp"

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

  opts.insert( u"toolbarSpacing"_s, settings.value( u"/qgis/stylesheet/toolbarSpacing"_s, QString() ) );

  opts.insert( u"iconSize"_s, settings.value( u"/qgis/toolbarIconSize"_s, QGIS_ICON_SIZE ) );

  return opts;
}

void QgisAppStyleSheet::applyStyleSheet( const QMap<QString, QVariant> &opts )
{
  const QgsSettings settings;
  QString ss;

  if ( mMacStyle )
  {
    ss += "QWidget#QgsTextFormatWidgetBase QTabWidget#mOptionsTab QTabBar::tab,"_L1;
    ss += "QWidget#QgsRendererMeshPropsWidgetBase QTabWidget#mStyleOptionsTab"_L1;
    ss += "QTabBar::tab { width: 1.2em; }"_L1;
  }

  ss += "QGroupBox{ font-weight: 600; }"_L1;

  const QString themeName = settings.value( u"UI/UITheme"_s, "default" ).toString();
  if ( themeName == "default"_L1 || !QgsApplication::uiThemes().contains( themeName ) )
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
                                    "}" )
                      .arg( frameMargin );

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
                             "}" )
               .arg( frameMargin );

    const QString toolbarSpacing = opts.value( u"toolbarSpacing"_s, QString() ).toString();
    if ( !toolbarSpacing.isEmpty() )
    {
      bool ok = false;
      const int toolbarSpacingInt = toolbarSpacing.toInt( &ok );
      if ( ok )
      {
        style += u"QToolBar > QToolButton { padding: %1px; } "_s.arg( toolbarSpacingInt );
      }
    }
    ss += style;

    // Fix selection color on losing focus (Windows)
    const QPalette palette = qApp->palette();

    ss += QString( "QTableView {"
                   "selection-background-color: %1;"
                   "selection-color: %2;"
                   "}" )
            .arg( palette.highlight().color().name(), palette.highlightedText().color().name() );
  }

  QgsDebugMsgLevel( u"Stylesheet built: %1"_s.arg( ss ), 2 );

  emit appStyleSheetChanged( ss );
}

void QgisAppStyleSheet::updateStyleSheet()
{
  applyStyleSheet( defaultOptions() );
}

void QgisAppStyleSheet::setUserFontSize( double size )
{
  QgsSettings settings;
  if ( size == mDefaultFont.pointSizeF() || size < 0 )
  {
    settings.remove( u"/app/fontPointSize"_s );
    mUserFontSize = -1;
  }
  else
  {
    mUserFontSize = size;
    settings.setValue( u"/app/fontPointSize"_s, mUserFontSize );
  }
}

void QgisAppStyleSheet::setUserFontFamily( const QString &family )
{
  QgsSettings settings;
  if ( family == mDefaultFont.family() || family.isEmpty() )
  {
    settings.remove( u"/app/fontFamily"_s );
    mUserFontFamily.clear();
  }
  else
  {
    mUserFontFamily = family;
    settings.setValue( u"/app/fontFamily"_s, mUserFontFamily );
  }
}

void QgisAppStyleSheet::saveToSettings( const QMap<QString, QVariant> &opts )
{
  QgsSettings settings;
  settings.beginGroup( u"qgis/stylesheet"_s );

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
  QgsDebugMsgLevel( u"Style name: %1"_s.arg( mStyle ), 2 );

  mMacStyle = mStyle.contains( "macintosh"_L1 ); // macintosh (aqua)
  mOxyStyle = mStyle.contains( "oxygen"_L1 );    // oxygen

  mDefaultFont = qApp->font(); // save before it is changed in any way

  QgsSettings settings;

  if ( mAndroidOS )
  {
    // TODO: find a better default fontsize maybe using DPI detection or so (from Marco Bernasocchi commit)
    mUserFontSize = 8;
  }
  else
  {
    const double fontSize = settings.value( u"/app/fontPointSize"_s, mDefaultFont.pointSizeF() ).toDouble();
    if ( fontSize != mDefaultFont.pointSizeF() )
    {
      mUserFontSize = fontSize;
    }
    else
    {
      mUserFontSize = -1;
    }
  }

  QString fontFamily = settings.value( u"/app/fontFamily"_s, mDefaultFont.family() ).toString();
  // make sure family exists on system
  if ( fontFamily != mDefaultFont.family() )
  {
    const QFont tempFont( fontFamily );
    if ( tempFont.family() == fontFamily )
    {
      // font exists on system
      mUserFontFamily = fontFamily;
    }
  }

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
