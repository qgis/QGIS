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

#include <memory>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgssettings.h"

#include <QApplication>
#include <QFile>
#include <QFont>
#include <QFontDatabase>
#include <QFontInfo>
#include <QMimeData>
#include <QStringList>

bool QgsFontUtils::fontMatchOnSystem( const QFont &f )
{
  const QFontInfo fi = QFontInfo( f );
  return fi.exactMatch();
}

bool QgsFontUtils::fontFamilyOnSystem( const QString &family )
{
  const QFont tmpFont = QFont( family );
  // compare just beginning of family string in case 'family [foundry]' differs
  return tmpFont.family().startsWith( family, Qt::CaseInsensitive );
}

bool QgsFontUtils::fontFamilyHasStyle( const QString &family, const QString &style )
{
  const QFontDatabase fontDB;
  if ( !fontFamilyOnSystem( family ) )
    return false;

  if ( fontDB.styles( family ).contains( style ) )
    return true;

#ifdef Q_OS_WIN
  QString modified( style );
  if ( style == "Roman" )
    modified = "Normal";
  if ( style == "Oblique" )
    modified = "Italic";
  if ( style == "Bold Oblique" )
    modified = "Bold Italic";
  if ( fontDB.styles( family ).contains( modified ) )
    return true;
#endif

  return false;
}

QString QgsFontUtils::resolveFontStyleName( const QFont &font )
{
  auto styleNameIsMatch = [&font]( const QString & candidate ) -> bool
  {
    // confirm that style name matches bold/italic flags
    QFont testFont( font.family() );
    testFont.setStyleName( candidate );
    return testFont.italic() == font.italic() && testFont.weight() == font.weight();
  };

  // attempt 1
  const QFontInfo fontInfo( font );
  QString styleName = fontInfo.styleName();
  if ( !styleName.isEmpty() )
  {
    if ( styleNameIsMatch( styleName ) )
      return styleName;
  }

  // attempt 2
  styleName = QFontDatabase().styleString( font );
  if ( !styleName.isEmpty() )
  {
    if ( styleNameIsMatch( styleName ) )
      return styleName;
  }

  // failed
  return QString();
}

bool QgsFontUtils::fontFamilyMatchOnSystem( const QString &family, QString *chosen, bool *match )
{
  const QFontDatabase fontDB;
  const QStringList fontFamilies = fontDB.families();
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
      const QFont f = QFont( family );
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

bool QgsFontUtils::updateFontViaStyle( QFont &f, const QString &fontstyle, bool fallback )
{
  if ( fontstyle.isEmpty() )
  {
    return false;
  }

  QFontDatabase fontDB;
  QString actualFontStyle = fontstyle;

  if ( !fallback )
  {
    // does the font even have the requested style?
    bool hasstyle = fontFamilyHasStyle( f.family(), actualFontStyle );
    if ( !hasstyle )
    {
      actualFontStyle = untranslateNamedStyle( fontstyle );
      hasstyle = fontFamilyHasStyle( f.family(), actualFontStyle );
    }

    if ( !hasstyle )
    {
      return false;
    }
  }

  // is the font's style already the same as requested?
  if ( actualFontStyle == fontDB.styleString( f ) )
  {
    return false;
  }

  const QFont appfont = QApplication::font();
  const int defaultSize = appfont.pointSize(); // QFontDatabase::font() needs an integer for size

  QFont styledfont;
  bool foundmatch = false;

  // if fontDB.font() fails, it returns the default app font; but, that may be the target style
  styledfont = fontDB.font( f.family(), actualFontStyle, defaultSize );
  if ( appfont != styledfont || actualFontStyle != fontDB.styleString( f ) )
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
    const auto constFamily = fontDB.styles( f.family() );
    for ( const QString &style : constFamily )
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
      for ( const QString &style : constFamily )
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
    if ( !qgsDoubleNear( f.pointSizeF(), -1 ) )
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
  return u"QGIS Vera Sans"_s;
}

bool QgsFontUtils::loadStandardTestFonts( const QStringList &loadstyles )
{
  // load standard test font from filesystem or testdata.qrc (for unit tests and general testing)
  bool fontsLoaded = false;

  QMap<QString, QString> fontStyles;
  fontStyles.insert( u"Roman"_s, u"QGIS-Vera/QGIS-Vera.ttf"_s );
  fontStyles.insert( u"Oblique"_s, u"QGIS-Vera/QGIS-VeraIt.ttf"_s );
  fontStyles.insert( u"Bold"_s, u"QGIS-Vera/QGIS-VeraBd.ttf"_s );
  fontStyles.insert( u"Bold Oblique"_s, u"QGIS-Vera/QGIS-VeraBI.ttf"_s );
  fontStyles.insert( u"Deja Bold"_s, u"QGIS-DejaVu/QGISDejaVuSans-Bold.ttf"_s );

  QMap<QString, QString>::const_iterator f = fontStyles.constBegin();
  for ( ; f != fontStyles.constEnd(); ++f )
  {
    const QString fontpath( f.value() );
    if ( !( loadstyles.contains( f.key() ) || loadstyles.contains( u"All"_s ) ) )
    {
      continue;
    }

    const QString fontFamily = !f.key().startsWith( "Deja"_L1 ) ? standardTestFontFamily() : u"QGIS DejaVu Sans"_s;
    const QString fontstyle  = !f.key().startsWith( "Deja"_L1 ) ?  f.key() : f.key().mid( 5 );

    if ( fontFamilyHasStyle( fontFamily, fontstyle ) )
    {
      QgsDebugMsgLevel( u"Test font '%1 %2' already available"_s.arg( fontFamily, fontstyle ), 2 );
    }
    else
    {
      bool loaded = false;
      if ( QgsApplication::isRunningFromBuildDir() )
      {
        // workaround for bugs with Qt 4.8.5 (other versions?) on Mac 10.9, where fonts
        // from qrc resources load but fail to work and default font is substituted [LS]:
        //   https://bugreports.qt.io/browse/QTBUG-30917
        //   https://bugreports.qt.io/browse/QTBUG-32789
        const QString fontPath( QgsApplication::buildSourcePath() + "/tests/testdata/font/" + fontpath );
        const int fontID = QFontDatabase::addApplicationFont( fontPath );
        loaded = ( fontID != -1 );
        fontsLoaded = ( fontsLoaded || loaded );
        QgsDebugMsgLevel( u"Test font '%1 %2' %3 from filesystem [%4]"_s
                          .arg( fontFamily, fontstyle, loaded ? "loaded" : "FAILED to load", fontPath ), 2 );
        QgsDebugMsgLevel( u"font families in %1: %2"_s.arg( fontID ).arg( QFontDatabase().applicationFontFamilies( fontID ).join( "," ) ), 2 );
      }
      else
      {
        QFile fontResource( ":/testdata/font/" + fontpath );
        if ( fontResource.open( QIODevice::ReadOnly ) )
        {
          const int fontID = QFontDatabase::addApplicationFontFromData( fontResource.readAll() );
          loaded = ( fontID != -1 );
          fontsLoaded = ( fontsLoaded || loaded );
        }
        QgsDebugMsgLevel( u"Test font '%1' (%2) %3 from testdata.qrc"_s
                          .arg( fontFamily, fontstyle, loaded ? "loaded" : "FAILED to load" ), 2 );
      }
    }
  }

  return fontsLoaded;
}

QFont QgsFontUtils::getStandardTestFont( const QString &style, int pointsize )
{
  const QString fontFamily = !style.startsWith( "Deja"_L1 ) ? standardTestFontFamily() : u"QGIS DejaVu Sans"_s;
  const QString fontStyle  = !style.startsWith( "Deja"_L1 ) ?  style : style.mid( 5 );

  if ( ! fontFamilyHasStyle( fontFamily, fontStyle ) )
  {
    loadStandardTestFonts( QStringList() << style );
  }

  const QFontDatabase fontDB;
  QFont f = fontDB.font( fontFamily, fontStyle, pointsize );
#ifdef Q_OS_WIN
  if ( !f.exactMatch() )
  {
    QString modified;
    if ( fontStyle == "Roman" )
      modified = "Normal";
    else if ( fontStyle == "Oblique" )
      modified = "Italic";
    else if ( fontStyle == "Bold Oblique" )
      modified = "Bold Italic";
    if ( !modified.isEmpty() )
      f = fontDB.font( fontFamily, modified, pointsize );
  }
  if ( !f.exactMatch() )
  {
    QgsDebugMsgLevel( u"Inexact font match - consider installing the %1 font."_s.arg( fontFamily ), 2 );
    QgsDebugMsgLevel( u"Requested: %1"_s.arg( f.toString() ), 2 );
    QFontInfo fi( f );
    QgsDebugMsgLevel( u"Replaced:  %1,%2,%3,%4,%5,%6,%7,%8,%9"_s.arg( fi.family() ).arg( fi.pointSizeF() ).arg( fi.pixelSize() ).arg( fi.styleHint() ).arg( fi.weight() ).arg( fi.style() ).arg( fi.underline() ).arg( fi.strikeOut() ).arg( fi.fixedPitch() ), 2 );
  }
#endif
  // in case above statement fails to set style
  f.setBold( fontStyle.contains( "Bold"_L1 ) );
  f.setItalic( fontStyle.contains( "Oblique"_L1 ) || fontStyle.contains( "Italic"_L1 ) );

  return f;
}

QDomElement QgsFontUtils::toXmlElement( const QFont &font, QDomDocument &document, const QString &elementName )
{
  QDomElement fontElem = document.createElement( elementName );
  fontElem.setAttribute( u"description"_s, font.toString() );
  fontElem.setAttribute( u"style"_s, untranslateNamedStyle( font.styleName() ) );
  fontElem.setAttribute( u"bold"_s, font.bold() ? QChar( '1' ) : QChar( '0' ) );
  fontElem.setAttribute( u"italic"_s, font.italic() ? QChar( '1' ) : QChar( '0' ) );
  fontElem.setAttribute( u"underline"_s, font.underline() ? QChar( '1' ) : QChar( '0' ) );
  fontElem.setAttribute( u"strikethrough"_s, font.strikeOut() ? QChar( '1' ) : QChar( '0' ) );
  return fontElem;
}

bool QgsFontUtils::setFromXmlElement( QFont &font, const QDomElement &element )
{
  if ( element.isNull() )
  {
    return false;
  }

  font.fromString( element.attribute( u"description"_s ) );

  if ( element.hasAttribute( u"bold"_s ) && element.attribute( u"bold"_s ) == QChar( '1' ) )
  {
    font.setBold( true );
  }
  if ( element.hasAttribute( u"italic"_s ) )
  {
    font.setItalic( element.attribute( u"italic"_s ) == QChar( '1' ) );
  }
  if ( element.hasAttribute( u"underline"_s ) )
  {
    font.setUnderline( element.attribute( u"underline"_s ) == QChar( '1' ) );
  }
  if ( element.hasAttribute( u"strikethrough"_s ) )
  {
    font.setStrikeOut( element.attribute( u"strikethrough"_s ) == QChar( '1' ) );
  }

  if ( element.hasAttribute( u"style"_s ) )
  {
    ( void )updateFontViaStyle( font, translateNamedStyle( element.attribute( u"style"_s ) ) );
  }

  return true;
}

bool QgsFontUtils::setFromXmlChildNode( QFont &font, const QDomElement &element, const QString &childNode )
{
  if ( element.isNull() )
  {
    return false;
  }

  const QDomNodeList nodeList = element.elementsByTagName( childNode );
  if ( !nodeList.isEmpty() )
  {
    const QDomElement fontElem = nodeList.at( 0 ).toElement();
    return setFromXmlElement( font, fontElem );
  }
  else
  {
    return false;
  }
}

QMimeData *QgsFontUtils::toMimeData( const QFont &font )
{
  std::unique_ptr< QMimeData >mimeData( new QMimeData );

  QDomDocument fontDoc;
  const QDomElement fontElem = toXmlElement( font, fontDoc, u"font"_s );
  fontDoc.appendChild( fontElem );
  mimeData->setText( fontDoc.toString() );

  return mimeData.release();
}

QFont QgsFontUtils::fromMimeData( const QMimeData *data, bool *ok )
{
  QFont font;
  if ( ok )
    *ok = false;

  if ( !data )
    return font;

  const QString text = data->text();
  if ( !text.isEmpty() )
  {
    QDomDocument doc;
    QDomElement elem;

    if ( doc.setContent( text ) )
    {
      elem = doc.documentElement();

      if ( elem.nodeName() != "font"_L1 )
        elem = elem.firstChildElement( u"font"_s );

      if ( setFromXmlElement( font, elem ) )
      {
        if ( ok )
          *ok = true;
      }
      return font;
    }
  }
  return font;
}

static QMap<QString, QString> createTranslatedStyleMap()
{
  QMap<QString, QString> translatedStyleMap;
  const QStringList words = QStringList()
                            << u"Normal"_s
                            << u"Regular"_s
                            << u"Light"_s
                            << u"Bold"_s
                            << u"Black"_s
                            << u"Demi"_s
                            << u"Italic"_s
                            << u"Oblique"_s;
  const auto constWords = words;
  for ( const QString &word : constWords )
  {
    translatedStyleMap.insert( QCoreApplication::translate( "QFontDatabase", qPrintable( word ) ), word );
  }
  return translatedStyleMap;
}

QString QgsFontUtils::translateNamedStyle( const QString &namedStyle )
{
  QStringList words = namedStyle.split( ' ', Qt::SkipEmptyParts );
  for ( int i = 0, n = words.length(); i < n; ++i )
  {
    words[i] = QCoreApplication::translate( "QFontDatabase", words[i].toLocal8Bit().constData() );
  }
  return words.join( ' '_L1 );
}

QString QgsFontUtils::untranslateNamedStyle( const QString &namedStyle )
{
  static const QMap<QString, QString> translatedStyleMap = createTranslatedStyleMap();
  QStringList words = namedStyle.split( ' ', Qt::SkipEmptyParts );

  for ( int i = 0, n = words.length(); i < n; ++i )
  {
    if ( translatedStyleMap.contains( words[i] ) )
    {
      words[i] = translatedStyleMap.value( words[i] );
    }
    else
    {
      QgsDebugMsgLevel( u"Warning: style map does not contain %1"_s.arg( words[i] ), 2 );
    }
  }
  return words.join( ' '_L1 );
}

QString QgsFontUtils::asCSS( const QFont &font, double pointToPixelScale )
{
  QString css = u"font-family: "_s + font.family() + ';';

  //style
  css += "font-style: "_L1;
  switch ( font.style() )
  {
    case QFont::StyleNormal:
      css += "normal"_L1;
      break;
    case QFont::StyleItalic:
      css += "italic"_L1;
      break;
    case QFont::StyleOblique:
      css += "oblique"_L1;
      break;
  }
  css += ';';

  //weight
  int cssWeight = 400;
  switch ( font.weight() )
  {
    case QFont::Light:
      cssWeight = 300;
      break;
    case QFont::Normal:
      cssWeight = 400;
      break;
    case QFont::DemiBold:
      cssWeight = 600;
      break;
    case QFont::Bold:
      cssWeight = 700;
      break;
    case QFont::Black:
      cssWeight = 900;
      break;
    case QFont::Thin:
      cssWeight = 100;
      break;
    case QFont::ExtraLight:
      cssWeight = 200;
      break;
    case QFont::Medium:
      cssWeight = 500;
      break;
    case QFont::ExtraBold:
      cssWeight = 800;
      break;
  }
  css += u"font-weight: %1;"_s.arg( cssWeight );

  //size
  css += u"font-size: %1px;"_s.arg( font.pointSizeF() >= 0 ? font.pointSizeF() * pointToPixelScale : font.pixelSize() );

  return css;
}

void QgsFontUtils::addRecentFontFamily( const QString &family )
{
  if ( family.isEmpty() )
  {
    return;
  }

  QgsSettings settings;
  QStringList recentFamilies = settings.value( u"fonts/recent"_s ).toStringList();

  //remove matching families
  recentFamilies.removeAll( family );

  //then add to start of list
  recentFamilies.prepend( family );

  //trim to 10 fonts
  recentFamilies = recentFamilies.mid( 0, 10 );

  settings.setValue( u"fonts/recent"_s, recentFamilies );
}

QStringList QgsFontUtils::recentFontFamilies()
{
  const QgsSettings settings;
  return settings.value( u"fonts/recent"_s ).toStringList();
}

void QgsFontUtils::setFontFamily( QFont &font, const QString &family )
{
  font.setFamily( family );
  if ( !font.exactMatch() )
  {
    // some Qt versions struggle with fonts with certain unusual characters
    // in their names, eg "ESRI Oil, Gas, & Water". Calling "setFamilies"
    // can workaround these issues... (in some cases!)
    font.setFamilies( { family } );
  }
}

QFont QgsFontUtils::createFont( const QString &family, int pointSize, int weight, bool italic )
{
  QFont font( family, pointSize, weight, italic );
  if ( !font.exactMatch() )
  {
    // some Qt versions struggle with fonts with certain unusual characters
    // in their names, eg "ESRI Oil, Gas, & Water". Calling "setFamilies"
    // can workaround these issues... (in some cases!)
    font.setFamilies( { family } );
  }
  return font;
}
