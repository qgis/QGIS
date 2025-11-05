/***************************************************************************
    blanksegmentutils_p.cpp
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

#include "qgsblanksegmentutils_p.h"
#include "qgsrendercontext.h"

QList<QList<QgsBlankSegmentUtils::BlankSegments>> QgsBlankSegmentUtils::parseBlankSegments( const QString &strBlankSegments, const QgsRenderContext &renderContext, Qgis::RenderUnit unit, QString &error )
{
  QString currentNumber;
  QList<QList<BlankSegments>> blankSegments;

  constexpr QStringView internalError = u"Internal error while processing blank segments";

  auto appendLevel = [&blankSegments, &internalError]( int level ) -> QString
  {

    if ( level == 0 )
    {
      blankSegments.append( QList<BlankSegments>() );
    }
    else if ( level == 1 )
    {
      if ( blankSegments.isEmpty() )
        return internalError.toString() ; // should not happen

      blankSegments.back().append( BlankSegments() );
    }
    else if ( level == 2 )
    {
      if ( blankSegments.isEmpty() || blankSegments.back().isEmpty() )
        return internalError.toString(); // should not happen

      blankSegments.back().back().append( QPair<double, double>( -1, -1 ) );
    }
    else
      return internalError.toString(); // should not happen

    return QString();
  };


  auto addNumber = [&blankSegments, &internalError, &currentNumber]( const QChar & c ) -> QString
  {
    if ( blankSegments.isEmpty() || blankSegments.back().isEmpty() || blankSegments.back().back().isEmpty() )
    {
      return internalError.toString(); // should not happen
    }
    else if ( ( c == ')' || c == ',' ) && blankSegments.back().back().back().first == -1 )
    {
      return QStringLiteral( "Missing number" );
    }
    else if ( blankSegments.back().back().back().second != -1 )
    {
      return QStringLiteral( "Too many number" );
    }
    else
    {
      bool ok;
      const double number = currentNumber.toDouble( &ok );
      if ( !ok )
      {
        return QStringLiteral( "bad formatted number '%1'" ).arg( currentNumber );
      }
      else
      {
        BlankSegments &segments = blankSegments.back().back();
        if ( segments.back().first == -1 )
        {
          if ( segments.count() > 1 && segments.at( segments.count() - 2 ).second > number )
          {
            return QStringLiteral( "Wrong blank segments distances, start (%1) < previous end (%2)" ).arg( number ).arg( segments.at( segments.count() - 2 ).second );
          }
          blankSegments.back().back().back().first = number;
        }
        else if ( blankSegments.back().back().back().first > number )
        {
          return QStringLiteral( "Wrong blank segments distances, start (%1) > end (%2)" ).arg( blankSegments.back().back().back().first ).arg( number );
        }
        else
        {
          blankSegments.back().back().back().second = number;
        }
        currentNumber.clear();
      }
    }

    return QString();
  };

  int level = -1;
  int iChar = 0;
  for ( const QChar &c : strBlankSegments )
  {
    if ( !currentNumber.isEmpty() && ( c.isSpace() || c == ')' || c == "," ) )
    {
      if ( level < 2 )
      {
        error = QStringLiteral( "Missing '('" );
      }
      else
      {
        error = addNumber( c );
      }
    }

    if ( !error.isEmpty() )
    {
      break;
    }

    if ( c == '(' )
    {
      if ( level >= 2 )
      {
        error = QStringLiteral( "Extraneous '('" );
      }
      else
      {
        error = appendLevel( ++level );
      }
    }
    else if ( c == ')' )
    {
      if ( level < 0 )
      {
        error = QStringLiteral( "Extraneous ')'" );
      }
      else
      {
        if ( level == 2 && !blankSegments.isEmpty() && !blankSegments.back().isEmpty() && blankSegments.back().back().count() == 1
             && blankSegments.back().back().back() == QPair<double, double>( -1, -1 ) )
        {
          blankSegments.back().back().pop_back();
        }
        level--;
      }
    }
    else if ( c == ',' )
    {
      if ( ( level == 0 && blankSegments.count() == 0 )
           || ( level == 1 && !blankSegments.isEmpty() && blankSegments.back().count() == 0 )
           || ( level == 2 && !blankSegments.isEmpty() && !blankSegments.back().isEmpty() &&  blankSegments.back().back().count() == 0 ) )
      {
        error = QStringLiteral( "No elements, Not expecting ','" );
      }
      else
      {
        error = appendLevel( level );
      }
    }
    else if ( c.isNumber() || c == '.' )
    {
      currentNumber.append( c );
    }
    else if ( !c.isSpace() )
    {
      error = QStringLiteral( "Invalid character '%1'" ).arg( c );
    }

    if ( !error.isEmpty() )
    {
      break;
    }

    iChar++;
  }


  if ( error.isEmpty() && level != -1 )
  {
    error = "Missing ')'";
  }

  if ( !error.isEmpty() )
  {
    blankSegments.clear();
    error += QStringLiteral( " (column: %1)" ).arg( iChar );
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
