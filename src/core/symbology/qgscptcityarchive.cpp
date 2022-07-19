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

#include "qgssettings.h"
#include "qgscptcityarchive.h"
#include "qgis.h"
#include "qgsdataprovider.h"
#include "qgslogger.h"
#include "qgsconfig.h"
#include "qgsmimedatautils.h"
#include "qgsapplication.h"
#include "qgssymbollayerutils.h"

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QVector>
#include <QStyle>
#include <QDomDocument>
#include <QDomElement>
#include <QRegularExpression>

typedef QMap< QString, QgsCptCityArchive * > ArchiveRegistry;
typedef QMap< QString, QMap< QString, QString > > CopyingInfoMap;

Q_GLOBAL_STATIC( QString, sDefaultArchiveName )
Q_GLOBAL_STATIC( ArchiveRegistry, sArchiveRegistry )
Q_GLOBAL_STATIC( CopyingInfoMap, sCopyingInfoMap )

QMap< QString, QgsCptCityArchive * > QgsCptCityArchive::archiveRegistry()
{
  return *sArchiveRegistry();
}

QgsCptCityArchive::QgsCptCityArchive( const QString &archiveName, const QString &baseDir )
  : mArchiveName( archiveName )
  , mBaseDir( baseDir )
{
  QgsDebugMsgLevel( "archiveName = " + archiveName + " baseDir = " + baseDir, 2 );

  // make Author items
  QgsCptCityDirectoryItem *dirItem = nullptr;
  const auto constEntryList = QDir( mBaseDir ).entryList( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );
  for ( const QString &path : constEntryList )
  {
    if ( path == QLatin1String( "selections" ) )
      continue;
    QgsDebugMsgLevel( "path= " + path, 2 );
    dirItem = new QgsCptCityDirectoryItem( nullptr, QFileInfo( path ).baseName(), path );
    if ( dirItem->isValid() )
      mRootItems << dirItem;
    else
      delete dirItem;
  }

  // make selection items
  QgsCptCitySelectionItem *selItem = nullptr;
  const QDir seldir( mBaseDir + '/' + "selections" );
  QgsDebugMsgLevel( "populating selection from " + seldir.path(), 2 );
  const QStringList fileList = seldir.entryList( QStringList() << QStringLiteral( "*.xml" ), QDir::Files );
  for ( const QString &selfile : fileList )
  {
    QgsDebugMsgLevel( "file= " + seldir.path() + '/' + selfile, 2 );
    selItem = new QgsCptCitySelectionItem( nullptr, QFileInfo( selfile ).baseName(),
                                           seldir.dirName() +  '/' + selfile );
    //TODO remove item if there are no children (e.g. esri in qgis-sel)
    if ( selItem->isValid() )
      mSelectionItems << selItem;
    else
      delete selItem;
  }

  // make "All Ramps items" (which will contain all ramps without hierarchy)
  QgsCptCityAllRampsItem *allRampsItem = nullptr;
  allRampsItem = new QgsCptCityAllRampsItem( nullptr, QObject::tr( "All Ramps" ),
      mRootItems );
  mRootItems.prepend( allRampsItem );
  allRampsItem = new QgsCptCityAllRampsItem( nullptr, QObject::tr( "All Ramps" ),
      mSelectionItems );
  mSelectionItems.prepend( allRampsItem );
}

QgsCptCityArchive::~QgsCptCityArchive()
{
  const auto constMRootItems = mRootItems;
  for ( QgsCptCityDataItem *item : constMRootItems )
    delete item;
  const auto constMSelectionItems = mSelectionItems;
  for ( QgsCptCityDataItem *item : constMSelectionItems )
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
    return QgsCptCityArchive::defaultBaseDir();
}

QString QgsCptCityArchive::baseDir( QString archiveName )
{
  // search for matching archive in the registry
  if ( archiveName.isNull() )
    archiveName = DEFAULT_CPTCITY_ARCHIVE;
  if ( QgsCptCityArchive *archive = sArchiveRegistry()->value( archiveName, nullptr ) )
    return archive->baseDir();
  else
    return defaultBaseDir();
}

QString QgsCptCityArchive::defaultBaseDir()
{
  QString baseDir, archiveName;
  const QgsSettings settings;

  // use CptCity/baseDir setting if set, default is user dir
  baseDir = settings.value( QStringLiteral( "CptCity/baseDir" ),
                            QString( QgsApplication::pkgDataPath() + "/resources" ) ).toString();
  // sub-dir defaults to cpt-city
  archiveName = settings.value( QStringLiteral( "CptCity/archiveName" ), DEFAULT_CPTCITY_ARCHIVE ).toString();

  return baseDir + '/' + archiveName;
}


QString QgsCptCityArchive::findFileName( const QString &target, const QString &startDir, const QString &baseDir )
{
  // QgsDebugMsgLevel( "target= " + target +  " startDir= " + startDir +  " baseDir= " + baseDir, 2 );

  if ( startDir.isEmpty() || ! startDir.startsWith( baseDir ) )
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
    return dir.path() + '/' + target;
}


QString QgsCptCityArchive::copyingFileName( const QString &path ) const
{
  return QgsCptCityArchive::findFileName( QStringLiteral( "COPYING.xml" ),
                                          baseDir() + '/' + path, baseDir() );
}

QString QgsCptCityArchive::descFileName( const QString &path ) const
{
  return QgsCptCityArchive::findFileName( QStringLiteral( "DESC.xml" ),
                                          baseDir() + '/' + path, baseDir() );
}

QgsStringMap QgsCptCityArchive::copyingInfo( const QString &fileName )
{
  QgsStringMap copyingMap;

  if ( fileName.isNull() )
    return copyingMap;

  if ( sCopyingInfoMap()->contains( fileName ) )
  {
    QgsDebugMsgLevel( "found copying info in copyingInfoMap, file = " + fileName, 2 );
    return sCopyingInfoMap()->value( fileName );
  }

  QgsDebugMsgLevel( "fileName = " + fileName, 2 );

  // import xml file
  QFile f( fileName );
  if ( !f.open( QFile::ReadOnly ) )
  {
    QgsDebugMsg( "Couldn't open xml file: " + fileName );
    return copyingMap;
  }

  // parse the document
  QDomDocument doc( QStringLiteral( "license" ) );
  if ( !doc.setContent( &f ) )
  {
    f.close();
    QgsDebugMsg( "Couldn't parse xml file: " + fileName );
    return copyingMap;
  }
  f.close();

  // get root element
  const QDomElement docElem = doc.documentElement();
  if ( docElem.tagName() != QLatin1String( "copying" ) )
  {
    QgsDebugMsg( "Incorrect root tag: " + docElem.tagName() );
    return copyingMap;
  }

  // load author information
  const QDomElement authorsElement = docElem.firstChildElement( QStringLiteral( "authors" ) );
  if ( authorsElement.isNull() )
  {
    QgsDebugMsgLevel( QStringLiteral( "authors tag missing" ), 2 );
  }
  else
  {
    QDomElement e = authorsElement.firstChildElement();
    QStringList authors;
    while ( ! e.isNull() )
    {
      if ( e.tagName() == QLatin1String( "author" ) )
      {
        if ( ! e.firstChildElement( QStringLiteral( "name" ) ).isNull() )
          authors << e.firstChildElement( QStringLiteral( "name" ) ).text().simplified();
        // org???
      }
      e = e.nextSiblingElement();
    }
    copyingMap[ QStringLiteral( "authors" )] = authors.join( QLatin1String( ", " ) );
  }

  // load license information
  const QDomElement licenseElement = docElem.firstChildElement( QStringLiteral( "license" ) );
  if ( licenseElement.isNull() )
  {
    QgsDebugMsgLevel( QStringLiteral( "license tag missing" ), 2 );
  }
  else
  {
    QDomElement e = licenseElement.firstChildElement( QStringLiteral( "informal" ) );
    if ( ! e.isNull() )
      copyingMap[ QStringLiteral( "license/informal" )] = e.text().simplified();
    e = licenseElement.firstChildElement( QStringLiteral( "year" ) );
    if ( ! e.isNull() )
      copyingMap[ QStringLiteral( "license/year" )] = e.text().simplified();
    e = licenseElement.firstChildElement( QStringLiteral( "text" ) );
    if ( ! e.isNull() && e.attribute( QStringLiteral( "href" ) ) != QString() )
      copyingMap[ QStringLiteral( "license/url" )] = e.attribute( QStringLiteral( "href" ) );
  }

  // load src information
  const QDomElement element = docElem.firstChildElement( QStringLiteral( "src" ) );
  if ( element.isNull() )
  {
    QgsDebugMsgLevel( QStringLiteral( "src tag missing" ), 2 );
  }
  else
  {
    const QDomElement e = element.firstChildElement( QStringLiteral( "link" ) );
    if ( ! e.isNull() && e.attribute( QStringLiteral( "href" ) ) != QString() )
      copyingMap[ QStringLiteral( "src/link" )] = e.attribute( QStringLiteral( "href" ) );
  }

  // save copyingMap for further access
  ( *sCopyingInfoMap() )[ fileName ] = copyingMap;
  return copyingMap;
}

QgsStringMap QgsCptCityArchive::description( const QString &fileName )
{
  QgsStringMap descMap;

  QgsDebugMsgLevel( "description fileName = " + fileName, 2 );

  QFile f( fileName );
  if ( ! f.open( QFile::ReadOnly ) )
  {
    QgsDebugMsgLevel( "description file " + fileName + " ] does not exist", 2 );
    return descMap;
  }

  // parse the document
  QString errMsg;
  QDomDocument doc( QStringLiteral( "description" ) );
  if ( !doc.setContent( &f, &errMsg ) )
  {
    f.close();
    QgsDebugMsg( "Couldn't parse file " + fileName + " : " + errMsg );
    return descMap;
  }
  f.close();

  // read description
  const QDomElement docElem = doc.documentElement();
  if ( docElem.tagName() != QLatin1String( "description" ) )
  {
    QgsDebugMsg( "Incorrect root tag: " + docElem.tagName() );
    return descMap;
  }
  // should we make sure the <dir> tag is OK?

  QDomElement e = docElem.firstChildElement( QStringLiteral( "name" ) );
  if ( e.isNull() )
  {
    QgsDebugMsgLevel( QStringLiteral( "name tag missing" ), 2 );
  }
  descMap[ QStringLiteral( "name" )] = e.text().simplified();
  e = docElem.firstChildElement( QStringLiteral( "full" ) );
  if ( e.isNull() )
  {
    QgsDebugMsgLevel( QStringLiteral( "full tag missing" ), 2 );
  }
  descMap[ QStringLiteral( "full" )] = e.text().simplified();

  return descMap;
}

QMap< double, QPair<QColor, QColor> >QgsCptCityArchive::gradientColorMap( const QString &fileName )
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
  QDomDocument doc( QStringLiteral( "gradient" ) );
  if ( !doc.setContent( &f ) )
  {
    f.close();
    QgsDebugMsg( "Couldn't parse SVG file: " + fileName );
    return colorMap;
  }
  f.close();

  const QDomElement docElem = doc.documentElement();

  if ( docElem.tagName() != QLatin1String( "svg" ) )
  {
    QgsDebugMsg( "Incorrect root tag: " + docElem.tagName() );
    return colorMap;
  }

  // load color ramp from first linearGradient node
  QDomElement rampsElement = docElem.firstChildElement( QStringLiteral( "linearGradient" ) );
  if ( rampsElement.isNull() )
  {
    const QDomNodeList nodeList = docElem.elementsByTagName( QStringLiteral( "linearGradient" ) );
    if ( ! nodeList.isEmpty() )
      rampsElement = nodeList.at( 0 ).toElement();
  }
  if ( rampsElement.isNull() )
  {
    QgsDebugMsg( QStringLiteral( "linearGradient tag missing" ) );
    return colorMap;
  }

  // loop for all stop tags
  QDomElement e = rampsElement.firstChildElement();

  while ( !e.isNull() )
  {
    if ( e.tagName() == QLatin1String( "stop" ) )
    {
      //todo integrate this into symbollayerutils, keep here for now...
      double offset;
      QString offsetStr = e.attribute( QStringLiteral( "offset" ) ); // offset="50.00%" | offset="0.5"
      const QString colorStr = e.attribute( QStringLiteral( "stop-color" ), QString() ); // stop-color="rgb(222,235,247)"
      const QString opacityStr = e.attribute( QStringLiteral( "stop-opacity" ), QStringLiteral( "1.0" ) ); // stop-opacity="1.0000"
      if ( offsetStr.endsWith( '%' ) )
        offset = offsetStr.remove( offsetStr.size() - 1, 1 ).toDouble() / 100.0;
      else
        offset = offsetStr.toDouble();

      QColor color;
      if ( colorStr.isEmpty() )
      {
        // SVG spec says that stops without color default to black!
        color = QColor( 0, 0, 0 );
      }
      else
      {
        color = QgsSymbolLayerUtils::parseColor( colorStr );
      }

      if ( color.isValid() )
      {
        const int alpha = opacityStr.toDouble() * 255; // test
        color.setAlpha( alpha );
        if ( colorMap.contains( offset ) )
          colorMap[offset].second = color;
        else
          colorMap[offset] = qMakePair( color, color );
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "at offset=%1 invalid color \"%2\"" ).arg( offset ).arg( colorStr ) );
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


QgsCptCityArchive *QgsCptCityArchive::defaultArchive()
{
  const QgsSettings settings;
  *sDefaultArchiveName() = settings.value( QStringLiteral( "CptCity/archiveName" ), DEFAULT_CPTCITY_ARCHIVE ).toString();
  if ( sArchiveRegistry()->contains( *sDefaultArchiveName() ) )
    return sArchiveRegistry()->value( *sDefaultArchiveName() );
  else
    return nullptr;
}

void QgsCptCityArchive::initArchive( const QString &archiveName, const QString &archiveBaseDir )
{
  QgsDebugMsgLevel( "archiveName = " + archiveName + " archiveBaseDir = " + archiveBaseDir, 2 );
  QgsCptCityArchive *archive = new QgsCptCityArchive( archiveName, archiveBaseDir );
  if ( sArchiveRegistry()->contains( archiveName ) )
    delete ( *sArchiveRegistry() )[ archiveName ];
  ( *sArchiveRegistry() )[ archiveName ] = archive;
}

void QgsCptCityArchive::initDefaultArchive()
{
  const QgsSettings settings;
  // use CptCity/baseDir setting if set, default is user dir
  const QString baseDir = settings.value( QStringLiteral( "CptCity/baseDir" ),
                                          QString( QgsApplication::pkgDataPath() + "/resources" ) ).toString();
  // sub-dir defaults to
  const QString defArchiveName = settings.value( QStringLiteral( "CptCity/archiveName" ), DEFAULT_CPTCITY_ARCHIVE ).toString();

  if ( ! sArchiveRegistry()->contains( defArchiveName ) )
    initArchive( defArchiveName, baseDir + '/' + defArchiveName );
}

void QgsCptCityArchive::initArchives( bool loadAll )
{
  QgsStringMap archivesMap;
  QString baseDir, defArchiveName;
  const QgsSettings settings;

  // use CptCity/baseDir setting if set, default is user dir
  baseDir = settings.value( QStringLiteral( "CptCity/baseDir" ),
                            QString( QgsApplication::pkgDataPath() + "/resources" ) ).toString();
  // sub-dir defaults to
  defArchiveName = settings.value( QStringLiteral( "CptCity/archiveName" ), DEFAULT_CPTCITY_ARCHIVE ).toString();

  QgsDebugMsgLevel( "baseDir= " + baseDir + " defArchiveName= " + defArchiveName, 2 );
  if ( loadAll )
  {
    const QDir dir( baseDir );
    const QStringList fileList = dir.entryList( QStringList() << QStringLiteral( "cpt-city*" ), QDir::Dirs );
    for ( const QString &entry : fileList )
    {
      if ( QFile::exists( baseDir + '/' + entry + "/VERSION.xml" ) )
        archivesMap[ entry ] = baseDir + '/' + entry;
    }
  }
  else
  {
    archivesMap[ defArchiveName ] = baseDir + '/' + defArchiveName;
  }

  for ( QgsStringMap::iterator it = archivesMap.begin();
        it != archivesMap.end(); ++it )
  {
    if ( QDir( it.value() ).exists() )
      QgsCptCityArchive::initArchive( it.key(), it.value() );
    else
    {
      QgsDebugMsg( QStringLiteral( "not loading archive [%1] because dir %2 does not exist " ).arg( it.key(), it.value() ) );
    }
  }
  *sDefaultArchiveName() = defArchiveName;
}

void QgsCptCityArchive::clearArchives()
{
  qDeleteAll( *sArchiveRegistry() );
  sArchiveRegistry()->clear();
}


// --------

QgsCptCityDataItem::QgsCptCityDataItem( QgsCptCityDataItem::Type type, QgsCptCityDataItem *parent,
                                        const QString &name, const QString &path )
// Do not pass parent to QObject, Qt would delete this when parent is deleted
  : mType( type )
  , mParent( parent )
  , mPopulated( false )
  , mName( name )
  , mPath( path )
  , mValid( true )
{
}

QVector<QgsCptCityDataItem *> QgsCptCityDataItem::createChildren()
{
  QVector<QgsCptCityDataItem *> children;
  return children;
}

void QgsCptCityDataItem::populate()
{
  if ( mPopulated )
    return;

  QgsDebugMsgLevel( "mPath = " + mPath, 2 );

  QApplication::setOverrideCursor( Qt::WaitCursor );

  const QVector<QgsCptCityDataItem *> children = createChildren();
  const auto constChildren = children;
  for ( QgsCptCityDataItem *child : constChildren )
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

int QgsCptCityDataItem::leafCount() const
{
  if ( !mPopulated )
    return 0;

  int count = 0;
  const auto constMChildren = mChildren;
  for ( QgsCptCityDataItem *child : constMChildren )
  {
    if ( child )
      count += child->leafCount();
  }
  return count;
}


bool QgsCptCityDataItem::hasChildren()
{
  return ( mPopulated ? !mChildren.isEmpty() : true );
}

void QgsCptCityDataItem::addChildItem( QgsCptCityDataItem *child, bool refresh )
{
  QgsDebugMsgLevel( QStringLiteral( "add child #%1 - %2 - %3" ).arg( mChildren.size() ).arg( child->mName ).arg( child->mType ), 2 );

  int i;
  if ( type() == ColorRamp )
  {
    for ( i = 0; i < mChildren.size(); i++ )
    {
      // sort items by type, so directories are after data items
      if ( mChildren.at( i )->mType == child->mType &&
           mChildren.at( i )->mName.localeAwareCompare( child->mName ) >= 0 )
        break;
    }
  }
  else
  {
    for ( i = 0; i < mChildren.size(); i++ )
    {
      if ( mChildren.at( i )->mName.localeAwareCompare( child->mName ) >= 0 )
        break;
    }
  }

  if ( refresh )
    emit beginInsertItems( this, i, i );

  mChildren.insert( i, child );

  connect( child, &QgsCptCityDataItem::beginInsertItems, this, &QgsCptCityDataItem::beginInsertItems );
  connect( child, &QgsCptCityDataItem::endInsertItems, this, &QgsCptCityDataItem::endInsertItems );
  connect( child, &QgsCptCityDataItem::beginRemoveItems, this, &QgsCptCityDataItem::beginRemoveItems );
  connect( child, &QgsCptCityDataItem::endRemoveItems, this, &QgsCptCityDataItem::endRemoveItems );

  if ( refresh )
    emit endInsertItems();
}
void QgsCptCityDataItem::deleteChildItem( QgsCptCityDataItem *child )
{
  // QgsDebugMsgLevel( "mName = " + child->mName, 2 );
  const int i = mChildren.indexOf( child );
  Q_ASSERT( i >= 0 );
  emit beginRemoveItems( this, i, i );
  mChildren.remove( i );
  delete child;
  emit endRemoveItems();
}

QgsCptCityDataItem *QgsCptCityDataItem::removeChildItem( QgsCptCityDataItem *child )
{
  // QgsDebugMsgLevel( "mName = " + child->mName, 2 );
  const int i = mChildren.indexOf( child );
  Q_ASSERT( i >= 0 );
  emit beginRemoveItems( this, i, i );
  mChildren.remove( i );
  emit endRemoveItems();
  disconnect( child, &QgsCptCityDataItem::beginInsertItems, this, &QgsCptCityDataItem::beginInsertItems );
  disconnect( child, &QgsCptCityDataItem::endInsertItems, this, &QgsCptCityDataItem::endInsertItems );
  disconnect( child, &QgsCptCityDataItem::beginRemoveItems, this, &QgsCptCityDataItem::beginRemoveItems );
  disconnect( child, &QgsCptCityDataItem::endRemoveItems, this, &QgsCptCityDataItem::endRemoveItems );
  child->setParent( nullptr );
  return child;
}

int QgsCptCityDataItem::findItem( QVector<QgsCptCityDataItem *> items, QgsCptCityDataItem *item )
{
  for ( int i = 0; i < items.size(); i++ )
  {
    // QgsDebugMsgLevel( QString::number( i ) + " : " + items[i]->mPath + " x " + item->mPath, 2 );
    if ( items[i]->equal( item ) )
      return i;
  }
  return -1;
}

void QgsCptCityDataItem::refresh()
{
  QgsDebugMsgLevel( "mPath = " + mPath, 2 );

  QApplication::setOverrideCursor( Qt::WaitCursor );

  const QVector<QgsCptCityDataItem *> items = createChildren();

  // Remove no more present items
  QVector<QgsCptCityDataItem *> remove;
  const auto constMChildren = mChildren;
  for ( QgsCptCityDataItem *child : constMChildren )
  {
    if ( findItem( items, child ) >= 0 )
      continue;
    remove.append( child );
  }
  const auto constRemove = remove;
  for ( QgsCptCityDataItem *child : constRemove )
  {
    deleteChildItem( child );
  }

  // Add new items
  const auto constItems = items;
  for ( QgsCptCityDataItem *item : constItems )
  {
    // Is it present in children?
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
  return ( metaObject()->className() == other->metaObject()->className() &&
           mPath == other->path() );
}

// ---------------------------------------------------------------------

QgsCptCityColorRampItem::QgsCptCityColorRampItem( QgsCptCityDataItem *parent,
    const QString &name, const QString &path, const QString &variantName, bool initialize )
  : QgsCptCityDataItem( ColorRamp, parent, name, path )
  , mInitialized( false )
  , mRamp( path, variantName, false )
{
  // QgsDebugMsgLevel( "name= " + name + " path= " + path, 2 );
  mPopulated = true;
  if ( initialize )
    init();
}

QgsCptCityColorRampItem::QgsCptCityColorRampItem( QgsCptCityDataItem *parent,
    const QString &name, const QString &path, const QStringList &variantList, bool initialize )
  : QgsCptCityDataItem( ColorRamp, parent, name, path )
  , mInitialized( false )
  , mRamp( path, variantList, QString(), false )
{
  // QgsDebugMsgLevel( "name= " + name + " path= " + path, 2 );
  mPopulated = true;
  if ( initialize )
    init();
}

// TODO only load file when icon is requested...
void QgsCptCityColorRampItem::init()
{
  if ( mInitialized )
    return;
  mInitialized = true;

  QgsDebugMsgLevel( "path = " + path(), 2 );

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
      mInfo = QString::number( count ) + ' ' + tr( "colors" ) + " - ";
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
      mInfo = QString::number( variantList.count() ) + ' ' + tr( "variants" );
      // mShortInfo = QFileInfo( mName ).fileName() + " (" + QString::number( variantList.count() ) + ')';
      mShortInfo = QFileInfo( mName ).fileName();
    }
  }
  else
  {
    mInfo.clear();
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
  const QgsCptCityColorRampItem *o = qobject_cast<const QgsCptCityColorRampItem *>( other );
  return o &&
         mPath == o->mPath &&
         mName == o->mName &&
         ramp().variantName() == o->ramp().variantName();
}

QIcon QgsCptCityColorRampItem::icon()
{
  return icon( QSize( 100, 15 ) );
}

QIcon QgsCptCityColorRampItem::icon( QSize size )
{
  const auto constMIcons = mIcons;
  for ( const QIcon &icon : constMIcons )
  {
    if ( icon.availableSizes().contains( size ) )
      return icon;
  }

  QIcon icon;

  init();

  if ( mValid && mRamp.count() > 0 )
  {
    icon = QgsSymbolLayerUtils::colorRampPreviewIcon( &mRamp, size );
  }
  else
  {
    QPixmap blankPixmap( size );
    blankPixmap.fill( Qt::white );
    icon = QIcon( blankPixmap );
    mInfo.clear();
  }

  mIcons.append( icon );
  return icon;
}

// ---------------------------------------------------------------------
QgsCptCityCollectionItem::QgsCptCityCollectionItem( QgsCptCityDataItem *parent,
    const QString &name, const QString &path )
  : QgsCptCityDataItem( Collection, parent, name, path )
  , mPopulatedRamps( false )
{
}

QgsCptCityCollectionItem::~QgsCptCityCollectionItem()
{
  qDeleteAll( mChildren );
}

QVector< QgsCptCityDataItem * > QgsCptCityCollectionItem::childrenRamps( bool recursive )
{
  QVector< QgsCptCityDataItem * > rampItems;
  QVector< QgsCptCityDataItem * > deleteItems;

  populate();

  // recursively add children
  const auto constChildren = children();
  for ( QgsCptCityDataItem *childItem : constChildren )
  {
    QgsCptCityCollectionItem *collectionItem = qobject_cast<QgsCptCityCollectionItem *>( childItem );
    QgsCptCityColorRampItem *rampItem = qobject_cast<QgsCptCityColorRampItem *>( childItem );
    QgsDebugMsgLevel( QStringLiteral( "child path= %1 coll= %2 ramp = %3" ).arg( childItem->path() ).arg( nullptr != collectionItem ).arg( nullptr != rampItem ), 2 );
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
      else
        deleteItems << rampItem;
    }
    else
    {
      QgsDebugMsg( "invalid item " + childItem->path() );
    }
  }

  // delete invalid items - this is not efficient, but should only happens once
  const auto constDeleteItems = deleteItems;
  for ( QgsCptCityDataItem *deleteItem : constDeleteItems )
  {
    QgsDebugMsg( QStringLiteral( "item %1 is invalid, will be deleted" ).arg( deleteItem->path() ) );
    const int i = mChildren.indexOf( deleteItem );
    if ( i != -1 )
      mChildren.remove( i );
    delete deleteItem;
  }

  return rampItems;
}

//-----------------------------------------------------------------------
QgsCptCityDirectoryItem::QgsCptCityDirectoryItem( QgsCptCityDataItem *parent,
    const QString &name, const QString &path )
  : QgsCptCityCollectionItem( parent, name, path )
{
  mType = Directory;
  mValid = QDir( QgsCptCityArchive::defaultBaseDir() + '/' + mPath ).exists();
  if ( ! mValid )
  {
    QgsDebugMsg( "created invalid dir item, path = " + QgsCptCityArchive::defaultBaseDir()
                 + '/' + mPath );
  }

  // parse DESC.xml to get mInfo
  mInfo.clear();
  const QString fileName = QgsCptCityArchive::defaultBaseDir() + '/' +
                           mPath + '/' + "DESC.xml";
  const QgsStringMap descMap = QgsCptCityArchive::description( fileName );
  if ( descMap.contains( QStringLiteral( "name" ) ) )
    mInfo = descMap.value( QStringLiteral( "name" ) );

  // populate();
}

QVector<QgsCptCityDataItem *> QgsCptCityDirectoryItem::createChildren()
{
  if ( ! mValid )
    return QVector<QgsCptCityDataItem *>();

  QVector<QgsCptCityDataItem *> children;

  // add children schemes
  QMapIterator< QString, QStringList> it( rampsMap() );
  while ( it.hasNext() )
  {
    it.next();
    // QgsDebugMsgLevel( "schemeName = " + it.key(), 2 );
    QgsCptCityDataItem *item =
      new QgsCptCityColorRampItem( this, it.key(), it.key(), it.value() );
    if ( item->isValid() )
      children << item;
    else
      delete item;
  }

  // add children dirs
  const auto constDirEntries = dirEntries();
  for ( const QString &childPath : constDirEntries )
  {
    QgsCptCityDataItem *childItem =
      QgsCptCityDirectoryItem::dataItem( this, childPath, mPath + '/' + childPath );
    if ( childItem )
      children << childItem;
  }

  QgsDebugMsgLevel( QStringLiteral( "name= %1 path= %2 found %3 children" ).arg( mName, mPath ).arg( children.count() ), 2 );

  return children;
}

QMap< QString, QStringList > QgsCptCityDirectoryItem::rampsMap()
{
  if ( ! mRampsMap.isEmpty() )
    return mRampsMap;

  QString curName, prevName, curVariant, curSep, schemeName;
  QStringList listVariant;
  QStringList schemeNamesAll, schemeNames;
  bool prevAdd, curAdd;

  const QDir dir( QgsCptCityArchive::defaultBaseDir() + '/' + mPath );
  schemeNamesAll = dir.entryList( QStringList( QStringLiteral( "*.svg" ) ), QDir::Files, QDir::Name );

  // TODO detect if there are duplicate names with different variant counts, combine in 1
  for ( int i = 0; i < schemeNamesAll.count(); i++ )
  {
    // schemeName = QFileInfo( schemeNamesAll[i] ).baseName();
    schemeName = schemeNamesAll[i];
    schemeName.chop( 4 );
    // QgsDebugMsgLevel("scheme = "+schemeName, 2);
    curName = schemeName;
    curVariant.clear();

    // find if name ends with 1-3 digit number
    // TODO need to detect if ends with b/c also
    if ( schemeName.length() > 1 && schemeName.endsWith( 'a' ) && ! listVariant.isEmpty() &&
         ( ( prevName + listVariant.last()  + 'a' ) == curName ) )
    {
      curName = prevName;
      curVariant = listVariant.last() + 'a';
    }
    else
    {
      const thread_local QRegularExpression rxVariant( "^(.*[^\\d])(\\d{1,3})$" );
      const QRegularExpressionMatch match = rxVariant.match( schemeName );
      if ( match.hasMatch() )
      {
        curName = match.captured( 1 );
        curVariant = match.captured( 2 );
      }
    }

    curSep = curName.right( 1 );
    if ( curSep == QLatin1String( "-" ) || curSep == QLatin1String( "_" ) )
    {
      curName.chop( 1 );
      curVariant = curSep + curVariant;
    }

    if ( prevName.isEmpty() )
      prevName = curName;

    // add element, unless it is empty, or a variant of last element
    prevAdd = false;
    curAdd = false;
    if ( curName.isEmpty() )
      curName = QStringLiteral( "__empty__" );
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
      if ( !prevName.isEmpty() )
      {
        prevAdd = true;
      }
      // add current element if it is the last one in the archive
      if ( i == schemeNamesAll.count() - 1 )
        curAdd = true;
    }

    // QgsDebugMsgLevel(QString("prevAdd=%1 curAdd=%2 prevName=%3 curName=%4 count=%5").arg(prevAdd).arg(curAdd).arg(prevName).arg(curName).arg(listVariant.count()), 2);

    if ( prevAdd )
    {
      // depending on number of variants, make one or more items
      if ( listVariant.isEmpty() )
      {
        // set num colors=-1 to parse file on request only
        // mSchemeNumColors[ prevName ] = -1;
        schemeNames << prevName;
        mRampsMap[ mPath + '/' + prevName ] = QStringList();
      }
      else if ( listVariant.count() <= 3 )
      {
        // for 1-2 items, create independent items
        for ( int j = 0; j < listVariant.count(); j++ )
        {
          // mSchemeNumColors[ prevName + listVariant[j] ] = -1;
          schemeNames << prevName + listVariant[j];
          mRampsMap[ mPath + '/' + prevName + listVariant[j] ] = QStringList();
        }
      }
      else
      {
        // mSchemeVariants[ path + '/' + prevName ] = listVariant;
        mRampsMap[ mPath + '/' + prevName ] = listVariant;
        schemeNames << prevName;
      }
      listVariant.clear();
    }
    if ( curAdd )
    {
      if ( !curVariant.isEmpty() )
        curName += curVariant;
      schemeNames << curName;
      mRampsMap[ mPath + '/' + curName ] = QStringList();
    }
    // save current to compare next
    if ( prevAdd || curAdd )
    {
      prevName = curName;
      if ( !curVariant.isEmpty() )
        listVariant << curVariant;
    }

  }
#if 0
  //TODO what to do with other vars? e.g. schemeNames
  // add schemes to archive
  mSchemeMap[ path ] = schemeNames;
  schemeCount += schemeName.count();
  schemeNames.clear();
  listVariant.clear();
  prevName = "";
#endif
  return mRampsMap;
}

QStringList QgsCptCityDirectoryItem::dirEntries() const
{
  return QDir( QgsCptCityArchive::defaultBaseDir() +
               '/' + mPath ).entryList( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );
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

QgsCptCityDataItem *QgsCptCityDirectoryItem::dataItem( QgsCptCityDataItem *parent,
    const QString &name, const QString &path )
{
  QgsDebugMsgLevel( "name= " + name + " path= " + path, 2 );

  // first create item with constructor
  QgsCptCityDirectoryItem *dirItem = new QgsCptCityDirectoryItem( parent, name, path );
  if ( dirItem && ! dirItem->isValid() )
  {
    delete dirItem;
    return nullptr;
  }
  if ( ! dirItem )
    return nullptr;

  // fetch sub-dirs and ramps to know what to do with this item
  const QStringList dirEntries = dirItem->dirEntries();
  QMap< QString, QStringList > rampsMap = dirItem->rampsMap();

  QgsDebugMsgLevel( QStringLiteral( "item has %1 dirs and %2 ramps" ).arg( dirEntries.count() ).arg( rampsMap.count() ), 2 );

  // return item if has at least one subdir
  if ( !dirEntries.isEmpty() )
    return dirItem;

  // if 0 ramps, delete item
  if ( rampsMap.isEmpty() )
  {
    delete dirItem;
    return nullptr;
  }
  // if 1 ramp, return this child's item
  // so we don't have a directory with just 1 item (with many variants possibly)
  else if ( rampsMap.count() == 1 )
  {
    delete dirItem;
    QgsCptCityColorRampItem *rampItem =
      new QgsCptCityColorRampItem( parent, rampsMap.begin().key(),
                                   rampsMap.begin().key(), rampsMap.begin().value() );
    if ( ! rampItem->isValid() )
    {
      delete rampItem;
      return nullptr;
    }
    return rampItem;
  }
  return dirItem;
}


//-----------------------------------------------------------------------
QgsCptCitySelectionItem::QgsCptCitySelectionItem( QgsCptCityDataItem *parent,
    const QString &name, const QString &path )
  : QgsCptCityCollectionItem( parent, name, path )
{
  mType = Selection;
  mValid = ! path.isNull();
  if ( mValid )
    parseXml();
}

QVector<QgsCptCityDataItem *> QgsCptCitySelectionItem::createChildren()
{
  if ( ! mValid )
    return QVector<QgsCptCityDataItem *>();

  QgsCptCityDataItem *item = nullptr;
  QVector<QgsCptCityDataItem *> children;

  QgsDebugMsgLevel( "name= " + mName + " path= " + mPath, 2 );

  // add children archives
  for ( QString childPath : std::as_const( mSelectionsList ) )
  {
    QgsDebugMsgLevel( "childPath = " + childPath + " name= " + QFileInfo( childPath ).baseName(), 2 );
    if ( childPath.endsWith( '/' ) )
    {
      childPath.chop( 1 );
      QgsCptCityDataItem *childItem =
        QgsCptCityDirectoryItem::dataItem( this, childPath, childPath );
      if ( childItem )
      {
        if ( childItem->isValid() )
          children << childItem;
        else
          delete childItem;
      }
    }
    else
    {
      const QString fileName = QgsCptCityColorRamp::fileNameForVariant( childPath, QString() );
      if ( !QFile::exists( fileName ) )
      {
        continue;
      }

      item = new QgsCptCityColorRampItem( this, childPath, childPath, QString(), true );
      if ( item->isValid() )
        children << item;
      else
        delete item;
    }
  }

  QgsDebugMsgLevel( QStringLiteral( "path= %1 inserted %2 children" ).arg( mPath ).arg( children.count() ), 2 );

  return children;
}

void QgsCptCitySelectionItem::parseXml()
{
  const QString filename = QgsCptCityArchive::defaultBaseDir() + '/' + mPath;

  QgsDebugMsgLevel( "reading file " + filename, 2 );

  QFile f( filename );
  if ( ! f.open( QFile::ReadOnly ) )
  {
    QgsDebugMsg( filename + " does not exist" );
    return;
  }

  // parse the document
  QString errMsg;
  QDomDocument doc( QStringLiteral( "selection" ) );
  if ( !doc.setContent( &f, &errMsg ) )
  {
    f.close();
    QgsDebugMsg( "Couldn't parse file " + filename + " : " + errMsg );
    return;
  }
  f.close();

  // read description
  const QDomElement docElem = doc.documentElement();
  if ( docElem.tagName() != QLatin1String( "selection" ) )
  {
    QgsDebugMsg( "Incorrect root tag: " + docElem.tagName() );
    return;
  }
  QDomElement e = docElem.firstChildElement( QStringLiteral( "name" ) );
  if ( ! e.isNull() && ! e.text().isNull() )
    mName = e.text();
  mInfo = docElem.firstChildElement( QStringLiteral( "synopsis" ) ).text().simplified();

  // get archives
  const QDomElement collectsElem = docElem.firstChildElement( QStringLiteral( "seealsocollects" ) );
  e = collectsElem.firstChildElement( QStringLiteral( "collect" ) );
  while ( ! e.isNull() )
  {
    if ( ! e.attribute( QStringLiteral( "dir" ) ).isNull() )
    {
      // TODO parse description and use that, instead of default archive name
      const QString dir = e.attribute( QStringLiteral( "dir" ) ) + '/';
      if ( QFile::exists( QgsCptCityArchive::defaultBaseDir() + '/' + dir ) )
      {
        mSelectionsList << dir;
      }
    }
    e = e.nextSiblingElement();
  }
  // get individual gradients
  const QDomElement gradientsElem = docElem.firstChildElement( QStringLiteral( "gradients" ) );
  e = gradientsElem.firstChildElement( QStringLiteral( "gradient" ) );
  while ( ! e.isNull() )
  {
    if ( ! e.attribute( QStringLiteral( "dir" ) ).isNull() )
    {
      // QgsDebugMsgLevel( "add " + e.attribute( "dir" ) + '/' + e.attribute( "file" ) + " to " + selname, 2 );
      // TODO parse description and save elsewhere
      const QString dir = e.attribute( QStringLiteral( "dir" ) );
      if ( QFile::exists( QgsCptCityArchive::defaultBaseDir() + '/' + dir ) )
      {
        mSelectionsList << dir + '/' + e.attribute( QStringLiteral( "file" ) );
      }
    }
    e = e.nextSiblingElement();
  }
}

bool QgsCptCitySelectionItem::equal( const QgsCptCityDataItem *other )
{
  //QgsDebugMsgLevel( mPath + " x " + other->mPath, 2 );
  if ( type() != other->type() )
  {
    return false;
  }
  return ( path() == other->path() );
}

//-----------------------------------------------------------------------
QgsCptCityAllRampsItem::QgsCptCityAllRampsItem( QgsCptCityDataItem *parent,
    const QString &name, const QVector<QgsCptCityDataItem *> &items )
  : QgsCptCityCollectionItem( parent, name, QString() )
  , mItems( items )
{
  mType = AllRamps;
  mValid = true;
  // populate();
}

QVector<QgsCptCityDataItem *> QgsCptCityAllRampsItem::createChildren()
{
  if ( ! mValid )
    return QVector<QgsCptCityDataItem *>();

  QVector<QgsCptCityDataItem *> children;

  // add children ramps of each item
  const auto constMItems = mItems;
  for ( QgsCptCityDataItem *item : constMItems )
  {
    QgsCptCityCollectionItem *colItem = qobject_cast< QgsCptCityCollectionItem * >( item );
    if ( colItem )
      children += colItem->childrenRamps( true );
  }

  return children;
}

//-----------------------------------------------------------------------

QgsCptCityBrowserModel::QgsCptCityBrowserModel( QObject *parent,
    QgsCptCityArchive *archive, ViewType viewType )
  : QAbstractItemModel( parent )
  , mArchive( archive )
  , mViewType( viewType )
{
  Q_ASSERT( mArchive );
  QgsDebugMsgLevel( QLatin1String( "archiveName = " ) + archive->archiveName() + " viewType=" + QString::number( static_cast< int >( viewType ) ), 2 );
  // keep iconsize for now, but not effectively used
  mIconSize = QSize( 100, 15 );
  addRootItems();
}

QgsCptCityBrowserModel::~QgsCptCityBrowserModel()
{
  removeRootItems();
}

void QgsCptCityBrowserModel::addRootItems()
{
  if ( mViewType == Authors )
  {
    mRootItems = mArchive->rootItems();
  }
  else if ( mViewType == Selections )
  {
    mRootItems = mArchive->selectionItems();
  }
  QgsDebugMsgLevel( QStringLiteral( "added %1 root items" ).arg( mRootItems.size() ), 2 );
}

void QgsCptCityBrowserModel::removeRootItems()
{
  mRootItems.clear();
}

Qt::ItemFlags QgsCptCityBrowserModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::ItemFlags();

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
      return QString( item->path() + '\n' + item->info() );
    return item->toolTip();
  }
  else if ( role == Qt::DecorationRole && index.column() == 1 &&
            item->type() == QgsCptCityDataItem::ColorRamp )
  {
    // keep iconsize for now, but not effectively used
    return item->icon( mIconSize );
  }
  else if ( role == Qt::FontRole &&
            qobject_cast< QgsCptCityCollectionItem * >( item ) )
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
  Q_UNUSED( section )
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
  Q_UNUSED( parent )
  return 2;
}

QModelIndex QgsCptCityBrowserModel::findPath( const QString &path )
{
  QModelIndex rootIndex; // starting from root
  bool foundParent = false, foundChild = true;
  QString itemPath;

  QgsDebugMsgLevel( "path = " + path, 2 );

  // special case if searching for first item "All Ramps", do not search into tree
  if ( path.isEmpty() )
  {
    for ( int i = 0; i < rowCount( rootIndex ); i++ )
    {
      QModelIndex idx = index( i, 0, rootIndex );
      QgsCptCityDataItem *item = dataItem( idx );
      if ( !item )
        return QModelIndex(); // an error occurred

      itemPath = item->path();

      if ( itemPath == path )
      {
        QgsDebugMsgLevel( "Arrived " + itemPath, 2 );
        return idx; // we have found the item we have been looking for
      }
    }
  }

  while ( foundChild )
  {
    foundChild = false; // assume that the next child item will not be found

    int i = 0;
    // if root skip first item "All Ramps"
    if ( itemPath.isEmpty() )
      i = 1;
    for ( ; i < rowCount( rootIndex ); i++ )
    {
      QModelIndex idx = index( i, 0, rootIndex );
      QgsCptCityDataItem *item = dataItem( idx );
      if ( !item )
        return QModelIndex(); // an error occurred

      itemPath = item->path();

      if ( itemPath == path )
      {
        QgsDebugMsgLevel( "Arrived " + itemPath, 2 );
        return idx; // we have found the item we have been looking for
      }

      if ( ! itemPath.endsWith( '/' ) )
        itemPath += '/';

      foundParent = false;

      // QgsDebugMsgLevel( "path= " + path + " itemPath= " + itemPath, 2 );

      // if we are using a selection collection, search for target in the mapping in this group
      if ( item->type() == QgsCptCityDataItem::Selection )
      {
        const QgsCptCitySelectionItem *selItem = qobject_cast<const QgsCptCitySelectionItem *>( item );
        if ( selItem )
        {
          const auto constSelectionsList = selItem->selectionsList();
          for ( QString childPath : constSelectionsList )
          {
            if ( childPath.endsWith( '/' ) )
              childPath.chop( 1 );
            // QgsDebugMsgLevel( "childPath= " + childPath, 2 );
            if ( path.startsWith( childPath ) )
            {
              foundParent = true;
              break;
            }
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
        QgsDebugMsgLevel( "found parent " + path, 2 );
        // we have found a preceding item: stop searching on this level and go deeper
        foundChild = true;
        rootIndex = idx;
        if ( canFetchMore( rootIndex ) )
          fetchMore( rootIndex );
        break;
      }
    }
  }

  return QModelIndex(); // not found
}

void QgsCptCityBrowserModel::reload()
{
  beginResetModel();
  removeRootItems();
  addRootItems();
  endResetModel();
}

/* Refresh dir path */
void QgsCptCityBrowserModel::refresh( const QString &path )
{
  const QModelIndex idx = findPath( path );
  if ( idx.isValid() )
  {
    QgsCptCityDataItem *item = dataItem( idx );
    if ( item )
      item->refresh();
  }
}

QModelIndex QgsCptCityBrowserModel::index( int row, int column, const QModelIndex &parent ) const
{
  QgsCptCityDataItem *p = dataItem( parent );
  const QVector<QgsCptCityDataItem *> &items = p ? p->children() : mRootItems;
  QgsCptCityDataItem *item = items.value( row, nullptr );
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
  const QVector<QgsCptCityDataItem *> &items = parent ? parent->children() : mRootItems;

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
void QgsCptCityBrowserModel::refresh( const QModelIndex &index )
{
  QgsCptCityDataItem *item = dataItem( index );
  if ( !item )
    return;

  QgsDebugMsgLevel( "Refresh " + item->path(), 2 );
  item->refresh();
}

void QgsCptCityBrowserModel::beginInsertItems( QgsCptCityDataItem *parent, int first, int last )
{
  QgsDebugMsgLevel( "parent mPath = " + parent->path(), 2 );
  const QModelIndex idx = findItem( parent );
  if ( !idx.isValid() )
    return;
  QgsDebugMsgLevel( QStringLiteral( "valid" ), 2 );
  beginInsertRows( idx, first, last );
  QgsDebugMsgLevel( QStringLiteral( "end" ), 2 );
}
void QgsCptCityBrowserModel::endInsertItems()
{
  endInsertRows();
}
void QgsCptCityBrowserModel::beginRemoveItems( QgsCptCityDataItem *parent, int first, int last )
{
  QgsDebugMsgLevel( "parent mPath = " + parent->path(), 2 );
  const QModelIndex idx = findItem( parent );
  if ( !idx.isValid() )
    return;
  beginRemoveRows( idx, first, last );
}
void QgsCptCityBrowserModel::endRemoveItems()
{
  endRemoveRows();
}
void QgsCptCityBrowserModel::connectItem( QgsCptCityDataItem *item )
{
  connect( item, &QgsCptCityDataItem::beginInsertItems, this, &QgsCptCityBrowserModel::beginInsertItems );
  connect( item, &QgsCptCityDataItem::endInsertItems, this, &QgsCptCityBrowserModel::endInsertItems );
  connect( item, &QgsCptCityDataItem::beginRemoveItems, this, &QgsCptCityBrowserModel::beginRemoveItems );
  connect( item, &QgsCptCityDataItem::endRemoveItems, this, &QgsCptCityBrowserModel::endRemoveItems );
}

bool QgsCptCityBrowserModel::canFetchMore( const QModelIndex &parent ) const
{
  QgsCptCityDataItem *item = dataItem( parent );
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

void QgsCptCityBrowserModel::fetchMore( const QModelIndex &parent )
{
  QgsCptCityDataItem *item = dataItem( parent );
  if ( item )
  {
    item->populate();
    QgsDebugMsgLevel( "path = " + item->path(), 2 );
  }
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

QMimeData *QgsCptCityBrowserModel::mimeData( const QModelIndexList &indexes ) const
{
  QgsMimeDataUtils::UriList lst;
  const auto constIndexes = indexes;
  for ( const QModelIndex &index : constIndexes )
  {
    if ( index.isValid() )
    {
      QgsCptCityDataItem *ptr = ( QgsCptCityDataItem * ) index.internalPointer();
      if ( ptr->type() != QgsCptCityDataItem::Layer ) continue;
      QgsLayerItem *layer = ( QgsLayerItem * ) ptr;
      lst.append( QgsMimeDataUtils::Uri( ayer ) );
    }
  }
  return QgsMimeDataUtils::encodeUriList( lst );
}

bool QgsCptCityBrowserModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent )
{
  Q_UNUSED( row )
  Q_UNUSED( column )

  QgsCptCityDataItem *destItem = dataItem( parent );
  if ( !destItem )
  {
    QgsDebugMsg( QStringLiteral( "DROP PROBLEM!" ) );
    return false;
  }

  return destItem->handleDrop( data, action );
}
#endif

QgsCptCityDataItem *QgsCptCityBrowserModel::dataItem( const QModelIndex &idx ) const
{
  void *v = idx.internalPointer();
  QgsCptCityDataItem *d = reinterpret_cast<QgsCptCityDataItem *>( v );
  Q_ASSERT( !v || d );
  return d;
}
