/***************************************************************************
    qgssimplifymethod.cpp
    ---------------------
    begin                : December 2013
    copyright            : (C) 2013 by Matthias Kuhn / Alvaro Huarte
    email                : 
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssimplifymethod.h"

QgsSimplifyMethod::QgsSimplifyMethod()
  : mMethodType( QgsSimplifyMethod::NoSimplification )
  , mTolerance( 1 )
  , mForceLocalOptimization( true )
{
}

QgsSimplifyMethod::QgsSimplifyMethod( const QgsSimplifyMethod &rh )
{
  operator=( rh );
}

QgsSimplifyMethod& QgsSimplifyMethod::operator=( const QgsSimplifyMethod &rh )
{
  mMethodType = rh.mMethodType;
  mTolerance = rh.mTolerance;
  mForceLocalOptimization = rh.mForceLocalOptimization;

  return *this;
}

void QgsSimplifyMethod::setMethodType( MethodType methodType )
{
  mMethodType = methodType;
}

void QgsSimplifyMethod::setTolerance( double tolerance )
{
  mTolerance = tolerance;
}

void QgsSimplifyMethod::setForceLocalOptimization( bool localOptimization )
{
  mForceLocalOptimization = localOptimization;
}
