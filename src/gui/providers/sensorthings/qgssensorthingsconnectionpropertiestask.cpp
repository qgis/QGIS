/***************************************************************************
    qgssensorthingsconnectionpropertiestask.cpp
    ---------------------
    Date                 : February 2024
    Copyright            : (C) 2024 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssensorthingsconnectionpropertiestask.h"
#include "moc_qgssensorthingsconnectionpropertiestask.cpp"
#include "qgsfeedback.h"
#include "qgssensorthingsutils.h"

///@cond PRIVATE

QgsSensorThingsConnectionPropertiesTask::QgsSensorThingsConnectionPropertiesTask( const QString &uri, Qgis::SensorThingsEntity entity )
  : QgsTask( tr( "Querying SensorThings connection" ), QgsTask::CanCancel | QgsTask::CancelWithoutPrompt | QgsTask::Silent )
  , mUri( uri )
  , mEntity( entity )
{
}

bool QgsSensorThingsConnectionPropertiesTask::run()
{
  mFeedback = std::make_unique<QgsFeedback>();
  mGeometryTypes = QgsSensorThingsUtils::availableGeometryTypes( mUri, mEntity, mFeedback.get() );
  return !mFeedback->isCanceled();
}

void QgsSensorThingsConnectionPropertiesTask::cancel()
{
  if ( mFeedback )
    mFeedback->cancel();
}

///@endcond PRIVATE
