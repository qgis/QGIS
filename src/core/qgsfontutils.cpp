/***************************************************************************
    qgsfontutils.h
    ---------------------
    begin                : June 5, 2013
    copyright            : (C) 2013 by Larry Shaffer
    email                : larrys at dakotacarto dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfontutils.h"

#include "qgsapplication.h"
#include "qgslogger.h"

#include <QApplication>
#include <QFile>
#include <QFont>
#include <QFontDatabase>
#include <QFontInfo>
#include <QStringList>


bool QgsFontUtils::fontMatchOnSystem( const QFont& f )
{
  QFontInfo fi = QFontInfo( f );
  return fi.exactMatch();
}

bool QgsFontUtils::fontFamilyOnSystem( const QString& family )
{
  QFont tmpFont = QFont( family );
  // compare just beginning of family string in case 'family [foundry]' differs
  return tmpFont.family().startsWith( family, Qt::CaseInsensitive );
}

bool QgsFontUtils::fontFamilyHasStyle( const QString& family, const QString& style )
{
  QFontDatabase fontDB;
  return ( fontFamilyOnSystem( family ) && fontDB.styles( family ).contains( style ) );
}

bool QgsFontUtils::fontFamilyMatchOnSystem( const QString& family, QString* chosen, bool* match )
{
  QFontDatabase fontDB;
  QStringList fontFamilies = fontDB.families();
  bool found = false;

  QList<QString>::const_iterator it = fontFamilies.constBegin();
  for ( ; it != fontFamilies.constEnd(); ++it )
  {
    // first compare just beginning of 'family [foundry]' string
    if ( it->startsWith( family, Qt::CaseInsensitive ) )
    {
      found = true;
      // keep looking if match info is requested
      if ( match )
      {
        // full 'family [foundry]' strings have to match
        *match = ( *it == family );
        if ( *match )
          break;
      }
      else
      {
        break;
      }
    }
  }

  if ( found )
  {
    if ( chosen )
    {
      // retrieve the family actually assigned by matching algorithm
      QFont f = QFont( family );
      *chosen = f.family();
    }
  }
  else
  {
    if ( chosen )
    {
      *chosen = QString();
    }

    if ( match )
    {
      *match = false;
    }
  }

  return found;
}

bool QgsFontUtils::updateFontViaStyle( QFont& f, const QString& fontstyle, bool fallback )
{
  if ( fontstyle.isEmpty() )
  {
    return false;
  }

  QFontDatabase fontDB;

  if ( !fallback )
  {
    // does the font even have the requested style?
    bool hasstyle = fontFamilyHasStyle( f.family(), fontstyle );
    if ( !hasstyle )
    {
      return false;
    }
  }

  // is the font's style already the same as requested?
  if ( fontstyle == fontDB.styleString( f ) )
  {
    return false;
  }

  QFont appfont = QApplication::font();
  int defaultSize = appfont.pointSize(); // QFontDatabase::font() needs an integer for size

  QFont styledfont;
  bool foundmatch = false;

  // if fontDB.font() fails, it returns the default app font; but, that may be the target style
  styledfont = fontDB.font( f.family(), fontstyle, defaultSize );
  if ( appfont != styledfont || fontstyle != fontDB.styleString( f ) )
  {
    foundmatch = true;
  }

  // default to first found style if requested style is unavailable
  // this helps in the situations where the passed-in font has to have a named style applied
  if ( fallback && !foundmatch )
  {
    QFont testFont = QFont( f );
    testFont.setPointSize( defaultSize );

    // prefer a style that mostly matches the passed-in font
    foreach ( const QString &style, fontDB.styles( f.family() ) )
    {
      styledfont = fontDB.font( f.family(), style, defaultSize );
      styledfont = styledfont.resolve( f );
      if ( testFont.toString() == styledfont.toString() )
      {
        foundmatch = true;
        break;
      }
    }

    // fallback to first style found that works
    if ( !foundmatch )
    {
      foreach ( const QString &style, fontDB.styles( f.family() ) )
      {
        styledfont = fontDB.font( f.family(), style, defaultSize );
        if ( QApplication::font() != styledfont )
        {
          foundmatch = true;
          break;
        }
      }
    }
  }

  // similar to QFont::resolve, but font may already have pixel size set
  // and we want to make sure that's preserved
  if ( foundmatch )
  {
    if ( f.pointSizeF() != -1 )
    {
      styledfont.setPointSizeF( f.pointSizeF() );
    }
    else if ( f.pixelSize() != -1 )
    {
      styledfont.setPixelSize( f.pixelSize() );
    }
    styledfont.setCapitalization( f.capitalization() );
    styledfont.setUnderline( f.underline() );
    styledfont.setStrikeOut( f.strikeOut() );
    styledfont.setWordSpacing( f.wordSpacing() );
    styledfont.setLetterSpacing( QFont::AbsoluteSpacing, f.letterSpacing() );
    f = styledfont;

    return true;
  }

  return false;
}

QString QgsFontUtils::standardTestFontFamily()
{
  return "QGIS Vera Sans";
}

bool QgsFontUtils::loadStandardTestFonts( QStringList loadstyles )
{
  // load standard test font from filesystem or testdata.qrc (for unit tests and general testing)
  bool fontsLoaded = false;

  QString fontFamily = standardTestFontFamily();
  QMap<QString, QString> fontStyles;
  fontStyles.insert( "Roman", "QGIS-Vera/QGIS-Vera.ttf" );
  fontStyles.insert( "Oblique", "QGIS-Vera/QGIS-VeraIt.ttf" );
  fontStyles.insert( "Bold", "QGIS-Vera/QGIS-VeraBd.ttf" );
  fontStyles.insert( "Bold Oblique", "QGIS-Vera/QGIS-VeraBI.ttf" );

  QMap<QString, QString>::const_iterator f = fontStyles.constBegin();
  for ( ; f != fontStyles.constEnd(); ++f )
  {
    QString fontstyle( f.key() );
    QString fontpath( f.value() );
    if ( !( loadstyles.contains( fontstyle ) || loadstyles.contains( "All" ) ) )
    {
      continue;
    }
    QString familyStyle = QString( "%1 %2" ).arg( fontFamily ).arg( fontstyle );

    if ( fontFamilyHasStyle( fontFamily, fontstyle ) )
    {
      fontsLoaded = ( fontsLoaded || false );
      QgsDebugMsg( QString( "Test font '%1' already available" ).arg( familyStyle ) );
    }
    else
    {
      bool loaded = false;
      if ( QgsApplication::isRunningFromBuildDir() )
      {
        // workaround for bugs with Qt 4.8.5 (other versions?) on Mac 10.9, where fonts
        // from qrc resources load but fail to work and default font is substituted [LS]:
        //   https://bugreports.qt-project.org/browse/QTBUG-30917
        //   https://bugreports.qt-project.org/browse/QTBUG-32789
        QString fontPath( QgsApplication::buildSourcePath() + "/tests/testdata/font/" + fontpath );
        int fontID = QFontDatabase::addApplicationFont( fontPath );
        loaded = ( fontID != -1 );
        fontsLoaded = ( fontsLoaded || loaded );
        QgsDebugMsg( QString( "Test font '%1' %2 from filesystem [%3]" )
                     .arg( familyStyle ).arg( loaded ? "loaded" : "FAILED to load" ).arg( fontPath ) );
      }
      else
      {
        QFile fontResource( ":/testdata/font/" + fontpath );
        if ( fontResource.open( QIODevice::ReadOnly ) )
        {
          int fontID = QFontDatabase::addApplicationFontFromData( fontResource.readAll() );
          loaded = ( fontID != -1 );
          fontsLoaded = ( fontsLoaded || loaded );
        }
        QgsDebugMsg( QString( "Test font '%1' %2 from testdata.qrc" )
                     .arg( familyStyle ).arg( loaded ? "loaded" : "FAILED to load" ) );
      }
    }
  }

  return fontsLoaded;
}

QFont QgsFontUtils::getStandardTestFont( const QString& style, int pointsize )
{
  QFontDatabase fontDB;
  if ( ! fontFamilyHasStyle( standardTestFontFamily(), style ) )
  {
    loadStandardTestFonts( QStringList() << style );
  }

  QFont f = fontDB.font( standardTestFontFamily(), style, pointsize );
  // in case above statement fails to set style
  f.setBold( style.contains( "Bold" ) );
  f.setItalic( style.contains( "Oblique" ) );

  return f;
}
