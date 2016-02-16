/***************************************************************************
    qgsauthmethodmetadata.cpp
    ---------------------
    begin                : September 1, 2015
    copyright            : (C) 2015 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsauthmethodmetadata.h"


QgsAuthMethodMetadata::QgsAuthMethodMetadata( QString const & _key,
    QString const & _description,
    QString const & _library )
    : key_( _key )
    , description_( _description )
    , library_( _library )
{}

QString QgsAuthMethodMetadata::key() const
{
  return key_;
}

QString QgsAuthMethodMetadata::description() const
{
  return description_;
}

QString QgsAuthMethodMetadata::library() const
{
  return library_;
}

