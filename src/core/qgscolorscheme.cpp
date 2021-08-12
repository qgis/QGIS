/***************************************************************************
                             qgscolorscheme.cpp
                             -------------------
    begin                : July 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscolorscheme.h"
#include "qgscolorschemeregistry.h"

#include "qgsproject.h"
#include "qgssymbollayerutils.h"
#include "qgsapplication.h"
#include "qgssettings.h"

#include <QDir>
#include <QRegularExpression>
#include <QTextStream>

bool QgsColorScheme::setColors( const QgsNamedColorList &colors, const QString &context, const QColor &baseColor )
{
  //base implementation does nothing
  Q_UNUSED( colors )
  Q_UNUSED( context )
  Q_UNUSED( baseColor )
  return false;
}


//
// QgsRecentColorScheme
//

QgsNamedColorList QgsRecentColorScheme::fetchColors( const QString &context, const QColor &baseColor )
{
  Q_UNUSED( context )
  Q_UNUSED( baseColor )

  //fetch recent colors
  const QgsSettings settings;
  const QList< QVariant > recentColorVariants = settings.value( QStringLiteral( "colors/recent" ) ).toList();

  //generate list from recent colors
  QgsNamedColorList colorList;
  const auto constRecentColorVariants = recentColorVariants;
  for ( const QVariant &color : constRecentColorVariants )
  {
    colorList.append( qMakePair( color.value<QColor>(), QgsSymbolLayerUtils::colorToName( color.value<QColor>() ) ) );
  }
  return colorList;
}

QgsRecentColorScheme *QgsRecentColorScheme::clone() const
{
  return new QgsRecentColorScheme();
}

void QgsRecentColorScheme::addRecentColor( const QColor &color )
{
  if ( !color.isValid() )
  {
    return;
  }

  //strip alpha from color
  QColor opaqueColor = color;
  opaqueColor.setAlpha( 255 );

  QgsSettings settings;
  QList< QVariant > recentColorVariants = settings.value( QStringLiteral( "colors/recent" ) ).toList();

  //remove colors by name
  for ( int colorIdx = recentColorVariants.length() - 1; colorIdx >= 0; --colorIdx )
  {
    if ( ( recentColorVariants.at( colorIdx ).value<QColor>() ).name() == opaqueColor.name() )
    {
      recentColorVariants.removeAt( colorIdx );
    }
  }

  //add color
  const QVariant colorVariant = QVariant( opaqueColor );
  recentColorVariants.prepend( colorVariant );

  //trim to 20 colors
  while ( recentColorVariants.count() > 20 )
  {
    recentColorVariants.pop_back();
  }

  settings.setValue( QStringLiteral( "colors/recent" ), recentColorVariants );
}

QColor QgsRecentColorScheme::lastUsedColor()
{
  //fetch recent colors
  const QgsSettings settings;
  const QList< QVariant > recentColorVariants = settings.value( QStringLiteral( "colors/recent" ) ).toList();

  if ( recentColorVariants.isEmpty() )
    return QColor();

  return recentColorVariants.at( 0 ).value<QColor>();
}

QgsNamedColorList QgsCustomColorScheme::fetchColors( const QString &context, const QColor &baseColor )
{
  Q_UNUSED( context )
  Q_UNUSED( baseColor )

  //fetch predefined custom colors
  QgsNamedColorList colorList;
  const QgsSettings settings;

  //check if settings contains custom palette
  if ( !settings.contains( QStringLiteral( "/colors/palettecolors" ) ) )
  {
    //no custom palette, return default colors
    colorList.append( qMakePair( QColor( 0, 0, 0 ), QString() ) );
    colorList.append( qMakePair( QColor( 255, 255, 255 ), QString() ) );
    colorList.append( qMakePair( QColor( 166, 206, 227 ), QString() ) );
    colorList.append( qMakePair( QColor( 31, 120, 180 ), QString() ) );
    colorList.append( qMakePair( QColor( 178, 223, 138 ), QString() ) );
    colorList.append( qMakePair( QColor( 51, 160, 44 ), QString() ) );
    colorList.append( qMakePair( QColor( 251, 154, 153 ), QString() ) );
    colorList.append( qMakePair( QColor( 227, 26, 28 ), QString() ) );
    colorList.append( qMakePair( QColor( 253, 191, 111 ), QString() ) );
    colorList.append( qMakePair( QColor( 255, 127, 0 ), QString() ) );

    return colorList;
  }

  QList< QVariant > customColorVariants = settings.value( QStringLiteral( "colors/palettecolors" ) ).toList();
  const QList< QVariant > customColorLabels = settings.value( QStringLiteral( "colors/palettelabels" ) ).toList();

  //generate list from custom colors
  int colorIndex = 0;
  for ( QList< QVariant >::iterator it = customColorVariants.begin();
        it != customColorVariants.end(); ++it )
  {
    const QColor color = ( *it ).value<QColor>();
    QString label;
    if ( customColorLabels.length() > colorIndex )
    {
      label = customColorLabels.at( colorIndex ).toString();
    }

    colorList.append( qMakePair( color, label ) );
    colorIndex++;
  }

  return colorList;
}

bool QgsCustomColorScheme::setColors( const QgsNamedColorList &colors, const QString &context, const QColor &baseColor )
{
  Q_UNUSED( context )
  Q_UNUSED( baseColor )

  // save colors to settings
  QgsSettings settings;
  QList< QVariant > customColors;
  QList< QVariant > customColorLabels;

  QgsNamedColorList::const_iterator colorIt = colors.constBegin();
  for ( ; colorIt != colors.constEnd(); ++colorIt )
  {
    const QVariant color = ( *colorIt ).first;
    const QVariant label = ( *colorIt ).second;
    customColors.append( color );
    customColorLabels.append( label );
  }
  settings.setValue( QStringLiteral( "colors/palettecolors" ), customColors );
  settings.setValue( QStringLiteral( "colors/palettelabels" ), customColorLabels );
  return true;
}

QgsCustomColorScheme *QgsCustomColorScheme::clone() const
{
  return new QgsCustomColorScheme();
}


QgsNamedColorList QgsProjectColorScheme::fetchColors( const QString &context, const QColor &baseColor )
{
  Q_UNUSED( context )
  Q_UNUSED( baseColor )

  QgsNamedColorList colorList;

  QStringList colorStrings = QgsProject::instance()->readListEntry( QStringLiteral( "Palette" ), QStringLiteral( "/Colors" ) );
  const QStringList colorLabels = QgsProject::instance()->readListEntry( QStringLiteral( "Palette" ), QStringLiteral( "/Labels" ) );

  //generate list from custom colors
  int colorIndex = 0;
  for ( QStringList::iterator it = colorStrings.begin();
        it != colorStrings.end(); ++it )
  {
    const QColor color = QgsSymbolLayerUtils::decodeColor( *it );
    QString label;
    if ( colorLabels.length() > colorIndex )
    {
      label = colorLabels.at( colorIndex );
    }

    colorList.append( qMakePair( color, label ) );
    colorIndex++;
  }

  return colorList;
}

bool QgsProjectColorScheme::setColors( const QgsNamedColorList &colors, const QString &context, const QColor &baseColor )
{
  Q_UNUSED( context )
  Q_UNUSED( baseColor )
  QgsProject::instance()->setProjectColors( colors );
  return true;
}

QgsProjectColorScheme *QgsProjectColorScheme::clone() const
{
  return new QgsProjectColorScheme();
}


//
// QgsGplColorScheme
//

QgsNamedColorList QgsGplColorScheme::fetchColors( const QString &context, const QColor &baseColor )
{
  Q_UNUSED( context )
  Q_UNUSED( baseColor )

  const QString sourceFilePath = gplFilePath();
  if ( sourceFilePath.isEmpty() )
  {
    QgsNamedColorList noColors;
    return noColors;
  }

  bool ok;
  QString name;
  QFile sourceFile( sourceFilePath );
  return QgsSymbolLayerUtils::importColorsFromGpl( sourceFile, ok, name );
}

bool QgsGplColorScheme::setColors( const QgsNamedColorList &colors, const QString &context, const QColor &baseColor )
{
  Q_UNUSED( context )
  Q_UNUSED( baseColor )

  const QString destFilePath = gplFilePath();
  if ( destFilePath.isEmpty() )
  {
    return false;
  }

  QFile destFile( destFilePath );
  if ( QgsSymbolLayerUtils::saveColorsToGpl( destFile, schemeName(), colors ) )
  {
    if ( QgsApplication::colorSchemeRegistry()->randomStyleColorScheme() == this )
    {
      // force a re-generation of the random style color list, since the color list has changed
      QgsApplication::colorSchemeRegistry()->setRandomStyleColorScheme( this );
    }
    return true;
  }
  else
  {
    return false;
  }
}


//
// QgsUserColorScheme
//

QgsUserColorScheme::QgsUserColorScheme( const QString &filename )
  : mFilename( filename )
{
  QFile sourceFile( gplFilePath() );

  //read in name
  if ( sourceFile.open( QIODevice::ReadOnly ) )
  {
    QTextStream in( &sourceFile );

    //find name line
    QString line;
    while ( !in.atEnd() && !line.startsWith( QLatin1String( "Name:" ) ) )
    {
      line = in.readLine();
    }
    if ( !in.atEnd() )
    {
      const QRegularExpression rx( "Name:\\s*(\\S.*)$" );
      const QRegularExpressionMatch match = rx.match( line );
      if ( match.hasMatch() )
      {
        mName = match.captured( 1 );
      }
    }
  }
  if ( mName.isEmpty() )
  {
    mName = mFilename;
  }

  // we consider this scheme writable if the user has permission, OR
  // if it DOESN'T already exist (since new schemes are only created when
  // first written to)
  const QFileInfo sourceFileInfo( gplFilePath() );
  mEditable = !sourceFileInfo.exists() || sourceFileInfo.isWritable();
}

QString QgsUserColorScheme::schemeName() const
{
  return mName;
}

QgsUserColorScheme *QgsUserColorScheme::clone() const
{
  return new QgsUserColorScheme( mFilename );
}

QgsColorScheme::SchemeFlags QgsUserColorScheme::flags() const
{
  QgsColorScheme::SchemeFlags f = QgsGplColorScheme::flags();

  const QgsSettings s;
  const QStringList showInMenuSchemes = s.value( QStringLiteral( "/colors/showInMenuList" ) ).toStringList();

  if ( showInMenuSchemes.contains( mName ) )
  {
    f |= QgsColorScheme::ShowInColorButtonMenu;
  }

  return f;
}

bool QgsUserColorScheme::erase()
{
  const QString filePath = gplFilePath();
  if ( filePath.isEmpty() )
  {
    return false;
  }

  // if file does not exist, nothing to do on the disk, so we can consider erasing done
  if ( ! QFile::exists( filePath ) )
  {
    return true;
  }

  //try to erase gpl file
  return QFile::remove( filePath );
}

void QgsUserColorScheme::setShowSchemeInMenu( bool show )
{
  QgsSettings s;
  QStringList showInMenuSchemes = s.value( QStringLiteral( "/colors/showInMenuList" ) ).toStringList();

  if ( show && !showInMenuSchemes.contains( mName ) )
  {
    showInMenuSchemes << mName;
  }
  else if ( !show && showInMenuSchemes.contains( mName ) )
  {
    showInMenuSchemes.removeAll( mName );
  }

  s.setValue( QStringLiteral( "/colors/showInMenuList" ), showInMenuSchemes );
}

QString QgsUserColorScheme::gplFilePath()
{
  const QString palettesDir = QgsApplication::qgisSettingsDirPath() + "palettes";

  const QDir localDir;
  if ( !localDir.mkpath( palettesDir ) )
  {
    return QString();
  }

  return QDir( palettesDir ).filePath( mFilename );
}
