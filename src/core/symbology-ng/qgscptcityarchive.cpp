/***************************************************************************
    qgscptcityarchive.cpp
    ---------------------
    begin                : August 2012
    copyright            : (C) 2009 by Martin Dobias
    copyright            : (C) 2011 Radim Blazek
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny.dev at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

#include "qgscptcityarchive.h"
#include "qgis.h"

#include "qgsdataprovider.h"
#include "qgslogger.h"
#include "qgsconfig.h"
#include "qgsmimedatautils.h"
#include "qgsapplication.h"


QString QgsCptCityArchive::mDefaultArchiveName;
QMap< QString, QgsCptCityArchive* > QgsCptCityArchive::mArchiveRegistry;
QMap< QString, QgsCptCityArchive* > QgsCptCityArchive::archiveRegistry() { return mArchiveRegistry; }
QMap< QString, QMap< QString, QString > > QgsCptCityArchive::mCopyingInfoMap;

QgsCptCityArchive::QgsCptCityArchive( QString archiveName, QString baseDir )
    : mArchiveName( archiveName ), mBaseDir( baseDir )
{
  QgsDebugMsg( "archiveName = " + archiveName + " baseDir = " + baseDir );

  // make Author items
  QgsCptCityDirectoryItem* dirItem = 0;
  foreach ( QString path, QDir( mBaseDir ).entryList( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name ) )
  {
    if ( path == "selections" )
      continue;
    QgsDebugMsg( "path= " + path );
    dirItem = new QgsCptCityDirectoryItem( NULL, QFileInfo( path ).baseName(), path );
    if ( dirItem->isValid() )
      mRootItems << dirItem;
    else
      delete dirItem;
  }

  // make selection items
  QgsCptCitySelectionItem* selItem = 0;
  QDir seldir( mBaseDir + QDir::separator() + "selections" );
  QgsDebugMsg( "populating selection from " + seldir.path() );
  foreach ( QString selfile, seldir.entryList( QStringList( "*.xml" ), QDir::Files ) )
  {
    QgsDebugMsg( "file= " + seldir.path() + "/" + selfile );
    selItem = new QgsCptCitySelectionItem( NULL, QFileInfo( selfile ).baseName(),
                                           seldir.dirName() +  QDir::separator() + selfile );
    //TODO remove item if there are no children (e.g. esri in qgis-sel)
    if ( selItem->isValid() )
      mSelectionItems << selItem;
    else
      delete selItem;
  }

  // make "All Ramps items" (which will contain all ramps without hierarchy)
  QgsCptCityAllRampsItem* allRampsItem;
  allRampsItem = new QgsCptCityAllRampsItem( NULL, QObject::tr( "All Ramps" ),
      mRootItems );
  mRootItems.prepend( allRampsItem );
  allRampsItem = new QgsCptCityAllRampsItem( NULL, QObject::tr( "All Ramps" ),
      mSelectionItems );
  mSelectionItems.prepend( allRampsItem );
}

QgsCptCityArchive::~QgsCptCityArchive( )
{
  foreach ( QgsCptCityDataItem* item, mRootItems )
    delete item;
  foreach ( QgsCptCityDataItem* item, mSelectionItems )
    delete item;
  mRootItems.clear();
  mSelectionItems.clear();
}

QString QgsCptCityArchive::baseDir() const
{
  // if was set with setBaseDir, return that value
  // else return global default
  if ( ! mBaseDir.isNull() )
    return mBaseDir;
  else
    return QgsCptCityArchive::defaultBaseDir( );
}

QString QgsCptCityArchive::baseDir( QString archiveName )
{
  // search for matching archive in the registry
  if ( archiveName.isNull() )
    archiveName = DEFAULT_CPTCITY_ARCHIVE;
  if ( mArchiveRegistry.contains( archiveName ) )
    return mArchiveRegistry.value( archiveName )->baseDir();
  else
    return defaultBaseDir();
}

QString QgsCptCityArchive::defaultBaseDir()
{
  QString baseDir, archiveName;
  QSettings settings;

  // use CptCity/baseDir setting if set, default is user dir
  baseDir = settings.value( "CptCity/baseDir",
                            QgsApplication::pkgDataPath() + "/resources" ).toString();
  // sub-dir defaults to cpt-city
  archiveName = settings.value( "CptCity/archiveName", DEFAULT_CPTCITY_ARCHIVE ).toString();

  return baseDir + QDir::separator() + archiveName;
}


QString QgsCptCityArchive::findFileName( const QString & target, const QString & startDir, const QString & baseDir )
{
  // QgsDebugMsg( "target= " + target +  " startDir= " + startDir +  " baseDir= " + baseDir );

  if ( startDir == "" || ! startDir.startsWith( baseDir ) )
    return QString();

  QDir dir = QDir( startDir );
  //todo test when
  while ( ! dir.exists( target ) && dir.path() != baseDir )
  {
    if ( ! dir.cdUp() )
      break;
  }
  if ( ! dir.exists( target ) )
    return QString();
  else
    return dir.path() + QDir::separator() + target;
}


QString QgsCptCityArchive::copyingFileName( const QString& path ) const
{
  return QgsCptCityArchive::findFileName( "COPYING.xml",
                                          baseDir() + QDir::separator() + path, baseDir() );
}

QString QgsCptCityArchive::descFileName( const QString& path ) const
{
  return QgsCptCityArchive::findFileName( "DESC.xml",
                                          baseDir() + QDir::separator() + path, baseDir() );
}

QgsStringMap QgsCptCityArchive::copyingInfo( const QString& fileName )
{
  QgsStringMap copyingMap;

  if ( fileName.isNull() )
    return copyingMap;

  if ( QgsCptCityArchive::mCopyingInfoMap.contains( fileName ) )
  {
    QgsDebugMsg( "found copying info in copyingInfoMap, file = " + fileName );
    return QgsCptCityArchive::mCopyingInfoMap.value( fileName );
  }

  QgsDebugMsg( "fileName = " + fileName );

  // import xml file
  QFile f( fileName );
  if ( !f.open( QFile::ReadOnly ) )
  {
    QgsDebugMsg( "Couldn't open xml file: " + fileName );
    return copyingMap;
  }

  // parse the document
  QDomDocument doc( "license" );
  if ( !doc.setContent( &f ) )
  {
    f.close();
    QgsDebugMsg( "Couldn't parse xml file: " + fileName );
    return copyingMap;
  }
  f.close();

  // get root element
  QDomElement docElem = doc.documentElement();
  if ( docElem.tagName() != "copying" )
  {
    QgsDebugMsg( "Incorrect root tag: " + docElem.tagName() );
    return copyingMap;
  }

  // load author information
  QDomElement authorsElement = docElem.firstChildElement( "authors" );
  if ( authorsElement.isNull() )
  {
    QgsDebugMsg( "authors tag missing" );
  }
  else
  {
    QDomElement e = authorsElement.firstChildElement();
    QStringList authors;
    while ( ! e.isNull() )
    {
      if ( e.tagName() == "author" )
      {
        if ( ! e.firstChildElement( "name" ).isNull() )
          authors << e.firstChildElement( "name" ).text().simplified();
        // org???
      }
      e = e.nextSiblingElement();
    }
    copyingMap[ "authors" ] = authors.join( ", " );
  }

  // load license information
  QDomElement licenseElement = docElem.firstChildElement( "license" );
  if ( licenseElement.isNull() )
  {
    QgsDebugMsg( "license tag missing" );
  }
  else
  {
    QDomElement e = licenseElement.firstChildElement( "informal" );
    if ( ! e.isNull() )
      copyingMap[ "license/informal" ] = e.text().simplified();
    e = licenseElement.firstChildElement( "year" );
    if ( ! e.isNull() )
      copyingMap[ "license/year" ] = e.text().simplified();
    e = licenseElement.firstChildElement( "text" );
    if ( ! e.isNull() && e.attribute( "href" ) != QString() )
      copyingMap[ "license/url" ] = e.attribute( "href" );
  }

  // load src information
  QDomElement element = docElem.firstChildElement( "src" );
  if ( element.isNull() )
  {
    QgsDebugMsg( "src tag missing" );
  }
  else
  {
    QDomElement e = element.firstChildElement( "link" );
    if ( ! e.isNull() && e.attribute( "href" ) != QString() )
      copyingMap[ "src/link" ] = e.attribute( "href" );
  }

  // save copyingMap for further access
  QgsCptCityArchive::mCopyingInfoMap[ fileName ] = copyingMap;
  return copyingMap;
}

QgsStringMap QgsCptCityArchive::description( const QString& fileName )
{
  QgsStringMap descMap;

  QgsDebugMsg( "description fileName = " + fileName );

  QFile f( fileName );
  if ( ! f.open( QFile::ReadOnly ) )
  {
    QgsDebugMsg( "description file " + fileName + " ] does not exist" );
    return descMap;
  }

  // parse the document
  QString errMsg;
  QDomDocument doc( "description" );
  if ( !doc.setContent( &f, &errMsg ) )
  {
    f.close();
    QgsDebugMsg( "Couldn't parse file " + fileName + " : " + errMsg );
    return descMap;
  }
  f.close();

  // read description
  QDomElement docElem = doc.documentElement();
  if ( docElem.tagName() != "description" )
  {
    QgsDebugMsg( "Incorrect root tag: " + docElem.tagName() );
    return descMap;
  }
  // should we make sure the <dir> tag is ok?

  QDomElement e = docElem.firstChildElement( "name" );
  if ( e.isNull() )
  {
    QgsDebugMsg( "name tag missing" );
  }
  descMap[ "name" ] = e.text().simplified();
  e = docElem.firstChildElement( "full" );
  if ( e.isNull() )
  {
    QgsDebugMsg( "full tag missing" );
  }
  descMap[ "full" ] = e.text().simplified();

  return descMap;
}

QMap< double, QPair<QColor, QColor> >QgsCptCityArchive::gradientColorMap( const QString& fileName )
{
  QMap< double, QPair<QColor, QColor> > colorMap;

  // import xml file
  QFile f( fileName );
  if ( !f.open( QFile::ReadOnly ) )
  {
    QgsDebugMsg( "Couldn't open SVG file: " + fileName );
    return colorMap;
  }

  // parse the document
  QDomDocument doc( "gradient" );
  if ( !doc.setContent( &f ) )
  {
    f.close();
    QgsDebugMsg( "Couldn't parse SVG file: " + fileName );
    return colorMap;
  }
  f.close();

  QDomElement docElem = doc.documentElement();

  if ( docElem.tagName() != "svg" )
  {
    QgsDebugMsg( "Incorrect root tag: " + docElem.tagName() );
    return colorMap;
  }

  // load color ramp from first linearGradient node
  QDomElement rampsElement = docElem.firstChildElement( "linearGradient" );
  if ( rampsElement.isNull() )
  {
    QDomNodeList nodeList = docElem.elementsByTagName( "linearGradient" );
    if ( ! nodeList.isEmpty() )
      rampsElement = nodeList.at( 0 ).toElement();
  }
  if ( rampsElement.isNull() )
  {
    QgsDebugMsg( "linearGradient tag missing" );
    return colorMap;
  }

  // loop for all stop tags
  QDomElement e = rampsElement.firstChildElement();

  while ( !e.isNull() )
  {
    if ( e.tagName() == "stop" )
    {
      //todo integrate this into symbollayerutils, keep here for now...
      double offset;
      QString offsetStr = e.attribute( "offset" ); // offset="50.00%" | offset="0.5"
      QString colorStr = e.attribute( "stop-color", "" ); // stop-color="rgb(222,235,247)"
      QString opacityStr = e.attribute( "stop-opacity", "1.0" ); // stop-opacity="1.0000"
      if ( offsetStr.endsWith( "%" ) )
        offset = offsetStr.remove( offsetStr.size() - 1, 1 ).toDouble() / 100.0;
      else
        offset = offsetStr.toDouble();

      // QColor color( 255, 0, 0 ); // red color as a warning :)
      QColor color = QgsSymbolLayerV2Utils::parseColor( colorStr );
      if ( color != QColor() )
      {
        int alpha = opacityStr.toDouble() * 255; // test
        color.setAlpha( alpha );
        if ( colorMap.contains( offset ) )
          colorMap[offset].second = color;
        else
          colorMap[offset] = qMakePair( color, color );
      }
      else
      {
        QgsDebugMsg( QString( "at offset=%1 invalid color" ).arg( offset ) );
      }
    }
    else
    {
      QgsDebugMsg( "unknown tag: " + e.tagName() );
    }

    e = e.nextSiblingElement();
  }

  return colorMap;
}

bool QgsCptCityArchive::isEmpty()
{
  return ( mRootItems.isEmpty() );
}


QgsCptCityArchive* QgsCptCityArchive::defaultArchive()
{
  QSettings settings;
  mDefaultArchiveName = settings.value( "CptCity/archiveName", DEFAULT_CPTCITY_ARCHIVE ).toString();
  if ( QgsCptCityArchive::mArchiveRegistry.contains( mDefaultArchiveName ) )
    return QgsCptCityArchive::mArchiveRegistry.value( mDefaultArchiveName );
  else
    return NULL;
}

void QgsCptCityArchive::initArchive( QString archiveName, QString archiveBaseDir )
{
  QgsDebugMsg( "archiveName = " + archiveName + " archiveBaseDir = " + archiveBaseDir );
  QgsCptCityArchive *archive = new QgsCptCityArchive( archiveName, archiveBaseDir );
  if ( mArchiveRegistry.contains( archiveName ) )
    delete mArchiveRegistry[ archiveName ];
  mArchiveRegistry[ archiveName ] = archive;
}

void QgsCptCityArchive::initDefaultArchive()
{
  QSettings settings;
  // use CptCity/baseDir setting if set, default is user dir
  QString baseDir = settings.value( "CptCity/baseDir",
                                    QgsApplication::pkgDataPath() + "/resources" ).toString();
  // sub-dir defaults to
  QString defArchiveName = settings.value( "CptCity/archiveName", DEFAULT_CPTCITY_ARCHIVE ).toString();

  if ( ! mArchiveRegistry.contains( defArchiveName ) )
    initArchive( defArchiveName, baseDir + QDir::separator() + defArchiveName );
}

void QgsCptCityArchive::initArchives( bool loadAll )
{
  QgsStringMap archivesMap;
  QString baseDir, defArchiveName;
  QSettings settings;

  // use CptCity/baseDir setting if set, default is user dir
  baseDir = settings.value( "CptCity/baseDir",
                            QgsApplication::pkgDataPath() + "/resources" ).toString();
  // sub-dir defaults to
  defArchiveName = settings.value( "CptCity/archiveName", DEFAULT_CPTCITY_ARCHIVE ).toString();

  QgsDebugMsg( "baseDir= " + baseDir + " defArchiveName= " + defArchiveName );
  if ( loadAll )
  {
    QDir dir( baseDir );
    foreach ( QString entry, dir.entryList( QStringList( "cpt-city*" ), QDir::Dirs ) )
    {
      if ( QFile::exists( baseDir + QDir::separator() + entry + "/VERSION.xml" ) )
        archivesMap[ entry ] = baseDir + QDir::separator() + entry;
    }
  }
  else
  {
    archivesMap[ defArchiveName ] = baseDir + QDir::separator() + defArchiveName;
  }

  for ( QgsStringMap::iterator it = archivesMap.begin();
        it != archivesMap.end(); ++it )
  {
    if ( QDir( it.value() ).exists() )
      QgsCptCityArchive::initArchive( it.key(), it.value() );
    else
    {
      QgsDebugMsg( QString( "not loading archive [%1] because dir %2 does not exist " ).arg( it.key() ).arg( it.value() ) );
    }
  }
  mDefaultArchiveName = defArchiveName;
}

void QgsCptCityArchive::clearArchives()
{
  for ( QMap< QString, QgsCptCityArchive* >::iterator it = mArchiveRegistry.begin();
        it != mArchiveRegistry.end(); ++it )
    delete it.value();
  mArchiveRegistry.clear();
}


// --------

QgsCptCityDataItem::QgsCptCityDataItem( QgsCptCityDataItem::Type type, QgsCptCityDataItem* parent,
                                        QString name, QString path )
// Do not pass parent to QObject, Qt would delete this when parent is deleted
    : QObject(), mType( type ), mParent( parent ), mPopulated( false ),
    mName( name ), mPath( path ), mValid( true )
{
}

QgsCptCityDataItem::~QgsCptCityDataItem()
{
  // QgsDebugMsg( "mName = " + mName + " mPath = " + mPath );
}

void QgsCptCityDataItem::emitBeginInsertItems( QgsCptCityDataItem* parent, int first, int last )
{
  emit beginInsertItems( parent, first, last );
}
void QgsCptCityDataItem::emitEndInsertItems()
{
  emit endInsertItems();
}
void QgsCptCityDataItem::emitBeginRemoveItems( QgsCptCityDataItem* parent, int first, int last )
{
  emit beginRemoveItems( parent, first, last );
}
void QgsCptCityDataItem::emitEndRemoveItems()
{
  emit endRemoveItems();
}

QVector<QgsCptCityDataItem*> QgsCptCityDataItem::createChildren()
{
  QVector<QgsCptCityDataItem*> children;
  return children;
}

void QgsCptCityDataItem::populate()
{
  if ( mPopulated )
    return;

  QgsDebugMsg( "mPath = " + mPath );

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QVector<QgsCptCityDataItem*> children = createChildren();
  foreach ( QgsCptCityDataItem *child, children )
  {
    // initialization, do not refresh! That would result in infinite loop (beginInsertItems->rowCount->populate)
    addChildItem( child );
  }
  mPopulated = true;

  QApplication::restoreOverrideCursor();
}

int QgsCptCityDataItem::rowCount()
{
  // if ( !mPopulated )
  //   populate();
  return mChildren.size();
}
bool QgsCptCityDataItem::hasChildren()
{
  return ( mPopulated ? mChildren.count() > 0 : true );
}

void QgsCptCityDataItem::addChildItem( QgsCptCityDataItem * child, bool refresh )
{
  QgsDebugMsg( QString( "add child #%1 - %2 - %3" ).arg( mChildren.size() ).arg( child->mName ).arg( child->mType ) );

  int i;
  if ( type() == ColorRamp )
  {
    for ( i = 0; i < mChildren.size(); i++ )
    {
      // sort items by type, so directories are after data items
      if ( mChildren[i]->mType == child->mType &&
           mChildren[i]->mName.localeAwareCompare( child->mName ) >= 0 )
        break;
    }
  }
  else
  {
    for ( i = 0; i < mChildren.size(); i++ )
    {
      if ( mChildren[i]->mName.localeAwareCompare( child->mName ) >= 0 )
        break;
    }
  }

  if ( refresh )
    emit beginInsertItems( this, i, i );

  mChildren.insert( i, child );

  connect( child, SIGNAL( beginInsertItems( QgsCptCityDataItem*, int, int ) ),
           this, SLOT( emitBeginInsertItems( QgsCptCityDataItem*, int, int ) ) );
  connect( child, SIGNAL( endInsertItems() ),
           this, SLOT( emitEndInsertItems() ) );
  connect( child, SIGNAL( beginRemoveItems( QgsCptCityDataItem*, int, int ) ),
           this, SLOT( emitBeginRemoveItems( QgsCptCityDataItem*, int, int ) ) );
  connect( child, SIGNAL( endRemoveItems() ),
           this, SLOT( emitEndRemoveItems() ) );

  if ( refresh )
    emit endInsertItems();
}
void QgsCptCityDataItem::deleteChildItem( QgsCptCityDataItem * child )
{
  // QgsDebugMsg( "mName = " + child->mName );
  int i = mChildren.indexOf( child );
  Q_ASSERT( i >= 0 );
  emit beginRemoveItems( this, i, i );
  mChildren.remove( i );
  delete child;
  emit endRemoveItems();
}

QgsCptCityDataItem * QgsCptCityDataItem::removeChildItem( QgsCptCityDataItem * child )
{
  // QgsDebugMsg( "mName = " + child->mName );
  int i = mChildren.indexOf( child );
  Q_ASSERT( i >= 0 );
  emit beginRemoveItems( this, i, i );
  mChildren.remove( i );
  emit endRemoveItems();
  disconnect( child, SIGNAL( beginInsertItems( QgsCptCityDataItem*, int, int ) ),
              this, SLOT( emitBeginInsertItems( QgsCptCityDataItem*, int, int ) ) );
  disconnect( child, SIGNAL( endInsertItems() ),
              this, SLOT( emitEndInsertItems() ) );
  disconnect( child, SIGNAL( beginRemoveItems( QgsCptCityDataItem*, int, int ) ),
              this, SLOT( emitBeginRemoveItems( QgsCptCityDataItem*, int, int ) ) );
  disconnect( child, SIGNAL( endRemoveItems() ),
              this, SLOT( emitEndRemoveItems() ) );
  child->setParent( 0 );
  return child;
}

int QgsCptCityDataItem::findItem( QVector<QgsCptCityDataItem*> items, QgsCptCityDataItem * item )
{
  for ( int i = 0; i < items.size(); i++ )
  {
    // QgsDebugMsg( QString::number( i ) + " : " + items[i]->mPath + " x " + item->mPath );
    if ( items[i]->equal( item ) )
      return i;
  }
  return -1;
}

void QgsCptCityDataItem::refresh()
{
  QgsDebugMsg( "mPath = " + mPath );

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QVector<QgsCptCityDataItem*> items = createChildren( );

  // Remove no more present items
  QVector<QgsCptCityDataItem*> remove;
  foreach ( QgsCptCityDataItem *child, mChildren )
  {
    if ( findItem( items, child ) >= 0 )
      continue;
    remove.append( child );
  }
  foreach ( QgsCptCityDataItem *child, remove )
  {
    deleteChildItem( child );
  }

  // Add new items
  foreach ( QgsCptCityDataItem *item, items )
  {
    // Is it present in childs?
    if ( findItem( mChildren, item ) >= 0 )
    {
      delete item;
      continue;
    }
    addChildItem( item, true );
  }

  QApplication::restoreOverrideCursor();
}

bool QgsCptCityDataItem::equal( const QgsCptCityDataItem *other )
{
  if ( metaObject()->className() == other->metaObject()->className() &&
       mPath == other->path() )
  {
    return true;
  }
  return false;
}

// ---------------------------------------------------------------------

QgsCptCityColorRampItem::QgsCptCityColorRampItem( QgsCptCityDataItem* parent,
    QString name, QString path, QString variantName )
    : QgsCptCityDataItem( ColorRamp, parent, name, path ),
    mInitialised( false ), mRamp( path, variantName, false )
{
  // QgsDebugMsg( "name= " + name + " path= " + path );
  mPopulated = true;
  //init();
}

QgsCptCityColorRampItem::QgsCptCityColorRampItem( QgsCptCityDataItem* parent,
    QString name, QString path, QStringList variantList )
    : QgsCptCityDataItem( ColorRamp, parent, name, path ),
    mInitialised( false ), mRamp( path, variantList, QString(), false )
{
  // QgsDebugMsg( "name= " + name + " path= " + path );
  mPopulated = true;
  //init();
}

// TODO only load file when icon is requested...
void QgsCptCityColorRampItem::init( )
{
  if ( mInitialised )
    return;
  mInitialised = true;

  QgsDebugMsg( "path = " + path() );

  // make preview from variant if exists
  QStringList variantList = mRamp.variantList();
  if ( mRamp.variantName().isNull() && ! variantList.isEmpty() )
    mRamp.setVariantName( variantList[ variantList.count() / 2 ] );

  mRamp.loadFile();

  // is this item valid? this might fail when there are variants, check
  if ( ! QFile::exists( mRamp.fileName() ) )
    mValid = false;
  else
    mValid = true;

  // load file and set info
  if ( mRamp.count() > 0 )
  {
    if ( variantList.isEmpty() )
    {
      int count = mRamp.count();
      if ( mRamp.isDiscrete() )
        count--;
      mInfo = QString::number( count ) + " " + tr( "colors" ) + " - ";
      if ( mRamp.isDiscrete() )
        mInfo += tr( "discrete" );
      else
      {
        if ( !mRamp.hasMultiStops() )
          mInfo += tr( "continuous" );
        else
          mInfo += tr( "continuous (multi)" );
      }
      mShortInfo = QFileInfo( mName ).fileName();
    }
    else
    {
      mInfo = QString::number( variantList.count() ) + " " + tr( "variants" );
      // mShortInfo = QFileInfo( mName ).fileName() + " (" + QString::number( variantList.count() ) + ")";
      mShortInfo = QFileInfo( mName ).fileName();
    }
  }
  else
  {
    mInfo = "";
  }

}

bool QgsCptCityColorRampItem::equal( const QgsCptCityDataItem *other )
{
  //QgsDebugMsg ( mPath + " x " + other->mPath );
  if ( type() != other->type() )
  {
    return false;
  }
  //const QgsCptCityColorRampItem *o = qobject_cast<const QgsCptCityColorRampItem *> ( other );
  const QgsCptCityColorRampItem *o = dynamic_cast<const QgsCptCityColorRampItem *>( other );
  return ( mPath == o->mPath && mName == o->mName &&
           ramp().variantName() == o->ramp().variantName() );
}

QIcon QgsCptCityColorRampItem::icon()
{
  return icon( QSize( 100, 15 ) );
}

QIcon QgsCptCityColorRampItem::icon( const QSize& size )
{
  foreach ( QIcon icon, mIcons )
  {
    if ( icon.availableSizes().contains( size ) )
      return icon;
  }

  QIcon icon( size );

  init();

  if ( mValid && mRamp.count() > 0 )
  {
    icon = QgsSymbolLayerV2Utils::colorRampPreviewIcon( &mRamp, size );
  }
  else
  {
    QPixmap blankPixmap( size );
    blankPixmap.fill( Qt::white );
    icon = QIcon( blankPixmap );
    mInfo = "";
  }

  mIcons.append( icon );
  return icon;
}

// ---------------------------------------------------------------------
QgsCptCityCollectionItem::QgsCptCityCollectionItem( QgsCptCityDataItem* parent,
    QString name, QString path )
    : QgsCptCityDataItem( Collection, parent, name, path ), mPopulatedRamps( false )
{
}

QgsCptCityCollectionItem::~QgsCptCityCollectionItem()
{
  // QgsDebugMsg( "Entered" );
  foreach ( QgsCptCityDataItem* i, mChildren )
  {
    // QgsDebugMsg( QString( "delete child = 0x%0" ).arg(( qlonglong )i, 8, 16, QLatin1Char( '0' ) ) );
    delete i;
  }
}

QVector< QgsCptCityDataItem* > QgsCptCityCollectionItem::childrenRamps( bool recursive )
{
  QVector< QgsCptCityDataItem* > rampItems;

  populate();

  // recursively add children
  foreach ( QgsCptCityDataItem* childItem, children() )
  {
    QgsCptCityCollectionItem* collectionItem = dynamic_cast<QgsCptCityCollectionItem*>( childItem );
    QgsCptCityColorRampItem* rampItem = dynamic_cast<QgsCptCityColorRampItem*>( childItem );
    QgsDebugMsg( QString( "child path= %1 coll= %2 ramp = %3" ).arg( childItem->path() ).arg( collectionItem != 0 ).arg( rampItem != 0 ) );
    if ( collectionItem && recursive )
    {
      collectionItem->populate();
      rampItems << collectionItem->childrenRamps( true );
    }
    else if ( rampItem )
    {
      // init rampItem to get palette and icon, test if is valid after loading file
      rampItem->init();
      if ( rampItem->isValid() )
        rampItems << rampItem;
      // should also delete item from parent, but we are in a loop now
    }
    else
    {
      QgsDebugMsg( "invalid item " + childItem->path() );
    }
  }
  return rampItems;
}

//-----------------------------------------------------------------------
QgsCptCityDirectoryItem::QgsCptCityDirectoryItem( QgsCptCityDataItem* parent,
    QString name, QString path )
    : QgsCptCityCollectionItem( parent, name, path )
{
  mType = Directory;
  mValid = QDir( QgsCptCityArchive::defaultBaseDir() + QDir::separator() + mPath ).exists();
  if ( ! mValid )
  {
    QgsDebugMsg( "created invalid dir item, path = " + QgsCptCityArchive::defaultBaseDir()
                 + QDir::separator() + mPath );
  }

  // parse DESC.xml to get mInfo
  mInfo = "";
  QString fileName = QgsCptCityArchive::defaultBaseDir() + QDir::separator() + \
                     mPath + QDir::separator() + "DESC.xml";
  QgsStringMap descMap = QgsCptCityArchive::description( fileName );
  if ( descMap.contains( "name" ) )
    mInfo = descMap.value( "name" );

  // populate();
}

QgsCptCityDirectoryItem::~QgsCptCityDirectoryItem()
{
}

QVector<QgsCptCityDataItem*> QgsCptCityDirectoryItem::createChildren()
{
  if ( ! mValid )
    return QVector<QgsCptCityDataItem*>();

  QVector<QgsCptCityDataItem*> children;

  // add children schemes
  QMapIterator< QString, QStringList> it( rampsMap() );
  while ( it.hasNext() )
  {
    it.next();
    // QgsDebugMsg( "schemeName = " + it.key() );
    QgsCptCityDataItem* item =
      new QgsCptCityColorRampItem( this, it.key(), it.key(), it.value() );
    if ( item->isValid() )
      children << item;
    else
      delete item;
  }

  // add children dirs
  foreach ( QString childPath, dirEntries() )
  {
    QgsCptCityDataItem* childItem =
      QgsCptCityDirectoryItem::dataItem( this, childPath, mPath + QDir::separator() + childPath );
    if ( childItem )
      children << childItem;
  }

  QgsDebugMsg( QString( "name= %1 path= %2 found %3 children" ).arg( mName ).arg( mPath ).arg( children.count() ) );

  return children;
}

QMap< QString, QStringList > QgsCptCityDirectoryItem::rampsMap()
{
  if ( ! mRampsMap.isEmpty() )
    return mRampsMap;

  QString curName, prevName, prevPath, curVariant, curSep, schemeName;
  QStringList listVariant;
  QStringList schemeNamesAll, schemeNames;
  int num;
  bool ok, prevAdd, curAdd;

  QDir dir( QgsCptCityArchive::defaultBaseDir() + QDir::separator() + mPath );
  schemeNamesAll = dir.entryList( QStringList( "*.svg" ), QDir::Files, QDir::Name );

  // TODO detect if there are duplicate names with different variant counts, combine in 1
  for ( int i = 0; i < schemeNamesAll.count(); i++ )
  {
    // schemeName = QFileInfo( schemeNamesAll[i] ).baseName();
    schemeName = schemeNamesAll[i];
    schemeName.chop( 4 );
    // QgsDebugMsg("=============");
    // QgsDebugMsg("scheme = "+schemeName);
    curName = schemeName;
    curVariant = "";

    // stupid code to find if name ends with 1-3 digit number - should use regexp
    // TODO need to detect if ends with b/c also
    if ( schemeName.length() > 1 && schemeName.endsWith( "a" ) && ! listVariant.isEmpty() &&
         (( prevName + listVariant.last()  + "a" ) == curName ) )
    {
      curName = prevName;
      curVariant = listVariant.last() + "a";
    }
    else
    {
      num = schemeName.right( 3 ).toInt( &ok );
      Q_UNUSED( num );
      if ( ok )
      {
        curName = schemeName.left( schemeName.size() - 3 );
        curVariant = schemeName.right( 3 );
      }
      else
      {
        num = schemeName.right( 2 ).toInt( &ok );
        if ( ok )
        {
          curName = schemeName.left( schemeName.size() - 2 );
          curVariant = schemeName.right( 2 );
        }
        else
        {
          num = schemeName.right( 1 ).toInt( &ok );
          if ( ok )
          {
            curName = schemeName.left( schemeName.size() - 1 );
            curVariant = schemeName.right( 1 );
          }
        }
      }
    }
    curSep = curName.right( 1 );
    if ( curSep == "-" || curSep == "_" )
    {
      curName.chop( 1 );
      curVariant = curSep + curVariant;
    }

    if ( prevName == "" )
      prevName = curName;

    // add element, unless it is empty, or a variant of last element
    prevAdd = false;
    curAdd = false;
    if ( curName == "" )
      curName = "__empty__";
    // if current is a variant of last, don't add previous and append current variant
    if ( curName == prevName )
    {
      // add current element if it is the last one in the archive
      if ( i == schemeNamesAll.count() - 1 )
        prevAdd = true;
      listVariant << curVariant;
    }
    else
    {
      if ( prevName != "" )
      {
        prevAdd = true;
      }
      // add current element if it is the last one in the archive
      if ( i == schemeNamesAll.count() - 1 )
        curAdd = true;
    }

    // QgsDebugMsg(QString("prevAdd=%1 curAdd=%2 prevName=%3 curName=%4 count=%5").arg(prevAdd).arg(curAdd).arg(prevName).arg(curName).arg(listVariant.count()));

    if ( prevAdd )
    {
      // depending on number of variants, make one or more items
      if ( listVariant.count() == 0 )
      {
        // set num colors=-1 to parse file on request only
        // mSchemeNumColors[ prevName ] = -1;
        schemeNames << prevName;
        mRampsMap[ mPath + QDir::separator() + prevName ] = QStringList();
      }
      else if ( listVariant.count() <= 3 )
      {
        // for 1-2 items, create independent items
        for ( int j = 0; j < listVariant.count(); j++ )
        {
          // mSchemeNumColors[ prevName + listVariant[j] ] = -1;
          schemeNames << prevName + listVariant[j];
          mRampsMap[ mPath + QDir::separator() + prevName + listVariant[j] ] = QStringList();
        }
      }
      else
      {
        // mSchemeVariants[ path + QDir::separator() + prevName ] = listVariant;
        mRampsMap[ mPath + QDir::separator() + prevName ] = listVariant;
        schemeNames << prevName;
      }
      listVariant.clear();
    }
    if ( curAdd )
    {
      if ( curVariant != "" )
        curName += curVariant;
      schemeNames << curName;
      mRampsMap[ mPath + QDir::separator() + curName ] = QStringList();
    }
    // save current to compare next
    if ( prevAdd || curAdd )
    {
      prevName = curName;
      if ( curVariant != "" )
        listVariant << curVariant;
    }

  }
  //TODO what to do with other vars? e.g. schemeNames
  // // add schemes to archive
  // mSchemeMap[ path ] = schemeNames;
  // schemeCount += schemeName.count();
  // schemeNames.clear();
  // listVariant.clear();
  // prevName = "";
  return mRampsMap;
}

QStringList QgsCptCityDirectoryItem::dirEntries() const
{
  return QDir( QgsCptCityArchive::defaultBaseDir() +                    \
               QDir::separator() + mPath ).entryList( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );
}

bool QgsCptCityDirectoryItem::equal( const QgsCptCityDataItem *other )
{
  //QgsDebugMsg ( mPath + " x " + other->mPath );
  if ( type() != other->type() )
  {
    return false;
  }
  return ( path() == other->path() );
}

QgsCptCityDataItem* QgsCptCityDirectoryItem::dataItem( QgsCptCityDataItem* parent,
    QString name, QString path )
{
  QgsDebugMsg( "name= " + name + " path= " + path );

  // first create item with constructor
  QgsCptCityDirectoryItem* dirItem = new QgsCptCityDirectoryItem( parent, name, path );
  if ( dirItem && ! dirItem->isValid() )
  {
    delete dirItem;
    return 0;
  }
  if ( ! dirItem )
    return 0;

  // fetch sub-dirs and ramps to know what to do with this item
  QStringList theDirEntries = dirItem->dirEntries();
  QMap< QString, QStringList > theRampsMap = dirItem->rampsMap();

  QgsDebugMsg( QString( "item has %1 dirs and %2 ramps" ).arg( theDirEntries.count() ).arg( theRampsMap.count() ) );

  // return item if has at least one subdir
  if ( theDirEntries.count() > 0 )
    return dirItem;

  // if 0 ramps, delete item
  if ( theRampsMap.count() == 0 )
  {
    delete dirItem;
    return 0;
  }
  // if 1 ramp, return this child's item
  // so we don't have a directory with just 1 item (with many variants possibly)
  else if ( theRampsMap.count() == 1 )
  {
    delete dirItem;
    QgsCptCityColorRampItem* rampItem =
      new QgsCptCityColorRampItem( parent, theRampsMap.begin().key(),
                                   theRampsMap.begin().key(), theRampsMap.begin().value() );
    if ( ! rampItem->isValid() )
    {
      delete rampItem;
      return 0;
    }
    return rampItem;
  }
  return dirItem;
}


//-----------------------------------------------------------------------
QgsCptCitySelectionItem::QgsCptCitySelectionItem( QgsCptCityDataItem* parent,
    QString name, QString path )
    : QgsCptCityCollectionItem( parent, name, path )
{
  mType = Selection;
  mValid = ! path.isNull();
  if ( mValid )
    parseXML();
}

QgsCptCitySelectionItem::~QgsCptCitySelectionItem()
{
}

QVector<QgsCptCityDataItem*> QgsCptCitySelectionItem::createChildren()
{
  if ( ! mValid )
    return QVector<QgsCptCityDataItem*>();

  QgsCptCityDataItem* item = 0;
  QVector<QgsCptCityDataItem*> children;

  QgsDebugMsg( "name= " + mName + " path= " + mPath );

  // add children archives
  foreach ( QString childPath, mSelectionsList )
  {
    QgsDebugMsg( "childPath = " + childPath + " name= " + QFileInfo( childPath ).baseName() );
    if ( childPath.endsWith( "/" ) )
    {
      childPath.chop( 1 );
      QgsCptCityDataItem* childItem =
        QgsCptCityDirectoryItem::dataItem( this, childPath, childPath );
      if ( childItem )
        children << childItem;
    }
    else
    {
      item = new QgsCptCityColorRampItem( this, childPath, childPath );
      if ( item->isValid() )
        children << item;
      else
        delete item;
    }
  }

  QgsDebugMsg( QString( "path= %1 inserted %2 children" ).arg( mPath ).arg( children.count() ) );

  return children;
}

void QgsCptCitySelectionItem::parseXML()
{
  QString filename = QgsCptCityArchive::defaultBaseDir() + QDir::separator() + mPath;

  QgsDebugMsg( "reading file " + filename );

  QFile f( filename );
  if ( ! f.open( QFile::ReadOnly ) )
  {
    QgsDebugMsg( filename + " does not exist" );
    return;
  }

  // parse the document
  QString errMsg;
  QDomDocument doc( "selection" );
  if ( !doc.setContent( &f, &errMsg ) )
  {
    f.close();
    QgsDebugMsg( "Couldn't parse file " + filename + " : " + errMsg );
    return;
  }
  f.close();

  // read description
  QDomElement docElem = doc.documentElement();
  if ( docElem.tagName() != "selection" )
  {
    QgsDebugMsg( "Incorrect root tag: " + docElem.tagName() );
    return;
  }
  QDomElement e = docElem.firstChildElement( "name" );
  if ( ! e.isNull() && ! e.text().isNull() )
    mName = e.text();
  mInfo = docElem.firstChildElement( "synopsis" ).text().simplified();

  // get archives
  QDomElement collectsElem = docElem.firstChildElement( "seealsocollects" );
  e = collectsElem.firstChildElement( "collect" );
  while ( ! e.isNull() )
  {
    if ( ! e.attribute( "dir" ).isNull() )
    {
      // TODO parse description and use that, instead of default archive name
      mSelectionsList << e.attribute( "dir" ) + "/";
    }
    e = e.nextSiblingElement();
  }
  // get individual gradients
  QDomElement gradientsElem = docElem.firstChildElement( "gradients" );
  e = gradientsElem.firstChildElement( "gradient" );
  while ( ! e.isNull() )
  {
    if ( ! e.attribute( "dir" ).isNull() )
    {
      // QgsDebugMsg( "add " + e.attribute( "dir" ) + "/" + e.attribute( "file" ) + " to " + selname );
      // TODO parse description and save elsewhere
      mSelectionsList << e.attribute( "dir" ) + "/" + e.attribute( "file" );
    }
    e = e.nextSiblingElement();
  }
}

bool QgsCptCitySelectionItem::equal( const QgsCptCityDataItem *other )
{
  //QgsDebugMsg ( mPath + " x " + other->mPath );
  if ( type() != other->type() )
  {
    return false;
  }
  return ( path() == other->path() );
}

//-----------------------------------------------------------------------
QgsCptCityAllRampsItem::QgsCptCityAllRampsItem( QgsCptCityDataItem* parent,
    QString name,  QVector<QgsCptCityDataItem*> items )
    : QgsCptCityCollectionItem( parent, name, QString() ), mItems( items )
{
  mType = AllRamps;
  mValid = true;
  // populate();
}

QgsCptCityAllRampsItem::~QgsCptCityAllRampsItem()
{
}

QVector<QgsCptCityDataItem*> QgsCptCityAllRampsItem::createChildren()
{
  if ( ! mValid )
    return QVector<QgsCptCityDataItem*>();

  QVector<QgsCptCityDataItem*> children;

  // add children ramps of each item
  foreach ( QgsCptCityDataItem* item, mItems )
  {
    QgsCptCityCollectionItem* colItem = dynamic_cast< QgsCptCityCollectionItem* >( item );
    if ( colItem )
      children += colItem->childrenRamps( true );
  }

  return children;
}

//-----------------------------------------------------------------------

QgsCptCityBrowserModel::QgsCptCityBrowserModel( QObject *parent,
    QgsCptCityArchive* archive, ViewType viewType )
    : QAbstractItemModel( parent ), mArchive( archive ), mViewType( viewType )
{
  Q_ASSERT( mArchive != NULL );
  QgsDebugMsg( "archiveName = " + archive->archiveName() + " viewType=" + ( int ) viewType );
  // keep iconsize for now, but not effectively used
  mIconSize = QSize( 100, 15 );
  addRootItems();
}

QgsCptCityBrowserModel::~QgsCptCityBrowserModel()
{
  removeRootItems();
}

void QgsCptCityBrowserModel::addRootItems( )
{
  if ( mViewType == Authors )
  {
    mRootItems = mArchive->rootItems();
  }
  else if ( mViewType == Selections )
  {
    mRootItems = mArchive->selectionItems();
  }
  QgsDebugMsg( QString( "added %1 root items" ).arg( mRootItems.size() ) );
}

void QgsCptCityBrowserModel::removeRootItems()
{
  // don't remove root items, they belong to the QgsCptCityArchive
  // foreach ( QgsCptCityDataItem* item, mRootItems )
  // {
  //   delete item;
  // }

  mRootItems.clear();
}

Qt::ItemFlags QgsCptCityBrowserModel::flags( const QModelIndex & index ) const
{
  if ( !index.isValid() )
    return 0;

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  return flags;
}

QVariant QgsCptCityBrowserModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  QgsCptCityDataItem *item = dataItem( index );

  if ( !item )
  {
    return QVariant();
  }
  else if ( role == Qt::DisplayRole )
  {
    if ( index.column() == 0 )
      return item->name();
    if ( index.column() == 1 )
    {
      return item->info();
    }
  }
  else if ( role == Qt::ToolTipRole )
  {
    if ( item->type() == QgsCptCityDataItem::ColorRamp &&
         mViewType == List )
      return item->path() + "\n" + item->info();
    return item->toolTip();
  }
  else if ( role == Qt::DecorationRole && index.column() == 1 &&
            item->type() == QgsCptCityDataItem::ColorRamp )
  {
    // keep iconsize for now, but not effectively used
    return item->icon( mIconSize );
  }
  else if ( role == Qt::FontRole &&
            ( dynamic_cast< QgsCptCityCollectionItem* >( item ) != 0 ) )
  {
    // collectionitems are larger and bold
    QFont font;
    font.setPointSize( 11 ); //FIXME why is the font so small?
    font.setBold( true );
    return font;
  }
  else
  {
    // unsupported role
    return QVariant();
  }
  return QVariant();
}

QVariant QgsCptCityBrowserModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  Q_UNUSED( section );
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
  {
    if ( section == 0 )
      return QVariant( tr( "Name" ) );
    else if ( section == 1 )
      return QVariant( tr( "Info" ) );
  }
  return QVariant();
}

int QgsCptCityBrowserModel::rowCount( const QModelIndex &parent ) const
{
  //qDebug("rowCount: idx: (valid %d) %d %d", parent.isValid(), parent.row(), parent.column());

  if ( !parent.isValid() )
  {
    // root item: its children are top level items
    return mRootItems.count(); // mRoot
  }
  else
  {
    // ordinary item: number of its children
    QgsCptCityDataItem *item = dataItem( parent );
    return item ? item->rowCount() : 0;
  }
}

bool QgsCptCityBrowserModel::hasChildren( const QModelIndex &parent ) const
{
  if ( !parent.isValid() )
    return true; // root item: its children are top level items

  QgsCptCityDataItem *item = dataItem( parent );

  return item && item->hasChildren();
}

int QgsCptCityBrowserModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return 2;
}

QModelIndex QgsCptCityBrowserModel::findPath( QString path )
{
  QModelIndex theIndex; // starting from root
  bool foundParent = false, foundChild = true;
  QString itemPath;

  QgsDebugMsg( "path = " + path );

  while ( foundChild )
  {
    foundChild = false; // assume that the next child item will not be found

    int i = 0;
    // if root skip first item "All Ramps"
    if ( itemPath.isEmpty() )
      i = 1;
    for ( ; i < rowCount( theIndex ); i++ )
    {
      QModelIndex idx = index( i, 0, theIndex );
      QgsCptCityDataItem *item = dataItem( idx );
      if ( !item )
        return QModelIndex(); // an error occurred

      itemPath = item->path();

      if ( itemPath == path )
      {
        QgsDebugMsg( "Arrived " + itemPath );
        return idx; // we have found the item we have been looking for
      }

      if ( ! itemPath.endsWith( "/" ) )
        itemPath += "/";

      foundParent = false;

      // QgsDebugMsg( "path= " + path + " itemPath= " + itemPath );

      // if we are using a selection collection, search for target in the mapping in this group
      if ( item->type() == QgsCptCityDataItem::Selection )
      {
        const QgsCptCitySelectionItem* selItem = dynamic_cast<const QgsCptCitySelectionItem *>( item );
        foreach ( QString childPath, selItem->selectionsList() )
        {
          if ( childPath.endsWith( "/" ) )
            childPath.chop( 1 );
          // QgsDebugMsg( "childPath= " + childPath );
          if ( path.startsWith( childPath ) )
          {
            foundParent = true;
            break;
          }
        }
      }
      // search for target in parent directory
      else if ( path.startsWith( itemPath ) )
      {
        foundParent = true;
      }

      if ( foundParent )
      {
        QgsDebugMsg( "found parent " + path );
        // we have found a preceding item: stop searching on this level and go deeper
        foundChild = true;
        theIndex = idx;
        if ( canFetchMore( theIndex ) )
          fetchMore( theIndex );
        break;
      }
    }
  }

  return QModelIndex(); // not found
}

void QgsCptCityBrowserModel::reload()
{
  removeRootItems();
  addRootItems();
  reset(); // Qt4.6 brings better methods beginResetModel + endResetModel
}

/* Refresh dir path */
void QgsCptCityBrowserModel::refresh( QString path )
{
  QModelIndex idx = findPath( path );
  if ( idx.isValid() )
  {
    QgsCptCityDataItem* item = dataItem( idx );
    if ( item )
      item->refresh();
  }
}

QModelIndex QgsCptCityBrowserModel::index( int row, int column, const QModelIndex &parent ) const
{
  QgsCptCityDataItem *p = dataItem( parent );
  const QVector<QgsCptCityDataItem*> &items = p ? p->children() : mRootItems;
  QgsCptCityDataItem *item = items.value( row, 0 );
  return item ? createIndex( row, column, item ) : QModelIndex();
}

QModelIndex QgsCptCityBrowserModel::parent( const QModelIndex &index ) const
{
  QgsCptCityDataItem *item = dataItem( index );
  if ( !item )
    return QModelIndex();

  return findItem( item->parent() );
}

QModelIndex QgsCptCityBrowserModel::findItem( QgsCptCityDataItem *item, QgsCptCityDataItem *parent ) const
{
  const QVector<QgsCptCityDataItem*> &items = parent ? parent->children() : mRootItems;

  for ( int i = 0; i < items.size(); i++ )
  {
    if ( items[i] == item )
      return createIndex( i, 0, item );

    QModelIndex childIndex = findItem( item, items[i] );
    if ( childIndex.isValid() )
      return childIndex;
  }

  return QModelIndex();
}

/* Refresh item */
void QgsCptCityBrowserModel::refresh( const QModelIndex& theIndex )
{
  QgsCptCityDataItem *item = dataItem( theIndex );
  if ( !item )
    return;

  QgsDebugMsg( "Refresh " + item->path() );
  item->refresh();
}

void QgsCptCityBrowserModel::beginInsertItems( QgsCptCityDataItem *parent, int first, int last )
{
  QgsDebugMsg( "parent mPath = " + parent->path() );
  QModelIndex idx = findItem( parent );
  if ( !idx.isValid() )
    return;
  QgsDebugMsg( "valid" );
  beginInsertRows( idx, first, last );
  QgsDebugMsg( "end" );
}
void QgsCptCityBrowserModel::endInsertItems()
{
  QgsDebugMsg( "Entered" );
  endInsertRows();
}
void QgsCptCityBrowserModel::beginRemoveItems( QgsCptCityDataItem *parent, int first, int last )
{
  QgsDebugMsg( "parent mPath = " + parent->path() );
  QModelIndex idx = findItem( parent );
  if ( !idx.isValid() )
    return;
  beginRemoveRows( idx, first, last );
}
void QgsCptCityBrowserModel::endRemoveItems()
{
  QgsDebugMsg( "Entered" );
  endRemoveRows();
}
void QgsCptCityBrowserModel::connectItem( QgsCptCityDataItem* item )
{
  connect( item, SIGNAL( beginInsertItems( QgsCptCityDataItem*, int, int ) ),
           this, SLOT( beginInsertItems( QgsCptCityDataItem*, int, int ) ) );
  connect( item, SIGNAL( endInsertItems() ),
           this, SLOT( endInsertItems() ) );
  connect( item, SIGNAL( beginRemoveItems( QgsCptCityDataItem*, int, int ) ),
           this, SLOT( beginRemoveItems( QgsCptCityDataItem*, int, int ) ) );
  connect( item, SIGNAL( endRemoveItems() ),
           this, SLOT( endRemoveItems() ) );
}

bool QgsCptCityBrowserModel::canFetchMore( const QModelIndex & parent ) const
{
  QgsCptCityDataItem* item = dataItem( parent );
  // fetch all items initially so we know which items have children
  // (nicer looking and less confusing)

  if ( ! item )
    return false;

  // except for "All Ramps" - this is populated when clicked on
  if ( item->type() == QgsCptCityDataItem::AllRamps )
    return false;

  item->populate();

  return ( ! item->isPopulated() );
}

void QgsCptCityBrowserModel::fetchMore( const QModelIndex & parent )
{
  QgsCptCityDataItem* item = dataItem( parent );
  if ( item )
    item->populate();
  QgsDebugMsg( "path = " + item->path() );
}


#if 0
QStringList QgsCptCityBrowserModel::mimeTypes() const
{
  QStringList types;
  // In theory the mime type convention is: application/x-vnd.<vendor>.<application>.<type>
  // but it seems a bit over formalized. Would be an application/x-qgis-uri better?
  types << "application/x-vnd.qgis.qgis.uri";
  return types;
}

QMimeData * QgsCptCityBrowserModel::mimeData( const QModelIndexList &indexes ) const
{
  QgsMimeDataUtils::UriList lst;
  foreach ( const QModelIndex &index, indexes )
  {
    if ( index.isValid() )
    {
      QgsCptCityDataItem* ptr = ( QgsCptCityDataItem* ) index.internalPointer();
      if ( ptr->type() != QgsCptCityDataItem::Layer ) continue;
      QgsLayerItem *layer = ( QgsLayerItem* ) ptr;
      lst.append( QgsMimeDataUtils::Uri( ayer ) );
    }
  }
  return QgsMimeDataUtils::encodeUriList( lst );
}

bool QgsCptCityBrowserModel::dropMimeData( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent )
{
  Q_UNUSED( row );
  Q_UNUSED( column );

  QgsCptCityDataItem* destItem = dataItem( parent );
  if ( !destItem )
  {
    QgsDebugMsg( "DROP PROBLEM!" );
    return false;
  }

  return destItem->handleDrop( data, action );
}
#endif

QgsCptCityDataItem *QgsCptCityBrowserModel::dataItem( const QModelIndex &idx ) const
{
  void *v = idx.internalPointer();
  QgsCptCityDataItem *d = reinterpret_cast<QgsCptCityDataItem*>( v );
  Q_ASSERT( !v || d );
  return d;
}
