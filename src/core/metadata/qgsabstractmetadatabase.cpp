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
#include "qgstranslationcontext.h"

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
  if ( mKeywords.contains( u"gmd:topicCategory"_s ) )
  {
    return mKeywords.value( u"gmd:topicCategory"_s );
  }
  else
  {
    return QStringList();
  }
}

void QgsAbstractMetadataBase::setCategories( const QStringList &category )
{
  mKeywords.insert( u"gmd:topicCategory"_s, category );
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

bool QgsAbstractMetadataBase::readMetadataXml( const QDomElement &metadataElement, const QgsReadWriteContext &context )
{
  QDomNode mnl;
  QDomElement mne;

  // set identifier
  mnl = metadataElement.namedItem( u"identifier"_s );
  mIdentifier = mnl.toElement().text();

  // set parent identifier
  mnl = metadataElement.namedItem( u"parentidentifier"_s );
  mParentIdentifier = mnl.toElement().text();

  // set language
  mnl = metadataElement.namedItem( u"language"_s );
  mLanguage = mnl.toElement().text();

  // set type
  mnl = metadataElement.namedItem( u"type"_s );
  mType = mnl.toElement().text();

  // set title
  mnl = metadataElement.namedItem( u"title"_s );
  mTitle = mnl.toElement().text();

  // set abstract
  mnl = metadataElement.namedItem( u"abstract"_s );
  mAbstract = mnl.toElement().text();

  mType = context.projectTranslator()->translate( "metadata", mType );
  mTitle = context.projectTranslator()->translate( "metadata", mTitle );
  mAbstract = context.projectTranslator()->translate( "metadata", mAbstract );

  // set keywords
  const QDomNodeList keywords = metadataElement.elementsByTagName( u"keywords"_s );
  mKeywords.clear();
  for ( int i = 0; i < keywords.size(); i++ )
  {
    QStringList keywordsList;
    mnl = keywords.at( i );
    mne = mnl.toElement();

    const QDomNodeList el = mne.elementsByTagName( u"keyword"_s );
    for ( int j = 0; j < el.size(); j++ )
    {
      keywordsList.append( el.at( j ).toElement().text() );
    }
    addKeywords( mne.attribute( u"vocabulary"_s ), keywordsList );
  }

  // contact
  const QDomNodeList contactsList = metadataElement.elementsByTagName( u"contact"_s );
  mContacts.clear();
  for ( int i = 0; i < contactsList.size(); i++ )
  {
    mnl = contactsList.at( i );
    mne = mnl.toElement();

    QgsAbstractMetadataBase::Contact oneContact;
    oneContact.name = mne.namedItem( u"name"_s ).toElement().text();
    oneContact.organization = mne.namedItem( u"organization"_s ).toElement().text();
    oneContact.position = mne.namedItem( u"position"_s ).toElement().text();
    oneContact.voice = mne.namedItem( u"voice"_s ).toElement().text();
    oneContact.fax = mne.namedItem( u"fax"_s ).toElement().text();
    oneContact.email = mne.namedItem( u"email"_s ).toElement().text();
    oneContact.role = mne.namedItem( u"role"_s ).toElement().text();

    QList< QgsAbstractMetadataBase::Address > addresses;
    const QDomNodeList addressList = mne.elementsByTagName( u"contactAddress"_s );
    for ( int j = 0; j < addressList.size(); j++ )
    {
      const QDomElement addressElement = addressList.at( j ).toElement();
      QgsAbstractMetadataBase::Address oneAddress;
      oneAddress.address = addressElement.namedItem( u"address"_s ).toElement().text();
      oneAddress.administrativeArea = addressElement.namedItem( u"administrativearea"_s ).toElement().text();
      oneAddress.city = addressElement.namedItem( u"city"_s ).toElement().text();
      oneAddress.country = addressElement.namedItem( u"country"_s ).toElement().text();
      oneAddress.postalCode = addressElement.namedItem( u"postalcode"_s ).toElement().text();
      oneAddress.type = addressElement.namedItem( u"type"_s ).toElement().text();
      addresses << oneAddress;
    }
    oneContact.addresses = addresses;
    addContact( oneContact );
  }

  // links
  mnl = metadataElement.namedItem( u"links"_s );
  mne = mnl.toElement();
  mLinks.clear();
  const QDomNodeList el = mne.elementsByTagName( u"link"_s );
  for ( int i = 0; i < el.size(); i++ )
  {
    mne = el.at( i ).toElement();
    QgsAbstractMetadataBase::Link oneLink;
    oneLink.name = mne.attribute( u"name"_s );
    oneLink.type = mne.attribute( u"type"_s );
    oneLink.url = mne.attribute( u"url"_s );
    oneLink.description = mne.attribute( u"description"_s );
    oneLink.format = mne.attribute( u"format"_s );
    oneLink.mimeType = mne.attribute( u"mimeType"_s );
    oneLink.size = mne.attribute( u"size"_s );
    addLink( oneLink );
  }

  // history
  const QDomNodeList historyNodeList = metadataElement.elementsByTagName( u"history"_s );
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
    const QDomElement dateElement = metadataElement.firstChildElement( u"dates"_s );
    if ( !dateElement.isNull() )
    {
      const QDomNodeList dateNodeList = dateElement.elementsByTagName( u"date"_s );
      const QMetaEnum dateEnum = QMetaEnum::fromType<Qgis::MetadataDateType>();
      for ( int i = 0; i < dateNodeList.size(); i++ )
      {
        const QDomElement dateElement = dateNodeList.at( i ).toElement();
        const Qgis::MetadataDateType type = static_cast< Qgis::MetadataDateType >( dateEnum.keyToValue( dateElement.attribute( u"type"_s ).toStdString().c_str() ) );
        const QDateTime value = QDateTime::fromString( dateElement.attribute( u"value"_s ), Qt::ISODate );
        if ( value.isValid() && !value.isNull() )
          mDates.insert( type, value );
      }
    }
  }

  return true;
}

bool QgsAbstractMetadataBase::writeMetadataXml( QDomElement &metadataElement, QDomDocument &document, const QgsReadWriteContext & ) const
{
  // identifier
  QDomElement identifier = document.createElement( u"identifier"_s );
  const QDomText identifierText = document.createTextNode( mIdentifier );
  identifier.appendChild( identifierText );
  metadataElement.appendChild( identifier );

  // parent identifier
  QDomElement parentIdentifier = document.createElement( u"parentidentifier"_s );
  const QDomText parentIdentifierText = document.createTextNode( mParentIdentifier );
  parentIdentifier.appendChild( parentIdentifierText );
  metadataElement.appendChild( parentIdentifier );

  // language
  QDomElement language = document.createElement( u"language"_s );
  const QDomText languageText = document.createTextNode( mLanguage );
  language.appendChild( languageText );
  metadataElement.appendChild( language );

  // type
  QDomElement type = document.createElement( u"type"_s );
  const QDomText typeText = document.createTextNode( mType );
  type.appendChild( typeText );
  metadataElement.appendChild( type );

  // title
  QDomElement title = document.createElement( u"title"_s );
  const QDomText titleText = document.createTextNode( mTitle );
  title.appendChild( titleText );
  metadataElement.appendChild( title );

  // abstract
  QDomElement abstract = document.createElement( u"abstract"_s );
  const QDomText abstractText = document.createTextNode( mAbstract );
  abstract.appendChild( abstractText );
  metadataElement.appendChild( abstract );

  // keywords
  QMapIterator<QString, QStringList> i( mKeywords );
  while ( i.hasNext() )
  {
    i.next();
    QDomElement keywordsElement = document.createElement( u"keywords"_s );
    keywordsElement.setAttribute( u"vocabulary"_s, i.key() );
    const QStringList values = i.value();
    for ( const QString &kw : values )
    {
      QDomElement keyword = document.createElement( u"keyword"_s );
      const QDomText keywordText = document.createTextNode( kw );
      keyword.appendChild( keywordText );
      keywordsElement.appendChild( keyword );
    }
    metadataElement.appendChild( keywordsElement );
  }

  // contact
  for ( const QgsAbstractMetadataBase::Contact &contact : mContacts )
  {
    QDomElement contactElement = document.createElement( u"contact"_s );
    QDomElement nameElement = document.createElement( u"name"_s );
    QDomElement organizationElement = document.createElement( u"organization"_s );
    QDomElement positionElement = document.createElement( u"position"_s );
    QDomElement voiceElement = document.createElement( u"voice"_s );
    QDomElement faxElement = document.createElement( u"fax"_s );
    QDomElement emailElement = document.createElement( u"email"_s );
    QDomElement roleElement = document.createElement( u"role"_s );

    const QDomText nameText = document.createTextNode( contact.name );
    const QDomText orgaText = document.createTextNode( contact.organization );
    const QDomText positionText = document.createTextNode( contact.position );
    const QDomText voiceText = document.createTextNode( contact.voice );
    const QDomText faxText = document.createTextNode( contact.fax );
    const QDomText emailText = document.createTextNode( contact.email );
    const QDomText roleText = document.createTextNode( contact.role );

    for ( const QgsAbstractMetadataBase::Address &oneAddress : contact.addresses )
    {
      QDomElement addressElement = document.createElement( u"contactAddress"_s );
      QDomElement typeElement = document.createElement( u"type"_s );
      QDomElement addressDetailedElement = document.createElement( u"address"_s );
      QDomElement cityElement = document.createElement( u"city"_s );
      QDomElement administrativeAreaElement = document.createElement( u"administrativearea"_s );
      QDomElement postalCodeElement = document.createElement( u"postalcode"_s );
      QDomElement countryElement = document.createElement( u"country"_s );

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
  QDomElement links = document.createElement( u"links"_s );
  for ( const QgsAbstractMetadataBase::Link &link : mLinks )
  {
    QDomElement linkElement = document.createElement( u"link"_s );
    linkElement.setAttribute( u"name"_s, link.name );
    linkElement.setAttribute( u"type"_s, link.type );
    linkElement.setAttribute( u"url"_s, link.url );
    linkElement.setAttribute( u"description"_s, link.description );
    linkElement.setAttribute( u"format"_s, link.format );
    linkElement.setAttribute( u"mimeType"_s, link.mimeType );
    linkElement.setAttribute( u"size"_s, link.size );
    links.appendChild( linkElement );
  }
  metadataElement.appendChild( links );

  // history
  for ( const QString &history : mHistory )
  {
    QDomElement historyElement = document.createElement( u"history"_s );
    const QDomText historyText = document.createTextNode( history );
    historyElement.appendChild( historyText );
    metadataElement.appendChild( historyElement );
  }

  // dates
  {
    const QMetaEnum dateEnum = QMetaEnum::fromType<Qgis::MetadataDateType>();
    QDomElement datesElement = document.createElement( u"dates"_s );
    for ( int k = 0; k < dateEnum.keyCount(); k++ )
    {
      const Qgis::MetadataDateType type = static_cast< Qgis::MetadataDateType >( dateEnum.value( k ) );
      if ( mDates.contains( type ) && mDates.value( type ).isValid() )
      {
        QDomElement dateElement = document.createElement( u"date"_s );
        dateElement.setAttribute( u"type"_s, dateEnum.valueToKey( static_cast< int >( type ) ) );
        dateElement.setAttribute( u"value"_s, mDates.value( type ).toString( Qt::ISODate ) );
        datesElement.appendChild( dateElement );
      }
    }
    metadataElement.appendChild( datesElement );
  }

  return true;
}

void QgsAbstractMetadataBase::registerTranslations( QgsTranslationContext *translationContext ) const
{
  if ( !mTitle.isEmpty() )
  {
    translationContext->registerTranslation( u"metadata"_s, mTitle );
  }
  if ( !mType.isEmpty() )
  {
    translationContext->registerTranslation( u"metadata"_s, mType );
  }
  if ( !mAbstract.isEmpty() )
  {
    translationContext->registerTranslation( u"metadata"_s, mAbstract );
  }
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
