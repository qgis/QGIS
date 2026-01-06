/***************************************************************************
                                  qgstestutils.cpp
                              --------------------
    begin                : January 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall.dawson@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstestutils.h"

#include "qgsconnectionpool.h"
#include "qgsvectordataprovider.h"

#include <QCryptographicHash>
#include <QtConcurrentMap>

///@cond PRIVATE
///

static void getFeaturesForProvider( const QPair< std::shared_ptr< QgsAbstractFeatureSource >, QgsFeatureRequest > &pair )
{
  QgsFeatureIterator it = pair.first->getFeatures( pair.second );
  QgsFeature f;
  while ( it.nextFeature( f ) )
  {

  }
}

bool QgsTestUtils::testProviderIteratorThreadSafety( QgsVectorDataProvider *provider, const QgsFeatureRequest &request )
{
  constexpr int JOBS_TO_RUN = 100;
  QList< QPair< std::shared_ptr< QgsAbstractFeatureSource >, QgsFeatureRequest > > jobs;
  jobs.reserve( JOBS_TO_RUN );
  for ( int i = 0; i < JOBS_TO_RUN; ++i )
  {
    jobs.append( qMakePair( std::shared_ptr< QgsAbstractFeatureSource >( provider->featureSource() ), request ) );
  }

  //freaking hammer the provider with a ton of concurrent requests.
  //thread unsafe providers... you better be ready!!!!
  QtConcurrent::blockingMap( jobs, getFeaturesForProvider );

  return true;
}

bool QgsTestUtils::compareDomElements( const QDomElement &element1, const QDomElement &element2 )
{
  QString tag1 = element1.tagName();
  tag1.replace( QRegularExpression( ".*:" ), QString() );
  QString tag2 = element2.tagName();
  tag2.replace( QRegularExpression( ".*:" ), QString() );
  if ( tag1 != tag2 )
  {
    qDebug( "Different tag names: %s, %s", tag1.toLatin1().data(), tag2.toLatin1().data() );
    return false;
  }

  if ( element1.hasAttributes() != element2.hasAttributes() )
  {
    qDebug( "Different hasAttributes: %s, %s", tag1.toLatin1().data(), tag2.toLatin1().data() );
    return false;
  }

  if ( element1.hasAttributes() )
  {
    const QDomNamedNodeMap attrs1 = element1.attributes();
    const QDomNamedNodeMap attrs2 = element2.attributes();

    if ( attrs1.size() != attrs2.size() )
    {
      qDebug( "Different attributes size: %s, %s", tag1.toLatin1().data(), tag2.toLatin1().data() );
      return false;
    }

    for ( int i = 0; i < attrs1.size(); ++i )
    {
      const QDomNode node1 = attrs1.item( i );
      const QDomAttr attr1 = node1.toAttr();

      if ( !element2.hasAttribute( attr1.name() ) )
      {
        qDebug( "Element2 has not attribute: %s, %s, %s", tag1.toLatin1().data(), tag2.toLatin1().data(), attr1.name().toLatin1().data() );
        return false;
      }

      if ( element2.attribute( attr1.name() ) != attr1.value() )
      {
        qDebug( "Element2 attribute has not the same value: %s, %s, %s", tag1.toLatin1().data(), tag2.toLatin1().data(), attr1.name().toLatin1().data() );
        return false;
      }
    }
  }

  if ( element1.hasChildNodes() != element2.hasChildNodes() )
  {
    qDebug( "Different childNodes: %s, %s", tag1.toLatin1().data(), tag2.toLatin1().data() );
    return false;
  }

  if ( element1.hasChildNodes() )
  {
    const QDomNodeList nodes1 = element1.childNodes();
    const QDomNodeList nodes2 = element2.childNodes();

    if ( nodes1.size() != nodes2.size() )
    {
      qDebug( "Different childNodes size: %s, %s", tag1.toLatin1().data(), tag2.toLatin1().data() );
      return false;
    }

    for ( int i = 0; i < nodes1.size(); ++i )
    {
      const QDomNode node1 = nodes1.at( i );
      const QDomNode node2 = nodes2.at( i );
      if ( node1.isElement() && node2.isElement() )
      {
        QDomElement elt1 = node1.toElement();
        QDomElement elt2 = node2.toElement();

        if ( !compareDomElements( elt1, elt2 ) )
          return false;
      }
      else if ( node1.isText() && node2.isText() )
      {
        const QDomText txt1 = node1.toText();
        const QDomText txt2 = node2.toText();

        if ( txt1.data() != txt2.data() )
        {
          qDebug( "Different text data: %s %s", tag1.toLatin1().data(), txt1.data().toLatin1().data() );
          qDebug( "Different text data: %s %s", tag2.toLatin1().data(), txt2.data().toLatin1().data() );
          return false;
        }
      }
    }
  }

  if ( element1.text() != element2.text() )
  {
    qDebug( "Different text: %s %s", tag1.toLatin1().data(), element1.text().toLatin1().data() );
    qDebug( "Different text: %s %s", tag2.toLatin1().data(), element2.text().toLatin1().data() );
    return false;
  }

  return true;
}

QString QgsTestUtils::sanitizeFakeHttpEndpoint( const QString &urlString )
{
  QString modifiedUrlString = urlString;

  // For REST API using URL subpaths, normalize the subpaths
  const int afterEndpointStartPos = static_cast<int>( modifiedUrlString.indexOf( "fake_qgis_http_endpoint" ) + strlen( "fake_qgis_http_endpoint" ) );
  QString afterEndpointStart = modifiedUrlString.mid( afterEndpointStartPos );
  afterEndpointStart.replace( "/"_L1, "_"_L1 );
  modifiedUrlString = modifiedUrlString.mid( 0, afterEndpointStartPos ) + afterEndpointStart;

  const qsizetype posQuotationMark = modifiedUrlString.indexOf( '?' );
  QString args = modifiedUrlString.mid( posQuotationMark );
  if ( modifiedUrlString.size() > 256 )
  {
    args.replace( '/', '_' );
    args = QCryptographicHash::hash( args.toUtf8(), QCryptographicHash::Md5 ).toHex();
  }
  else
  {
    args.replace( '?', '_' );
    args.replace( '&', '_' );
    args.replace( '<', '_' );
    args.replace( '>', '_' );
    args.replace( '\"', '_' );
    args.replace( '\'', '_' );
    args.replace( ' ', '_' );
    args.replace( ':', '_' );
    args.replace( '/', '_' );
    args.replace( '\n', '_' );
  }
  return modifiedUrlString.mid( 0, posQuotationMark ) + args;
}

///@endcond
