/***************************************************************************
                             QgsAbstractMetadataBase.cpp
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

#include "qgsabstractmetadatabase.h"
#include "qgsmaplayer.h"

QString QgsAbstractMetadataBase::identifier() const
{
  return mIdentifier;
}

void QgsAbstractMetadataBase::setIdentifier( const QString &identifier )
{
  mIdentifier = identifier;
}

QString QgsAbstractMetadataBase::parentIdentifier() const
{
  return mParentIdentifier;
}

void QgsAbstractMetadataBase::setParentIdentifier( const QString &parentIdentifier )
{
  mParentIdentifier = parentIdentifier;
}

QString QgsAbstractMetadataBase::type() const
{
  return mType;
}

void QgsAbstractMetadataBase::setType( const QString &type )
{
  mType = type;
}

QString QgsAbstractMetadataBase::title() const
{
  return mTitle;
}

void QgsAbstractMetadataBase::setTitle( const QString &title )
{
  mTitle = title;
}

QString QgsAbstractMetadataBase::abstract() const
{
  return mAbstract;
}

void QgsAbstractMetadataBase::setAbstract( const QString &abstract )
{
  mAbstract = abstract;
}

QStringList QgsAbstractMetadataBase::history() const
{
  return mHistory;
}

void QgsAbstractMetadataBase::setHistory( const QStringList &history )
{
  mHistory = history;
}

void QgsAbstractMetadataBase::addHistoryItem( const QString &text )
{
  mHistory << text;
}

QMap<QString, QStringList> QgsAbstractMetadataBase::keywords() const
{
  return mKeywords;
}

void QgsAbstractMetadataBase::setKeywords( const QMap<QString, QStringList> &keywords )
{
  mKeywords = keywords;
}

void QgsAbstractMetadataBase::addKeywords( const QString &vocabulary, const QStringList &keywords )
{
  mKeywords.insert( vocabulary, keywords );
}

bool QgsAbstractMetadataBase::removeKeywords( const QString &vocabulary )
{
  return mKeywords.remove( vocabulary );
}

QStringList QgsAbstractMetadataBase::keywordVocabularies() const
{
  return mKeywords.keys();
}

QStringList QgsAbstractMetadataBase::keywords( const QString &vocabulary ) const
{
  return mKeywords.value( vocabulary );
}

QStringList QgsAbstractMetadataBase::categories() const
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

void QgsAbstractMetadataBase::setCategories( const QStringList &category )
{
  mKeywords.insert( QStringLiteral( "gmd:topicCategory" ), category );
}

QList<QgsAbstractMetadataBase::Contact> QgsAbstractMetadataBase::contacts() const
{
  return mContacts;
}

void QgsAbstractMetadataBase::setContacts( const QList<QgsAbstractMetadataBase::Contact> &contacts )
{
  mContacts = contacts;
}

void QgsAbstractMetadataBase::addContact( const QgsAbstractMetadataBase::Contact &contact )
{
  mContacts << contact;
}

QList<QgsAbstractMetadataBase::Link> QgsAbstractMetadataBase::links() const
{
  return mLinks;
}

void QgsAbstractMetadataBase::setLinks( const QList<QgsAbstractMetadataBase::Link> &links )
{
  mLinks = links;
}

void QgsAbstractMetadataBase::addLink( const QgsAbstractMetadataBase::Link &link )
{
  mLinks << link;
}

QDateTime QgsAbstractMetadataBase::dateTime( Qgis::MetadataDateType type ) const
{
  return mDates.value( type );
}

void QgsAbstractMetadataBase::setDateTime( Qgis::MetadataDateType type, QDateTime date )
{
  if ( !date.isValid() || date.isNull() )
    mDates.remove( type );
  else
    mDates[type] = date;
}

QString QgsAbstractMetadataBase::language() const
{
  return mLanguage;
}

void QgsAbstractMetadataBase::setLanguage( const QString &language )
{
  mLanguage = language;
}

bool QgsAbstractMetadataBase::readMetadataXml( const QDomElement &metadataElement )
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
  const QDomNodeList keywords = metadataElement.elementsByTagName( QStringLiteral( "keywords" ) );
  mKeywords.clear();
  for ( int i = 0; i < keywords.size(); i++ )
  {
    QStringList keywordsList;
    mnl = keywords.at( i );
    mne = mnl.toElement();

    const QDomNodeList el = mne.elementsByTagName( QStringLiteral( "keyword" ) );
    for ( int j = 0; j < el.size(); j++ )
    {
      keywordsList.append( el.at( j ).toElement().text() );
    }
    addKeywords( mne.attribute( QStringLiteral( "vocabulary" ) ), keywordsList );
  }

  // contact
  const QDomNodeList contactsList = metadataElement.elementsByTagName( QStringLiteral( "contact" ) );
  mContacts.clear();
  for ( int i = 0; i < contactsList.size(); i++ )
  {
    mnl = contactsList.at( i );
    mne = mnl.toElement();

    QgsAbstractMetadataBase::Contact oneContact;
    oneContact.name = mne.namedItem( QStringLiteral( "name" ) ).toElement().text();
    oneContact.organization = mne.namedItem( QStringLiteral( "organization" ) ).toElement().text();
    oneContact.position = mne.namedItem( QStringLiteral( "position" ) ).toElement().text();
    oneContact.voice = mne.namedItem( QStringLiteral( "voice" ) ).toElement().text();
    oneContact.fax = mne.namedItem( QStringLiteral( "fax" ) ).toElement().text();
    oneContact.email = mne.namedItem( QStringLiteral( "email" ) ).toElement().text();
    oneContact.role = mne.namedItem( QStringLiteral( "role" ) ).toElement().text();

    QList< QgsAbstractMetadataBase::Address > addresses;
    const QDomNodeList addressList = mne.elementsByTagName( QStringLiteral( "contactAddress" ) );
    for ( int j = 0; j < addressList.size(); j++ )
    {
      const QDomElement addressElement = addressList.at( j ).toElement();
      QgsAbstractMetadataBase::Address oneAddress;
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
  const QDomNodeList el = mne.elementsByTagName( QStringLiteral( "link" ) );
  for ( int i = 0; i < el.size(); i++ )
  {
    mne = el.at( i ).toElement();
    QgsAbstractMetadataBase::Link oneLink;
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
  const QDomNodeList historyNodeList = metadataElement.elementsByTagName( QStringLiteral( "history" ) );
  QStringList historyList;
  for ( int i = 0; i < historyNodeList.size(); i++ )
  {
    mnl = historyNodeList.at( i );
    mne = mnl.toElement();
    historyList.append( mne.text() );
  }
  setHistory( historyList );

  {
    mDates.clear();
    const QDomElement dateElement = metadataElement.firstChildElement( QStringLiteral( "dates" ) );
    if ( !dateElement.isNull() )
    {
      const QDomNodeList dateNodeList = dateElement.elementsByTagName( QStringLiteral( "date" ) );
      const QMetaEnum dateEnum = QMetaEnum::fromType<Qgis::MetadataDateType>();
      for ( int i = 0; i < dateNodeList.size(); i++ )
      {
        const QDomElement dateElement = dateNodeList.at( i ).toElement();
        const Qgis::MetadataDateType type = static_cast< Qgis::MetadataDateType >( dateEnum.keyToValue( dateElement.attribute( QStringLiteral( "type" ) ).toStdString().c_str() ) );
        const QDateTime value = QDateTime::fromString( dateElement.attribute( QStringLiteral( "value" ) ), Qt::ISODate );
        if ( value.isValid() && !value.isNull() )
          mDates.insert( type, value );
      }
    }
  }

  return true;
}

bool QgsAbstractMetadataBase::writeMetadataXml( QDomElement &metadataElement, QDomDocument &document ) const
{
  // identifier
  QDomElement identifier = document.createElement( QStringLiteral( "identifier" ) );
  const QDomText identifierText = document.createTextNode( mIdentifier );
  identifier.appendChild( identifierText );
  metadataElement.appendChild( identifier );

  // parent identifier
  QDomElement parentIdentifier = document.createElement( QStringLiteral( "parentidentifier" ) );
  const QDomText parentIdentifierText = document.createTextNode( mParentIdentifier );
  parentIdentifier.appendChild( parentIdentifierText );
  metadataElement.appendChild( parentIdentifier );

  // language
  QDomElement language = document.createElement( QStringLiteral( "language" ) );
  const QDomText languageText = document.createTextNode( mLanguage );
  language.appendChild( languageText );
  metadataElement.appendChild( language );

  // type
  QDomElement type = document.createElement( QStringLiteral( "type" ) );
  const QDomText typeText = document.createTextNode( mType );
  type.appendChild( typeText );
  metadataElement.appendChild( type );

  // title
  QDomElement title = document.createElement( QStringLiteral( "title" ) );
  const QDomText titleText = document.createTextNode( mTitle );
  title.appendChild( titleText );
  metadataElement.appendChild( title );

  // abstract
  QDomElement abstract = document.createElement( QStringLiteral( "abstract" ) );
  const QDomText abstractText = document.createTextNode( mAbstract );
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
      const QDomText keywordText = document.createTextNode( kw );
      keyword.appendChild( keywordText );
      keywordsElement.appendChild( keyword );
    }
    metadataElement.appendChild( keywordsElement );
  }

  // contact
  for ( const QgsAbstractMetadataBase::Contact &contact : mContacts )
  {
    QDomElement contactElement = document.createElement( QStringLiteral( "contact" ) );
    QDomElement nameElement = document.createElement( QStringLiteral( "name" ) );
    QDomElement organizationElement = document.createElement( QStringLiteral( "organization" ) );
    QDomElement positionElement = document.createElement( QStringLiteral( "position" ) );
    QDomElement voiceElement = document.createElement( QStringLiteral( "voice" ) );
    QDomElement faxElement = document.createElement( QStringLiteral( "fax" ) );
    QDomElement emailElement = document.createElement( QStringLiteral( "email" ) );
    QDomElement roleElement = document.createElement( QStringLiteral( "role" ) );

    const QDomText nameText = document.createTextNode( contact.name );
    const QDomText orgaText = document.createTextNode( contact.organization );
    const QDomText positionText = document.createTextNode( contact.position );
    const QDomText voiceText = document.createTextNode( contact.voice );
    const QDomText faxText = document.createTextNode( contact.fax );
    const QDomText emailText = document.createTextNode( contact.email );
    const QDomText roleText = document.createTextNode( contact.role );

    for ( const QgsAbstractMetadataBase::Address &oneAddress : contact.addresses )
    {
      QDomElement addressElement = document.createElement( QStringLiteral( "contactAddress" ) );
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
  for ( const QgsAbstractMetadataBase::Link &link : mLinks )
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
    const QDomText historyText = document.createTextNode( history );
    historyElement.appendChild( historyText );
    metadataElement.appendChild( historyElement );
  }

  // dates
  {
    const QMetaEnum dateEnum = QMetaEnum::fromType<Qgis::MetadataDateType>();
    QDomElement datesElement = document.createElement( QStringLiteral( "dates" ) );
    for ( int k = 0; k < dateEnum.keyCount(); k++ )
    {
      const Qgis::MetadataDateType type = static_cast< Qgis::MetadataDateType >( dateEnum.value( k ) );
      if ( mDates.contains( type ) && mDates.value( type ).isValid() )
      {
        QDomElement dateElement = document.createElement( QStringLiteral( "date" ) );
        dateElement.setAttribute( QStringLiteral( "type" ), dateEnum.valueToKey( static_cast< int >( type ) ) );
        dateElement.setAttribute( QStringLiteral( "value" ), mDates.value( type ).toString( Qt::ISODate ) );
        datesElement.appendChild( dateElement );
      }
    }
    metadataElement.appendChild( datesElement );
  }

  return true;
}

void QgsAbstractMetadataBase::combine( const QgsAbstractMetadataBase *other )
{
  if ( !other )
    return;

  if ( !other->identifier().isEmpty() )
    mIdentifier = other->identifier();

  if ( !other->parentIdentifier().isEmpty() )
    mParentIdentifier = other->parentIdentifier();

  if ( !other->language().isEmpty() )
    mLanguage = other->language();

  if ( !other->type().isEmpty() )
    mType = other->type();

  if ( !other->title().isEmpty() )
    mTitle = other->title();

  if ( !other->abstract().isEmpty() )
    mAbstract = other->abstract();

  if ( !other->history().isEmpty() )
    mHistory = other->history();

  if ( !other->keywords().isEmpty() )
    mKeywords = other->keywords();

  if ( !other->contacts().isEmpty() )
    mContacts = other->contacts();

  if ( !other->links().isEmpty() )
    mLinks = other->links();

  const QMetaEnum dateEnum = QMetaEnum::fromType<Qgis::MetadataDateType>();
  for ( int k = 0; k < dateEnum.keyCount(); k++ )
  {
    const Qgis::MetadataDateType type = static_cast< Qgis::MetadataDateType >( dateEnum.value( k ) );
    if ( other->mDates.contains( type ) && other->mDates.value( type ).isValid() )
    {
      mDates.insert( type, other->mDates[type] );
    }
  }

}

bool QgsAbstractMetadataBase::equals( const QgsAbstractMetadataBase &metadataOther )  const
{
  return ( ( mIdentifier == metadataOther.mIdentifier ) &&
           ( mParentIdentifier == metadataOther.mParentIdentifier ) &&
           ( mLanguage == metadataOther.mLanguage ) &&
           ( mType == metadataOther.mType ) &&
           ( mTitle == metadataOther.mTitle ) &&
           ( mAbstract == metadataOther.mAbstract ) &&
           ( mHistory == metadataOther.mHistory ) &&
           ( mKeywords == metadataOther.mKeywords ) &&
           ( mContacts == metadataOther.mContacts ) &&
           ( mLinks == metadataOther.mLinks ) &&
           ( mDates == metadataOther.mDates ) );
}


bool QgsAbstractMetadataBase::Contact::operator==( const QgsAbstractMetadataBase::Contact &other ) const
{
  return name == other.name &&
         organization == other.organization &&
         position == other.position &&
         addresses == other.addresses &&
         voice == other.voice &&
         fax == other.fax &&
         email == other.email &&
         role == other.role;
}

bool QgsAbstractMetadataBase::Link::operator==( const QgsAbstractMetadataBase::Link &other ) const
{
  return name == other.name &&
         type == other.type &&
         description == other.description &&
         url == other.url &&
         format == other.format &&
         mimeType == other.mimeType &&
         size == other.size;
}

bool QgsAbstractMetadataBase::Address::operator==( const QgsAbstractMetadataBase::Address &other ) const
{
  return type == other.type &&
         address == other.address &&
         city == other.city &&
         administrativeArea == other.administrativeArea &&
         postalCode == other.postalCode &&
         country == other.country;
}
