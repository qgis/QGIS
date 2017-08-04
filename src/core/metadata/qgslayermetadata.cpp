/***************************************************************************
                             qgslayermetadata.cpp
                             --------------------
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

#include "qgslayermetadata.h"
#include "qgsmaplayer.h"

QString QgsLayerMetadata::identifier() const
{
  return mIdentifier;
}

void QgsLayerMetadata::setIdentifier( const QString &identifier )
{
  mIdentifier = identifier;
}

QString QgsLayerMetadata::parentIdentifier() const
{
  return mParentIdentifier;
}

void QgsLayerMetadata::setParentIdentifier( const QString &parentIdentifier )
{
  mParentIdentifier = parentIdentifier;
}

QString QgsLayerMetadata::type() const
{
  return mType;
}

void QgsLayerMetadata::setType( const QString &type )
{
  mType = type;
}

QString QgsLayerMetadata::title() const
{
  return mTitle;
}

void QgsLayerMetadata::setTitle( const QString &title )
{
  mTitle = title;
}

QString QgsLayerMetadata::abstract() const
{
  return mAbstract;
}

void QgsLayerMetadata::setAbstract( const QString &abstract )
{
  mAbstract = abstract;
}

QString QgsLayerMetadata::fees() const
{
  return mFees;
}

void QgsLayerMetadata::setFees( const QString &fees )
{
  mFees = fees;
}

QList<QgsLayerMetadata::Constraint> QgsLayerMetadata::constraints() const
{
  return mConstraints;
}

void QgsLayerMetadata::setConstraints( const QList<Constraint> &constraints )
{
  mConstraints = constraints;
}

QStringList QgsLayerMetadata::rights() const
{
  return mRights;
}

void QgsLayerMetadata::setRights( const QStringList &rights )
{
  mRights = rights;
}

QStringList QgsLayerMetadata::licenses() const
{
  return mLicenses;
}

void QgsLayerMetadata::setLicenses( const QStringList &licenses )
{
  mLicenses = licenses;
}

QStringList QgsLayerMetadata::history() const
{
  return mHistory;
}

void QgsLayerMetadata::setHistory( const QStringList &history )
{
  mHistory = history;
}

void QgsLayerMetadata::addHistoryItem( const QString &text )
{
  mHistory << text;
}

QString QgsLayerMetadata::encoding() const
{
  return mEncoding;
}

void QgsLayerMetadata::setEncoding( const QString &encoding )
{
  mEncoding = encoding;
}

QgsCoordinateReferenceSystem QgsLayerMetadata::crs() const
{
  return mCrs;
}

void QgsLayerMetadata::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCrs = crs;
}

QMap<QString, QStringList> QgsLayerMetadata::keywords() const
{
  return mKeywords;
}

void QgsLayerMetadata::setKeywords( const QMap<QString, QStringList> &keywords )
{
  mKeywords = keywords;
}

void QgsLayerMetadata::addKeywords( const QString &vocabulary, const QStringList &keywords )
{
  mKeywords.insert( vocabulary, keywords );
}

bool QgsLayerMetadata::removeKeywords( const QString &vocabulary )
{
  return mKeywords.remove( vocabulary );
}

QStringList QgsLayerMetadata::keywordVocabularies() const
{
  return mKeywords.keys();
}

QStringList QgsLayerMetadata::keywords( const QString &vocabulary ) const
{
  return mKeywords.value( vocabulary );
}

QStringList QgsLayerMetadata::categories() const
{
  if ( mKeywords.contains( "gmd:topicCategory" ) )
  {
    return mKeywords.value( "gmd:topicCategory" );
  }
  else
  {
    return QStringList();
  }
}

void QgsLayerMetadata::setCategories( const QStringList &category )
{
  mKeywords.insert( "gmd:topicCategory", category );
}

QList<QgsLayerMetadata::Contact> QgsLayerMetadata::contacts() const
{
  return mContacts;
}

void QgsLayerMetadata::setContacts( const QList<Contact> &contacts )
{
  mContacts = contacts;
}

void QgsLayerMetadata::addContact( const QgsLayerMetadata::Contact &contact )
{
  mContacts << contact;
}

QList<QgsLayerMetadata::Link> QgsLayerMetadata::links() const
{
  return mLinks;
}

void QgsLayerMetadata::setLinks( const QList<QgsLayerMetadata::Link> &links )
{
  mLinks = links;
}

void QgsLayerMetadata::addLink( const QgsLayerMetadata::Link &link )
{
  mLinks << link;
}

QString QgsLayerMetadata::language() const
{
  return mLanguage;
}

void QgsLayerMetadata::setLanguage( const QString &language )
{
  mLanguage = language;
}

void QgsLayerMetadata::saveToLayer( QgsMapLayer *layer ) const
{
  layer->setCustomProperty( QStringLiteral( "metadata/identifier" ), mIdentifier );
  layer->setCustomProperty( QStringLiteral( "metadata/parentIdentifier" ), mParentIdentifier );
  layer->setCustomProperty( QStringLiteral( "metadata/language" ), mLanguage );
  layer->setCustomProperty( QStringLiteral( "metadata/type" ), mType );
  layer->setCustomProperty( QStringLiteral( "metadata/title" ), mTitle );
  layer->setCustomProperty( QStringLiteral( "metadata/extent" ), QVariant::fromValue( mExtent ) );
  layer->setCustomProperty( QStringLiteral( "metadata/abstract" ), mAbstract );
  layer->setCustomProperty( QStringLiteral( "metadata/fees" ), mFees );
  layer->setCustomProperty( QStringLiteral( "metadata/rights" ), mRights );
  layer->setCustomProperty( QStringLiteral( "metadata/licenses" ), mLicenses );
  layer->setCustomProperty( QStringLiteral( "metadata/history" ), mHistory );
  layer->setCustomProperty( QStringLiteral( "metadata/encoding" ), mEncoding );
  layer->setCustomProperty( QStringLiteral( "metadata/crs" ), mCrs.authid() );
  layer->setCustomProperty( QStringLiteral( "metadata/constraints" ), QVariant::fromValue( mConstraints ) );
  layer->setCustomProperty( QStringLiteral( "metadata/keywords" ), QVariant::fromValue( mKeywords ) );
  layer->setCustomProperty( QStringLiteral( "metadata/contacts" ), QVariant::fromValue( mContacts ) );
  layer->setCustomProperty( QStringLiteral( "metadata/links" ), QVariant::fromValue( mLinks ) );
}

void QgsLayerMetadata::readFromLayer( const QgsMapLayer *layer )
{
  mIdentifier = layer->customProperty( QStringLiteral( "metadata/identifier" ) ).toString();
  mParentIdentifier = layer->customProperty( QStringLiteral( "metadata/parentIdentifier" ) ).toString();
  mLanguage = layer->customProperty( QStringLiteral( "metadata/language" ) ).toString();
  mType = layer->customProperty( QStringLiteral( "metadata/type" ) ).toString();
  mTitle = layer->customProperty( QStringLiteral( "metadata/title" ) ).toString();
  mAbstract = layer->customProperty( QStringLiteral( "metadata/abstract" ) ).toString();
  mFees = layer->customProperty( QStringLiteral( "metadata/fees" ) ).toString();
  mRights = layer->customProperty( QStringLiteral( "metadata/rights" ) ).toStringList();
  mLicenses = layer->customProperty( QStringLiteral( "metadata/licenses" ) ).toStringList();
  mHistory = layer->customProperty( QStringLiteral( "metadata/history" ) ).toStringList();
  mEncoding = layer->customProperty( QStringLiteral( "metadata/encoding" ) ).toString();
  QString crsAuthId = layer->customProperty( QStringLiteral( "metadata/crs" ) ).toString();
  mCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crsAuthId );
  mExtent = layer->customProperty( QStringLiteral( "metadata/extent" ) ).value<Extent>();
  mConstraints = layer->customProperty( QStringLiteral( "metadata/constraints" ) ).value<ConstraintList>();
  mKeywords = layer->customProperty( QStringLiteral( "metadata/keywords" ) ).value<KeywordMap>();
  mContacts = layer->customProperty( QStringLiteral( "metadata/contacts" ) ).value<ContactList>();
  mLinks = layer->customProperty( QStringLiteral( "metadata/links" ) ).value<LinkList>();
}

const QgsLayerMetadata::Extent &QgsLayerMetadata::extent() const
{
  return mExtent;
}

QgsLayerMetadata::Extent &QgsLayerMetadata::extent()
{
  return mExtent;
}

void QgsLayerMetadata::setExtent( const Extent &extent )
{
  mExtent = extent;
}

QList<QgsLayerMetadata::SpatialExtent> QgsLayerMetadata::Extent::spatialExtents() const
{
  return mSpatialExtents;
}

void QgsLayerMetadata::Extent::setSpatialExtents( const QList<QgsLayerMetadata::SpatialExtent> &spatialExtents )
{
  mSpatialExtents = spatialExtents;
}

QList<QgsDateTimeRange> QgsLayerMetadata::Extent::temporalExtents() const
{
  return mTemporalExtents;
}

void QgsLayerMetadata::Extent::setTemporalExtents( const QList<QgsDateTimeRange> &temporalExtents )
{
  mTemporalExtents = temporalExtents;
}
