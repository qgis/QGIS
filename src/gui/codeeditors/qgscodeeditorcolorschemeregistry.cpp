/***************************************************************************
    qgscodeeditorcolorschemeregistry.cpp
     --------------------------------------
    Date                 : October 2020
    Copyright            : (C) 2020 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgscodeeditorcolorschemeregistry.h"

#include <QObject>

QgsCodeEditorColorSchemeRegistry::QgsCodeEditorColorSchemeRegistry()
{
  QgsCodeEditorColorScheme defaultScheme( QStringLiteral( "default" ), QObject::tr( "Default" ) );
  defaultScheme.setColors(
    {
      { QgsCodeEditorColorScheme::ColorRole::Default, QColor( "#4d4d4c" ) },
      { QgsCodeEditorColorScheme::ColorRole::Keyword, QColor( "#8959a8" ) },
      { QgsCodeEditorColorScheme::ColorRole::Class, QColor( "#4271ae" ) },
      { QgsCodeEditorColorScheme::ColorRole::Method, QColor( "#4271ae" ) },
      { QgsCodeEditorColorScheme::ColorRole::Decoration, QColor( "#3e999f" ) },
      { QgsCodeEditorColorScheme::ColorRole::Number, QColor( "#c82829" ) },
      { QgsCodeEditorColorScheme::ColorRole::Comment, QColor( "#8e908c" ) },
      { QgsCodeEditorColorScheme::ColorRole::CommentLine, QColor( "#8e908c" ) },
      { QgsCodeEditorColorScheme::ColorRole::CommentBlock, QColor( "#8e908c" ) },
      { QgsCodeEditorColorScheme::ColorRole::Background, QColor( "#ffffff" ) },
      { QgsCodeEditorColorScheme::ColorRole::Operator, QColor( "#8959a8" ) },
      { QgsCodeEditorColorScheme::ColorRole::QuotedOperator, QColor( "#8959a8" ) },
      { QgsCodeEditorColorScheme::ColorRole::Identifier, QColor( "#4271ae" ) },
      { QgsCodeEditorColorScheme::ColorRole::QuotedIdentifier, QColor( "#4271ae" ) },
      { QgsCodeEditorColorScheme::ColorRole::Tag, QColor( "#4271ae" ) },
      { QgsCodeEditorColorScheme::ColorRole::UnknownTag, QColor( "#4271ae" ) },
      { QgsCodeEditorColorScheme::ColorRole::Cursor, QColor( "#636363" ) },
      { QgsCodeEditorColorScheme::ColorRole::CaretLine, QColor( "#efefef" ) },
      { QgsCodeEditorColorScheme::ColorRole::SingleQuote, QColor( "#718c00" ) },
      { QgsCodeEditorColorScheme::ColorRole::DoubleQuote, QColor( "#718c00" ) },
      { QgsCodeEditorColorScheme::ColorRole::TripleSingleQuote, QColor( "#eab700" ) },
      { QgsCodeEditorColorScheme::ColorRole::TripleDoubleQuote, QColor( "#eab700" ) },
      { QgsCodeEditorColorScheme::ColorRole::MarginBackground, QColor( "#efefef" ) },
      { QgsCodeEditorColorScheme::ColorRole::MarginForeground, QColor( "#636363" ) },
      { QgsCodeEditorColorScheme::ColorRole::SelectionBackground, QColor( "#d7d7d7" ) },
      { QgsCodeEditorColorScheme::ColorRole::SelectionForeground, QColor( "#303030" ) },
      { QgsCodeEditorColorScheme::ColorRole::MatchedBraceBackground, QColor( "#b7f907" ) },
      { QgsCodeEditorColorScheme::ColorRole::MatchedBraceForeground, QColor( "#303030" ) },
      { QgsCodeEditorColorScheme::ColorRole::Edge, QColor( "#efefef" ) },
      { QgsCodeEditorColorScheme::ColorRole::Fold, QColor( "#efefef" ) },
      { QgsCodeEditorColorScheme::ColorRole::Error, QColor( "#e31a1c" ) },
      { QgsCodeEditorColorScheme::ColorRole::ErrorBackground, QColor( "#ffffff" ) },
      { QgsCodeEditorColorScheme::ColorRole::FoldIconForeground, QColor( "#ffffff" ) },
      { QgsCodeEditorColorScheme::ColorRole::FoldIconHalo, QColor( "#000000" ) },
      { QgsCodeEditorColorScheme::ColorRole::IndentationGuide, QColor( "#d5d5d5" ) },
      { QgsCodeEditorColorScheme::ColorRole::SearchMatchBackground, QColor( "#dadada" ) },
    }
  );
  addColorScheme( defaultScheme );

  QgsCodeEditorColorScheme solarizedLight( QStringLiteral( "solarized" ), QObject::tr( "Solarized (Light)" ) );
  solarizedLight.setColors(
    {
      { QgsCodeEditorColorScheme::ColorRole::Default, QColor( "#586E75" ) },
      { QgsCodeEditorColorScheme::ColorRole::Keyword, QColor( "#859900" ) },
      { QgsCodeEditorColorScheme::ColorRole::Class, QColor( "#268BD2" ) },
      { QgsCodeEditorColorScheme::ColorRole::Method, QColor( "#268BD2" ) },
      { QgsCodeEditorColorScheme::ColorRole::Decoration, QColor( "#6C71C4" ) },
      { QgsCodeEditorColorScheme::ColorRole::Number, QColor( "#2AA198" ) },
      { QgsCodeEditorColorScheme::ColorRole::Comment, QColor( "#93A1A1" ) },
      { QgsCodeEditorColorScheme::ColorRole::CommentLine, QColor( "#93A1A1" ) },
      { QgsCodeEditorColorScheme::ColorRole::CommentBlock, QColor( "#3D8080" ) },
      { QgsCodeEditorColorScheme::ColorRole::Background, QColor( "#FDF6E3" ) },
      { QgsCodeEditorColorScheme::ColorRole::Cursor, QColor( "#DC322F" ) },
      { QgsCodeEditorColorScheme::ColorRole::CaretLine, QColor( "#EEE8D5" ) },
      { QgsCodeEditorColorScheme::ColorRole::Operator, QColor( "#586E75" ) },
      { QgsCodeEditorColorScheme::ColorRole::QuotedOperator, QColor( "#586E75" ) },
      { QgsCodeEditorColorScheme::ColorRole::Identifier, QColor( "#586E75" ) },
      { QgsCodeEditorColorScheme::ColorRole::QuotedIdentifier, QColor( "#586E75" ) },
      { QgsCodeEditorColorScheme::ColorRole::Tag, QColor( "#2AA198" ) },
      { QgsCodeEditorColorScheme::ColorRole::UnknownTag, QColor( "#2AA198" ) },
      { QgsCodeEditorColorScheme::ColorRole::SingleQuote, QColor( "#3D8080" ) },
      { QgsCodeEditorColorScheme::ColorRole::DoubleQuote, QColor( "#3D8080" ) },
      { QgsCodeEditorColorScheme::ColorRole::TripleSingleQuote, QColor( "#3D8080" ) },
      { QgsCodeEditorColorScheme::ColorRole::TripleDoubleQuote, QColor( "#3D8080" ) },
      { QgsCodeEditorColorScheme::ColorRole::MarginBackground, QColor( "#EEE8D5" ) },
      { QgsCodeEditorColorScheme::ColorRole::MarginForeground, QColor( "#93A1A1" ) },
      { QgsCodeEditorColorScheme::ColorRole::SelectionBackground, QColor( "#839496" ) },
      { QgsCodeEditorColorScheme::ColorRole::SelectionForeground, QColor( "#FDF6E3" ) },
      { QgsCodeEditorColorScheme::ColorRole::MatchedBraceBackground, QColor( "#CBCCA3" ) },
      { QgsCodeEditorColorScheme::ColorRole::MatchedBraceForeground, QColor( "#586E75" ) },
      { QgsCodeEditorColorScheme::ColorRole::Edge, QColor( "#EEE8D5" ) },
      { QgsCodeEditorColorScheme::ColorRole::Fold, QColor( "#EEE8D5" ) },
      { QgsCodeEditorColorScheme::ColorRole::Error, QColor( "#DC322F" ) },
      { QgsCodeEditorColorScheme::ColorRole::ErrorBackground, QColor( "#ffffff" ) },
      { QgsCodeEditorColorScheme::ColorRole::FoldIconForeground, QColor( "#ffffff" ) },
      { QgsCodeEditorColorScheme::ColorRole::FoldIconHalo, QColor( "#93a1a1" ) },
      { QgsCodeEditorColorScheme::ColorRole::IndentationGuide, QColor( "#c2beb3" ) },
      { QgsCodeEditorColorScheme::ColorRole::SearchMatchBackground, QColor( "#b58900" ) },
    }
  );
  addColorScheme( solarizedLight );

  QgsCodeEditorColorScheme solarizedDark( QStringLiteral( "solarized_dark" ), QObject::tr( "Solarized (Dark)" ) );
  solarizedDark.setColors(
    {
      { QgsCodeEditorColorScheme::ColorRole::Default, QColor( "#839496" ) },
      { QgsCodeEditorColorScheme::ColorRole::Keyword, QColor( "#859900" ) },
      { QgsCodeEditorColorScheme::ColorRole::Class, QColor( "#268BD2" ) },
      { QgsCodeEditorColorScheme::ColorRole::Method, QColor( "#268BD2" ) },
      { QgsCodeEditorColorScheme::ColorRole::Decoration, QColor( "#94558D" ) },
      { QgsCodeEditorColorScheme::ColorRole::Number, QColor( "#2AA198" ) },
      { QgsCodeEditorColorScheme::ColorRole::Comment, QColor( "#2AA198" ) },
      { QgsCodeEditorColorScheme::ColorRole::CommentLine, QColor( "#2AA198" ) },
      { QgsCodeEditorColorScheme::ColorRole::CommentBlock, QColor( "#2AA198" ) },
      { QgsCodeEditorColorScheme::ColorRole::Background, QColor( "#002B36" ) },
      { QgsCodeEditorColorScheme::ColorRole::Cursor, QColor( "#DC322F" ) },
      { QgsCodeEditorColorScheme::ColorRole::CaretLine, QColor( "#073642" ) },
      { QgsCodeEditorColorScheme::ColorRole::Operator, QColor( "#839496" ) },
      { QgsCodeEditorColorScheme::ColorRole::QuotedOperator, QColor( "#839496" ) },
      { QgsCodeEditorColorScheme::ColorRole::Identifier, QColor( "#839496" ) },
      { QgsCodeEditorColorScheme::ColorRole::QuotedIdentifier, QColor( "#839496" ) },
      { QgsCodeEditorColorScheme::ColorRole::Tag, QColor( "#268BD2" ) },
      { QgsCodeEditorColorScheme::ColorRole::UnknownTag, QColor( "#268BD2" ) },
      { QgsCodeEditorColorScheme::ColorRole::SingleQuote, QColor( "#3D8080" ) },
      { QgsCodeEditorColorScheme::ColorRole::DoubleQuote, QColor( "#3D8080" ) },
      { QgsCodeEditorColorScheme::ColorRole::TripleSingleQuote, QColor( "#3D8080" ) },
      { QgsCodeEditorColorScheme::ColorRole::TripleDoubleQuote, QColor( "#3D8080" ) },
      { QgsCodeEditorColorScheme::ColorRole::MarginBackground, QColor( "#073642" ) },
      { QgsCodeEditorColorScheme::ColorRole::MarginForeground, QColor( "#586E75" ) },
      { QgsCodeEditorColorScheme::ColorRole::SelectionBackground, QColor( "#657B83" ) },
      { QgsCodeEditorColorScheme::ColorRole::SelectionForeground, QColor( "#002B36" ) },
      { QgsCodeEditorColorScheme::ColorRole::MatchedBraceBackground, QColor( "#1F4D2C" ) },
      { QgsCodeEditorColorScheme::ColorRole::MatchedBraceForeground, QColor( "#839496" ) },
      { QgsCodeEditorColorScheme::ColorRole::Edge, QColor( "#586E75" ) },
      { QgsCodeEditorColorScheme::ColorRole::Fold, QColor( "#073642" ) },
      { QgsCodeEditorColorScheme::ColorRole::Error, QColor( "#DC322F" ) },
      { QgsCodeEditorColorScheme::ColorRole::ErrorBackground, QColor( "#ffffff" ) },
      { QgsCodeEditorColorScheme::ColorRole::FoldIconForeground, QColor( "#586e75" ) },
      { QgsCodeEditorColorScheme::ColorRole::FoldIconHalo, QColor( "#839496" ) },
      { QgsCodeEditorColorScheme::ColorRole::IndentationGuide, QColor( "#586E75" ) },
      { QgsCodeEditorColorScheme::ColorRole::SearchMatchBackground, QColor( "#075e42" ) },
    }
  );
  addColorScheme( solarizedDark );
}

bool QgsCodeEditorColorSchemeRegistry::addColorScheme( const QgsCodeEditorColorScheme &scheme )
{
  if ( mColorSchemes.contains( scheme.id() ) )
    return false;

  mColorSchemes.insert( scheme.id(), scheme );
  return true;
}

bool QgsCodeEditorColorSchemeRegistry::removeColorScheme( const QString &id )
{
  if ( !mColorSchemes.contains( id ) )
    return false;

  mColorSchemes.remove( id );
  return true;
}

QStringList QgsCodeEditorColorSchemeRegistry::schemes() const
{
  return mColorSchemes.keys();
}

QgsCodeEditorColorScheme QgsCodeEditorColorSchemeRegistry::scheme( const QString &id ) const
{
  if ( !mColorSchemes.contains( id ) )
    return mColorSchemes.value( QStringLiteral( "default" ) );

  return mColorSchemes.value( id );
}
