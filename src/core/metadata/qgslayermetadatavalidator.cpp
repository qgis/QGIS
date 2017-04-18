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

bool QgsNativeMetadataValidator::validate( const QgsLayerMetadata &metadata, QList<ValidationResult> &results ) const
{
  results.clear();

  bool result = true;
  if ( metadata.identifier().isEmpty() )
  {
    result = false;
    results << ValidationResult( QObject::tr( "identifier" ), QObject::tr( "Identifier element is required." ) );
  }

  if ( metadata.language().isEmpty() )
  {
    result = false;
    results << ValidationResult( QObject::tr( "language" ), QObject::tr( "Language element is required." ) );
  }

  if ( metadata.type().isEmpty() )
  {
    result = false;
    results << ValidationResult( QObject::tr( "type" ), QObject::tr( "Type element is required." ) );
  }

  if ( metadata.title().isEmpty() )
  {
    result = false;
    results << ValidationResult( QObject::tr( "title" ), QObject::tr( "Title element is required." ) );
  }

  if ( metadata.abstract().isEmpty() )
  {
    result = false;
    results << ValidationResult( QObject::tr( "abstract" ), QObject::tr( "Abstract element is required." ) );
  }

  //result = result && !metadata.license().isEmpty();

  if ( !metadata.crs().isValid() )
  {
    result = false;
    results << ValidationResult( QObject::tr( "crs" ), QObject::tr( "A valid CRS element is required." ) );
  }

  if ( metadata.contacts().isEmpty() )
  {
    result = false;
    results << ValidationResult( QObject::tr( "contacts" ), QObject::tr( "At least one contact is required." ) );
  }

  if ( metadata.links().isEmpty() )
  {
    result = false;
    results << ValidationResult( QObject::tr( "links" ), QObject::tr( "At least one link is required." ) );
  }

  // validate keywords
  QgsLayerMetadata::KeywordMap keywords = metadata.keywords();
  QgsLayerMetadata::KeywordMap::const_iterator keywordIt = keywords.constBegin();
  int index = 0;
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
  Q_FOREACH ( const QgsLayerMetadata::Contact &contact, metadata.contacts() )
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
  Q_FOREACH ( const QgsLayerMetadata::Link &link, metadata.links() )
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
