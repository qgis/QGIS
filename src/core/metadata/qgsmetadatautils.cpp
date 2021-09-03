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
  const QDomElement metadataElem = document.firstChildElement( QStringLiteral( "metadata" ) );

  const QDomElement esri = metadataElem.firstChildElement( QStringLiteral( "Esri" ) );
  const QDomElement dataProperties = esri.firstChildElement( QStringLiteral( "DataProperties" ) );
  const QDomElement itemProps = dataProperties.firstChildElement( QStringLiteral( "itemProps" ) );
  metadata.setIdentifier( itemProps.firstChildElement( QStringLiteral( "itemName" ) ).text() );

  const QDomElement dataIdInfo = metadataElem.firstChildElement( QStringLiteral( "dataIdInfo" ) );
  const QDomElement idInfo = metadataElem.firstChildElement( QStringLiteral( "idinfo" ) );

  // title
  const QDomElement idCitation = dataIdInfo.firstChildElement( QStringLiteral( "idCitation" ) );
  const QString title = idCitation.firstChildElement( QStringLiteral( "resTitle" ) ).text();
  metadata.setTitle( title );

  // if no explicit identifier we use the title
  if ( metadata.identifier().isEmpty()  && !title.isEmpty() )
    metadata.setIdentifier( title );

  // abstract
  const QDomElement idAbs = dataIdInfo.firstChildElement( QStringLiteral( "idAbs" ) );
  const QString abstractPlainText = QTextDocumentFragment::fromHtml( idAbs.text() ).toPlainText();
  metadata.setAbstract( abstractPlainText );

  // purpose
  const QDomElement idPurp = dataIdInfo.firstChildElement( QStringLiteral( "idPurp" ) );
  const QString purposePlainText = QTextDocumentFragment::fromHtml( idPurp.text() ).toPlainText();
  if ( !metadata.abstract().isEmpty() )
    metadata.setAbstract( metadata.abstract() + QStringLiteral( "\n\n" ) + purposePlainText );
  else
    metadata.setAbstract( purposePlainText );

  // older metadata format used "descript" element instead
  const QDomElement descript = idInfo.firstChildElement( QStringLiteral( "descript" ) );
  if ( !descript.isNull() )
  {
    const QDomElement abstract = descript.firstChildElement( QStringLiteral( "abstract" ) );
    const QString abstractPlainText = QTextDocumentFragment::fromHtml( abstract.text() ).toPlainText();
    if ( !abstractPlainText.isEmpty() )
    {
      if ( !metadata.abstract().isEmpty() )
        metadata.setAbstract( metadata.abstract() + QStringLiteral( "\n\n" ) + abstractPlainText );
      else
        metadata.setAbstract( abstractPlainText );
    }

    const QDomElement purpose = descript.firstChildElement( QStringLiteral( "purpose" ) );
    const QString purposePlainText = QTextDocumentFragment::fromHtml( purpose.text() ).toPlainText();
    if ( !purposePlainText.isEmpty() )
    {
      if ( !metadata.abstract().isEmpty() )
        metadata.setAbstract( metadata.abstract() + QStringLiteral( "\n\n" ) + purposePlainText );
      else
        metadata.setAbstract( purposePlainText );
    }

    const QDomElement supplinf = descript.firstChildElement( QStringLiteral( "supplinf" ) );
    const QString supplinfPlainText = QTextDocumentFragment::fromHtml( supplinf.text() ).toPlainText();
    if ( !supplinfPlainText.isEmpty() )
    {
      if ( !metadata.abstract().isEmpty() )
        metadata.setAbstract( metadata.abstract() + QStringLiteral( "\n\n" ) + supplinfPlainText );
      else
        metadata.setAbstract( supplinfPlainText );
    }
  }

  // supplementary info
  const QDomElement suppInfo = dataIdInfo.firstChildElement( QStringLiteral( "suppInfo" ) );
  const QString suppInfoPlainText = QTextDocumentFragment::fromHtml( suppInfo.text() ).toPlainText();
  if ( !suppInfoPlainText.isEmpty() )
  {
    if ( !metadata.abstract().isEmpty() )
      metadata.setAbstract( metadata.abstract() + QStringLiteral( "\n\n" ) + suppInfoPlainText );
    else
      metadata.setAbstract( suppInfoPlainText );
  }

  // language
  const QDomElement dataLang = dataIdInfo.firstChildElement( QStringLiteral( "dataLang" ) );
  const QDomElement languageCode = dataLang.firstChildElement( QStringLiteral( "languageCode" ) );
  const QString language = languageCode.attribute( QStringLiteral( "value" ) ).toUpper();
  metadata.setLanguage( language );

  // keywords
  QDomElement searchKeys = dataIdInfo.firstChildElement( QStringLiteral( "searchKeys" ) );
  QStringList keywords;
  while ( !searchKeys.isNull() )
  {
    QDomElement keyword = searchKeys.firstChildElement( QStringLiteral( "keyword" ) );
    while ( !keyword.isNull() )
    {
      keywords << keyword.text();
      keyword = keyword.nextSiblingElement( QStringLiteral( "keyword" ) );
    }

    searchKeys = searchKeys.nextSiblingElement( QStringLiteral( "searchKeys" ) );
  }

  // categories
  QDomElement themeKeys = dataIdInfo.firstChildElement( QStringLiteral( "themeKeys" ) );
  QStringList categories;
  while ( !themeKeys.isNull() )
  {
    const QDomElement thesaName = themeKeys.firstChildElement( QStringLiteral( "thesaName" ) );
    const QString thesaTitle = thesaName.firstChildElement( QStringLiteral( "resTitle" ) ).text();

    const bool isSearchKeyWord = thesaTitle.compare( QLatin1String( "Common Search Terms" ), Qt::CaseInsensitive ) == 0;

    QDomElement themeKeyword = themeKeys.firstChildElement( QStringLiteral( "keyword" ) );
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
      themeKeyword = themeKeyword.nextSiblingElement( QStringLiteral( "keyword" ) );
    }
    themeKeys = themeKeys.nextSiblingElement( QStringLiteral( "themeKeys" ) );
  }

  // older xml format
  QDomElement keywordsElem = idInfo.firstChildElement( QStringLiteral( "keywords" ) );
  while ( !keywordsElem.isNull() )
  {
    QDomElement theme = keywordsElem.firstChildElement( QStringLiteral( "theme" ) );
    while ( !theme.isNull() )
    {
      categories << theme.firstChildElement( QStringLiteral( "themekey" ) ).text();
      theme = theme.nextSiblingElement( QStringLiteral( "theme" ) );
    }

    keywordsElem = keywordsElem.nextSiblingElement( QStringLiteral( "keywords" ) );
  }

  if ( !categories.isEmpty() )
    metadata.setCategories( categories );

  if ( !keywords.empty() )
    metadata.addKeywords( QObject::tr( "Search keys" ), keywords );

  QgsLayerMetadata::Extent extent;

  // pubDate
  const QDomElement date = idCitation.firstChildElement( QStringLiteral( "date" ) );
  const QString pubDate = date.firstChildElement( QStringLiteral( "pubDate" ) ).text();
  const QDateTime publicationDate = QDateTime::fromString( pubDate, Qt::ISODate );
  if ( publicationDate.isValid() )
  {
    extent.setTemporalExtents( { publicationDate, QDateTime() } );
  }
  else
  {
    // older XML format
    QDomElement timeperd = idInfo.firstChildElement( QStringLiteral( "timeperd" ) );
    while ( !timeperd.isNull() )
    {
      if ( timeperd.firstChildElement( QStringLiteral( "current" ) ).text().compare( QLatin1String( "publication date" ) ) == 0 )
      {
        const QDomElement timeinfo = timeperd.firstChildElement( QStringLiteral( "timeinfo" ) );
        const QDomElement sngdate = timeinfo.firstChildElement( QStringLiteral( "sngdate" ) );
        if ( !sngdate.isNull() )
        {
          const QDomElement caldate = sngdate.firstChildElement( QStringLiteral( "caldate" ) );
          const QString caldateString = caldate.text();
          const QDateTime publicationDate = QDateTime::fromString( caldateString, QStringLiteral( "MMMM yyyy" ) );
          if ( publicationDate.isValid() )
          {
            extent.setTemporalExtents( { publicationDate, QDateTime() } );
            break;
          }
        }
        const QDomElement rngdates = timeinfo.firstChildElement( QStringLiteral( "rngdates" ) );
        if ( !rngdates.isNull() )
        {
          const QDomElement begdate = rngdates.firstChildElement( QStringLiteral( "begdate" ) );
          const QDomElement enddate = rngdates.firstChildElement( QStringLiteral( "enddate" ) );
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

      timeperd = timeperd.nextSiblingElement( QStringLiteral( "timeperd" ) );
    }
  }

  //crs
  QgsCoordinateReferenceSystem crs;
  QDomElement refSysInfo = metadataElem.firstChildElement( QStringLiteral( "refSysInfo" ) );
  while ( !refSysInfo.isNull() )
  {
    const QDomElement refSystem = refSysInfo.firstChildElement( QStringLiteral( "RefSystem" ) );
    const QDomElement refSysID = refSystem.firstChildElement( QStringLiteral( "refSysID" ) );
    const QDomElement identAuth = refSysID.firstChildElement( QStringLiteral( "identAuth" ) );
    if ( !identAuth.isNull() )
    {
      if ( identAuth.firstChildElement( QStringLiteral( "resTitle" ) ).text().compare( QLatin1String( "EPSG Geodetic Parameter Dataset" ) ) == 0 )
      {
        const QString code = refSysID.firstChildElement( QStringLiteral( "identCode" ) ).attribute( QStringLiteral( "code" ) );
        crs = QgsCoordinateReferenceSystem( code );
      }
    }
    else
    {
      const QString code = refSysID.firstChildElement( QStringLiteral( "identCode" ) ).attribute( QStringLiteral( "code" ) );
      const QString auth = refSysID.firstChildElement( QStringLiteral( "idCodeSpace" ) ).text();
      crs = QgsCoordinateReferenceSystem( QStringLiteral( "%1:%2" ).arg( auth, code ) );
    }

    if ( crs.isValid() )
    {
      metadata.setCrs( crs );
      break;
    }
    refSysInfo = refSysInfo.nextSiblingElement( QStringLiteral( "refSysInfo" ) );
  }

  // extent
  QDomElement dataExt = dataIdInfo.firstChildElement( QStringLiteral( "dataExt" ) );
  while ( !dataExt.isNull() )
  {
    const QDomElement geoEle = dataExt.firstChildElement( QStringLiteral( "geoEle" ) );
    if ( !geoEle.isNull() )
    {
      const QDomElement geoBndBox = geoEle.firstChildElement( QStringLiteral( "GeoBndBox" ) );
      const double west = geoBndBox.firstChildElement( QStringLiteral( "westBL" ) ).text().toDouble();
      const double east = geoBndBox.firstChildElement( QStringLiteral( "eastBL" ) ).text().toDouble();
      const double south = geoBndBox.firstChildElement( QStringLiteral( "northBL" ) ).text().toDouble();
      const double north = geoBndBox.firstChildElement( QStringLiteral( "southBL" ) ).text().toDouble();

      QgsLayerMetadata::SpatialExtent spatialExtent;
      spatialExtent.extentCrs = crs.isValid() ? crs : QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) );
      spatialExtent.bounds = QgsBox3d( west, south, 0, east, north, 0 );

      extent.setSpatialExtents( { spatialExtent } );
      break;
    }
    dataExt = dataExt.nextSiblingElement( QStringLiteral( "dataExt" ) );
  }

  metadata.setExtent( extent );

  // licenses, constraints
  QStringList licenses;
  QStringList rights;
  QgsLayerMetadata::ConstraintList constraints;
  QDomElement resConst = dataIdInfo.firstChildElement( QStringLiteral( "resConst" ) );
  while ( !resConst.isNull() )
  {
    QDomElement legConsts = resConst.firstChildElement( QStringLiteral( "LegConsts" ) );
    while ( !legConsts.isNull() )
    {
      const QString restrictCd = legConsts.firstChildElement( QStringLiteral( "useConsts" ) ).firstChildElement( QStringLiteral( "RestrictCd" ) ).attribute( QStringLiteral( "value" ) );

      if ( restrictCd.compare( QLatin1String( "005" ) ) == 0 )
      {
        licenses << QTextDocumentFragment::fromHtml( legConsts.firstChildElement( QStringLiteral( "useLimit" ) ).text() ).toPlainText();
      }
      else if ( restrictCd.compare( QLatin1String( "006" ) ) == 0 )
      {
        rights << QTextDocumentFragment::fromHtml( legConsts.firstChildElement( QStringLiteral( "useLimit" ) ).text() ).toPlainText();
      }
      legConsts = legConsts.nextSiblingElement( QStringLiteral( "LegConsts" ) );
    }

    QDomElement secConsts = resConst.firstChildElement( QStringLiteral( "SecConsts" ) );
    while ( !secConsts.isNull() )
    {
      QgsLayerMetadata::Constraint constraint;
      constraint.type = QObject::tr( "Security constraints" );
      constraint.constraint = QTextDocumentFragment::fromHtml( secConsts.firstChildElement( QStringLiteral( "userNote" ) ).text() ).toPlainText();
      constraints << constraint;
      secConsts = secConsts.nextSiblingElement( QStringLiteral( "SecConsts" ) );
    }

    QDomElement consts = resConst.firstChildElement( QStringLiteral( "Consts" ) );
    while ( !consts.isNull() )
    {
      QgsLayerMetadata::Constraint constraint;
      constraint.type = QObject::tr( "Limitations of use" );
      constraint.constraint = QTextDocumentFragment::fromHtml( consts.firstChildElement( QStringLiteral( "useLimit" ) ).text() ).toPlainText();
      constraints << constraint;
      consts = consts.nextSiblingElement( QStringLiteral( "Consts" ) );
    }

    resConst = resConst.nextSiblingElement( QStringLiteral( "resConst" ) );
  }

  const QDomElement idCredit = dataIdInfo.firstChildElement( QStringLiteral( "idCredit" ) );
  const QString credit = idCredit.text();
  if ( !credit.isEmpty() )
    rights << credit;

  // older xml format
  QDomElement accconst = idInfo.firstChildElement( QStringLiteral( "accconst" ) );
  while ( !accconst.isNull() )
  {
    QgsLayerMetadata::Constraint constraint;
    constraint.type = QObject::tr( "Access" );
    constraint.constraint = QTextDocumentFragment::fromHtml( accconst.text() ).toPlainText();
    constraints << constraint;

    accconst = accconst.nextSiblingElement( QStringLiteral( "accconst" ) );
  }
  QDomElement useconst = idInfo.firstChildElement( QStringLiteral( "useconst" ) );
  while ( !useconst.isNull() )
  {
    rights << QTextDocumentFragment::fromHtml( useconst.text() ).toPlainText();
    useconst = useconst.nextSiblingElement( QStringLiteral( "useconst" ) );
  }

  metadata.setLicenses( licenses );
  metadata.setRights( rights );
  metadata.setConstraints( constraints );

  // links
  const QDomElement distInfo = metadataElem.firstChildElement( QStringLiteral( "distInfo" ) );
  const QDomElement distributor = distInfo.firstChildElement( QStringLiteral( "distributor" ) );

  QDomElement distorTran = distributor.firstChildElement( QStringLiteral( "distorTran" ) );
  while ( !distorTran.isNull() )
  {
    const QDomElement onLineSrc = distorTran.firstChildElement( QStringLiteral( "onLineSrc" ) );
    if ( !onLineSrc.isNull() )
    {
      QgsAbstractMetadataBase::Link link;
      link.url = onLineSrc.firstChildElement( QStringLiteral( "linkage" ) ).text();

      const QDomElement distorFormat = distributor.firstChildElement( QStringLiteral( "distorFormat" ) );
      link.name = distorFormat.firstChildElement( QStringLiteral( "formatName" ) ).text();
      link.type = distorFormat.firstChildElement( QStringLiteral( "formatSpec" ) ).text();

      if ( link.type.isEmpty() )
      {
        // older xml format
        link.type = onLineSrc.firstChildElement( QStringLiteral( "protocol" ) ).text();
      }
      metadata.addLink( link );
    }

    distorTran = distorTran.nextSiblingElement( QStringLiteral( "distorTran" ) );
  }

  // lineage
  const QDomElement dqInfo = metadataElem.firstChildElement( QStringLiteral( "dqInfo" ) );
  const QDomElement dataLineage = dqInfo.firstChildElement( QStringLiteral( "dataLineage" ) );
  const QString statement = QTextDocumentFragment::fromHtml( dataLineage.firstChildElement( QStringLiteral( "statement" ) ).text() ).toPlainText();
  if ( !statement.isEmpty() )
    metadata.addHistoryItem( statement );

  QDomElement dataSource = dataLineage.firstChildElement( QStringLiteral( "dataSource" ) );
  while ( !dataSource.isNull() )
  {
    metadata.addHistoryItem( QObject::tr( "Data source: %1" ).arg( QTextDocumentFragment::fromHtml( dataSource.firstChildElement( QStringLiteral( "srcDesc" ) ).text() ).toPlainText() ) );
    dataSource = dataSource.nextSiblingElement( QStringLiteral( "dataSource" ) );
  }

  // contacts
  const QDomElement mdContact = metadataElem.firstChildElement( QStringLiteral( "mdContact" ) );
  if ( !mdContact.isNull() )
  {
    QgsAbstractMetadataBase::Contact contact;
    contact.name = mdContact.firstChildElement( QStringLiteral( "rpIndName" ) ).text();
    contact.organization = mdContact.firstChildElement( QStringLiteral( "rpOrgName" ) ).text();
    contact.position = mdContact.firstChildElement( QStringLiteral( "rpPosName" ) ).text();

    const QString role = mdContact.firstChildElement( QStringLiteral( "role" ) ).firstChildElement( QStringLiteral( "RoleCd" ) ).attribute( QStringLiteral( "value" ) );
    if ( role == QLatin1String( "007" ) )
      contact.role = QObject::tr( "Point of contact" );

    const QDomElement rpCntInfo = mdContact.firstChildElement( QStringLiteral( "rpCntInfo" ) );
    contact.email = rpCntInfo.firstChildElement( QStringLiteral( "cntAddress" ) ).firstChildElement( QStringLiteral( "eMailAdd" ) ).text();
    contact.voice = rpCntInfo.firstChildElement( QStringLiteral( "cntPhone" ) ).firstChildElement( QStringLiteral( "voiceNum" ) ).text();

    QDomElement cntAddress = rpCntInfo.firstChildElement( QStringLiteral( "cntAddress" ) );
    while ( !cntAddress.isNull() )
    {
      QgsAbstractMetadataBase::Address address;

      address.type = cntAddress.attribute( QStringLiteral( "addressType" ) );
      address.address = cntAddress.firstChildElement( QStringLiteral( "delPoint" ) ).text();
      address.city = cntAddress.firstChildElement( QStringLiteral( "city" ) ).text();
      address.administrativeArea = cntAddress.firstChildElement( QStringLiteral( "adminArea" ) ).text();
      address.postalCode = cntAddress.firstChildElement( QStringLiteral( "postCode" ) ).text();
      address.country = cntAddress.firstChildElement( QStringLiteral( "country" ) ).text();

      contact.addresses.append( address );

      cntAddress = cntAddress.nextSiblingElement( QStringLiteral( "cntAddress" ) );
    }


    metadata.addContact( contact );
  }

  // older xml format
  const QDomElement ptcontac = idInfo.firstChildElement( QStringLiteral( "ptcontac" ) );
  const QDomElement cntinfo = ptcontac.firstChildElement( QStringLiteral( "cntinfo" ) );
  if ( !cntinfo.isNull() )
  {
    QgsAbstractMetadataBase::Contact contact;
    const QDomElement cntorgp = cntinfo.firstChildElement( QStringLiteral( "cntorgp" ) );
    const QString org = cntorgp.firstChildElement( QStringLiteral( "cntorg" ) ).text();

    contact.name = org;
    contact.organization = org;
    contact.role = QObject::tr( "Point of contact" );

    const QDomElement rpCntInfo = mdContact.firstChildElement( QStringLiteral( "rpCntInfo" ) );
    contact.email = cntinfo.firstChildElement( QStringLiteral( "cntemail" ) ).text();
    contact.fax = cntinfo.firstChildElement( QStringLiteral( "cntfax" ) ).text();
    contact.voice = cntinfo.firstChildElement( QStringLiteral( "cntvoice" ) ).text();

    QDomElement cntaddr = cntinfo.firstChildElement( QStringLiteral( "cntaddr" ) );
    while ( !cntaddr.isNull() )
    {
      QgsAbstractMetadataBase::Address address;

      QDomElement addressElem = cntaddr.firstChildElement( QStringLiteral( "address" ) );
      while ( !addressElem.isNull() )
      {
        const QString addressPart = addressElem.text();
        address.address = address.address.isEmpty() ? addressPart : address.address + '\n' + addressPart;
        addressElem = addressElem.nextSiblingElement( QStringLiteral( "address" ) );
      }
      address.type = cntaddr.firstChildElement( QStringLiteral( "addrtype" ) ).text();
      address.city = cntaddr.firstChildElement( QStringLiteral( "city" ) ).text();
      address.administrativeArea = cntaddr.firstChildElement( QStringLiteral( "state" ) ).text();
      address.postalCode = cntaddr.firstChildElement( QStringLiteral( "postal" ) ).text();
      address.country = cntaddr.firstChildElement( QStringLiteral( "country" ) ).text();

      contact.addresses.append( address );

      cntaddr = cntaddr.nextSiblingElement( QStringLiteral( "cntaddr" ) );
    }

    metadata.addContact( contact );
  }

  return metadata;
}
