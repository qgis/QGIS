#include <QStringBuilder>

#include "qgslayermetadataformatter.h"
#include "qgslayermetadata.h"


QgsLayerMetadataFormatter::QgsLayerMetadataFormatter( const QgsLayerMetadata &metadata )
  : mMetadata( metadata )
{
}

QString QgsLayerMetadataFormatter::accessSectionHtml() const
{
  QString myMetadata = QStringLiteral( "<table class=\"list-view\">\n" );
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + QObject::tr( "Fees" ) + QStringLiteral( "</td><td>" ) + mMetadata.fees() + QStringLiteral( "</td></tr>\n" );
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + QObject::tr( "Licenses" ) + QStringLiteral( "</td><td>" ) + mMetadata.licenses().join( QStringLiteral( "<br />" ) ) + QStringLiteral( "</td></tr>\n" );
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + QObject::tr( "Rights" ) + QStringLiteral( "</td><td>" ) + mMetadata.rights().join( QStringLiteral( "<br />" ) ) + QStringLiteral( "</td></tr>\n" );
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + QObject::tr( "Constraints" ) + QStringLiteral( "</td><td>" );
  const QList<QgsLayerMetadata::Constraint> &constraints = mMetadata.constraints();
  bool notFirstRow = false;
  for ( const QgsLayerMetadata::Constraint &constraint : constraints )
  {
    if ( notFirstRow )
    {
      myMetadata += QLatin1String( "<br />" );
    }
    myMetadata += QStringLiteral( "<strong>" ) + constraint.type + QStringLiteral( ": </strong>" ) + constraint.constraint;
    notFirstRow = true;
  }
  myMetadata += QLatin1String( "</td></tr>\n" );
  mMetadata.rights().join( QStringLiteral( "<br />" ) ) + QStringLiteral( "</td></tr>\n" );
  myMetadata += QLatin1String( "</table>\n" );
  return myMetadata;
}

QString QgsLayerMetadataFormatter::contactsSectionHtml() const
{
  const QList<QgsLayerMetadata::Contact> &contacts = mMetadata.contacts();
  QString myMetadata;
  if ( contacts.isEmpty() )
  {
    myMetadata += QStringLiteral( "<p>" ) + QObject::tr( "No contact yet." ) + QStringLiteral( "</p>" );
  }
  else
  {
    myMetadata = QStringLiteral( "<table width=\"100%\" class=\"tabular-view\">\n" );
    myMetadata += "<tr><th>" + QObject::tr( "ID" ) + "</th><th>" + QObject::tr( "Name" ) + "</th><th>" + QObject::tr( "Position" ) + "</th><th>" + QObject::tr( "Organization" ) + "</th><th>" + QObject::tr( "Role" ) + "</th><th>" + QObject::tr( "Email" ) + "</th><th>" + QObject::tr( "Voice" ) + "</th><th>" + QObject::tr( "Fax" ) + "</th><th>" + QObject::tr( "Addresses" ) + "</th></tr>\n";
    int i = 1;
    for ( const QgsLayerMetadata::Contact &contact : contacts )
    {
      QString rowClass;
      if ( i % 2 )
        rowClass = QStringLiteral( "class=\"odd-row\"" );
      myMetadata += "<tr " + rowClass + "><td>" + QString::number( i ) + "</td><td>" + contact.name + "</td><td>" + contact.position + "</td><td>" + contact.organization + "</td><td>" + contact.role + "</td><td>" + contact.email + "</td><td>" + contact.voice + "</td><td>" + contact.fax + "</td><td>";
      bool notFirstRow = false;
      for ( const QgsLayerMetadata::Address &oneAddress : contact.addresses )
      {
        if ( notFirstRow )
        {
          myMetadata += QLatin1String( "<br />\n" );
        }
        if ( ! oneAddress.type.isEmpty() )
        {
          myMetadata += oneAddress.type + QStringLiteral( "<br />" );
        }
        if ( ! oneAddress.address.isEmpty() )
        {
          myMetadata += oneAddress.address + QStringLiteral( "<br />" );
        }
        if ( ! oneAddress.postalCode.isEmpty() )
        {
          myMetadata += oneAddress.postalCode + QStringLiteral( "<br />" );
        }
        if ( ! oneAddress.city.isEmpty() )
        {
          myMetadata += oneAddress.city + QStringLiteral( "<br />" );
        }
        if ( ! oneAddress.administrativeArea.isEmpty() )
        {
          myMetadata += oneAddress.administrativeArea + QStringLiteral( "<br />" );
        }
        if ( ! oneAddress.country.isEmpty() )
        {
          myMetadata += oneAddress.country;
        }
        notFirstRow = true;
      }
      myMetadata += QLatin1String( "</td></tr>\n" );
      i++;
    }
    myMetadata += QLatin1String( "</table>\n" );
  }
  return myMetadata;
}

QString QgsLayerMetadataFormatter::extentSectionHtml() const
{
  QString myMetadata = QStringLiteral( "<table class=\"list-view\">\n" );
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + QObject::tr( "CRS" ) + QStringLiteral( "</td><td>" ) + mMetadata.crs().authid() + QStringLiteral( " - " );
  myMetadata += mMetadata.crs().description() + QStringLiteral( " - " );
  if ( mMetadata.crs().isGeographic() )
    myMetadata += QObject::tr( "Geographic" );
  else
    myMetadata += QObject::tr( "Projected" );
  myMetadata += QLatin1String( "</td></tr>\n" );

  const QgsLayerMetadata::Extent extent = mMetadata.extent();
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + QObject::tr( "Spatial Extent" ) + QStringLiteral( "</td><td>" );
  const QList< QgsLayerMetadata::SpatialExtent > spatialExtents = extent.spatialExtents();
  bool notFirstRow = false;
  for ( const QgsLayerMetadata::SpatialExtent &spatialExtent : spatialExtents )
  {
    if ( notFirstRow )
    {
      myMetadata += QLatin1String( "<br />\n" );
    }
    myMetadata += QStringLiteral( "<strong>" ) + QObject::tr( "CRS" ) + QStringLiteral( ": </strong>" ) + spatialExtent.extentCrs.authid() + QStringLiteral( " - " );
    myMetadata += spatialExtent.extentCrs.description() + QStringLiteral( " - " );
    if ( spatialExtent.extentCrs.isGeographic() )
      myMetadata += QObject::tr( "Geographic" );
    else
      myMetadata += QObject::tr( "Projected" );
    myMetadata += QStringLiteral( "<br />" );
    myMetadata += QStringLiteral( "<strong>" ) + QObject::tr( "X Minimum" ) + QStringLiteral( ": </strong>" ) +  qgsDoubleToString( spatialExtent.bounds.xMinimum() ) + QStringLiteral( "<br />" );
    myMetadata += QStringLiteral( "<strong>" ) + QObject::tr( "Y Minimum" ) + QStringLiteral( ": </strong>" ) +  qgsDoubleToString( spatialExtent.bounds.yMinimum() ) + QStringLiteral( "<br />" );
    myMetadata += QStringLiteral( "<strong>" ) + QObject::tr( "X Maximum" ) + QStringLiteral( ": </strong>" ) +  qgsDoubleToString( spatialExtent.bounds.xMaximum() ) + QStringLiteral( "<br />" );
    myMetadata += QStringLiteral( "<strong>" ) + QObject::tr( "Y Maximum" ) + QStringLiteral( ": </strong>" ) +  qgsDoubleToString( spatialExtent.bounds.yMaximum() ) + QStringLiteral( "<br />" );
    if ( spatialExtent.bounds.zMinimum() || spatialExtent.bounds.zMinimum() )
    {
      myMetadata += QStringLiteral( "<strong>" ) + QObject::tr( "Z Minimum" ) + QStringLiteral( ": </strong>" ) +  qgsDoubleToString( spatialExtent.bounds.zMinimum() ) + QStringLiteral( "<br />" );
      myMetadata += QStringLiteral( "<strong>" ) + QObject::tr( "Z Maximum" ) + QStringLiteral( ": </strong>" ) +  qgsDoubleToString( spatialExtent.bounds.zMaximum() );
    }
    notFirstRow = true;
  }
  myMetadata += QLatin1String( "</td></tr>\n" );
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + QObject::tr( "Temporal Extent" ) + QStringLiteral( "</td><td>" );
  const QList< QgsDateTimeRange > temporalExtents = extent.temporalExtents();
  notFirstRow = false;
  for ( const QgsDateTimeRange &temporalExtent : temporalExtents )
  {
    if ( notFirstRow )
    {
      myMetadata += QLatin1String( "<br />\n" );
    }
    if ( temporalExtent.isInstant() )
    {
      myMetadata += QStringLiteral( "<strong>" ) + QObject::tr( "Instant" ) + QStringLiteral( ": </strong>" ) + temporalExtent.begin().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate );
    }
    else
    {
      myMetadata += QStringLiteral( "<strong>" ) + QObject::tr( "Start" ) + QStringLiteral( ": </strong>" ) + temporalExtent.begin().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate ) + QStringLiteral( "<br />\n" );
      myMetadata += QStringLiteral( "<strong>" ) + QObject::tr( "End" ) + QStringLiteral( ": </strong>" ) + temporalExtent.end().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate );
    }
    notFirstRow = true;
  }
  myMetadata += QLatin1String( "</td></tr>\n" );
  myMetadata += QLatin1String( "</table>\n" );
  return myMetadata;
}

QString QgsLayerMetadataFormatter::identificationSectionHtml() const
{
  QString myMetadata = QStringLiteral( "<table class=\"list-view\">\n" );

  // Identifier
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + QObject::tr( "Identifier" ) + QStringLiteral( "</td><td>" ) + mMetadata.identifier() + QStringLiteral( "</td></tr>\n" );

  // Parent Identifier
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + QObject::tr( "Parent Identifier" ) + QStringLiteral( "</td><td>" ) + mMetadata.parentIdentifier() + QStringLiteral( "</td></tr>\n" );

  // Title
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + QObject::tr( "Title" ) + QStringLiteral( "</td><td>" ) + mMetadata.title() + QStringLiteral( "</td></tr>\n" );

  // Type
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + QObject::tr( "Type" ) + QStringLiteral( "</td><td>" ) + mMetadata.type() + QStringLiteral( "</td></tr>\n" );

  // Language
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + QObject::tr( "Language" ) + QStringLiteral( "</td><td>" ) + mMetadata.language() + QStringLiteral( "</td></tr>\n" );

  // Abstract
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + QObject::tr( "Abstract" ) + QStringLiteral( "</td><td>" ) + mMetadata.abstract() + QStringLiteral( "</td></tr>\n" );

  // Categories
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + QObject::tr( "Categories" ) + QStringLiteral( "</td><td>" ) + mMetadata.categories().join( QStringLiteral( ", " ) ) + QStringLiteral( "</td></tr>\n" );

  // Keywords
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + QObject::tr( "Keywords" ) + QStringLiteral( "</td><td>\n" );
  QMapIterator<QString, QStringList> i( mMetadata.keywords() );
  if ( i.hasNext() )
  {
    myMetadata += QStringLiteral( "<table width=\"100%\" class=\"tabular-view\">\n" );
    myMetadata += "<tr><th>" + QObject::tr( "Vocabulary" ) + "</th><th>" + QObject::tr( "Items" ) + "</th></tr>\n";
    int j = 1;
    while ( i.hasNext() )
    {
      i.next();
      QString rowClass;
      if ( j % 2 )
        rowClass = QStringLiteral( "class=\"odd-row\"" );
      myMetadata += "<tr " + rowClass + "><td>" + i.key() + "</td><td>" + i.value().join( QStringLiteral( ", " ) ) + "</td></tr>\n";
      j++;
    }
    myMetadata += QLatin1String( "</table>\n" ); // End keywords table
  }
  myMetadata += QLatin1String( "</td></tr>\n" ); // End of keywords row
  myMetadata += QLatin1String( "</table>\n" ); // End identification table
  return myMetadata;
}

QString QgsLayerMetadataFormatter::historySectionHtml() const
{
  QString myMetadata;
  const QStringList historyItems = mMetadata.history();
  if ( historyItems.isEmpty() )
  {
    myMetadata += QStringLiteral( "<p>" ) + QObject::tr( "No history yet." ) + QStringLiteral( "</p>\n" );
  }
  else
  {
    myMetadata = QStringLiteral( "<table width=\"100%\" class=\"tabular-view\">\n" );
    myMetadata += "<tr><th>" + QObject::tr( "ID" ) + "</th><th>" + QObject::tr( "Action" ) + "</th></tr>\n";
    int i = 1;
    for ( const QString &history : historyItems )
    {
      QString rowClass;
      if ( i % 2 )
        rowClass = QStringLiteral( "class=\"odd-row\"" );
      myMetadata += "<tr " + rowClass + "><td width=\"5%\">" + QString::number( i ) + "</td><td>" + history + "</td></tr>\n";
      i++;
    }
    myMetadata += QLatin1String( "</table>\n" );
  }
  return myMetadata;
}

QString QgsLayerMetadataFormatter::linksSectionHtml() const
{
  QString myMetadata;
  const QList<QgsLayerMetadata::Link> &links = mMetadata.links();
  if ( links.isEmpty() )
  {
    myMetadata += QStringLiteral( "<p>" ) + QObject::tr( "No links yet." ) + QStringLiteral( "</p>\n" );
  }
  else
  {
    myMetadata = QStringLiteral( "<table width=\"100%\" class=\"tabular-view\">\n" );
    myMetadata += "<tr><th>" + QObject::tr( "ID" ) + "</th><th>" + QObject::tr( "Name" ) + "</th><th>" + QObject::tr( "Type" ) + "</th><th>" + QObject::tr( "URL" ) + "</th><th>" + QObject::tr( "Description" ) + "</th><th>" + QObject::tr( "Format" ) + "</th><th>" + QObject::tr( "MIME Type" ) + "</th><th>" + QObject::tr( "Size" ) + "</th></tr>\n";
    int i = 1;
    for ( const QgsLayerMetadata::Link &link : links )
    {
      QString rowClass;
      if ( i % 2 )
        rowClass = QStringLiteral( "class=\"odd-row\"" );
      myMetadata += "<tr " + rowClass + "><td>" + QString::number( i ) + "</td><td>" + link.name + "</td><td>" + link.type + "</td><td>" + link.url + "</td><td>" + link.description + "</td><td>" + link.format + "</td><td>" + link.mimeType + "</td><td>" + link.size + "</td></tr>\n";
      i++;
    }
    myMetadata += QLatin1String( "</table>\n" );
  }
  return myMetadata;
}
