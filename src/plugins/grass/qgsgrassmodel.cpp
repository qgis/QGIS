/*******************************************************************
                              qgsgrasstree.cpp
                             -------------------
    begin                : February, 2006
    copyright            : (C) 2006 by Radim Blazek
    email                : radim.blazek@gmail.com
********************************************************************/
/********************************************************************
 This program is free software; you can redistribute it and/or modify  
 it under the terms of the GNU General Public License as published by 
 the Free Software Foundation; either version 2 of the License, or     
 (at your option) any later version.                                   
*******************************************************************/
#include <iostream>
#include <vector>
#include <map>

#include <QApplication>
#include <QStyle>
#include <qdir.h>
#include <qfile.h>
#include <qsettings.h>
#include <qstringlist.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qnamespace.h>
#include <qevent.h>
#include <qsize.h>
#include <qicon.h>
#include <QTreeWidgetItem>
#include <QModelIndex>
#include <QVariant>

#include "qgis.h"
#include "qgsapplication.h"

extern "C" {
#include <grass/gis.h>
#include <grass/Vect.h>
}

#include "../../src/providers/grass/qgsgrass.h"
#include "qgsgrassmodel.h"
#include "qgsgrassselect.h"

/* 
 * Internal data structure starts (at present) with LOCATION
 * as top level element, that is root but it does not appear 
 * in the tree view. First elements shown in the view are mapsets
 */

class QgsGrassModelItem
{
public:
    QgsGrassModelItem(QgsGrassModelItem *parent, int row, QString name,
	            QString path, int type);
    QgsGrassModelItem();
    ~QgsGrassModelItem();

    // Copy mGisbase, mLocation, mMapset, mMap, mLayer
    void copyNames ( QgsGrassModelItem *item);

    void populate();
    bool populated() { return mPopulated; }

    int type() { return mType; }

    // Map URI
    QString uri();

    QgsGrassModelItem *child ( int i );
    QgsGrassModelItem *mParent;
    QVariant data (int role = Qt::DisplayRole);
    QString name();
    QString info();

    int mType;

    QString mGisbase;
    QString mLocation;
    QString mMapset;
    QString mMap;
    QString mLayer;

    QVector<QgsGrassModelItem*> mChildren;
    bool mPopulated;

    QgsGrassModel *mModel;
};

QgsGrassModelItem::QgsGrassModelItem()
     :mParent(0),mPopulated(false),mType(QgsGrassModel::None)
{
}

QgsGrassModelItem::~QgsGrassModelItem() 
{ 
    for (int i = 0; i < mChildren.size();i++) 
    {
	delete mChildren[i];
    }
    mChildren.clear(); 
}

void QgsGrassModelItem::copyNames ( QgsGrassModelItem *item  ) 
{ 
    mModel = item->mModel;
    mGisbase = item->mGisbase;
    mLocation = item->mLocation;
    mMapset = item->mMapset;
    mMap = item->mMap;
    mLayer = item->mLayer;
}

QVariant QgsGrassModelItem::data (int role) 
{ 
    if (role != Qt::DisplayRole) return QVariant();

    return name();
}

QString QgsGrassModelItem::info() 
{ 
    switch ( mType ) 
    {
	case QgsGrassModel::Location:
	    return "Location: " + mLocation; 
	    break;
	case QgsGrassModel::Mapset:
	    return "Location: " + mLocation + "<br>Mapset: " + mMapset; 
	    break;
	case QgsGrassModel::Vectors:
	case QgsGrassModel::Rasters:
	    return "Location: " + mLocation + "<br>Mapset: " + mMapset; 
	    break;
	case QgsGrassModel::Vector:
	    return "Vector: " + mMap; 
	case QgsGrassModel::Raster:
	  {
	    QString nl = "<br>";
	    QString str = "<b>Raster: " + mMap + "</b>" + nl;
	    
	    struct Cell_head head;
	    QgsGrass::setLocation( mGisbase, mLocation );
	    
	    if( G_get_cellhd( mMap.toLocal8Bit().data(), 
			      mMapset.toLocal8Bit().data(), &head) != 0 ) 
	    {
		str += "Cannot open raster header" + nl;
	    }
	    else
	    {
		str += QString::number(head.rows) + " rows" + nl;
		str += QString::number(head.cols) + " columns" + nl;
		str += QString::number(head.ns_res) + " N-S resolution" + nl;
		str += QString::number(head.ew_res) + " E-W resolution" + nl;
		str += QString::number(head.north) + " noth" + nl;
		str += QString::number(head.south) + " south" + nl;
		str += QString::number(head.east) + " east" + nl;
		str += QString::number(head.west) + " west" + nl;
		
	        int rasterType = G_raster_map_type( mMap.toLocal8Bit().data(),
		                                mMapset.toLocal8Bit().data() );

	        if( rasterType == CELL_TYPE ) 
		{
		    str += "integer (" + QString::number(head.format);
		    str += head.format==0 ? " byte)" : "bytes)";
		}
 	        else if( rasterType == FCELL_TYPE ) 
		{
		    str += "floating point (4 bytes)";
		}
		else if( rasterType == DCELL_TYPE )
		{
		    str += "floating point (8 bytes)";
		}
		else
		{
		    str += "unknown";
		}
		str += " format" + nl;
	    }

	    struct FPRange range;
	    if ( G_read_fp_range( mMap.toLocal8Bit().data(),
                           mMapset.toLocal8Bit().data(), &range ) != -1 )
	    {
		double min, max;
		G_get_fp_range_min_max( &range, &min, &max );

		str += QString::number(min) + " minimum value" + nl;
		str += QString::number(max) + " maximum value" + nl;
	    }

	    
	    struct History hist;
	    if ( G_read_history( mMap.toLocal8Bit().data(),
  			         mMapset.toLocal8Bit().data(), &hist) >= 0 )
	    {
		str += "Data source: " + QString(hist.datsrc_1) + " " 
		                       + QString(hist.datsrc_2) + nl;
		str += "Data description: " + QString(hist.keywrd) + nl;
		str += "Comments: " + nl;
		for (int i = 0; i < hist.edlinecnt; i++)
		{
		    str += QString(hist.edhist[i]) + nl;
		}
	    }
						
	    return str; 
	  }
	  break;
	case QgsGrassModel::VectorLayer:
	    return "Vector: " + mMap + "<br>Layer: " + mLayer; 
	    break;
    }
    return QString();
}

QString QgsGrassModelItem::name() 
{ 
    switch ( mType ) 
    {
	case QgsGrassModel::Location:
	    return mLocation; 
	    break;
	case QgsGrassModel::Mapset:
	    return mMapset;
	    break;
	case QgsGrassModel::Vectors:
	    return "vector";
	    break;
	case QgsGrassModel::Rasters:
	    return "raster";
	    break;
	case QgsGrassModel::Vector:
	case QgsGrassModel::Raster:
	    return mMap;
	    break;
	case QgsGrassModel::VectorLayer:
	    return mLayer; 
	    break;
    }
    return QString();
}

QString QgsGrassModelItem::uri () 
{ 
    switch ( mType ) 
    {
	case QgsGrassModel::VectorLayer:
	    return mGisbase + "/" + mLocation + "/" + mMapset + "/" 
		   + mMap + "/" + mLayer;
	    break;
	case QgsGrassModel::Raster:
	    return mGisbase + "/" + mLocation + "/" + mMapset + "/cellhd/" + mMap;
	    break;
    }
    return QString();
}

QgsGrassModelItem *QgsGrassModelItem::child ( int i ) 
{
    Q_ASSERT(i < mChildren.size());
    //return &(mChildren[i]);
    return mChildren[i];
}

void QgsGrassModelItem::populate()
{
    std::cerr << "QgsGrassModelItem::populate()" << std::endl;

    if ( mPopulated ) return;

    mModel->refreshItem(this);
}

/*********************** MODEL ***********************/

QgsGrassModel::QgsGrassModel ( QObject * parent )
             :QAbstractItemModel ( parent  )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassModel()" << std::endl;
    #endif

    // Icons
    QStyle *style = QApplication::style();
    mIconDirectory = QIcon(style->standardPixmap(QStyle::SP_DirClosedIcon));
    mIconDirectory.addPixmap(style->standardPixmap(QStyle::SP_DirOpenIcon), 
	                     QIcon::Normal, QIcon::On);
    QString location = QgsGrass::getDefaultGisdbase() 
		       + "/" + QgsGrass::getDefaultLocation();

    mIconFile = QIcon(style->standardPixmap(QStyle::SP_FileIcon));
    
    mRoot = new QgsGrassModelItem();
    mRoot->mType = QgsGrassModel::Location;
    mRoot->mModel = this;
    mRoot->mGisbase = QgsGrass::getDefaultGisdbase();
    mRoot->mLocation = QgsGrass::getDefaultLocation();
    //mRoot->refresh(); 
    refreshItem(mRoot);
}

QgsGrassModel::~QgsGrassModel() { }

void QgsGrassModel::refresh() 
{ 
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassModel::refresh()" << std::endl;
    #endif
    
    //mRoot->refresh(); 
    refreshItem(mRoot); 
}

QModelIndex QgsGrassModel::index(QgsGrassModelItem *item)
{
    // Item index 
    QModelIndex index;
    if ( item->mParent ) {
	Q_ASSERT ( item->mParent->mChildren.size() > 0 );
	//QVector<QgsGrassModelItem> children = item->mParent->mChildren;
	//int row = (item - &(item->mParent->mChildren.at(0)));
	int row = -1;
	for ( int i = 0; i < item->mParent->mChildren.size(); i++ )
	{
	   if ( item == item->mParent->mChildren[i] )
	   {
	       row = i;
	       break;
	   }
	} 
        Q_ASSERT ( row>=0 );    
	index = createIndex( row, 0, item );
    } else {
	index = QModelIndex();
    }
    return index;
}

void QgsGrassModel::removeItems(QgsGrassModelItem *item, QStringList list)
{
    QModelIndex index = QgsGrassModel::index ( item );
    // Remove items not present in the list
    for (int i = 0; i < item->mChildren.size();) 
    {
	if ( !list.contains(item->mChildren[i]->name()) )
	{
	    //std::cerr << "remove " << item->mChildren[i]->name().ascii() << std::endl;
	    beginRemoveRows( index, i, i );
	    delete item->mChildren[i];
	    item->mChildren.remove(i);
	    endRemoveRows();
	}
	else
	{
	   i++;
	} 
    }
}

void QgsGrassModel::addItems(QgsGrassModelItem *item, QStringList list, int type)
{
    //std::cerr << "QgsGrassModel::addItems" << std::endl;
    QModelIndex index = QgsGrassModel::index ( item );

    // Add new items
    
    for (int i = 0; i < list.size();i++) 
    {
	QString name = list.at(i);
	//std::cerr << "? add " << name.ascii() << std::endl;

	int insertAt = item->mChildren.size();
	for (int i = 0; i < item->mChildren.size();i++) 
	{
	    if ( item->mChildren[i]->name() == name ) 
	    {
		insertAt = -1;
		break;
	    }
	    if ( QString::localeAwareCompare(item->mChildren[i]->name(),name) > 0 )
	    {
		insertAt = i;
		break;
	    }
	}
	
	if ( insertAt >= 0 )
	{
	    std::cerr << "-> add " << name.ascii() << std::endl;
	    beginInsertRows( index, insertAt, insertAt );
	    QgsGrassModelItem *newItem = new QgsGrassModelItem();
	    item->mChildren.insert( insertAt, newItem );
	    //QgsGrassModelItem *newItem = &(item->mChildren[insertAt]);
	    newItem->mType = type;
	    newItem->mParent = item;
	    newItem->copyNames(item);
	    switch ( newItem->mType ) 
	    {
		case QgsGrassModel::Location:
		    newItem->mLocation = name;
		    break;
		case QgsGrassModel::Mapset:
		    newItem->mMapset = name;
		    break;
		case QgsGrassModel::Vectors:
		case QgsGrassModel::Rasters:
		    break;
		case QgsGrassModel::Vector:
		case QgsGrassModel::Raster:
		    newItem->mMap = name;
		    break;
		case QgsGrassModel::VectorLayer:
		    newItem->mLayer = name;
		    break;
	    }
	    
	    endInsertRows();
	}
    }
}

void QgsGrassModel::refreshItem(QgsGrassModelItem *item)
{
    std::cerr << "QgsGrassModel::refreshItem() item->mType = " << item->mType << std::endl;
    

    switch ( item->mType ) 
    {
	case QgsGrassModel::Location:
	  {
	    QStringList list = QgsGrass::mapsets ( item->mGisbase, item->mLocation );
            removeItems(item, list);
            addItems(item, list, QgsGrassModel::Mapset );

	  }
	  break;

	case QgsGrassModel::Mapset:
	  {
	    QStringList vectors = QgsGrass::vectors ( item->mGisbase, 
		                     item->mLocation, item->mMapset );
	    QStringList rasters = QgsGrass::rasters ( item->mGisbase, 
		                     item->mLocation, item->mMapset );

	    QStringList list;
	    if ( vectors.count() > 0 ) list.append("vector");
	    if ( rasters.count() > 0 ) list.append("raster");

            removeItems(item, list);
	    
	    if ( vectors.count() > 0 )
                addItems(item, QStringList("vector"), QgsGrassModel::Vectors );

	    if ( rasters.count() > 0 )
                addItems(item, QStringList("raster"), QgsGrassModel::Rasters );

	  }
	  break;

	case QgsGrassModel::Vectors:
	case QgsGrassModel::Rasters:
	  {
	    QStringList list;
	    int type;
	    if ( item->mType == QgsGrassModel::Vectors )
	    {
	        list = QgsGrass::vectors ( item->mGisbase, item->mLocation, 
			                   item->mMapset );
		type = QgsGrassModel::Vector;
	    }
	    else
	    {
	        list = QgsGrass::rasters ( item->mGisbase, item->mLocation, 
			                   item->mMapset );
		type = QgsGrassModel::Raster;
	    }

            removeItems(item, list);
            addItems(item, list, type );
    
	  }
	  break;

	case QgsGrassModel::Vector:
	  {
	    QStringList list = QgsGrassSelect::vectorLayers ( 
		   QgsGrass::getDefaultGisdbase(), 
		   QgsGrass::getDefaultLocation(),
		   item->mMapset, item->mMap );

            removeItems(item, list);
            addItems(item, list, QgsGrassModel::VectorLayer );
	  }
	    break;

	case QgsGrassModel::Raster:
	    break;

	case QgsGrassModel::VectorLayer:
	    break;
    }
    for ( int i = 0; i < item->mChildren.size(); i++ )
    {
	if ( item->mChildren[i]->mPopulated ) {
	    refreshItem( item->mChildren[i] );
	}
    }

    item->mPopulated = true;
}

QModelIndex QgsGrassModel::index( int row, int column, 
	                        const QModelIndex & parent ) const
{
    //std::cerr << "QgsGrassModel::index row = " << row 
    //                      << " column = " << column << std::endl;
    
    QgsGrassModelItem *item;
    if (!parent.isValid()) { 
	item = mRoot; 
    } else { 
        item = static_cast<QgsGrassModelItem*>(parent.internalPointer());
    }
    //if ( !item->populated() ) refreshItem(item);
    if ( !item->populated() ) item->populate();
    return createIndex ( row, column, item->child(row) );
}

QModelIndex QgsGrassModel::parent ( const QModelIndex & index ) const
{
    //std::cerr << "QgsGrassModel::parent" << std::endl;

    if (!index.isValid()) return QModelIndex();

    QgsGrassModelItem *item =
        static_cast<QgsGrassModelItem*>(index.internalPointer());
    
    QgsGrassModelItem *parentNode = item->mParent;

    if ( parentNode == 0 || parentNode == mRoot) return QModelIndex();

    // parent's row
    QVector<QgsGrassModelItem*> children = parentNode->mParent ? 
    	       parentNode->mParent->mChildren : mRoot->mChildren;
    int row = -1;
    for ( int i = 0; i < children.size(); i++ )
    {
       if ( parentNode == children[i] )
       {
	   row = i;
	   break;
       }
    } 
    Q_ASSERT ( row>=0 );    
    return createIndex(row, 0, parentNode);
}

int QgsGrassModel::rowCount ( const QModelIndex & parent ) const
{
    //std::cerr << "QgsGrassModel::rowCount" << std::endl;
    QgsGrassModelItem *item;
    if (!parent.isValid()) { 
	item = mRoot; 
    } else { 
        item = static_cast<QgsGrassModelItem*>(parent.internalPointer());
    }
    //std::cerr << "name = " << item->name().ascii() << std::endl;
    //std::cerr << "count = " << item->mChildren.size() << std::endl;
    if ( !item->populated() ) item->populate();
    //if ( !item->populated() ) refreshItem(item);
    return item->mChildren.size();
}

int QgsGrassModel::columnCount ( const QModelIndex & parent ) const
{
    //std::cerr << "QgsGrassModel::columnCount" << std::endl;
    return 1;
}

QVariant QgsGrassModel::data ( const QModelIndex &index, int role ) const 
{
    //std::cerr << "QgsGrassModel::data" << std::endl;

    if (!index.isValid()) { return QVariant(); } 
    if (role != Qt::DisplayRole && role != Qt::DecorationRole) return QVariant();

    QgsGrassModelItem *item;
    item = static_cast<QgsGrassModelItem*>(index.internalPointer());

    if ( role == Qt::DecorationRole ) 
    {
	if ( item->type() == QgsGrassModel::Raster ||
	     item->type() == QgsGrassModel::VectorLayer )
	{
	        return mIconFile;
	}
        return mIconDirectory;
    }
    return item->data(role);
}

QString QgsGrassModel::itemName ( const QModelIndex &index)
{
    if (!index.isValid()) { return QString(); } 

    QgsGrassModelItem *item;
    item = static_cast<QgsGrassModelItem*>(index.internalPointer());

    return item->name();
}

QString QgsGrassModel::itemInfo ( const QModelIndex &index)
{
    if (!index.isValid()) { return QString(); } 

    QgsGrassModelItem *item;
    item = static_cast<QgsGrassModelItem*>(index.internalPointer());

    return item->info();
}

int QgsGrassModel::itemType ( const QModelIndex &index ) const 
{
    if (!index.isValid()) { return QgsGrassModel::None; } 
    QgsGrassModelItem *item;
    item = static_cast<QgsGrassModelItem*>(index.internalPointer());
    return item->type();
}

QString QgsGrassModel::uri ( const QModelIndex &index ) const 
{
    if (!index.isValid()) { return QString(); } 
    QgsGrassModelItem *item;
    item = static_cast<QgsGrassModelItem*>(index.internalPointer());
    return item->uri();
}

void QgsGrassModel::setLocation( const QString &gisbase, const QString &location ) 
{ 
    mGisbase = gisbase;
    mLocation = location;
}

QVariant QgsGrassModel::headerData(int section, 
	Qt::Orientation orientation, int role) const
{
    //std::cerr << "QgsGrassModel::headerData" << std::endl;
    
    //TODO
    //if (orientation == Qt::Horizontal && role == Qt::DisplayRole)

    return QVariant();
}

Qt::ItemFlags QgsGrassModel::flags(const QModelIndex &index) const
{
    //std::cerr << "QgsGrassModel::flags" << std::endl;

    //TODO
    if (!index.isValid())
         return Qt::ItemIsEnabled;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

