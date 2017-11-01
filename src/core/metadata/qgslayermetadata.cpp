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

void QgsLayerMetadata::addConstraint( const QgsLayerMetadata::Constraint &constraint )
{
  mConstraints << constraint;
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
  if ( mKeywords.contains( QStringLiteral( "gmd:topicCategory" ) ) )
  {
    return mKeywords.value( QStringLiteral( "gmd:topicCategory" ) );
  }
  else
  {
    return QStringList();
  }
}

void QgsLayerMetadata::setCategories( const QStringList &category )
{
  mKeywords.insert( QStringLiteral( "gmd:topicCategory" ), category );
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

bool QgsLayerMetadata::readMetadataXml( const QDomElement &metadataElement )
{
  QDomNode mnl;
  QDomElement mne;

  // set identifier
  mnl = metadataElement.namedItem( QStringLiteral( "identifier" ) );
  mIdentifier = mnl.toElement().text();

  // set parent identifier
  mnl = metadataElement.namedItem( QStringLiteral( "parentidentifier" ) );
  mParentIdentifier = mnl.toElement().text();

  // set language
  mnl = metadataElement.namedItem( QStringLiteral( "language" ) );
  mLanguage = mnl.toElement().text();

  // set type
  mnl = metadataElement.namedItem( QStringLiteral( "type" ) );
  mType = mnl.toElement().text();

  // set title
  mnl = metadataElement.namedItem( QStringLiteral( "title" ) );
  mTitle = mnl.toElement().text();

  // set abstract
  mnl = metadataElement.namedItem( QStringLiteral( "abstract" ) );
  mAbstract = mnl.toElement().text();

  // set keywords
  QDomNodeList keywords = metadataElement.elementsByTagName( QStringLiteral( "keywords" ) );
  mKeywords.clear();
  for ( int i = 0; i < keywords.size(); i++ )
  {
    QStringList keywordsList;
    mnl = keywords.at( i );
    mne = mnl.toElement();

    QDomNodeList el = mne.elementsByTagName( QStringLiteral( "keyword" ) );
    for ( int j = 0; j < el.size(); j++ )
    {
      keywordsList.append( el.at( j ).toElement().text() );
    }
    addKeywords( mne.attribute( QStringLiteral( "vocabulary" ) ), keywordsList );
  }

  // set fees
  mnl = metadataElement.namedItem( QStringLiteral( "fees" ) );
  mFees = mnl.toElement().text();

  // constraints
  QDomNodeList constraintsList = metadataElement.elementsByTagName( QStringLiteral( "constraints" ) );
  mConstraints.clear();
  for ( int i = 0; i < constraintsList.size(); i++ )
  {
    mnl = constraintsList.at( i );
    mne = mnl.toElement();
    addConstraint( QgsLayerMetadata::Constraint( mne.text(), mne.attribute( QStringLiteral( "type" ) ) ) );
  }

  // rights
  QDomNodeList rightsNodeList = metadataElement.elementsByTagName( QStringLiteral( "rights" ) );
  QStringList rightsList;
  for ( int i = 0; i < rightsNodeList.size(); i++ )
  {
    mnl = rightsNodeList.at( i );
    mne = mnl.toElement();
    rightsList.append( mne.text() );
  }
  setRights( rightsList );

  // licenses
  QDomNodeList licensesNodeList = metadataElement.elementsByTagName( QStringLiteral( "license" ) );
  QStringList licensesList;
  for ( int i = 0; i < licensesNodeList.size(); i++ )
  {
    mnl = licensesNodeList.at( i );
    mne = mnl.toElement();
    licensesList.append( mne.text() );
  }
  setLicenses( licensesList );

  // encoding
  mnl = metadataElement.namedItem( QStringLiteral( "encoding" ) );
  mEncoding = mnl.toElement().text();

  // crs
  mnl = metadataElement.namedItem( QStringLiteral( "crs" ) );
  mCrs.readXml( mnl );

  // extent
  mnl = metadataElement.namedItem( QStringLiteral( "extent" ) );
  QgsLayerMetadata::Extent metadataExtent;

  // spatial extent
  QDomNodeList spatialList = mnl.toElement().elementsByTagName( QStringLiteral( "spatial" ) );
  QList< QgsLayerMetadata::SpatialExtent > metadataSpatialExtents;
  for ( int i = 0; i < spatialList.size(); i++ )
  {
    mnl = spatialList.at( i );
    mne = mnl.toElement();
    QgsLayerMetadata::SpatialExtent se = QgsLayerMetadata::SpatialExtent();
    se.extentCrs = QgsCoordinateReferenceSystem( mne.attribute( QStringLiteral( "crs" ) ) );
    se.bounds = QgsBox3d();
    se.bounds.setXMinimum( mne.attribute( QStringLiteral( "minx" ) ).toDouble() );
    se.bounds.setYMinimum( mne.attribute( QStringLiteral( "miny" ) ).toDouble() );
    se.bounds.setZMinimum( mne.attribute( QStringLiteral( "minz" ) ).toDouble() );
    se.bounds.setXMaximum( mne.attribute( QStringLiteral( "maxx" ) ).toDouble() );
    se.bounds.setYMaximum( mne.attribute( QStringLiteral( "maxy" ) ).toDouble() );
    se.bounds.setZMaximum( mne.attribute( QStringLiteral( "maxz" ) ).toDouble() );
    metadataSpatialExtents.append( se );
  }
  metadataExtent.setSpatialExtents( metadataSpatialExtents );

  // temporal extent
  mnl = metadataElement.namedItem( QStringLiteral( "extent" ) );
  QDomNodeList temporalList = mnl.toElement().elementsByTagName( QStringLiteral( "temporal" ) );
  QList<QgsDateTimeRange> metadataDates;
  for ( int j = 0; j < temporalList.size(); j++ )
  {
    mnl = temporalList.at( j );
    QDomNodeList instantList = mnl.toElement().elementsByTagName( QStringLiteral( "instant" ) );
    for ( int i = 0; i < instantList.size(); i++ )
    {
      mnl = instantList.at( i );
      QDateTime d = QDateTime().fromString( mnl.toElement().text(), Qt::ISODate );
      QgsDateTimeRange date = QgsDateTimeRange( d, d );
      metadataDates << date;
    }
    QDomNodeList periodList = mnl.toElement().elementsByTagName( QStringLiteral( "period" ) );
    for ( int i = 0; i < periodList.size(); i++ )
    {
      QDomNode begin = periodList.at( i ).namedItem( QStringLiteral( "start" ) );
      QDomNode end = periodList.at( i ).namedItem( QStringLiteral( "end" ) );
      QDateTime beginDate = QDateTime().fromString( begin.toElement().text(), Qt::ISODate );
      QDateTime endDate = QDateTime().fromString( end.toElement().text(), Qt::ISODate );
      QgsDateTimeRange date = QgsDateTimeRange( beginDate, endDate );
      metadataDates << date;
    }
  }
  metadataExtent.setTemporalExtents( metadataDates );
  setExtent( metadataExtent );

  // contact
  QDomNodeList contactsList = metadataElement.elementsByTagName( QStringLiteral( "contact" ) );
  mContacts.clear();
  for ( int i = 0; i < contactsList.size(); i++ )
  {
    mnl = contactsList.at( i );
    mne = mnl.toElement();

    QgsLayerMetadata::Contact oneContact;
    oneContact.name = mne.namedItem( QStringLiteral( "name" ) ).toElement().text();
    oneContact.organization = mne.namedItem( QStringLiteral( "organization" ) ).toElement().text();
    oneContact.position = mne.namedItem( QStringLiteral( "position" ) ).toElement().text();
    oneContact.voice = mne.namedItem( QStringLiteral( "voice" ) ).toElement().text();
    oneContact.fax = mne.namedItem( QStringLiteral( "fax" ) ).toElement().text();
    oneContact.email = mne.namedItem( QStringLiteral( "email" ) ).toElement().text();
    oneContact.role = mne.namedItem( QStringLiteral( "role" ) ).toElement().text();

    QList< QgsLayerMetadata::Address > addresses;
    QDomNodeList addressList = mne.elementsByTagName( QStringLiteral( "address" ) );
    for ( int j = 0; j < addressList.size(); j++ )
    {
      QDomElement addressElement = addressList.at( j ).toElement();
      QgsLayerMetadata::Address oneAddress;
      oneAddress.address = addressElement.namedItem( QStringLiteral( "address" ) ).toElement().text();
      oneAddress.administrativeArea = addressElement.namedItem( QStringLiteral( "administrativearea" ) ).toElement().text();
      oneAddress.city = addressElement.namedItem( QStringLiteral( "city" ) ).toElement().text();
      oneAddress.country = addressElement.namedItem( QStringLiteral( "country" ) ).toElement().text();
      oneAddress.postalCode = addressElement.namedItem( QStringLiteral( "postalcode" ) ).toElement().text();
      oneAddress.type = addressElement.namedItem( QStringLiteral( "type" ) ).toElement().text();
      addresses << oneAddress;
    }
    oneContact.addresses = addresses;
    addContact( oneContact );
  }

  // links
  mnl = metadataElement.namedItem( QStringLiteral( "links" ) );
  mne = mnl.toElement();
  mLinks.clear();
  QDomNodeList el = mne.elementsByTagName( QStringLiteral( "link" ) );
  for ( int i = 0; i < el.size(); i++ )
  {
    mne = el.at( i ).toElement();
    QgsLayerMetadata::Link oneLink;
    oneLink.name = mne.attribute( QStringLiteral( "name" ) );
    oneLink.type = mne.attribute( QStringLiteral( "type" ) );
    oneLink.url = mne.attribute( QStringLiteral( "url" ) );
    oneLink.description = mne.attribute( QStringLiteral( "description" ) );
    oneLink.format = mne.attribute( QStringLiteral( "format" ) );
    oneLink.mimeType = mne.attribute( QStringLiteral( "mimeType" ) );
    oneLink.size = mne.attribute( QStringLiteral( "size" ) );
    addLink( oneLink );
  }

  // history
  QDomNodeList historyNodeList = metadataElement.elementsByTagName( QStringLiteral( "history" ) );
  QStringList historyList;
  for ( int i = 0; i < historyNodeList.size(); i++ )
  {
    mnl = historyNodeList.at( i );
    mne = mnl.toElement();
    historyList.append( mne.text() );
  }
  setHistory( historyList );

  return true;
}

bool QgsLayerMetadata::writeMetadataXml( QDomElement &metadataElement, QDomDocument &document ) const
{
  // identifier
  QDomElement identifier = document.createElement( QStringLiteral( "identifier" ) );
  QDomText identifierText = document.createTextNode( mIdentifier );
  identifier.appendChild( identifierText );
  metadataElement.appendChild( identifier );

  // parent identifier
  QDomElement parentIdentifier = document.createElement( QStringLiteral( "parentidentifier" ) );
  QDomText parentIdentifierText = document.createTextNode( mParentIdentifier );
  parentIdentifier.appendChild( parentIdentifierText );
  metadataElement.appendChild( parentIdentifier );

  // language
  QDomElement language = document.createElement( QStringLiteral( "language" ) );
  QDomText languageText = document.createTextNode( mLanguage );
  language.appendChild( languageText );
  metadataElement.appendChild( language );

  // type
  QDomElement type = document.createElement( QStringLiteral( "type" ) );
  QDomText typeText = document.createTextNode( mType );
  type.appendChild( typeText );
  metadataElement.appendChild( type );

  // title
  QDomElement title = document.createElement( QStringLiteral( "title" ) );
  QDomText titleText = document.createTextNode( mTitle );
  title.appendChild( titleText );
  metadataElement.appendChild( title );

  // abstract
  QDomElement abstract = document.createElement( QStringLiteral( "abstract" ) );
  QDomText abstractText = document.createTextNode( mAbstract );
  abstract.appendChild( abstractText );
  metadataElement.appendChild( abstract );

  // keywords
  QMapIterator<QString, QStringList> i( mKeywords );
  while ( i.hasNext() )
  {
    i.next();
    QDomElement keywordsElement = document.createElement( QStringLiteral( "keywords" ) );
    keywordsElement.setAttribute( QStringLiteral( "vocabulary" ), i.key() );
    const QStringList values = i.value();
    for ( const QString &kw : values )
    {
      QDomElement keyword = document.createElement( QStringLiteral( "keyword" ) );
      QDomText keywordText = document.createTextNode( kw );
      keyword.appendChild( keywordText );
      keywordsElement.appendChild( keyword );
    }
    metadataElement.appendChild( keywordsElement );
  }

  // fees
  QDomElement fees = document.createElement( QStringLiteral( "fees" ) );
  QDomText feesText = document.createTextNode( mFees );
  fees.appendChild( feesText );
  metadataElement.appendChild( fees );

  // constraints
  for ( const QgsLayerMetadata::Constraint &constraint : mConstraints )
  {
    QDomElement constraintElement = document.createElement( QStringLiteral( "constraints" ) );
    constraintElement.setAttribute( QStringLiteral( "type" ), constraint.type );
    QDomText constraintText = document.createTextNode( constraint.constraint );
    constraintElement.appendChild( constraintText );
    metadataElement.appendChild( constraintElement );
  }

  // rights
  for ( const QString &right : mRights )
  {
    QDomElement rightElement = document.createElement( QStringLiteral( "rights" ) );
    QDomText rightText = document.createTextNode( right );
    rightElement.appendChild( rightText );
    metadataElement.appendChild( rightElement );
  }

  // license
  for ( const QString &license : mLicenses )
  {
    QDomElement licenseElement = document.createElement( QStringLiteral( "license" ) );
    QDomText licenseText = document.createTextNode( license );
    licenseElement.appendChild( licenseText );
    metadataElement.appendChild( licenseElement );
  }

  // encoding
  QDomElement encoding = document.createElement( QStringLiteral( "encoding" ) );
  QDomText encodingText = document.createTextNode( mEncoding );
  encoding.appendChild( encodingText );
  metadataElement.appendChild( encoding );

  // crs
  QDomElement crsElement = document.createElement( QStringLiteral( "crs" ) );
  mCrs.writeXml( crsElement, document );
  metadataElement.appendChild( crsElement );

  // extent
  QDomElement extentElement = document.createElement( QStringLiteral( "extent" ) );

  // spatial extents
  const QList< QgsLayerMetadata::SpatialExtent > sExtents = extent().spatialExtents();
  for ( const QgsLayerMetadata::SpatialExtent &spatialExtent : sExtents )
  {
    QDomElement spatialElement = document.createElement( QStringLiteral( "spatial" ) );
    // Dimensions fixed in the XSD
    spatialElement.setAttribute( QStringLiteral( "dimensions" ), QStringLiteral( "2" ) );
    spatialElement.setAttribute( QStringLiteral( "crs" ), spatialExtent.extentCrs.authid() );
    spatialElement.setAttribute( QStringLiteral( "minx" ), qgsDoubleToString( spatialExtent.bounds.xMinimum() ) );
    spatialElement.setAttribute( QStringLiteral( "miny" ), qgsDoubleToString( spatialExtent.bounds.yMinimum() ) );
    spatialElement.setAttribute( QStringLiteral( "minz" ), qgsDoubleToString( spatialExtent.bounds.zMinimum() ) );
    spatialElement.setAttribute( QStringLiteral( "maxx" ), qgsDoubleToString( spatialExtent.bounds.xMaximum() ) );
    spatialElement.setAttribute( QStringLiteral( "maxy" ), qgsDoubleToString( spatialExtent.bounds.yMaximum() ) );
    spatialElement.setAttribute( QStringLiteral( "maxz" ), qgsDoubleToString( spatialExtent.bounds.zMaximum() ) );
    extentElement.appendChild( spatialElement );
  }

  // temporal extents
  const QList< QgsDateTimeRange > tExtents = extent().temporalExtents();
  for ( const QgsDateTimeRange &temporalExtent : tExtents )
  {
    QDomElement temporalElement = document.createElement( QStringLiteral( "temporal" ) );
    if ( temporalExtent.isInstant() )
    {
      QDomElement instantElement = document.createElement( QStringLiteral( "instant" ) );
      QDomText instantText = document.createTextNode( temporalExtent.begin().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate ) );
      instantElement.appendChild( instantText );
      temporalElement.appendChild( instantElement );
    }
    else
    {
      QDomElement periodElement = document.createElement( QStringLiteral( "period" ) );
      QDomElement startElement = document.createElement( QStringLiteral( "start" ) );
      QDomElement endElement = document.createElement( QStringLiteral( "end" ) );
      QDomText startText = document.createTextNode( temporalExtent.begin().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate ) );
      QDomText endText = document.createTextNode( temporalExtent.end().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate ) );
      startElement.appendChild( startText );
      endElement.appendChild( endText );
      periodElement.appendChild( startElement );
      periodElement.appendChild( endElement );
      temporalElement.appendChild( periodElement );
    }
    extentElement.appendChild( temporalElement );
  }

  metadataElement.appendChild( extentElement );

  // contact
  for ( const QgsLayerMetadata::Contact &contact : mContacts )
  {
    QDomElement contactElement = document.createElement( QStringLiteral( "contact" ) );
    QDomElement nameElement = document.createElement( QStringLiteral( "name" ) );
    QDomElement organizationElement = document.createElement( QStringLiteral( "organization" ) );
    QDomElement positionElement = document.createElement( QStringLiteral( "position" ) );
    QDomElement voiceElement = document.createElement( QStringLiteral( "voice" ) );
    QDomElement faxElement = document.createElement( QStringLiteral( "fax" ) );
    QDomElement emailElement = document.createElement( QStringLiteral( "email" ) );
    QDomElement roleElement = document.createElement( QStringLiteral( "role" ) );

    QDomText nameText = document.createTextNode( contact.name );
    QDomText orgaText = document.createTextNode( contact.organization );
    QDomText positionText = document.createTextNode( contact.position );
    QDomText voiceText = document.createTextNode( contact.voice );
    QDomText faxText = document.createTextNode( contact.fax );
    QDomText emailText = document.createTextNode( contact.email );
    QDomText roleText = document.createTextNode( contact.role );

    for ( const QgsLayerMetadata::Address &oneAddress : contact.addresses )
    {
      QDomElement addressElement = document.createElement( QStringLiteral( "address" ) );
      QDomElement typeElement = document.createElement( QStringLiteral( "type" ) );
      QDomElement addressDetailedElement = document.createElement( QStringLiteral( "address" ) );
      QDomElement cityElement = document.createElement( QStringLiteral( "city" ) );
      QDomElement administrativeAreaElement = document.createElement( QStringLiteral( "administrativearea" ) );
      QDomElement postalCodeElement = document.createElement( QStringLiteral( "postalcode" ) );
      QDomElement countryElement = document.createElement( QStringLiteral( "country" ) );

      typeElement.appendChild( document.createTextNode( oneAddress.type ) );
      addressDetailedElement.appendChild( document.createTextNode( oneAddress.address ) );
      cityElement.appendChild( document.createTextNode( oneAddress.city ) );
      administrativeAreaElement.appendChild( document.createTextNode( oneAddress.administrativeArea ) );
      postalCodeElement.appendChild( document.createTextNode( oneAddress.postalCode ) );
      countryElement.appendChild( document.createTextNode( oneAddress.country ) );

      addressElement.appendChild( typeElement );
      addressElement.appendChild( addressDetailedElement );
      addressElement.appendChild( cityElement );
      addressElement.appendChild( administrativeAreaElement );
      addressElement.appendChild( postalCodeElement );
      addressElement.appendChild( countryElement );
      contactElement.appendChild( addressElement );
    }

    nameElement.appendChild( nameText );
    organizationElement.appendChild( orgaText );
    positionElement.appendChild( positionText );
    voiceElement.appendChild( voiceText );
    faxElement.appendChild( faxText );
    emailElement.appendChild( emailText );
    roleElement.appendChild( roleText );

    contactElement.appendChild( nameElement );
    contactElement.appendChild( organizationElement );
    contactElement.appendChild( positionElement );
    contactElement.appendChild( voiceElement );
    contactElement.appendChild( faxElement );
    contactElement.appendChild( emailElement );
    contactElement.appendChild( roleElement );
    metadataElement.appendChild( contactElement );
  }

  // links
  QDomElement links = document.createElement( QStringLiteral( "links" ) );
  for ( const QgsLayerMetadata::Link &link : mLinks )
  {
    QDomElement linkElement = document.createElement( QStringLiteral( "link" ) );
    linkElement.setAttribute( QStringLiteral( "name" ), link.name );
    linkElement.setAttribute( QStringLiteral( "type" ), link.type );
    linkElement.setAttribute( QStringLiteral( "url" ), link.url );
    linkElement.setAttribute( QStringLiteral( "description" ), link.description );
    linkElement.setAttribute( QStringLiteral( "format" ), link.format );
    linkElement.setAttribute( QStringLiteral( "mimeType" ), link.mimeType );
    linkElement.setAttribute( QStringLiteral( "size" ), link.size );
    links.appendChild( linkElement );
  }
  metadataElement.appendChild( links );

  // history
  for ( const QString &history : mHistory )
  {
    QDomElement historyElement = document.createElement( QStringLiteral( "history" ) );
    QDomText historyText = document.createTextNode( history );
    historyElement.appendChild( historyText );
    metadataElement.appendChild( historyElement );
  }

  return true;
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
