/***************************************************************************
    qgslayermetadataformatter.cpp
    ---------------------
    begin                : September 2017
    copyright            : (C) 2017 by Etienne Trimaille
    email                : etienne.trimaille at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgslayermetadataformatter.h"

#include "qgslayermetadata.h"
#include "qgsstringutils.h"

#include <QDateTime>
#include <QStringBuilder>

QgsLayerMetadataFormatter::QgsLayerMetadataFormatter( const QgsLayerMetadata &metadata )
  : mMetadata( metadata )
{
}

QString QgsLayerMetadataFormatter::accessSectionHtml() const
{
  QString myMetadata = u"<table class=\"list-view\">\n"_s;
  myMetadata += u"<tr><td class=\"highlight\">"_s + tr( "Fees" ) + u"</td><td>"_s + QgsStringUtils::insertLinks( mMetadata.fees() ) + u"</td></tr>\n"_s;
  myMetadata += u"<tr><td class=\"highlight\">"_s + tr( "Licenses" ) + u"</td><td>"_s + QgsStringUtils::insertLinks( mMetadata.licenses().join( "<br />"_L1 ) ) + u"</td></tr>\n"_s;
  myMetadata += u"<tr><td class=\"highlight\">"_s + tr( "Rights" ) + u"</td><td>"_s + QgsStringUtils::insertLinks( mMetadata.rights().join( "<br />"_L1 ) ) + u"</td></tr>\n"_s;
  myMetadata += u"<tr><td class=\"highlight\">"_s + tr( "Constraints" ) + u"</td><td>"_s;
  const QList<QgsLayerMetadata::Constraint> &constraints = mMetadata.constraints();
  bool notFirstRow = false;
  for ( const QgsLayerMetadata::Constraint &constraint : constraints )
  {
    if ( notFirstRow )
    {
      myMetadata += "<br />"_L1;
    }
    myMetadata += u"<strong>"_s + constraint.type + u": </strong>"_s + QgsStringUtils::insertLinks( constraint.constraint );
    notFirstRow = true;
  }
  myMetadata += "</td></tr>\n"_L1;
  myMetadata += "</table>\n"_L1;
  return myMetadata;
}

QString QgsLayerMetadataFormatter::contactsSectionHtml() const
{
  const QList<QgsAbstractMetadataBase::Contact> &contacts = mMetadata.contacts();
  QString myMetadata;
  if ( contacts.isEmpty() )
  {
    myMetadata += u"<p>"_s + tr( "No contact yet." ) + u"</p>"_s;
  }
  else
  {
    myMetadata += "<table width=\"100%\" class=\"tabular-view\">\n"_L1;
    myMetadata += "<tr><th>"_L1 + tr( "ID" ) + "</th><th>"_L1 + tr( "Name" ) + "</th><th>"_L1 + tr( "Position" ) + "</th><th>"_L1 + tr( "Organization" ) + "</th><th>"_L1 + tr( "Role" ) + "</th><th>"_L1 + tr( "Email" ) + "</th><th>"_L1 + tr( "Voice" ) + "</th><th>"_L1 + tr( "Fax" ) + "</th><th>"_L1 + tr( "Addresses" ) + "</th></tr>\n"_L1;
    int i = 1;
    for ( const QgsAbstractMetadataBase::Contact &contact : contacts )
    {
      QString rowClass;
      if ( i % 2 )
        rowClass = u"class=\"odd-row\""_s;
      myMetadata += "<tr "_L1 + rowClass + "><td>"_L1 + QString::number( i ) + "</td><td>"_L1 + contact.name + "</td><td>"_L1 + contact.position + "</td><td>"_L1 + contact.organization + "</td><td>"_L1 + contact.role + u"</td><td><a href=\"mailto:%1\">%1</a></td><td>"_s.arg( contact.email ) + contact.voice + "</td><td>"_L1 + contact.fax + "</td><td>"_L1;
      bool notFirstRow = false;
      for ( const QgsAbstractMetadataBase::Address &oneAddress : contact.addresses )
      {
        if ( notFirstRow )
        {
          myMetadata += "<br />\n"_L1;
        }
        if ( ! oneAddress.type.isEmpty() )
        {
          myMetadata += oneAddress.type + u"<br />"_s;
        }
        if ( ! oneAddress.address.isEmpty() )
        {
          myMetadata += oneAddress.address + u"<br />"_s;
        }
        if ( ! oneAddress.postalCode.isEmpty() )
        {
          myMetadata += oneAddress.postalCode + u"<br />"_s;
        }
        if ( ! oneAddress.city.isEmpty() )
        {
          myMetadata += oneAddress.city + u"<br />"_s;
        }
        if ( ! oneAddress.administrativeArea.isEmpty() )
        {
          myMetadata += oneAddress.administrativeArea + u"<br />"_s;
        }
        if ( ! oneAddress.country.isEmpty() )
        {
          myMetadata += oneAddress.country;
        }
        notFirstRow = true;
      }
      myMetadata += "</td></tr>\n"_L1;
      i++;
    }
    myMetadata += "</table>\n"_L1;
  }
  return myMetadata;
}

QString QgsLayerMetadataFormatter::extentSectionHtml( const bool showSpatialExtent ) const
{
  const QgsLayerMetadata::Extent extent = mMetadata.extent();
  bool notFirstRow = false;
  QString myMetadata = u"<table class=\"list-view\">\n"_s;
  if ( showSpatialExtent )
  {
    myMetadata += u"<tr><td class=\"highlight\">"_s + tr( "CRS" ) + u"</td><td>"_s;
    if ( mMetadata.crs().isValid() )
    {
      myMetadata += mMetadata.crs().userFriendlyIdentifier() + u" - "_s;
      if ( mMetadata.crs().isGeographic() )
        myMetadata += tr( "Geographic" );
      else
        myMetadata += tr( "Projected" );
    }
    myMetadata += "</td></tr>\n"_L1;

    myMetadata += u"<tr><td class=\"highlight\">"_s + tr( "Spatial Extent" ) + u"</td><td>"_s;
    const QList< QgsLayerMetadata::SpatialExtent > spatialExtents = extent.spatialExtents();
    for ( const QgsLayerMetadata::SpatialExtent &spatialExtent : spatialExtents )
    {
      if ( spatialExtent.bounds.isNull() || spatialExtent.bounds.toRectangle().isNull() )
        continue;

      if ( notFirstRow )
      {
        myMetadata += "<br />\n"_L1;
      }
      myMetadata += u"<strong>"_s + tr( "CRS" ) + u": </strong>"_s + spatialExtent.extentCrs.userFriendlyIdentifier() + u" - "_s;
      if ( spatialExtent.extentCrs.isGeographic() )
        myMetadata += tr( "Geographic" );
      else
        myMetadata += tr( "Projected" );
      myMetadata += "<br />"_L1;
      myMetadata += u"<strong>"_s + tr( "X Minimum:" ) + u" </strong>"_s +  qgsDoubleToString( spatialExtent.bounds.xMinimum() ) + u"<br />"_s;
      myMetadata += u"<strong>"_s + tr( "Y Minimum:" ) + u" </strong>"_s +  qgsDoubleToString( spatialExtent.bounds.yMinimum() ) + u"<br />"_s;
      myMetadata += u"<strong>"_s + tr( "X Maximum:" ) + u" </strong>"_s +  qgsDoubleToString( spatialExtent.bounds.xMaximum() ) + u"<br />"_s;
      myMetadata += u"<strong>"_s + tr( "Y Maximum:" ) + u" </strong>"_s +  qgsDoubleToString( spatialExtent.bounds.yMaximum() ) + u"<br />"_s;
      if ( spatialExtent.bounds.zMinimum() || spatialExtent.bounds.zMaximum() )
      {
        myMetadata += u"<strong>"_s + tr( "Z Minimum:" ) + u" </strong>"_s +  qgsDoubleToString( spatialExtent.bounds.zMinimum() ) + u"<br />"_s;
        myMetadata += u"<strong>"_s + tr( "Z Maximum:" ) + u" </strong>"_s +  qgsDoubleToString( spatialExtent.bounds.zMaximum() );
      }
      notFirstRow = true;
    }
    myMetadata += "</td></tr>\n"_L1;
  }
  myMetadata += u"<tr><td class=\"highlight\">"_s + tr( "Temporal Extent" ) + u"</td><td>"_s;
  const QList< QgsDateTimeRange > temporalExtents = extent.temporalExtents();
  notFirstRow = false;
  for ( const QgsDateTimeRange &temporalExtent : temporalExtents )
  {
    if ( notFirstRow )
    {
      myMetadata += "<br />\n"_L1;
    }
    if ( temporalExtent.isInstant() )
    {
      myMetadata += u"<strong>"_s + tr( "Instant:" ) + u" </strong>"_s + temporalExtent.begin().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate );
    }
    else
    {
      myMetadata += u"<strong>"_s + tr( "Start:" ) + u" </strong>"_s + temporalExtent.begin().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate ) + u"<br />\n"_s;
      myMetadata += u"<strong>"_s + tr( "End:" ) + u" </strong>"_s + temporalExtent.end().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate );
    }
    notFirstRow = true;
  }
  myMetadata += "</td></tr>\n"_L1;
  myMetadata += "</table>\n"_L1;
  return myMetadata;
}

QString QgsLayerMetadataFormatter::identificationSectionHtml() const
{
  QString myMetadata = u"<table class=\"list-view\">\n"_s;

  // Identifier
  myMetadata += u"<tr><td class=\"highlight\">"_s + tr( "Identifier" ) + u"</td><td>"_s + mMetadata.identifier() + u"</td></tr>\n"_s;

  // Parent Identifier
  myMetadata += u"<tr><td class=\"highlight\">"_s + tr( "Parent Identifier" ) + u"</td><td>"_s + mMetadata.parentIdentifier() + u"</td></tr>\n"_s;

  // Title
  myMetadata += u"<tr><td class=\"highlight\">"_s + tr( "Title" ) + u"</td><td>"_s + mMetadata.title() + u"</td></tr>\n"_s;

  // Type
  myMetadata += u"<tr><td class=\"highlight\">"_s + tr( "Type" ) + u"</td><td>"_s + mMetadata.type() + u"</td></tr>\n"_s;

  // Language
  myMetadata += u"<tr><td class=\"highlight\">"_s + tr( "Language" ) + u"</td><td>"_s + mMetadata.language() + u"</td></tr>\n"_s;

  // Abstract
  myMetadata += u"<tr><td class=\"highlight\">"_s + tr( "Abstract" ) + u"</td><td>"_s + QgsStringUtils::insertLinks( mMetadata.abstract() ).replace( '\n', "<br>"_L1 ) + u"</td></tr>\n"_s;

  // Categories
  myMetadata += u"<tr><td class=\"highlight\">"_s + tr( "Categories" ) + u"</td><td>"_s + mMetadata.categories().join( ", "_L1 ) + u"</td></tr>\n"_s;

  // Keywords
  myMetadata += u"<tr><td class=\"highlight\">"_s + tr( "Keywords" ) + u"</td><td>\n"_s;
  QMapIterator<QString, QStringList> i( mMetadata.keywords() );
  if ( i.hasNext() )
  {
    myMetadata += "<table width=\"100%\" class=\"tabular-view\">\n"_L1;
    myMetadata += "<tr><th>"_L1 + tr( "Vocabulary" ) + "</th><th>"_L1 + tr( "Items" ) + "</th></tr>\n"_L1;
    int j = 1;
    while ( i.hasNext() )
    {
      i.next();
      QString rowClass;
      if ( j % 2 )
        rowClass = u"class=\"odd-row\""_s;
      myMetadata += "<tr "_L1 + rowClass + "><td>"_L1 + i.key() + "</td><td>"_L1 + i.value().join( ", "_L1 ) + "</td></tr>\n"_L1;
      j++;
    }
    myMetadata += "</table>\n"_L1; // End keywords table
  }
  myMetadata += "</td></tr>\n"_L1; // End of keywords row
  myMetadata += "</table>\n"_L1; // End identification table
  return myMetadata;
}

QString QgsLayerMetadataFormatter::historySectionHtml() const
{
  QString myMetadata;
  const QStringList historyItems = mMetadata.history();
  if ( historyItems.isEmpty() )
  {
    myMetadata += u"<p>"_s + tr( "No history yet." ) + u"</p>\n"_s;
  }
  else
  {
    myMetadata += "<table width=\"100%\" class=\"tabular-view\">\n"_L1;
    myMetadata += "<tr><th>"_L1 + tr( "ID" ) + "</th><th>"_L1 + tr( "Action" ) + "</th></tr>\n"_L1;
    int i = 1;
    for ( const QString &history : historyItems )
    {
      QString rowClass;
      if ( i % 2 )
        rowClass = u"class=\"odd-row\""_s;
      myMetadata += "<tr "_L1 + rowClass + "><td width=\"5%\">"_L1 + QString::number( i ) + "</td><td>"_L1 + QgsStringUtils::insertLinks( history ) + "</td></tr>\n"_L1;
      i++;
    }
    myMetadata += "</table>\n"_L1;
  }
  return myMetadata;
}

QString QgsLayerMetadataFormatter::linksSectionHtml() const
{
  QString myMetadata;
  const QList<QgsAbstractMetadataBase::Link> &links = mMetadata.links();
  if ( links.isEmpty() )
  {
    myMetadata += u"<p>"_s + tr( "No links yet." ) + u"</p>\n"_s;
  }
  else
  {
    myMetadata += "<table width=\"100%\" class=\"tabular-view\">\n"_L1;
    myMetadata += "<tr><th>"_L1 + tr( "ID" ) + "</th><th>"_L1 + tr( "Name" ) + "</th><th>"_L1 + tr( "Type" ) + "</th><th>"_L1 + tr( "URL" ) + "</th><th>"_L1 + tr( "Description" ) + "</th><th>"_L1 + tr( "Format" ) + "</th><th>"_L1 + tr( "MIME Type" ) + "</th><th>"_L1 + tr( "Size" ) + "</th></tr>\n"_L1;
    int i = 1;
    for ( const QgsAbstractMetadataBase::Link &link : links )
    {
      QString rowClass;
      if ( i % 2 )
        rowClass = u"class=\"odd-row\""_s;
      myMetadata += "<tr "_L1 + rowClass + "><td>"_L1 + QString::number( i ) + "</td><td>"_L1 + link.name + "</td><td>"_L1 + link.type + u"</td><td><a href=\"%1\">%1</a></td><td>"_s.arg( link.url ) + link.description + "</td><td>"_L1 + link.format + "</td><td>"_L1 + link.mimeType + "</td><td>"_L1 + link.size + "</td></tr>\n"_L1;
      i++;
    }
    myMetadata += "</table>\n"_L1;
  }
  return myMetadata;
}
