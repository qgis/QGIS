/***************************************************************************
  sadbtablemodel.h
  A model that holds the tables of a database in a hierarchy where the
  schemas are the root elements that contain the individual tables as children.
  -------------------
    begin                : Dec 2010
    copyright            : (C) 2010 by iAnywhere Solutions, Inc.
    author		 : David DeHaan
    email                : ddehaan at sybase dot com

 This class was copied and modified from QgsDbTableModel because that 
 class is not accessible to QGIS plugins.  Therefore, the author gratefully
 acknowledges the following copyright on the original content:
			 qgsdbtablemodel.cpp
    begin                : Dec 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id$ */


#include "sqlanywhere.h"
#include "sadbtablemodel.h"

#include <QIcon>

SaDbTableModel::SaDbTableModel(): QStandardItemModel(), mTableCount( 0 )
{
  QStringList headerLabels;
  headerLabels << tr( "Schema" );
  headerLabels << tr( "Table" );
  headerLabels << tr( "Type" );
  headerLabels << tr( "SRID" );
  headerLabels << tr( "Line Interpretation" );
  headerLabels << tr( "Geometry column" );
  headerLabels << tr( "Sql" );
  setHorizontalHeaderLabels( headerLabels );
}

SaDbTableModel::~SaDbTableModel()
{

}

void SaDbTableModel::addTableEntry( QString type, QString schemaName, QString tableName, QString srid, QString lineInterp, QString geometryColName, QString sql )
{
  //is there already a root item with the given scheme Name?
  QStandardItem *schemaItem;
  QList<QStandardItem*> schemaItems = findItems( schemaName, Qt::MatchExactly, dbtmSchema );

  //there is already an item for this schema
  if ( schemaItems.size() > 0 )
  {
    schemaItem = schemaItems.at( dbtmSchema );
  }
  else //create a new toplevel item for this schema
  {
    schemaItem = new QStandardItem( schemaName );
    schemaItem->setFlags( Qt::ItemIsEnabled );
    invisibleRootItem()->setChild( invisibleRootItem()->rowCount(), schemaItem );
  }

  //path to icon for specified type
  QString typeName;

  QGis::WkbType wkbType = qgisTypeFromDbType( type );
  QIcon iconFile = iconForType( wkbType );

  QList<QStandardItem*> childItemList;
  QStandardItem* schemaNameItem = new QStandardItem( schemaName );
  schemaNameItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  QStandardItem* tableItem = new QStandardItem( tableName );
  tableItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  QStandardItem* typeItem = new QStandardItem( QIcon( iconFile ), type );
  typeItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  QStandardItem* sridItem = new QStandardItem( srid );
  sridItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  QStandardItem* lineInterpItem = new QStandardItem( lineInterp );
  sridItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  QStandardItem* geomItem = new QStandardItem( geometryColName );
  geomItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  QStandardItem* sqlItem = new QStandardItem( sql );
  sqlItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );

  childItemList.push_back( schemaNameItem );
  childItemList.push_back( tableItem );
  childItemList.push_back( typeItem );
  childItemList.push_back( sridItem );
  childItemList.push_back( lineInterpItem );
  childItemList.push_back( geomItem );
  childItemList.push_back( sqlItem );

  schemaItem->appendRow( childItemList );
  ++mTableCount;
}

void SaDbTableModel::setSql( const QModelIndex &index, const QString &sql )
{
  if ( !index.isValid() || !index.parent().isValid() )
  {
    return;
  }

  //find out schema name and table name
  QModelIndex schemaSibling = index.sibling( index.row(), dbtmSchema );
  QModelIndex tableSibling = index.sibling( index.row(), dbtmTable );
  QModelIndex geomSibling = index.sibling( index.row(), dbtmGeomCol );

  if ( !schemaSibling.isValid() || !tableSibling.isValid() || !geomSibling.isValid() )
  {
    return;
  }

  QString schemaName = itemFromIndex( schemaSibling )->text();
  QString tableName = itemFromIndex( tableSibling )->text();
  QString geomName = itemFromIndex( geomSibling )->text();

  QList<QStandardItem*> schemaItems = findItems( schemaName, Qt::MatchExactly, dbtmSchema );
  if ( schemaItems.size() < 1 )
  {
    return;
  }

  QStandardItem* schemaItem = schemaItems.at( dbtmSchema );
  int numChildren = schemaItem->rowCount();

  QModelIndex currentChildIndex;
  QModelIndex currentTableIndex;
  QModelIndex currentGeomIndex;

  for ( int i = 0; i < numChildren; ++i )
  {
    currentChildIndex = indexFromItem( schemaItem->child( i, dbtmSchema ) );
    if ( !currentChildIndex.isValid() )
    {
      continue;
    }
    currentTableIndex = currentChildIndex.sibling( i, dbtmTable );
    if ( !currentTableIndex.isValid() )
    {
      continue;
    }

    currentGeomIndex = currentChildIndex.sibling( i, dbtmGeomCol );
    if ( !currentGeomIndex.isValid() )
    {
      continue;
    }

    if ( itemFromIndex( currentTableIndex )->text() == tableName &&
         itemFromIndex( currentGeomIndex )->text() == geomName )
    {
      QModelIndex sqlIndex = currentChildIndex.sibling( i, dbtmSql );
      if ( sqlIndex.isValid() )
      {
        itemFromIndex( sqlIndex )->setText( sql );
        break;
      }
    }
  }
}

QString
makeSubsetSql( QString prevSql, QString geomCol, QString geomType )
{
    QString	sql;
    QStringList types;
    if( geomType == "ST_POINT" ) {
	types << "'ST_POINT'";
	types << "'ST_MULTIPOINT'";

    } else if( geomType == "ST_LINESTRING" ) {
	types << "'ST_LINESTRING'";
	types << "'ST_MULTILINESTRING'";

    } else if( geomType == "ST_POLYGON" ) {
	types << "'ST_POLYGON'";
	types << "'ST_MULTIPOLYGON'";
    }
   
    if( types.isEmpty() ) {
	sql = prevSql;

    } else {
	sql = geomCol 
	    + ".ST_GeometryType() IN ( " 
	    + types.join( "," )
	    + " ) ";
	if( !prevSql.isEmpty() ) {
	    sql += "AND ( " + prevSql + ") ";
	}
    }

    return sql;
}

void SaDbTableModel::setGeometryTypesForTable( const QString& schema, const QString& table, const QString& attribute, const QString& type, const QString& srid, const QString& lineInterp )
{
  QStringList typeList = type.split( "," );

  //find schema item and table item
  QStandardItem* schemaItem;
  QList<QStandardItem*> schemaItems = findItems( schema, Qt::MatchExactly, dbtmSchema );

  if ( schemaItems.size() < 1 )
  {
    return;
  }
  schemaItem = schemaItems.at( 0 );
  int numChildren = schemaItem->rowCount();

  QModelIndex currentChildIndex;
  QModelIndex currentTableIndex;
  QModelIndex currentTypeIndex;
  QModelIndex currentSridIndex;
  QModelIndex currentLineInterpIndex;
  QModelIndex currentGeomColumnIndex;
  QModelIndex currentSqlIndex;

  for ( int i = 0; i < numChildren; ++i )
  {
    currentChildIndex = indexFromItem( schemaItem->child( i, dbtmSchema ) );
    if ( !currentChildIndex.isValid() )
    {
      continue;
    }
    currentTableIndex = currentChildIndex.sibling( i, dbtmTable );
    currentTypeIndex = currentChildIndex.sibling( i, dbtmType );
    currentSridIndex = currentChildIndex.sibling( i, dbtmSrid );
    currentLineInterpIndex = currentChildIndex.sibling( i, dbtmLineInterp );
    currentGeomColumnIndex = currentChildIndex.sibling( i, dbtmGeomCol );
    QString geomColText = itemFromIndex( currentGeomColumnIndex )->text();
    currentSqlIndex = currentChildIndex.sibling( i, dbtmSql );
    QString sqlText = itemFromIndex( currentSqlIndex )->text();

    if ( !currentTypeIndex.isValid() 
	    || !currentTableIndex.isValid() 
	    || !currentSridIndex.isValid() 
	    || !currentLineInterpIndex.isValid() 
	    || !currentSqlIndex.isValid() 
	    || !currentGeomColumnIndex.isValid() )
    {
      continue;
    }

    if ( itemFromIndex( currentTableIndex )->text() == table 
	    && ( geomColText == attribute ) )
    {
      if ( type.isEmpty() ) 
      {
	//the table has no valid geometry entry and so the item for 
	//this table should be removed
        removeRow( i, indexFromItem( schemaItem ) );
        return;
      }

      itemFromIndex( currentSridIndex )->setText( srid );
      itemFromIndex( currentLineInterpIndex )->setText( lineInterp );

      // update row with first type and add new rows for remaining types
      QGis::WkbType wkbType = qgisTypeFromDbType( typeList.at( 0 ) );
      QIcon myIcon = iconForType( wkbType );
      itemFromIndex( currentTypeIndex )->setText( typeList.at( 0 ) );
      itemFromIndex( currentTypeIndex )->setIcon( myIcon );
      itemFromIndex( currentSqlIndex )->setText( makeSubsetSql( sqlText, geomColText, typeList.at( 0 ) ) );

      for ( int j = 1; j < typeList.size(); j++ )
      {
        addTableEntry( typeList.at( j ), schema, table, srid, lineInterp, geomColText, makeSubsetSql( sqlText, geomColText, typeList.at( j ) ) );
      }

    }
  }
}

QIcon SaDbTableModel::iconForType( QGis::WkbType type ) const
{
  if ( type == QGis::WKBPoint || type == QGis::WKBPoint25D || type == QGis::WKBMultiPoint || type == QGis::WKBMultiPoint25D )
  {
    return SqlAnywhere::getThemeIcon( "/mIconPointLayer.png" );
  }
  else if ( type == QGis::WKBLineString || type == QGis::WKBLineString25D || type == QGis::WKBMultiLineString || type == QGis::WKBMultiLineString25D )
  {
    return SqlAnywhere::getThemeIcon( "/mIconLineLayer.png" );
  }
  else if ( type == QGis::WKBPolygon || type == QGis::WKBPolygon25D || type == QGis::WKBMultiPolygon || type == QGis::WKBMultiPolygon25D )
  {
    return SqlAnywhere::getThemeIcon( "/mIconPolygonLayer.png" );
  }
  else return QIcon();
}

QGis::WkbType SaDbTableModel::qgisTypeFromDbType( const QString& dbType ) const
{
  if ( dbType == "ST_POINT" )
  {
    return QGis::WKBPoint;
  }
  else if ( dbType == "ST_MULTIPOINT" )
  {
    return QGis::WKBMultiPoint;
  }
  else if ( dbType == "ST_LINESTRING" )
  {
    return QGis::WKBLineString;
  }
  else if ( dbType == "ST_MULTILINESTRING" )
  {
    return QGis::WKBMultiLineString;
  }
  else if ( dbType == "ST_POLYGON" )
  {
    return QGis::WKBPolygon;
  }
  else if ( dbType == "ST_MULTIPOLYGON" )
  {
    return QGis::WKBMultiPolygon;
  }
  return QGis::WKBUnknown;
}
