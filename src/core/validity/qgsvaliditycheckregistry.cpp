/***************************************************************************
    qgsvaliditycheckregistry.cpp
    ----------------------------
    begin                : November 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvaliditycheckregistry.h"
#include "qgsfeedback.h"

QgsValidityCheckRegistry::QgsValidityCheckRegistry()
{
}

QgsValidityCheckRegistry::~QgsValidityCheckRegistry()
{
  qDeleteAll( mChecks );
}

QList<const QgsAbstractValidityCheck *> QgsValidityCheckRegistry::checks() const
{
  QList<const QgsAbstractValidityCheck *> results;
  for ( const QgsAbstractValidityCheck *check : mChecks )
  {
    if ( check )
      results.append( check );
  }
  return results;
}

QList<const QgsAbstractValidityCheck *> QgsValidityCheckRegistry::checks( int type ) const
{
  QList< const QgsAbstractValidityCheck * > results;
  for ( const QgsAbstractValidityCheck *check : mChecks )
  {
    if ( check && check->checkType() == type )
      results << check;
  }
  return results;
}

void QgsValidityCheckRegistry::addCheck( QgsAbstractValidityCheck *check )
{
  mChecks.append( check );
}

void QgsValidityCheckRegistry::removeCheck( QgsAbstractValidityCheck *check )
{
  const int index = mChecks.indexOf( check );
  if ( index >= 0 )
    delete mChecks.takeAt( index );
}

QList<QgsValidityCheckResult> QgsValidityCheckRegistry::runChecks( int type, const QgsValidityCheckContext *context, QgsFeedback *feedback ) const
{
  QList<QgsValidityCheckResult> result;
  const std::vector<std::unique_ptr<QgsAbstractValidityCheck> > toCheck = createChecks( type );
  int i = 0;
  for ( const std::unique_ptr< QgsAbstractValidityCheck > &check : toCheck )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    if ( !check->prepareCheck( context, feedback ) )
    {
      if ( feedback )
      {
        feedback->setProgress( static_cast< double >( i ) / toCheck.size() * 100 );
      }
      continue;
    }

    const QList< QgsValidityCheckResult > checkResults = check->runCheck( context, feedback );
    for ( QgsValidityCheckResult checkResult : checkResults )
    {
      checkResult.checkId = check->id();
      result << checkResult;
    }
    i++;
    if ( feedback )
    {
      feedback->setProgress( static_cast< double >( i ) / toCheck.size() * 100 );
    }
  }
  return result;
}

std::vector<std::unique_ptr<QgsAbstractValidityCheck> > QgsValidityCheckRegistry::createChecks( int type ) const
{
  const QList< const QgsAbstractValidityCheck *> toCheck = checks( type );
  std::vector<std::unique_ptr<QgsAbstractValidityCheck> > results;
  results.reserve( toCheck.size() );
  for ( const QgsAbstractValidityCheck *check : toCheck )
  {
    results.emplace_back( std::unique_ptr< QgsAbstractValidityCheck >( check->create() ) );
  }
  return results;
}
