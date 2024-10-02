/***************************************************************************
    qgsstaclink.cpp
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

#include "qgsstaclink.h"

QgsStacLink::QgsStacLink( const QString &href, const QString &relation, const QString &mediaType, const QString &title )
  : mHref( href )
  , mRelation( relation )
  , mMediaType( mediaType )
  , mTitle( title )
{
}

QString QgsStacLink::href() const
{
  return mHref;
}

QString QgsStacLink::relation() const
{
  return mRelation;
}

QString QgsStacLink::title() const
{
  return mTitle;
}

QString QgsStacLink::mediaType() const
{
  return mMediaType;
}
