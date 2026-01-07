/***************************************************************************
    blanksegmentutils.cpp
    ---------------------
    begin                : 2025/11/05
    copyright            : (C) 2025 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsblanksegmentutils.h"

#include "qgsrendercontext.h"

QList<QList<QgsBlankSegmentUtils::BlankSegments>> QgsBlankSegmentUtils::parseBlankSegments( const QString &strBlankSegments, const QgsRenderContext &renderContext, Qgis::RenderUnit unit, QString &error )
{
  QString currentNumber;
  QList<QList<BlankSegments>> blankSegments;
  constexpr char internalError[] = "Internal error while processing blank segments";

  auto appendLevel = [&blankSegments, &internalError]( int level ) -> void
  {
    if ( level == 0 )
    {
      blankSegments.append( QList<BlankSegments>() );
    }
    else if ( level == 1 )
    {
      if ( blankSegments.isEmpty() )
        throw std::runtime_error( internalError ); // should not happen
      blankSegments.back().append( BlankSegments() );
    }
    else if ( level == 2 )
    {
      if ( blankSegments.isEmpty() || blankSegments.back().isEmpty() )
        throw std::runtime_error( internalError ); // should not happen
      blankSegments.back().back().append( QPair<double, double>( -1, -1 ) );
    }
    else
      throw std::runtime_error( internalError ); // should not happen
  };

  auto addNumber = [&blankSegments, &internalError, &currentNumber]( const QChar & c ) -> void
  {
    if ( blankSegments.isEmpty() || blankSegments.back().isEmpty() || blankSegments.back().back().isEmpty() )
    {
      throw std::runtime_error( internalError ); // should not happen
    }

    if ( ( c == ')' || c == ',' ) && blankSegments.back().back().back().first == -1 )
    {
      throw std::runtime_error( "Missing number" );
    }

    if ( blankSegments.back().back().back().second != -1 )
    {
      throw std::runtime_error( "Too many number" );
    }

    bool ok;
    const double number = currentNumber.toDouble( &ok );
    if ( !ok )
    {
      throw std::runtime_error( u"bad formatted number '%1'"_s.arg( currentNumber ).toStdString() );
    }

    BlankSegments &segments = blankSegments.back().back();
    if ( segments.back().first == -1 )
    {
      if ( segments.count() > 1 && segments.at( segments.count() - 2 ).second > number )
        throw std::runtime_error( u"Wrong blank segments distances, start (%1) < previous end (%2)"_s.arg( number ).arg( segments.at( segments.count() - 2 ).second ).toStdString() );

      blankSegments.back().back().back().first = number;
    }
    else if ( blankSegments.back().back().back().first > number )
    {
      throw std::runtime_error( u"Wrong blank segments distances, start (%1) > end (%2)"_s.arg( blankSegments.back().back().back().first ).arg( number ).toStdString() );
    }
    else
    {
      blankSegments.back().back().back().second = number;
    }
    currentNumber.clear();
  };

  int level = -1;
  int iChar = 0;
  try
  {
    for ( const QChar &c : strBlankSegments )
    {
      if ( !currentNumber.isEmpty() && ( c.isSpace() || c == ')' || c == ',' ) )
      {
        if ( level < 2 )
          throw std::runtime_error( "Missing '('" );
        addNumber( c );
      }

      if ( c == '(' )
      {
        if ( level >= 2 )
          throw std::runtime_error( "Extraneous '('" );
        appendLevel( ++level );
      }
      else if ( c == ')' )
      {
        if ( level < 0 )
          throw std::runtime_error( "Extraneous ')'" );
        if ( level == 2 && !blankSegments.isEmpty() && !blankSegments.back().isEmpty() && blankSegments.back().back().count() == 1
             && blankSegments.back().back().back() == QPair<double, double>( -1, -1 ) )
        {
          blankSegments.back().back().pop_back();
        }
        level--;

      }
      else if ( c == ',' )
      {
        if ( ( level == 0 && blankSegments.count() == 0 )
             || ( level == 1 && !blankSegments.isEmpty() && blankSegments.back().count() == 0 )
             || ( level == 2 && !blankSegments.isEmpty() && !blankSegments.back().isEmpty() &&  blankSegments.back().back().count() == 0 ) )
          throw std::runtime_error( "No elements, Not expecting ','" );

        appendLevel( level );
      }
      else if ( c.isNumber() || c == '.' )
      {
        currentNumber.append( c );
      }
      else if ( !c.isSpace() )
      {
        throw std::runtime_error( u"Invalid character '%1'"_s.arg( c ).toStdString() );
      }
      iChar++;
    }
    if ( level != -1 )
    {
      throw std::runtime_error( "Missing ')'" );
    }
  }
  catch ( const std::exception &e )
  {
    blankSegments.clear();
    error = u"%1 (column: %2)"_s.arg( e.what() ).arg( iChar );
  }

  // convert in pixels
  std::for_each( blankSegments.begin(), blankSegments.end(), [&renderContext, &unit]( QList<BlankSegments> &rings )
  {
    std::for_each( rings.begin(), rings.end(), [&renderContext, &unit]( BlankSegments & blankSegments )
    {
      std::for_each( blankSegments.begin(), blankSegments.end(), [&renderContext, &unit]( QPair<double, double> &blankSegment )
      {
        blankSegment.first = renderContext.convertToPainterUnits( blankSegment.first, unit );
        blankSegment.second = renderContext.convertToPainterUnits( blankSegment.second, unit );
      } );
    } );
  } );

  return blankSegments;
}
