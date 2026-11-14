/***************************************************************************
                          qgshtmlrendersettings.h
                             -------------------
    begin                : November 2026
    copyright            : (C) 2026 by Nyall Dawson
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

#include "qgshtmlrendersettings.h"

#include <QJsonValue>
#include <QString>

using namespace Qt::StringLiterals;

bool QgsHtmlRenderSettings::isEmpty() const
{
  return mBaseUrl.isEmpty() && mUserStylesheet.isEmpty() && mFeatureJson.isEmpty();
}

QVariantMap QgsHtmlRenderSettings::asMap() const
{
  QVariantMap res;
  if ( !baseUrl().isEmpty() )
  {
    res.insert( u"baseUrl"_s, baseUrl() );
  }
  if ( !userStylesheet().isEmpty() )
  {
    res.insert( u"userStylesheet"_s, userStylesheet() );
  }
  if ( !featureJson().isEmpty() )
  {
    res.insert( u"feature"_s, featureJson() );
  }
  return res;
}
