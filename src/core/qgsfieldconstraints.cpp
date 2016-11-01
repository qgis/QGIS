/***************************************************************************
                     qgsfieldconstraints.cpp
                     -----------------------
               Date                 : November 2016
               Copyright            : (C) 2016 by Nyall Dawson
               email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsfieldconstraints.h"




QgsFieldConstraints::QgsFieldConstraints()
    : mConstraints( 0 )
{}

QgsFieldConstraints::ConstraintOrigin QgsFieldConstraints::constraintOrigin( QgsFieldConstraints::Constraint constraint ) const
{
  if ( !( mConstraints & constraint ) )
    return ConstraintOriginNotSet;

  return mConstraintOrigins.value( constraint, ConstraintOriginNotSet );
}

void QgsFieldConstraints::setConstraint( QgsFieldConstraints::Constraint constraint, QgsFieldConstraints::ConstraintOrigin origin )
{
  if ( origin == ConstraintOriginNotSet )
  {
    mConstraints &= ~constraint;
    mConstraintOrigins.remove( constraint );
  }
  else
  {
    mConstraints |= constraint;
    mConstraintOrigins.insert( constraint, origin );
  }
}

QString QgsFieldConstraints::constraintExpression() const
{
  return mConstraints & QgsFieldConstraints::ConstraintExpression ? mExpressionConstraint : QString();
}

void QgsFieldConstraints::setConstraintExpression( const QString& expression, const QString& description )
{
  if ( expression.isEmpty() )
    mConstraints &= ~QgsFieldConstraints::ConstraintExpression;
  else
  {
    mConstraints |= QgsFieldConstraints::ConstraintExpression;
    mConstraintOrigins.insert( QgsFieldConstraints::ConstraintExpression, QgsFieldConstraints::ConstraintOriginLayer );
  }

  mExpressionConstraint = expression;
  mExpressionConstraintDescription = description;
}

bool QgsFieldConstraints::operator==( const QgsFieldConstraints& other ) const
{
  return mConstraints == other.mConstraints && mConstraintOrigins == other.mConstraintOrigins
         && mExpressionConstraint == other.mExpressionConstraint && mExpressionConstraintDescription == other.mExpressionConstraintDescription;
}
