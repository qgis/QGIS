/***************************************************************************
    qgsauthmethodmetadata.cpp
    ---------------------
    begin                : September 1, 2015
    copyright            : (C) 2015 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/


#include "qgsauthmethodmetadata.h"


QgsAuthMethodMetadata::QgsAuthMethodMetadata( QString const &_key,
    QString const &_description,
    QString const &_library )
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

