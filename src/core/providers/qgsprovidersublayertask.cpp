/***************************************************************************
                             qgsprovidersublayertask.cpp
                             ----------------------
    begin                : June 2021
    copyright            : (C) 2021 by Nyall Dawson
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

#include "qgsprovidersublayertask.h"
#include "qgsfeedback.h"
#include "qgsproviderregistry.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsreadwritelocker.h"

QgsProviderSublayerTask::QgsProviderSublayerTask( const QString &uri )
  : QgsTask( tr( "Retrieving layers" ), QgsTask::CanCancel | QgsTask::CancelWithoutPrompt )
  , mUri( uri )
{
}

QList<QgsProviderSublayerDetails> QgsProviderSublayerTask::results() const
{
  QgsReadWriteLocker locker( mLock, QgsReadWriteLocker::Read );
  return mResults;
}

QgsProviderSublayerTask::~QgsProviderSublayerTask() = default;

bool QgsProviderSublayerTask::run()
{
  mFeedback = std::make_unique< QgsFeedback >();

  const QList<QgsProviderSublayerDetails> res = QgsProviderRegistry::instance()->querySublayers( mUri, Qgis::SublayerQueryFlag::ResolveGeometryType | Qgis::SublayerQueryFlag::CountFeatures, mFeedback.get() );

  QgsReadWriteLocker locker( mLock, QgsReadWriteLocker::Write );
  mResults = res;

  return true;
}

void QgsProviderSublayerTask::cancel()
{
  if ( mFeedback )
    mFeedback->cancel();

  QgsTask::cancel();
}
