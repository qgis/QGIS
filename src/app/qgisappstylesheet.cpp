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
#include "moc_qgisappstylesheet.cpp"
#include "qgsapplication.h"
#include "qgisapp.h"
#include "qgsproxystyle.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsguiutils.h"

bool QgisAppStyleSheet::sIsFirstRun = true;

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

  opts.insert( QStringLiteral( "toolbarSpacing" ), settings.value( QStringLiteral( "/qgis/stylesheet/toolbarSpacing" ), QString() ) );

  opts.insert( QStringLiteral( "iconSize" ), settings.value( QStringLiteral( "/qgis/toolbarIconSize" ), QGIS_ICON_SIZE ) );

  return opts;
}

void QgisAppStyleSheet::applyStyleSheet( const QMap<QString, QVariant> &opts )
{
  const QgsSettings settings;
  QString ss;

  // QgisApp-wide font
  {
    bool overriddenFontSize = false;
    double currentFontSize = fontSize();
    const QFont appFont = QApplication::font();
    if ( opts.contains( QStringLiteral( "fontPointSize" ) ) )
    {
      const double fontSizeFromOpts = opts.value( QStringLiteral( "fontPointSize" ) ).toDouble();
      currentFontSize = fontSizeFromOpts;
    }
    QgsDebugMsgLevel( QStringLiteral( "fontPointSize: %1" ).arg( currentFontSize ), 2 );
    if ( currentFontSize != appFont.pointSizeF() )
    {
      overriddenFontSize = true;
    }

    bool overriddenFontFamily = false;
    QString currentFontFamily = fontFamily();
    if ( opts.contains( QStringLiteral( "fontFamily" ) ) )
    {
      currentFontFamily = opts.value( QStringLiteral( "fontFamily" ) ).toString();
    }
    QgsDebugMsgLevel( QStringLiteral( "fontFamily: %1" ).arg( currentFontFamily ), 2 );
    if ( !currentFontFamily.isEmpty() && currentFontFamily != appFont.family() )
    {
      overriddenFontFamily = true;
    }

    if ( overriddenFontFamily || overriddenFontSize )
    {
      // this seems only safe to do at startup, at least on Windows.
      // see https://github.com/qgis/QGIS/issues/54402, https://github.com/qgis/QGIS/issues/54295
      // Let's play it safe and require a restart to change the font.
      if ( sIsFirstRun )
      {
        QFont font = QApplication::font();
        if ( overriddenFontFamily )
          font.setFamily( currentFontFamily );
        if ( overriddenFontSize )
          font.setPointSizeF( currentFontSize );
        QApplication::setFont( font );
      }
    }
  }

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
            .arg( palette.highlight().color().name(), palette.highlightedText().color().name() );
  }

  QgsDebugMsgLevel( QStringLiteral( "Stylesheet built: %1" ).arg( ss ), 2 );

  sIsFirstRun = false;

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
    settings.remove( QStringLiteral( "/app/fontPointSize" ) );
    mUserFontSize = -1;
  }
  else
  {
    mUserFontSize = size;
    settings.setValue( QStringLiteral( "/app/fontPointSize" ), mUserFontSize );
  }
}

void QgisAppStyleSheet::setUserFontFamily( const QString &family )
{
  QgsSettings settings;
  if ( family == mDefaultFont.family() || family.isEmpty() )
  {
    settings.remove( QStringLiteral( "/app/fontFamily" ) );
    mUserFontFamily.clear();
  }
  else
  {
    mUserFontFamily = family;
    settings.setValue( QStringLiteral( "/app/fontFamily" ), mUserFontFamily );
  }
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
  mOxyStyle = mStyle.contains( QLatin1String( "oxygen" ) );    // oxygen

  mDefaultFont = qApp->font(); // save before it is changed in any way

  QgsSettings settings;

  if ( mAndroidOS )
  {
    // TODO: find a better default fontsize maybe using DPI detection or so (from Marco Bernasocchi commit)
    mUserFontSize = 8;
  }
  else
  {
    const double fontSize = settings.value( QStringLiteral( "/app/fontPointSize" ), mDefaultFont.pointSizeF() ).toDouble();
    if ( fontSize != mDefaultFont.pointSizeF() )
    {
      mUserFontSize = fontSize;
    }
    else
    {
      mUserFontSize = -1;
    }
  }

  QString fontFamily = settings.value( QStringLiteral( "/app/fontFamily" ), mDefaultFont.family() ).toString();
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
