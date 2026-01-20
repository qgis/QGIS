/***************************************************************************
    extraitemutils.cpp
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

#include "qgsextraitemutils.h"

#include <QString>

using namespace Qt::StringLiterals;

QgsExtraItemUtils::ExtraItems QgsExtraItemUtils::parseExtraItems( const QString &strExtraItems, QString &error )
{
  QString currentNumber;
  QList<std::tuple<double, double, double>> extraItems;
  int nbNumbers = 0;

  auto addNumber = [&extraItems, &currentNumber, &nbNumbers]( const QChar & c ) -> QString
  {
    if ( c == ',' && nbNumbers == 0 )
    {
      return u"Missing number"_s;
    }
    else if ( nbNumbers >= 3 )
    {
      return u"Too many number"_s;
    }
    else
    {
      bool ok;
      const double number = currentNumber.toDouble( &ok );
      if ( !ok )
      {
        return u"bad formatted number '%1'"_s.arg( currentNumber );
      }
      else
      {
        switch ( nbNumbers )
        {
          case 0:
            extraItems.append( { number, 0, 0 } );
            break;

          case 1:
            std::get<1>( extraItems.back() ) = number;
            break;

          case 2:
            if ( number < 0 || number > 360 )
            {
              return u"Angle must be between 0° and 360°"_s;
            }

            std::get<2>( extraItems.back() ) = number;
            break;

          default:
            return u"Too many number"_s;
        }

        nbNumbers++;
        currentNumber.clear();
      }
    }

    return QString();
  };

  int iChar = 0;
  for ( const QChar &c : strExtraItems )
  {
    if ( !currentNumber.isEmpty() && ( c.isSpace() || c == "," ) )
    {
      error = addNumber( c );
    }

    if ( !error.isEmpty() )
    {
      break;
    }

    if ( c == ',' )
    {
      if ( extraItems.count() == 0 )
      {
        error = u"No elements, Not expecting ','"_s;
      }
      nbNumbers = 0;
    }
    else if ( c.isNumber() || c == '.' || c == '-' )
    {
      currentNumber.append( c );
    }
    else if ( !c.isSpace() )
    {
      error = u"Invalid character '%1'"_s.arg( c );
    }

    if ( !error.isEmpty() )
    {
      break;
    }

    iChar++;
  }

  // This is the end but there may have still a number to add
  if ( error.isEmpty() && !currentNumber.isEmpty() )
  {
    error = addNumber( ' ' );
  }

  if ( error.isEmpty() && nbNumbers != 0 && nbNumbers != 3 )
  {
    error = u"Missing number"_s;
  }

  if ( !error.isEmpty() )
  {
    extraItems.clear();
    error += u" (column: %1)"_s.arg( iChar );
  }

  return extraItems;
}
