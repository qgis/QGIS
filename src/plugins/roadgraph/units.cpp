/***************************************************************************
 *   Copyright (C) 2009 by Sergey Yakushev                                 *
 *   yakushevs@list.ru                                                     *
 *                                                                         *
 *   This is file implements Units classes                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

/**
 * \file units.cpp
 * \brief implementation of VRP plugins utils
 */

#include "units.h"


Unit::Unit()
{
  mMultipler = 1.0;
}

Unit::Unit( const QString& name, double multipler )
    : mName( name )
    , mMultipler( multipler )
{
}

QString Unit::name() const
{
  return mName;
}

double Unit::multipler() const
{
  return mMultipler;
}
Unit Unit::byName( const QString& name )
{
  if ( name == QLatin1String( "h" ) )
    return Unit( name, 60*60 );
  else if ( name == QLatin1String( "km" ) )
    return Unit( name, 1000 );
  else if ( name == QLatin1String( "s" ) )
    return Unit( name, 1 );
  else if ( name == QLatin1String( "m" ) )
    return Unit( name, 1 );
  return Unit();
}

SpeedUnit::SpeedUnit()
    : mTimeUnit( QLatin1String( "" ), 1 )
    , mDistanceUnit( QLatin1String( "" ), 1 )
{

}

SpeedUnit::SpeedUnit( const Unit& distanceUnit, const Unit& timeUnit )
    : mTimeUnit( timeUnit )
    , mDistanceUnit( distanceUnit )
{
}

QString SpeedUnit::name() const
{
  if ( mDistanceUnit.name().isNull() || mTimeUnit.name().isNull() )
    return QString();
  return mDistanceUnit.name() + QStringLiteral( "/" ) + mTimeUnit.name();
}

SpeedUnit SpeedUnit::byName( const QString& name )
{
  if ( name == QLatin1String( "km/h" ) )
    return SpeedUnit( Unit::byName( QStringLiteral( "km" ) ), Unit::byName( QStringLiteral( "h" ) ) );
  else if ( name == QLatin1String( "m/s" ) )
    return SpeedUnit( Unit::byName( QStringLiteral( "m" ) ), Unit::byName( QStringLiteral( "s" ) ) );
  return SpeedUnit();
}

double SpeedUnit::multipler() const
{
  return mDistanceUnit.multipler() / mTimeUnit.multipler();
}

Unit SpeedUnit::timeUnit() const
{
  return mTimeUnit;
}

Unit SpeedUnit::distanceUnit() const
{
  return mDistanceUnit;
}
