/***************************************************************************
    qgsstacasset.cpp
    ---------------------
    begin                : October 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstacasset.h"

QgsStacAsset::QgsStacAsset( const QString &href,
                            const QString &title,
                            const QString &description,
                            const QString &mediaType,
                            const QStringList &roles )
  : mHref( href )
  , mTitle( title )
  , mDescription( description )
  , mMediaType( mediaType )
  , mRoles( roles )
{
}

QString QgsStacAsset::href() const
{
  return mHref;
}

QString QgsStacAsset::title() const
{
  return mTitle;
}

QString QgsStacAsset::description() const
{
  return mDescription;
}

QString QgsStacAsset::mediaType() const
{
  return mMediaType;
}

QStringList QgsStacAsset::roles() const
{
  return mRoles;
}
