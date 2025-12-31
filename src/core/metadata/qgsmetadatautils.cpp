/***************************************************************************
                             qgsmetadatautils.cpp
                             -------------------
    begin                : April 2021
    copyright            : (C) 2021 by Nyall Dawson
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

#include "qgsmetadatautils.h"

#include "qgslayermetadata.h"

#include <QDomDocument>
#include <QTextDocumentFragment>

QgsLayerMetadata QgsMetadataUtils::convertFromEsri( const QDomDocument &document )
{
  QgsLayerMetadata metadata;
  const QDomElement metadataElem = document.firstChildElement( u"metadata"_s );

  const QDomElement esri = metadataElem.firstChildElement( u"Esri"_s );
  const QDomElement dataProperties = esri.firstChildElement( u"DataProperties"_s );
  const QDomElement itemProps = dataProperties.firstChildElement( u"itemProps"_s );
  metadata.setIdentifier( itemProps.firstChildElement( u"itemName"_s ).text() );

  const QDomElement dataIdInfo = metadataElem.firstChildElement( u"dataIdInfo"_s );
  const QDomElement idInfo = metadataElem.firstChildElement( u"idinfo"_s );

  // title
  const QDomElement idCitation = dataIdInfo.firstChildElement( u"idCitation"_s );
  const QString title = idCitation.firstChildElement( u"resTitle"_s ).text();
  metadata.setTitle( title );

  // if no explicit identifier we use the title
  if ( metadata.identifier().isEmpty()  && !title.isEmpty() )
    metadata.setIdentifier( title );

  const QDomElement citationDatesElement = idCitation.firstChildElement( u"date"_s ).toElement();
  if ( !citationDatesElement.isNull() )
  {
    {
      const QDomElement createDateElement = citationDatesElement.firstChildElement( u"createDate"_s ).toElement();
      if ( !createDateElement.isNull() )
      {
        metadata.setDateTime( Qgis::MetadataDateType::Created, QDateTime::fromString( createDateElement.text(), Qt::ISODate ) );
      }
    }
    {
      const QDomElement pubDateElement = citationDatesElement.firstChildElement( u"pubDate"_s ).toElement();
      if ( !pubDateElement.isNull() )
      {
        metadata.setDateTime( Qgis::MetadataDateType::Published, QDateTime::fromString( pubDateElement.text(), Qt::ISODate ) );
      }
    }
    {
      const QDomElement reviseDateElement = citationDatesElement.firstChildElement( u"reviseDate"_s ).toElement();
      if ( !reviseDateElement.isNull() )
      {
        metadata.setDateTime( Qgis::MetadataDateType::Revised, QDateTime::fromString( reviseDateElement.text(), Qt::ISODate ) );
      }
    }
    {
      const QDomElement supersededDateElement = citationDatesElement.firstChildElement( u"supersDate"_s ).toElement();
      if ( !supersededDateElement.isNull() )
      {
        metadata.setDateTime( Qgis::MetadataDateType::Superseded, QDateTime::fromString( supersededDateElement.text(), Qt::ISODate ) );
      }
    }
  }

  // abstract
  const QDomElement idAbs = dataIdInfo.firstChildElement( u"idAbs"_s );
  const QString abstractPlainText = QTextDocumentFragment::fromHtml( idAbs.text() ).toPlainText();
  metadata.setAbstract( abstractPlainText );

  // purpose
  const QDomElement idPurp = dataIdInfo.firstChildElement( u"idPurp"_s );
  const QString purposePlainText = QTextDocumentFragment::fromHtml( idPurp.text() ).toPlainText();
  if ( !metadata.abstract().isEmpty() )
    metadata.setAbstract( metadata.abstract() + u"\n\n"_s + purposePlainText );
  else
    metadata.setAbstract( purposePlainText );

  // older metadata format used "descript" element instead
  const QDomElement descript = idInfo.firstChildElement( u"descript"_s );
  if ( !descript.isNull() )
  {
    const QDomElement abstract = descript.firstChildElement( u"abstract"_s );
    const QString abstractPlainText = QTextDocumentFragment::fromHtml( abstract.text() ).toPlainText();
    if ( !abstractPlainText.isEmpty() )
    {
      if ( !metadata.abstract().isEmpty() )
        metadata.setAbstract( metadata.abstract() + u"\n\n"_s + abstractPlainText );
      else
        metadata.setAbstract( abstractPlainText );
    }

    const QDomElement purpose = descript.firstChildElement( u"purpose"_s );
    const QString purposePlainText = QTextDocumentFragment::fromHtml( purpose.text() ).toPlainText();
    if ( !purposePlainText.isEmpty() )
    {
      if ( !metadata.abstract().isEmpty() )
        metadata.setAbstract( metadata.abstract() + u"\n\n"_s + purposePlainText );
      else
        metadata.setAbstract( purposePlainText );
    }

    const QDomElement supplinf = descript.firstChildElement( u"supplinf"_s );
    const QString supplinfPlainText = QTextDocumentFragment::fromHtml( supplinf.text() ).toPlainText();
    if ( !supplinfPlainText.isEmpty() )
    {
      if ( !metadata.abstract().isEmpty() )
        metadata.setAbstract( metadata.abstract() + u"\n\n"_s + supplinfPlainText );
      else
        metadata.setAbstract( supplinfPlainText );
    }
  }

  // supplementary info
  const QDomElement suppInfo = dataIdInfo.firstChildElement( u"suppInfo"_s );
  const QString suppInfoPlainText = QTextDocumentFragment::fromHtml( suppInfo.text() ).toPlainText();
  if ( !suppInfoPlainText.isEmpty() )
  {
    if ( !metadata.abstract().isEmpty() )
      metadata.setAbstract( metadata.abstract() + u"\n\n"_s + suppInfoPlainText );
    else
      metadata.setAbstract( suppInfoPlainText );
  }

  // language
  const QDomElement dataLang = dataIdInfo.firstChildElement( u"dataLang"_s );
  const QDomElement languageCode = dataLang.firstChildElement( u"languageCode"_s );
  const QString language = languageCode.attribute( u"value"_s ).toUpper();
  metadata.setLanguage( language );

  // keywords
  QDomElement searchKeys = dataIdInfo.firstChildElement( u"searchKeys"_s );
  QStringList keywords;
  while ( !searchKeys.isNull() )
  {
    QDomElement keyword = searchKeys.firstChildElement( u"keyword"_s );
    while ( !keyword.isNull() )
    {
      keywords << keyword.text();
      keyword = keyword.nextSiblingElement( u"keyword"_s );
    }

    searchKeys = searchKeys.nextSiblingElement( u"searchKeys"_s );
  }

  // categories
  QDomElement themeKeys = dataIdInfo.firstChildElement( u"themeKeys"_s );
  QStringList categories;
  while ( !themeKeys.isNull() )
  {
    const QDomElement thesaName = themeKeys.firstChildElement( u"thesaName"_s );
    const QString thesaTitle = thesaName.firstChildElement( u"resTitle"_s ).text();

    const bool isSearchKeyWord = thesaTitle.compare( "Common Search Terms"_L1, Qt::CaseInsensitive ) == 0;

    QDomElement themeKeyword = themeKeys.firstChildElement( u"keyword"_s );
    while ( !themeKeyword.isNull() )
    {
      if ( isSearchKeyWord )
      {
        keywords.append( themeKeyword.text().split( ',' ) );
      }
      else
      {
        categories << themeKeyword.text();
      }
      themeKeyword = themeKeyword.nextSiblingElement( u"keyword"_s );
    }
    themeKeys = themeKeys.nextSiblingElement( u"themeKeys"_s );
  }

  // older xml format
  QDomElement keywordsElem = idInfo.firstChildElement( u"keywords"_s );
  while ( !keywordsElem.isNull() )
  {
    QDomElement theme = keywordsElem.firstChildElement( u"theme"_s );
    while ( !theme.isNull() )
    {
      categories << theme.firstChildElement( u"themekey"_s ).text();
      theme = theme.nextSiblingElement( u"theme"_s );
    }

    keywordsElem = keywordsElem.nextSiblingElement( u"keywords"_s );
  }

  if ( !categories.isEmpty() )
    metadata.setCategories( categories );

  if ( !keywords.empty() )
    metadata.addKeywords( QObject::tr( "Search keys" ), keywords );

  QgsLayerMetadata::Extent extent;

  // pubDate
  const QDomElement date = idCitation.firstChildElement( u"date"_s );
  const QString pubDate = date.firstChildElement( u"pubDate"_s ).text();
  const QDateTime publicationDate = QDateTime::fromString( pubDate, Qt::ISODate );
  if ( publicationDate.isValid() )
  {
    extent.setTemporalExtents( { publicationDate, QDateTime() } );
  }
  else
  {
    // older XML format
    QDomElement timeperd = idInfo.firstChildElement( u"timeperd"_s );
    while ( !timeperd.isNull() )
    {
      if ( timeperd.firstChildElement( u"current"_s ).text().compare( "publication date"_L1 ) == 0 )
      {
        const QDomElement timeinfo = timeperd.firstChildElement( u"timeinfo"_s );
        const QDomElement sngdate = timeinfo.firstChildElement( u"sngdate"_s );
        if ( !sngdate.isNull() )
        {
          const QDomElement caldate = sngdate.firstChildElement( u"caldate"_s );
          const QString caldateString = caldate.text();
          const QDateTime publicationDate = QDateTime::fromString( caldateString, u"MMMM yyyy"_s );
          if ( publicationDate.isValid() )
          {
            extent.setTemporalExtents( { publicationDate, QDateTime() } );
            break;
          }
        }
        const QDomElement rngdates = timeinfo.firstChildElement( u"rngdates"_s );
        if ( !rngdates.isNull() )
        {
          const QDomElement begdate = rngdates.firstChildElement( u"begdate"_s );
          const QDomElement enddate = rngdates.firstChildElement( u"enddate"_s );
          const QString begdateString = begdate.text();
          const QString enddateString = enddate.text();
          QDateTime begin;
          QDateTime end;
          for ( const QString format : { "yyyy-MM-dd", "dd/MM/yyyy" } )
          {
            if ( !begin.isValid() )
              begin = QDateTime::fromString( begdateString, format );
            if ( !end.isValid() )
              end = QDateTime::fromString( enddateString, format );
          }

          if ( begin.isValid() || end.isValid() )
          {
            extent.setTemporalExtents( {QgsDateTimeRange{ begin, end } } );
            break;
          }
        }
      }

      timeperd = timeperd.nextSiblingElement( u"timeperd"_s );
    }
  }

  //crs
  QgsCoordinateReferenceSystem crs;
  QDomElement refSysInfo = metadataElem.firstChildElement( u"refSysInfo"_s );
  while ( !refSysInfo.isNull() )
  {
    const QDomElement refSystem = refSysInfo.firstChildElement( u"RefSystem"_s );
    const QDomElement refSysID = refSystem.firstChildElement( u"refSysID"_s );
    const QDomElement identAuth = refSysID.firstChildElement( u"identAuth"_s );
    if ( !identAuth.isNull() )
    {
      if ( identAuth.firstChildElement( u"resTitle"_s ).text().compare( "EPSG Geodetic Parameter Dataset"_L1 ) == 0 )
      {
        const QString code = refSysID.firstChildElement( u"identCode"_s ).attribute( u"code"_s );
        crs = QgsCoordinateReferenceSystem( code );
      }
    }
    else
    {
      const QString code = refSysID.firstChildElement( u"identCode"_s ).attribute( u"code"_s );
      const QString auth = refSysID.firstChildElement( u"idCodeSpace"_s ).text();
      crs = QgsCoordinateReferenceSystem( u"%1:%2"_s.arg( auth, code ) );
    }

    if ( crs.isValid() )
    {
      metadata.setCrs( crs );
      break;
    }
    refSysInfo = refSysInfo.nextSiblingElement( u"refSysInfo"_s );
  }

  // extent
  QDomElement dataExt = dataIdInfo.firstChildElement( u"dataExt"_s );
  while ( !dataExt.isNull() )
  {
    const QDomElement geoEle = dataExt.firstChildElement( u"geoEle"_s );
    if ( !geoEle.isNull() )
    {
      const QDomElement geoBndBox = geoEle.firstChildElement( u"GeoBndBox"_s );
      const double west = geoBndBox.firstChildElement( u"westBL"_s ).text().toDouble();
      const double east = geoBndBox.firstChildElement( u"eastBL"_s ).text().toDouble();
      const double south = geoBndBox.firstChildElement( u"northBL"_s ).text().toDouble();
      const double north = geoBndBox.firstChildElement( u"southBL"_s ).text().toDouble();

      QgsLayerMetadata::SpatialExtent spatialExtent;
      spatialExtent.extentCrs = crs.isValid() ? crs : QgsCoordinateReferenceSystem( u"EPSG:4326"_s );
      spatialExtent.bounds = QgsBox3D( west, south, 0, east, north, 0 );

      extent.setSpatialExtents( { spatialExtent } );
      break;
    }
    dataExt = dataExt.nextSiblingElement( u"dataExt"_s );
  }

  metadata.setExtent( extent );

  // licenses, constraints
  QStringList licenses;
  QStringList rights;
  QgsLayerMetadata::ConstraintList constraints;
  QDomElement resConst = dataIdInfo.firstChildElement( u"resConst"_s );
  while ( !resConst.isNull() )
  {
    QDomElement legConsts = resConst.firstChildElement( u"LegConsts"_s );
    while ( !legConsts.isNull() )
    {
      const QString restrictCd = legConsts.firstChildElement( u"useConsts"_s ).firstChildElement( u"RestrictCd"_s ).attribute( u"value"_s );

      if ( restrictCd.compare( "005"_L1 ) == 0 )
      {
        licenses << QTextDocumentFragment::fromHtml( legConsts.firstChildElement( u"useLimit"_s ).text() ).toPlainText();
      }
      else if ( restrictCd.compare( "006"_L1 ) == 0 )
      {
        rights << QTextDocumentFragment::fromHtml( legConsts.firstChildElement( u"useLimit"_s ).text() ).toPlainText();
      }
      legConsts = legConsts.nextSiblingElement( u"LegConsts"_s );
    }

    QDomElement secConsts = resConst.firstChildElement( u"SecConsts"_s );
    while ( !secConsts.isNull() )
    {
      QgsLayerMetadata::Constraint constraint;
      constraint.type = QObject::tr( "Security constraints" );
      constraint.constraint = QTextDocumentFragment::fromHtml( secConsts.firstChildElement( u"userNote"_s ).text() ).toPlainText();
      constraints << constraint;
      secConsts = secConsts.nextSiblingElement( u"SecConsts"_s );
    }

    QDomElement consts = resConst.firstChildElement( u"Consts"_s );
    while ( !consts.isNull() )
    {
      QgsLayerMetadata::Constraint constraint;
      constraint.type = QObject::tr( "Limitations of use" );
      constraint.constraint = QTextDocumentFragment::fromHtml( consts.firstChildElement( u"useLimit"_s ).text() ).toPlainText();
      constraints << constraint;
      consts = consts.nextSiblingElement( u"Consts"_s );
    }

    resConst = resConst.nextSiblingElement( u"resConst"_s );
  }

  const QDomElement idCredit = dataIdInfo.firstChildElement( u"idCredit"_s );
  const QString credit = idCredit.text();
  if ( !credit.isEmpty() )
    rights << credit;

  // older xml format
  QDomElement accconst = idInfo.firstChildElement( u"accconst"_s );
  while ( !accconst.isNull() )
  {
    QgsLayerMetadata::Constraint constraint;
    constraint.type = QObject::tr( "Access" );
    constraint.constraint = QTextDocumentFragment::fromHtml( accconst.text() ).toPlainText();
    constraints << constraint;

    accconst = accconst.nextSiblingElement( u"accconst"_s );
  }
  QDomElement useconst = idInfo.firstChildElement( u"useconst"_s );
  while ( !useconst.isNull() )
  {
    rights << QTextDocumentFragment::fromHtml( useconst.text() ).toPlainText();
    useconst = useconst.nextSiblingElement( u"useconst"_s );
  }

  metadata.setLicenses( licenses );
  metadata.setRights( rights );
  metadata.setConstraints( constraints );

  // links
  const QDomElement distInfo = metadataElem.firstChildElement( u"distInfo"_s );
  const QDomElement distributor = distInfo.firstChildElement( u"distributor"_s );

  QDomElement distorTran = distributor.firstChildElement( u"distorTran"_s );
  while ( !distorTran.isNull() )
  {
    const QDomElement onLineSrc = distorTran.firstChildElement( u"onLineSrc"_s );
    if ( !onLineSrc.isNull() )
    {
      QgsAbstractMetadataBase::Link link;
      link.url = onLineSrc.firstChildElement( u"linkage"_s ).text();

      const QDomElement distorFormat = distributor.firstChildElement( u"distorFormat"_s );
      link.name = distorFormat.firstChildElement( u"formatName"_s ).text();
      link.type = distorFormat.firstChildElement( u"formatSpec"_s ).text();

      if ( link.type.isEmpty() )
      {
        // older xml format
        link.type = onLineSrc.firstChildElement( u"protocol"_s ).text();
      }
      metadata.addLink( link );
    }

    distorTran = distorTran.nextSiblingElement( u"distorTran"_s );
  }

  // lineage
  const QDomElement dqInfo = metadataElem.firstChildElement( u"dqInfo"_s );
  const QDomElement dataLineage = dqInfo.firstChildElement( u"dataLineage"_s );
  const QString statement = QTextDocumentFragment::fromHtml( dataLineage.firstChildElement( u"statement"_s ).text() ).toPlainText();
  if ( !statement.isEmpty() )
    metadata.addHistoryItem( statement );

  QDomElement dataSource = dataLineage.firstChildElement( u"dataSource"_s );
  while ( !dataSource.isNull() )
  {
    metadata.addHistoryItem( QObject::tr( "Data source: %1" ).arg( QTextDocumentFragment::fromHtml( dataSource.firstChildElement( u"srcDesc"_s ).text() ).toPlainText() ) );
    dataSource = dataSource.nextSiblingElement( u"dataSource"_s );
  }

  // contacts
  const QDomElement mdContact = metadataElem.firstChildElement( u"mdContact"_s );
  if ( !mdContact.isNull() )
  {
    QgsAbstractMetadataBase::Contact contact;
    contact.name = mdContact.firstChildElement( u"rpIndName"_s ).text();
    contact.organization = mdContact.firstChildElement( u"rpOrgName"_s ).text();
    contact.position = mdContact.firstChildElement( u"rpPosName"_s ).text();

    const QString role = mdContact.firstChildElement( u"role"_s ).firstChildElement( u"RoleCd"_s ).attribute( u"value"_s );
    if ( role == "007"_L1 )
      contact.role = QObject::tr( "Point of contact" );

    const QDomElement rpCntInfo = mdContact.firstChildElement( u"rpCntInfo"_s );
    contact.email = rpCntInfo.firstChildElement( u"cntAddress"_s ).firstChildElement( u"eMailAdd"_s ).text();
    contact.voice = rpCntInfo.firstChildElement( u"cntPhone"_s ).firstChildElement( u"voiceNum"_s ).text();

    QDomElement cntAddress = rpCntInfo.firstChildElement( u"cntAddress"_s );
    while ( !cntAddress.isNull() )
    {
      QgsAbstractMetadataBase::Address address;

      address.type = cntAddress.attribute( u"addressType"_s );
      address.address = cntAddress.firstChildElement( u"delPoint"_s ).text();
      address.city = cntAddress.firstChildElement( u"city"_s ).text();
      address.administrativeArea = cntAddress.firstChildElement( u"adminArea"_s ).text();
      address.postalCode = cntAddress.firstChildElement( u"postCode"_s ).text();
      address.country = cntAddress.firstChildElement( u"country"_s ).text();

      contact.addresses.append( address );

      cntAddress = cntAddress.nextSiblingElement( u"cntAddress"_s );
    }


    metadata.addContact( contact );
  }

  // older xml format
  const QDomElement ptcontac = idInfo.firstChildElement( u"ptcontac"_s );
  const QDomElement cntinfo = ptcontac.firstChildElement( u"cntinfo"_s );
  if ( !cntinfo.isNull() )
  {
    QgsAbstractMetadataBase::Contact contact;
    const QDomElement cntorgp = cntinfo.firstChildElement( u"cntorgp"_s );
    const QString org = cntorgp.firstChildElement( u"cntorg"_s ).text();

    contact.name = org;
    contact.organization = org;
    contact.role = QObject::tr( "Point of contact" );

    const QDomElement rpCntInfo = mdContact.firstChildElement( u"rpCntInfo"_s );
    contact.email = cntinfo.firstChildElement( u"cntemail"_s ).text();
    contact.fax = cntinfo.firstChildElement( u"cntfax"_s ).text();
    contact.voice = cntinfo.firstChildElement( u"cntvoice"_s ).text();

    QDomElement cntaddr = cntinfo.firstChildElement( u"cntaddr"_s );
    while ( !cntaddr.isNull() )
    {
      QgsAbstractMetadataBase::Address address;

      QDomElement addressElem = cntaddr.firstChildElement( u"address"_s );
      while ( !addressElem.isNull() )
      {
        const QString addressPart = addressElem.text();
        address.address = address.address.isEmpty() ? addressPart : address.address + '\n' + addressPart;
        addressElem = addressElem.nextSiblingElement( u"address"_s );
      }
      address.type = cntaddr.firstChildElement( u"addrtype"_s ).text();
      address.city = cntaddr.firstChildElement( u"city"_s ).text();
      address.administrativeArea = cntaddr.firstChildElement( u"state"_s ).text();
      address.postalCode = cntaddr.firstChildElement( u"postal"_s ).text();
      address.country = cntaddr.firstChildElement( u"country"_s ).text();

      contact.addresses.append( address );

      cntaddr = cntaddr.nextSiblingElement( u"cntaddr"_s );
    }

    metadata.addContact( contact );
  }

  return metadata;
}
