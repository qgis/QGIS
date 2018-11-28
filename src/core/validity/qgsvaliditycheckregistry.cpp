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

QList<QgsAbstractValidityCheck *> QgsValidityCheckRegistry::checks() const
{
  QList<QgsAbstractValidityCheck *> results;
  for ( const QPointer< QgsAbstractValidityCheck > &check : mChecks )
  {
    if ( check )
      results.append( check.data() );
  }
  return results;
}

QList<QgsAbstractValidityCheck *> QgsValidityCheckRegistry::checks( int type ) const
{
  QList< QgsAbstractValidityCheck * > results;
  for ( const QPointer< QgsAbstractValidityCheck > &check : mChecks )
  {
    if ( check && check->checkType() == type )
      results << check.data();
  }
  return results;
}

void QgsValidityCheckRegistry::addCheck( QgsAbstractValidityCheck *check )
{
  mChecks.append( check );
}

void QgsValidityCheckRegistry::removeCheck( QgsAbstractValidityCheck *check )
{
  int index = mChecks.indexOf( check );
  if ( index >= 0 )
    delete mChecks.takeAt( index );
}

QList<QgsValidityCheckResult> QgsValidityCheckRegistry::runChecks( int type, const QgsValidityCheckContext *context, QgsFeedback *feedback ) const
{
  QList<QgsValidityCheckResult> result;
  const QList<QgsAbstractValidityCheck *> toCheck = checks( type );
  int i = 0;
  for ( QgsAbstractValidityCheck *check : toCheck )
  {
    const QList< QgsValidityCheckResult > checkResults = check->runCheck( context, feedback );
    for ( QgsValidityCheckResult checkResult : checkResults )
    {
      checkResult.checkId = check->id();
      result << checkResult;
    }
    i++;
    if ( feedback )
    {
      if ( feedback->isCanceled() )
        break;

      feedback->setProgress( static_cast< double >( i ) / toCheck.count() * 100 );
    }
  }
  return result;
}
