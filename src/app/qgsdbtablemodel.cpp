/***************************************************************************
                         qgsdbtablemodel.cpp  -  description
                         -------------------
    begin                : Dec 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdbtablemodel.h"
#include "qgsapplication.h"

QgsDbTableModel::QgsDbTableModel(): QStandardItemModel(), mTableCount(0)
{
  QStringList headerLabels;
  headerLabels << tr("Schema");
  headerLabels << tr("Table");
  headerLabels << tr("Type");
  headerLabels << tr("Geometry column");
  headerLabels << tr("Sql");
  setHorizontalHeaderLabels(headerLabels);
}

QgsDbTableModel::~QgsDbTableModel()
{

}

void QgsDbTableModel::addTableEntry(QString type, QString schemaName, QString tableName, QString geometryColName, QString sql)
{
  //is there already a root item with the given scheme Name?
  QStandardItem* schemaItem;
  QList<QStandardItem*> schemaItems = findItems(schemaName, Qt::MatchExactly, 0);

  //there is already an item for this schema
  if(schemaItems.size() > 0)
  {
    schemaItem = schemaItems.at(0);
  }
  else //create a new toplevel item for this schema
  {
    schemaItem = new QStandardItem(schemaName);
    schemaItem->setFlags(Qt::ItemIsEnabled);
    invisibleRootItem()->setChild (invisibleRootItem()->rowCount(), schemaItem);
  }

  //path to icon for specified type
  QString myThemePath = QgsApplication::themePath();
  QString typeName;
  QString iconFile;

  QGis::WKBTYPE wkbType = qgisTypeFromDbType(type);
  iconFile = iconFilePathForType(wkbType);

  QList<QStandardItem*> childItemList;
  QStandardItem* schemaNameItem = new QStandardItem(schemaName);
  schemaNameItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  QStandardItem* typeItem = new QStandardItem(QIcon(iconFile), type);
  typeItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  QStandardItem* tableItem = new QStandardItem(tableName);
  tableItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  QStandardItem* geomItem = new QStandardItem(geometryColName);
  geomItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  QStandardItem* sqlItem = new QStandardItem(sql);
  sqlItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);


  childItemList.push_back(schemaNameItem);
  childItemList.push_back(tableItem);
  childItemList.push_back(typeItem);
  childItemList.push_back(geomItem);
  childItemList.push_back(sqlItem);

  schemaItem->appendRow(childItemList);
  ++mTableCount;
}

void QgsDbTableModel::setSql(const QModelIndex& index, const QString& sql)
{
  if(!index.isValid() || !index.parent().isValid())
  {
    return;
  }

  //find out schema name and table name
  QModelIndex schemaSibling = index.sibling(index.row(), 0);
  QModelIndex tableSibling = index.sibling(index.row(), 1);
  QModelIndex geomSibling = index.sibling(index.row(), 3);

  if(!schemaSibling.isValid() || !tableSibling.isValid() || !geomSibling.isValid())
  {
    return;
  }

  QString schemaName = itemFromIndex(schemaSibling)->text();
  QString tableName = itemFromIndex(tableSibling)->text();
  QString geomName = itemFromIndex(geomSibling)->text();

  QList<QStandardItem*> schemaItems = findItems(schemaName, Qt::MatchExactly, 0);
  if(schemaItems.size() < 1)
  {
    return;
  }

  QStandardItem* schemaItem = schemaItems.at(0);
  int numChildren = schemaItem->rowCount();

  QModelIndex currentChildIndex;
  QModelIndex currentTableIndex;
  QModelIndex currentGeomIndex;

  for(int i = 0; i < numChildren; ++i)
  {
    currentChildIndex = indexFromItem(schemaItem->child(i, 0));
    if(!currentChildIndex.isValid())
    {
      continue;
    }
    currentTableIndex = currentChildIndex.sibling(i, 1);
    if(!currentTableIndex.isValid())
    {
      continue;
    }

    currentGeomIndex = currentChildIndex.sibling(i, 3);
    if(!currentGeomIndex.isValid())
    {
      continue;
    }

    if(itemFromIndex(currentTableIndex)->text() == tableName &&
       itemFromIndex(currentGeomIndex)->text() == geomName )
    {
      QModelIndex sqlIndex = currentChildIndex.sibling(i, 4);
      if(sqlIndex.isValid())
      {
        itemFromIndex(sqlIndex)->setText(sql);
        break;
      }
    }
  }
}

void QgsDbTableModel::setGeometryTypesForTable(const QString& schema, const QString& table, const QString& attribute, const QString& type)
{
  bool typeIsEmpty = type.isEmpty(); //true means the table has no valid geometry entry and the item for this table should be removed
  QStringList typeList = type.split(",");

  //find schema item and table item
  QStandardItem* schemaItem;
  QList<QStandardItem*> schemaItems = findItems(schema, Qt::MatchExactly, 0);

  if(schemaItems.size() < 1)
  {
    return;
  }
  schemaItem = schemaItems.at(0);
  int numChildren = schemaItem->rowCount();

  QModelIndex currentChildIndex;
  QModelIndex currentTableIndex;
  QModelIndex currentTypeIndex;
  QModelIndex currentGeomColumnIndex;

  for(int i = 0; i < numChildren; ++i)
  {
    currentChildIndex = indexFromItem(schemaItem->child(i, 0));
    if(!currentChildIndex.isValid())
    {
      continue;
    }
    currentTableIndex = currentChildIndex.sibling(i, 1);
    currentTypeIndex = currentChildIndex.sibling(i, 2);
    currentGeomColumnIndex = currentChildIndex.sibling(i, 3);
    QString geomColText = itemFromIndex(currentGeomColumnIndex)->text();

    if(!currentTypeIndex.isValid() || !currentTableIndex.isValid() || !currentGeomColumnIndex.isValid())
    {
      continue;
    }

    if(itemFromIndex(currentTableIndex)->text() == table &&
       (geomColText==attribute || geomColText.startsWith( attribute + " AS ") ) )
    {
      if(typeIsEmpty)
      {
        removeRow(i, indexFromItem(schemaItem));
        return;
      }

      QGis::WKBTYPE wkbType = qgisTypeFromDbType(typeList.at(0));
      QString iconPath = iconFilePathForType(wkbType);
      itemFromIndex(currentTypeIndex)->setText(typeList.at(0)); //todo: add other rows
      itemFromIndex(currentTypeIndex)->setIcon(QIcon(iconPath));
      if(!geomColText.contains(" AS "))
      {
        itemFromIndex(currentGeomColumnIndex)->setText(geomColText + " AS " + typeList.at(0));
      }

      for(int j = 1; j < typeList.size(); ++j)
      {
        //todo: add correct type
        addTableEntry(typeList.at(j), schema, table, geomColText + " AS " + typeList.at(j), ""); 
      }
    }
  }
}

QString QgsDbTableModel::iconFilePathForType(QGis::WKBTYPE type) const
{
  QString myThemePath = QgsApplication::themePath();
  if(type == QGis::WKBPoint || type == QGis::WKBPoint25D || type == QGis::WKBMultiPoint || type == QGis::WKBMultiPoint25D)
  {
    return myThemePath+"/mIconPointLayer.png";
  }
  else if(type == QGis::WKBLineString || type == QGis::WKBLineString25D || type == QGis::WKBMultiLineString || type == QGis::WKBMultiLineString25D)
  {
    return myThemePath+"/mIconLineLayer.png";
  }
  else if(type == QGis::WKBPolygon || type == QGis::WKBPolygon25D || type == QGis::WKBMultiPolygon || type == QGis::WKBMultiPolygon25D)
  {
    return myThemePath+"/mIconPolygonLayer.png";
  }
  else return "";
}

QString QgsDbTableModel::displayStringForType(QGis::WKBTYPE type) const
{
  if(type == QGis::WKBPoint || type == QGis::WKBPoint25D)
  {
    return tr("Point");
  }
  else if(type == QGis::WKBMultiPoint || type == QGis::WKBMultiPoint25D)
  {
    return tr("Multipoint");
  }
  else if(type == QGis::WKBLineString || type == QGis::WKBLineString25D)
  {
    return tr("Line");
  }
  else if(type == QGis::WKBMultiLineString || type == QGis::WKBMultiLineString25D)
  {
    return tr("Multiline");
  }
  else if(type == QGis::WKBPolygon || type == QGis::WKBPolygon25D)
  {
    return tr("Polygon");
  }
  else if(type == QGis::WKBMultiPolygon || type == QGis::WKBMultiPolygon25D)
  {
    return tr("Multipolygon");
  }
  return "Unknown";
}

QGis::WKBTYPE QgsDbTableModel::qgisTypeFromDbType(const QString& dbType) const
{
  if(dbType == "POINT")
  {
    return QGis::WKBPoint;
  }
  else if(dbType == "MULTIPOINT")
  {
    return QGis::WKBMultiPoint;
  }
  else if(dbType == "LINESTRING")
  {
    return QGis::WKBLineString;
  }
  else if(dbType == "MULTILINESTRING")
  {
    return QGis::WKBMultiLineString;
  }
  else if(dbType == "POLYGON")
  {
    return QGis::WKBPolygon;
  }
  else if(dbType == "MULTIPOLYGON")
  {
    return QGis::WKBMultiPolygon;
  }
  return QGis::WKBUnknown;
}
