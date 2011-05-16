/***************************************************************************
               qgsdataitem.cpp  - Data items
                             -------------------
    begin                : 2011-04-01
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include <typeinfo>

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QMouseEvent>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVector>
#include <QStyle>
#include <QSettings>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsdataitem.h"

#include "qgsdataprovider.h" 
#include "qgslogger.h" 
#include "qgsproviderregistry.h" 

// shared icons
QIcon QgsLayerItem::sIconPoint, QgsLayerItem::sIconLine, QgsLayerItem::sIconPolygon, QgsLayerItem::sIconTable, QgsLayerItem::sIconDefault;
QIcon QgsDataCollectionItem::sDirIcon;


QgsDataItem::QgsDataItem(QgsDataItem::Type type, QgsDataItem* parent, QString name, QString path)
  : QObject(parent), mType(type), mParent(parent), mPopulated(false), mName(name), mPath(path)
{
  QgsDebugMsg(QString("@ %1 %2 %3").arg(mType).arg((unsigned long)mParent).arg(mPath));
}

QIcon QgsDataItem::icon()
{
  return mIcon;
}

// TODO: This is copy from QgisApp, bad
// TODO: add some caching mechanism ?
QPixmap QgsDataItem::getThemePixmap( const QString theName )
{
  QString myPreferredPath = QgsApplication::activeThemePath()  + QDir::separator() + theName;
  QString myDefaultPath = QgsApplication::defaultThemePath()  + QDir::separator() + theName;
  //QgsDebugMsg( "myPreferredPath = " + myPreferredPath );
  //QgsDebugMsg( "myDefaultPath = " + myDefaultPath );
  if ( QFile::exists( myPreferredPath ) )
  {
    return QPixmap( myPreferredPath );
  }
  else
  {
    //could still return an empty icon if it
    //doesnt exist in the default theme either!
    return QPixmap( myDefaultPath );
  }
}

void QgsDataItem::doubleClick()
{

}

void QgsDataItem::emitBeginInsertItems( QgsDataItem* parent, int first, int last )
{
  emit beginInsertItems ( parent, first, last );
}
void QgsDataItem::emitEndInsertItems()
{
  emit endInsertItems();
}
void QgsDataItem::emitBeginRemoveItems( QgsDataItem* parent, int first, int last )
{
  emit beginRemoveItems ( parent, first, last );
}
void QgsDataItem::emitEndRemoveItems()
{
  emit endRemoveItems();
}

QVector<QgsDataItem*> QgsDataItem::createChildren( )
{
  QVector<QgsDataItem*> children;
  return children;
}

void QgsDataItem::populate()
{
  QVector<QgsDataItem*> children = createChildren( );
  foreach ( QgsDataItem *child, children )
  {
    // initialization, do not refresh! That would result in infinite loop (beginInsertItems->rowCount->populate)
    addChildItem ( child);
  }
  mPopulated = true;
}

int QgsDataItem::rowCount()
{
  if (!mPopulated) populate();
  return mChildren.size();
}
bool QgsDataItem::hasChildren()
{
  return ( mPopulated ? mChildren.count() > 0 : true);
}

void QgsDataItem::addChildItem ( QgsDataItem * child, bool refresh ) 
{
  QgsDebugMsg(  "mName = " + child->mName );
  int i;
  for ( i = 0; i < mChildren.size(); i++ )
  {
    if ( mChildren[i]->mName.localeAwareCompare ( child->mName ) >= 0 ) break;
  }

  if ( refresh ) emit beginInsertItems ( this, i, i );
  mChildren.insert ( i, child );

  connect ( child, SIGNAL(beginInsertItems ( QgsDataItem*, int, int )),
            this, SLOT(emitBeginInsertItems( QgsDataItem*, int, int )) );
  connect ( child, SIGNAL(endInsertItems ()),
            this, SLOT(emitEndInsertItems()) );
  connect ( child, SIGNAL(beginRemoveItems ( QgsDataItem*, int, int )),
            this, SLOT(emitBeginRemoveItems( QgsDataItem*, int, int )) );	
  connect ( child, SIGNAL(endRemoveItems ()),
            this, SLOT(emitEndRemoveItems()) );

  if ( refresh ) emit endInsertItems();

}
void QgsDataItem::deleteChildItem ( QgsDataItem * child ) 
{
  QgsDebugMsg(  "mName = " + child->mName );
  int i = mChildren.indexOf( child );
  Q_ASSERT( i >= 0 );
  emit beginRemoveItems ( this, i, i );
  mChildren.remove(i);
  delete child;
  emit endRemoveItems ();
}

int QgsDataItem::findItem ( QVector<QgsDataItem*> items, QgsDataItem * item )
{
  for ( int i = 0; i < items.size(); i++ )
  {
    QgsDebugMsg( QString::number(i) + " : " + items[i]->mPath + " x " + item->mPath );
    if ( items[i]->equal ( item ) ) return i;
  }
  return -1;
}

void QgsDataItem::refresh()
{
  QgsDebugMsg ( "mPath = " + mPath );

  QVector<QgsDataItem*> items = createChildren( );

  // Remove no more present items
  QVector<QgsDataItem*> remove;
  foreach ( QgsDataItem *child, mChildren )
  {
    if ( findItem(items, child ) >= 0 ) continue;
    remove.append ( child );
  }
  foreach ( QgsDataItem *child, remove )
  {
    deleteChildItem ( child );
  }

  // Add new items
  foreach ( QgsDataItem *item, items )
  {
    // Is it present in childs?
    if ( findItem ( mChildren, item ) >= 0 ) 
    {
      delete item;
      continue;
    }
    addChildItem ( item, true );
  }
}

// ---------------------------------------------------------------------

QgsLayerItem::QgsLayerItem(QgsDataItem* parent, QString name, QString path, QString uri, LayerType layerType)
  : QgsDataItem(Layer, parent, name, path), mUri(uri), mLayerType(layerType)
{
  if (sIconPoint.isNull())
  {
    // initialize shared icons
    sIconPoint = QIcon( getThemePixmap( "/mIconPointLayer.png" ) );
    sIconLine = QIcon( getThemePixmap( "/mIconLineLayer.png" ) );
    sIconPolygon = QIcon( getThemePixmap( "/mIconPolygonLayer.png" ) );
    sIconTable = QIcon( getThemePixmap( "/mIconTableLayer.png" ) );
    sIconDefault = QIcon( getThemePixmap( "/mIconLayer.png" ) );
  }

  switch (layerType)
  {
    case Point:      mIcon = sIconPoint; break;
    case Line:       mIcon = sIconLine; break;
    case Polygon:    mIcon = sIconPolygon; break;
    case TableLayer: mIcon = sIconTable; break;
    default:         mIcon = sIconDefault; break;
  }
}

bool QgsLayerItem::equal(const QgsDataItem *other)
{
  //QgsDebugMsg ( mPath + " x " + other->mPath );
  if ( typeid ( *this ) != typeid ( *other ) )
  {
    //QgsDebugMsg ( "different typeid" );
    return false;
  }
  //const QgsLayerItem *o = qobject_cast<const QgsLayerItem *> ( other );
  const QgsLayerItem *o = dynamic_cast<const QgsLayerItem *> ( other );
  return ( mPath == o->mPath && mName == o->mName && mUri == o->mUri && mProvider == o->mProvider );
}

// ---------------------------------------------------------------------
QgsDataCollectionItem::QgsDataCollectionItem( QgsDataItem* parent, QString name, QString path)
  : QgsDataItem( Collection, parent, name, path)
{

  if (sDirIcon.isNull())
  {
    // initialize shared icons
    QStyle *style = QApplication::style();
    sDirIcon = QIcon( style->standardPixmap( QStyle::SP_DirClosedIcon ) );
    sDirIcon.addPixmap( style->standardPixmap( QStyle::SP_DirOpenIcon ),
                            QIcon::Normal, QIcon::On );
  }
}
QgsDataCollectionItem::~QgsDataCollectionItem()
{
  foreach (QgsDataItem* i, mChildren)
    delete i;
}

//-----------------------------------------------------------------------
QVector<QgsDataProvider*> QgsDirectoryItem::mProviders = QVector<QgsDataProvider*>();
QVector<QLibrary*> QgsDirectoryItem::mLibraries = QVector<QLibrary*>();


QgsDirectoryItem::QgsDirectoryItem(QgsDataItem* parent, QString name, QString path)
  : QgsDataCollectionItem(parent, name, path)
{
  mType = Directory;
  mIcon = sDirIcon;

  if ( mLibraries.size() == 0 ) {
    QStringList keys = QgsProviderRegistry::instance()->providerList();
    QStringList::const_iterator i;
    for ( i = keys.begin(); i != keys.end(); ++i)
    {
      QString k(*i);
      // some providers hangs with empty uri (Postgis) etc...
      // -> using libraries directly
      QLibrary *library = QgsProviderRegistry::instance()->getLibrary(k);
      if ( library )
      {
        dataCapabilities_t * dataCapabilities = (dataCapabilities_t *) cast_to_fptr( library->resolve ("dataCapabilities") );
        if ( !dataCapabilities ) 
        {
          QgsDebugMsg ( library->fileName() + " does not have dataCapabilities" );
          continue;
        }
        if ( dataCapabilities() == QgsDataProvider::NoDataCapabilities ) 
        {
          QgsDebugMsg ( library->fileName() + " does not have File capability" );
          continue;
        }
        mLibraries.append( library );
      }
      else
      {
        //QgsDebugMsg ( "Cannot get provider " + k );
      }
    }
  }
}

QgsDirectoryItem::~QgsDirectoryItem()
{
}

QVector<QgsDataItem*> QgsDirectoryItem::createChildren( )
{
  QVector<QgsDataItem*> children;
  QDir dir(mPath);
  QStringList entries = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase);
  foreach (QString subdir, entries)
  {
    QString subdirPath = dir.absoluteFilePath(subdir);
    qDebug("creating subdir: %s", subdirPath.toAscii().data());

    QgsDirectoryItem *item = new QgsDirectoryItem(this, subdir, subdirPath);
    // propagate signals up to top

    children.append( item );
  }

  QStringList fileEntries = dir.entryList( QDir::Files, QDir::Name);
  foreach (QString name, fileEntries)
  {
    QString path = dir.absoluteFilePath( name );
    foreach ( QLibrary *library, mLibraries )
    {
      // we could/should create separate list of providers for each purpose

      // TODO: use existing fileVectorFilters(),directoryDrivers() ?
      dataCapabilities_t * dataCapabilities = (dataCapabilities_t *) cast_to_fptr( library->resolve ("dataCapabilities") );
      if ( !dataCapabilities ) continue;

      int capabilities = dataCapabilities();

      if ( ! (capabilities & QgsDataProvider::File) ) continue;

      dataItem_t * dataItem = (dataItem_t *) cast_to_fptr( library->resolve ("dataItem" ) );
      if ( ! dataItem ) 
      {
        QgsDebugMsg ( library->fileName() + " does not have dataItem" );
        continue;
      }

      QgsDataItem * item = dataItem ( path, this );
      if ( item )
      {
        children.append( item );
      }
    }
  }
  return children;
}

bool QgsDirectoryItem::equal(const QgsDataItem *other)
{
  //QgsDebugMsg ( mPath + " x " + other->mPath );
  if ( typeid ( *this ) != typeid ( *other ) )
  {
    //QgsDebugMsg ( "different typeid" );
    return false;
  }
  return ( mPath == other->mPath );
}

QWidget * QgsDirectoryItem::paramWidget()
{
  return new QgsDirectoryParamWidget(mPath);
}

QgsDirectoryParamWidget::QgsDirectoryParamWidget(QString path, QWidget* parent)
  : QTreeWidget(parent)
{
  setRootIsDecorated(false);

  // name, size, date, permissions, owner, group, type
  setColumnCount (7);
  QStringList labels;
  labels << tr("Name") << tr("Size") << tr("Date") << tr("Permissions") << tr("Owner") << tr("Group") << tr("Type");
  setHeaderLabels ( labels );

  QStyle* style = QApplication::style();
  QIcon iconDirectory = QIcon( style->standardPixmap( QStyle::SP_DirClosedIcon ) );
  QIcon iconFile = QIcon( style->standardPixmap( QStyle::SP_FileIcon ) );
  QIcon iconLink = QIcon( style->standardPixmap( QStyle::SP_FileLinkIcon ) ); // TODO: symlink to directory?

  QList<QTreeWidgetItem *> items;

  QDir dir(path);
  QStringList entries = dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase);
  foreach (QString name, entries)
  {
    QFileInfo fi ( dir.absoluteFilePath(name) );
    QStringList texts;
    texts << name;
    QString size;
    if ( fi.size() > 1024 ) 
    {
      size = size.sprintf ( "%.1f KiB", fi.size()/1024.0 );
    }
    else if ( fi.size() > 1.048576e6 ) 
    {
      size = size.sprintf ( "%.1f MiB", fi.size()/1.048576e6 );
    }
    else
    {
      size = QString( "%1 B" ).arg( fi.size() );
    }
    texts << size;
    texts << fi.lastModified().toString (Qt::SystemLocaleShortDate);
    QString perm;
    perm += fi.permission( QFile::ReadOwner ) ? 'r' : '-';
    perm += fi.permission( QFile::WriteOwner ) ? 'w' : '-';
    perm += fi.permission( QFile::ExeOwner ) ? 'x' : '-';
    // QFile::ReadUser, QFile::WriteUser, QFile::ExeUser
    perm += fi.permission( QFile::ReadGroup ) ? 'r' : '-';
    perm += fi.permission( QFile::WriteGroup ) ? 'w' : '-';
    perm += fi.permission( QFile::ExeGroup ) ? 'x' : '-';
    perm += fi.permission( QFile::ReadOther ) ? 'r' : '-';
    perm += fi.permission( QFile::WriteOther ) ? 'w' : '-';
    perm += fi.permission( QFile::ExeOther ) ? 'x' : '-';
    texts << perm;

    texts << fi.owner();
    texts << fi.group();

    QString type;
    QIcon icon;
    if ( fi.isDir() )
    {
      type = tr ( "folder" );
      icon = iconDirectory;
    }
    else if ( fi.isFile() )
    {
      type = tr ( "file" );
      icon = iconFile;
    }
    else if ( fi.isSymLink() )
    {
      type = tr ( "link" );
      icon = iconLink;
    }

    texts << type;

    QTreeWidgetItem *item = new QTreeWidgetItem (texts);
    item->setIcon(0, icon);
    items << item;
  }

  addTopLevelItems(items);

  // hide columns that are not requested
  QSettings settings;
  QList<QVariant> lst = settings.value("/dataitem/directoryHiddenColumns").toList();
  foreach (QVariant colVariant, lst)
  {
    setColumnHidden( colVariant.toInt(), true );
  }
}

void QgsDirectoryParamWidget::mousePressEvent(QMouseEvent* event)
{
  if ( event->button() == Qt::RightButton )
  {
    // show the popup menu
    QMenu popupMenu;

    QStringList labels;
    labels << tr("Name") << tr("Size") << tr("Date") << tr("Permissions") << tr("Owner") << tr("Group") << tr("Type");
    for (int i = 0; i < labels.count(); i++)
    {
      QAction* action = popupMenu.addAction( labels[i], this, SLOT(showHideColumn()) );
      action->setObjectName(QString::number(i));
      action->setCheckable(true);
      action->setChecked( !isColumnHidden(i) );
    }

    popupMenu.exec( event->globalPos() );
  }
}

void QgsDirectoryParamWidget::showHideColumn()
{
  QAction* action = qobject_cast<QAction*>(sender());
  if (!action)
    return; // something is wrong

  int columnIndex = action->objectName().toInt();
  setColumnHidden(columnIndex, !isColumnHidden(columnIndex));

  // save in settings
  QSettings settings;
  QList<QVariant> lst;
  for (int i = 0; i < columnCount(); i++)
  {
    if (isColumnHidden(i))
      lst.append(QVariant(i));
  }
  settings.setValue("/dataitem/directoryHiddenColumns", lst);
}
