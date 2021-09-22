/***************************************************************************
                             qgslayermetadatavalidator.cpp
                             -----------------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgslayermetadatavalidator.h"
#include "qgslayermetadata.h"
#include "qgsprojectmetadata.h"

//
// QgsNativeMetadataBaseValidator
//

bool QgsNativeMetadataBaseValidator::validate( const QgsAbstractMetadataBase *metadata, QList<QgsAbstractMetadataBaseValidator::ValidationResult> &results ) const
{
  results.clear();
  if ( !metadata )
    return false;

  int index = 0;
  bool result = true;
  if ( metadata->identifier().isEmpty() )
  {
    result = false;
    results << ValidationResult( QObject::tr( "identifier" ), QObject::tr( "Identifier element is required." ) );
  }

  if ( metadata->language().isEmpty() )
  {
    result = false;
    results << ValidationResult( QObject::tr( "language" ), QObject::tr( "Language element is required." ) );
  }

  if ( metadata->type().isEmpty() )
  {
    result = false;
    results << ValidationResult( QObject::tr( "type" ), QObject::tr( "Type element is required." ) );
  }

  if ( metadata->title().isEmpty() )
  {
    result = false;
    results << ValidationResult( QObject::tr( "title" ), QObject::tr( "Title element is required." ) );
  }

  if ( metadata->abstract().isEmpty() )
  {
    result = false;
    results << ValidationResult( QObject::tr( "abstract" ), QObject::tr( "Abstract element is required." ) );
  }

  if ( metadata->contacts().isEmpty() )
  {
    result = false;
    results << ValidationResult( QObject::tr( "contacts" ), QObject::tr( "At least one contact is required." ) );
  }

  if ( metadata->links().isEmpty() )
  {
    result = false;
    results << ValidationResult( QObject::tr( "links" ), QObject::tr( "At least one link is required." ) );
  }

  // validate keywords
  const QgsAbstractMetadataBase::KeywordMap keywords = metadata->keywords();
  QgsAbstractMetadataBase::KeywordMap::const_iterator keywordIt = keywords.constBegin();
  index = 0;
  for ( ; keywordIt != keywords.constEnd(); ++keywordIt )
  {
    if ( keywordIt.key().isEmpty() )
    {
      result = false;
      results << ValidationResult( QObject::tr( "keywords" ), QObject::tr( "Keyword vocabulary cannot be empty." ), index );
    }
    if ( keywordIt.value().isEmpty() )
    {
      result = false;
      results << ValidationResult( QObject::tr( "keywords" ), QObject::tr( "Keyword list cannot be empty." ), index );
    }
    index++;
  }

  // validate contacts
  index = 0;
  const auto constContacts = metadata->contacts();
  for ( const QgsAbstractMetadataBase::Contact &contact : constContacts )
  {
    if ( contact.name.isEmpty() )
    {
      result = false;
      results << ValidationResult( QObject::tr( "contacts" ), QObject::tr( "Contact name cannot be empty." ), index );
    }
    index++;
  }

  // validate links
  index = 0;
  const auto constLinks = metadata->links();
  for ( const QgsAbstractMetadataBase::Link &link : constLinks )
  {
    if ( link.name.isEmpty() )
    {
      result = false;
      results << ValidationResult( QObject::tr( "links" ), QObject::tr( "Link name cannot be empty." ), index );
    }
    if ( link.type.isEmpty() )
    {
      result = false;
      results << ValidationResult( QObject::tr( "links" ), QObject::tr( "Link type cannot be empty." ), index );
    }
    if ( link.url.isEmpty() )
    {
      result = false;
      results << ValidationResult( QObject::tr( "links" ), QObject::tr( "Link url cannot be empty." ), index );
    }
    index++;
  }

  return result;
}

//
// QgsNativeMetadataValidator
//

bool QgsNativeMetadataValidator::validate( const QgsAbstractMetadataBase *baseMetadata, QList<ValidationResult> &results ) const
{
  results.clear();

  const QgsLayerMetadata *metadata = dynamic_cast< const QgsLayerMetadata * >( baseMetadata );
  if ( !metadata )
    return false;

  bool result = true;
  if ( !QgsNativeMetadataBaseValidator::validate( metadata, results ) )
    result = false;

  if ( metadata->licenses().isEmpty() )
  {
    result = false;
    results << ValidationResult( QObject::tr( "license" ), QObject::tr( "At least one license is required." ) );
  }

  if ( !metadata->crs().isValid() )
  {
    result = false;
    results << ValidationResult( QObject::tr( "crs" ), QObject::tr( "A valid CRS element is required." ) );
  }

  int index = 0;
  const auto constSpatialExtents = metadata->extent().spatialExtents();
  for ( const QgsLayerMetadata::SpatialExtent &extent : constSpatialExtents )
  {
    if ( !extent.extentCrs.isValid() )
    {
      result = false;
      results << ValidationResult( QObject::tr( "extent" ), QObject::tr( "A valid CRS element for the spatial extent is required." ), index );
    }

    if ( extent.bounds.width() == 0.0 || extent.bounds.height() == 0.0 )
    {
      result = false;
      results << ValidationResult( QObject::tr( "extent" ), QObject::tr( "A valid spatial extent is required." ), index );
    }
    index++;
  }

  return result;
}


//
// QgsNativeProjectMetadataValidator
//

bool QgsNativeProjectMetadataValidator::validate( const QgsAbstractMetadataBase *baseMetadata, QList<QgsAbstractMetadataBaseValidator::ValidationResult> &results ) const
{
  results.clear();

  const QgsProjectMetadata *metadata = dynamic_cast< const QgsProjectMetadata * >( baseMetadata );
  if ( !metadata )
    return false;

  bool result = true;
  if ( !QgsNativeMetadataBaseValidator::validate( metadata, results ) )
    result = false;

  if ( metadata->author().isEmpty() )
  {
    result = false;
    results << ValidationResult( QObject::tr( "author" ), QObject::tr( "A project author is required." ) );
  }

  if ( !metadata->creationDateTime().isValid() )
  {
    result = false;
    results << ValidationResult( QObject::tr( "creation" ), QObject::tr( "The project creation date/time is required." ) );
  }

  return result;
}
