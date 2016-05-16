/***************************************************************************
                   qgsvirtuallayerfeatureiterator.cpp
            Feature iterator for the virtual layer provider
begin                : Nov 2015
copyright            : (C) 2015 Hugo Mercier, Oslandia
email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qgsvirtuallayerfeatureiterator.h>
#include <qgsmessagelog.h>
#include <qgsgeometry.h>
#include <stdexcept>
#include "qgsvirtuallayerblob.h"

static QString quotedColumn( QString name )
{
  return "\"" + name.replace( "\"", "\"\"" ) + "\"";
}

QgsVirtualLayerFeatureIterator::QgsVirtualLayerFeatureIterator( QgsVirtualLayerFeatureSource* source, bool ownSource, const QgsFeatureRequest& request )
    : QgsAbstractFeatureIteratorFromSource<QgsVirtualLayerFeatureSource>( source, ownSource, request )
{
  try
  {
    mSqlite = mSource->provider()->mSqlite.get();
    mDefinition = mSource->provider()->mDefinition;

    QString tableName = mSource->provider()->mTableName;

    QStringList wheres;
    QString subset = mSource->provider()->mSubset;
    if ( !subset.isNull() )
    {
      wheres << subset;
    }

    if ( mDefinition.hasDefinedGeometry() && !request.filterRect().isNull() )
    {
      bool do_exact = request.flags() & QgsFeatureRequest::ExactIntersect;
      QgsRectangle rect( request.filterRect() );
      QString mbr = QString( "%1,%2,%3,%4" ).arg( rect.xMinimum() ).arg( rect.yMinimum() ).arg( rect.xMaximum() ).arg( rect.yMaximum() );
      wheres << quotedColumn( mDefinition.geometryField() ) + " is not null";
      wheres <<  QString( "%1Intersects(%2,BuildMbr(%3))" )
      .arg( do_exact ? "" : "Mbr",
            quotedColumn( mDefinition.geometryField() ),
            mbr );
    }
    else if ( !mDefinition.uid().isNull() && request.filterType() == QgsFeatureRequest::FilterFid )
    {
      wheres << QString( "%1=%2" )
      .arg( quotedColumn( mDefinition.uid() ) )
      .arg( request.filterFid() );
    }
    else if ( !mDefinition.uid().isNull() && request.filterType() == QgsFeatureRequest::FilterFids )
    {
      QString values = quotedColumn( mDefinition.uid() ) + " IN (";
      bool first = true;
      Q_FOREACH ( QgsFeatureId v, request.filterFids() )
      {
        if ( !first )
        {
          values += ",";
        }
        first = false;
        values += QString::number( v );
      }
      values += ")";
      wheres << values;
    }

    mFields = mSource->provider()->fields();
    if ( request.flags() & QgsFeatureRequest::SubsetOfAttributes )
    {
      // copy only selected fields
      Q_FOREACH ( int idx, request.subsetOfAttributes() )
      {
        mAttributes << idx;
      }

      // ensure that all attributes required for expression filter are being fetched
      if ( request.filterType() == QgsFeatureRequest::FilterExpression )
      {
        Q_FOREACH ( const QString& field, request.filterExpression()->referencedColumns() )
        {
          int attrIdx = mFields.fieldNameIndex( field );
          if ( !mAttributes.contains( attrIdx ) )
            mAttributes << attrIdx;
        }
      }
    }
    else
    {
      mAttributes = mFields.allAttributesList();
    }

    QString columns;
    {
      // the first column is always the uid (or 0)
      if ( !mDefinition.uid().isNull() )
      {
        columns = quotedColumn( mDefinition.uid() );
      }
      else
      {
        columns = "0";
      }
      Q_FOREACH ( int i, mAttributes )
      {
        columns += ",";
        QString cname = mFields.at( i ).name().toLower();
        columns += quotedColumn( cname );
      }
    }
    // the last column is the geometry, if any
    if (( !( request.flags() & QgsFeatureRequest::NoGeometry )
          || ( request.filterType() == QgsFeatureRequest::FilterExpression && request.filterExpression()->needsGeometry() ) )
        && !mDefinition.geometryField().isNull() && mDefinition.geometryField() != "*no*" )
    {
      columns += "," + quotedColumn( mDefinition.geometryField() );
    }

    mSqlQuery = "SELECT " + columns + " FROM " + tableName;
    if ( !wheres.isEmpty() )
    {
      mSqlQuery += " WHERE " + wheres.join( " AND " );
    }

    mQuery.reset( new Sqlite::Query( mSqlite, mSqlQuery ) );

    mFid = 0;
  }
  catch ( std::runtime_error& e )
  {
    QgsMessageLog::logMessage( e.what(), QObject::tr( "VLayer" ) );
    close();
  }
}

QgsVirtualLayerFeatureIterator::~QgsVirtualLayerFeatureIterator()
{
  close();
}

bool QgsVirtualLayerFeatureIterator::rewind()
{
  if ( mClosed )
  {
    return false;
  }

  mQuery->reset();

  return true;
}

bool QgsVirtualLayerFeatureIterator::close()
{
  if ( mClosed )
  {
    return false;
  }

  // this call is absolutely needed
  iteratorClosed();

  mClosed = true;
  return true;
}

bool QgsVirtualLayerFeatureIterator::fetchFeature( QgsFeature& feature )
{
  feature.setValid( false );

  if ( mClosed )
  {
    return false;
  }
  if ( mQuery->step() != SQLITE_ROW )
  {
    return false;
  }

  feature.setFields( mFields, /* init */ true );

  if ( mDefinition.uid().isNull() )
  {
    // no id column => autoincrement
    feature.setFeatureId( mFid++ );
  }
  else
  {
    // first column: uid
    feature.setFeatureId( mQuery->columnInt64( 0 ) );
  }

  int n = mQuery->columnCount();
  int i = 0;
  Q_FOREACH ( int idx, mAttributes )
  {
    int type = mQuery->columnType( i + 1 );
    switch ( type )
    {
      case SQLITE_INTEGER:
        feature.setAttribute( idx, mQuery->columnInt64( i + 1 ) );
        break;
      case SQLITE_FLOAT:
        feature.setAttribute( idx, mQuery->columnDouble( i + 1 ) );
        break;
      case SQLITE_TEXT:
      default:
        feature.setAttribute( idx, mQuery->columnText( i + 1 ) );
        break;
    };
    i++;
  }
  if ( n > mAttributes.size() + 1 )
  {
    // geometry field
    QByteArray blob( mQuery->columnBlob( n - 1 ) );
    if ( blob.size() > 0 )
    {
      feature.setGeometry( spatialiteBlobToQgsGeometry( blob.constData(), blob.size() ) );
    }
    else
    {
      feature.setGeometry( nullptr );
    }
  }

  feature.setValid( true );
  return true;
}

QgsVirtualLayerFeatureSource::QgsVirtualLayerFeatureSource( const QgsVirtualLayerProvider* p )
    : mProvider( p )
{
}

QgsVirtualLayerFeatureSource::~QgsVirtualLayerFeatureSource()
{
}

QgsFeatureIterator QgsVirtualLayerFeatureSource::getFeatures( const QgsFeatureRequest& request )
{
  return QgsFeatureIterator( new QgsVirtualLayerFeatureIterator( this, false, request ) );
}
