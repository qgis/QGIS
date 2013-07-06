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

#include <QApplication>
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
  return family.startsWith( tmpFont.family(), Qt::CaseInsensitive );
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
        if ( match ) { break; }
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
    bool hasstyle = false;
    foreach ( const QString &style, fontDB.styles( f.family() ) )
    {
      if ( style == fontstyle )
      {
        hasstyle = true;
        break;
      }
    }

    if ( !hasstyle )
    {
      return false;
    }
  }

  // is the font's style already the same as requested?
  if ( fontstyle == fontDB.styleString( f ) )
  {
    return true;
  }

  int defaultSize = QApplication::font().pointSize(); // QFontDatabase::font() needs an integer for size

  QFont styledfont;
  bool foundmatch = false;

  styledfont = fontDB.font( f.family(), fontstyle, defaultSize );
  if ( QApplication::font() != styledfont )
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
