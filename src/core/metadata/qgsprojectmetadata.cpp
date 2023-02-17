/***************************************************************************
                             qgsprojectmetadata.cpp
                             --------------------
    begin                : March 2018
    copyright            : (C) 2018 by Nyall Dawson
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

#include "qgsprojectmetadata.h"
#include <QDomNode>
#include <QDomDocument>

bool QgsProjectMetadata::readMetadataXml( const QDomElement &metadataElement )
{
  QgsAbstractMetadataBase::readMetadataXml( metadataElement );

  QDomNode mnl;

  // set author
  mnl = metadataElement.namedItem( QStringLiteral( "author" ) );
  mAuthor = mnl.toElement().text();

  if ( !mDates.contains( Qgis::MetadataDateType::Created ) )
  {
    // creation datetime -- old format
    mnl = metadataElement.namedItem( QStringLiteral( "creation" ) );
    const QDateTime creationDateTime = QDateTime::fromString( mnl.toElement().text(), Qt::ISODate );
    mDates.insert( Qgis::MetadataDateType::Created, creationDateTime );
  }

  return true;
}

bool QgsProjectMetadata::writeMetadataXml( QDomElement &metadataElement, QDomDocument &document ) const
{
  QgsAbstractMetadataBase::writeMetadataXml( metadataElement, document );

  // author
  QDomElement author = document.createElement( QStringLiteral( "author" ) );
  const QDomText authorText = document.createTextNode( mAuthor );
  author.appendChild( authorText );
  metadataElement.appendChild( author );

  // creation datetime
  QDomElement creation = document.createElement( QStringLiteral( "creation" ) );
  const QDomText creationText = document.createTextNode( mDates.value( Qgis::MetadataDateType::Created ).toString( Qt::ISODate ) );
  creation.appendChild( creationText );
  metadataElement.appendChild( creation );

  return true;
}

void QgsProjectMetadata::combine( const QgsAbstractMetadataBase *other )
{
  QgsAbstractMetadataBase::combine( other );

  if ( const QgsProjectMetadata *otherProjectMetadata = dynamic_cast< const QgsProjectMetadata * >( other ) )
  {
    if ( !otherProjectMetadata->author().isEmpty() )
      mAuthor = otherProjectMetadata->author();
  }
}

bool QgsProjectMetadata::operator==( const QgsProjectMetadata &metadataOther )  const
{
  return equals( metadataOther ) &&
         mAuthor == metadataOther.mAuthor;
}

QgsProjectMetadata *QgsProjectMetadata::clone() const
{
  return new QgsProjectMetadata( *this );
}

QString QgsProjectMetadata::author() const
{
  return mAuthor;
}

void QgsProjectMetadata::setAuthor( const QString &author )
{
  mAuthor = author;
}

QDateTime QgsProjectMetadata::creationDateTime() const
{
  return mDates.value( Qgis::MetadataDateType::Created );
}

void QgsProjectMetadata::setCreationDateTime( const QDateTime &creationDateTime )
{
  mDates[ Qgis::MetadataDateType::Created ] = creationDateTime;
}
