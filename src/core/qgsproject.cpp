/***************************************************************************
                          qgsproject.cpp -  description
                             -------------------
  begin                : July 23, 2004
  copyright            : (C) 2004 by Mark Coletti
  email                : mcoletti at gmail.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsproject.h"

#include "qgsdatasourceuri.h"
#include "qgsexception.h"
#include "qgslayertree.h"
#include "qgslayertreeutils.h"
#include "qgslayertreeregistrybridge.h"
#include "qgslogger.h"
#include "qgsmaplayerregistry.h"
#include "qgsmessagelog.h"
#include "qgspluginlayer.h"
#include "qgspluginlayerregistry.h"
#include "qgsprojectfiletransform.h"
#include "qgsprojectproperty.h"
#include "qgsprojectversion.h"
#include "qgsrasterlayer.h"
#include "qgsrectangle.h"
#include "qgsrelationmanager.h"
#include "qgsvectorlayer.h"
#include "qgsvisibilitypresetcollection.h"
#include "qgslayerdefinition.h"
#include "qgsunittypes.h"
#include "qgstransaction.h"
#include "qgstransactiongroup.h"
#include "qgsvectordataprovider.h"

#include <QApplication>
#include <QFileInfo>
#include <QDomNode>
#include <QObject>
#include <QTextStream>
#include <QTemporaryFile>
#include <QDir>
#include <QUrl>
#include <QSettings>

#ifdef Q_OS_UNIX
#include <utime.h>
#elif _MSC_VER
#include <sys/utime.h>
#endif

// canonical project instance
QgsProject *QgsProject::theProject_ = nullptr;

/**
    Take the given scope and key and convert them to a string list of key
    tokens that will be used to navigate through a Property hierarchy

    E.g., scope "someplugin" and key "/foo/bar/baz" will become a string list
    of { "properties", "someplugin", "foo", "bar", "baz" }.  "properties" is
    always first because that's the permanent ``root'' Property node.
 */
QStringList makeKeyTokens_( const QString& scope, const QString& key )
{
  QStringList keyTokens = QStringList( scope );
  keyTokens += key.split( '/', QString::SkipEmptyParts );

  // be sure to include the canonical root node
  keyTokens.push_front( "properties" );

  //check validy of keys since an unvalid xml name will will be dropped upon saving the xml file. If not valid, we print a message to the console.
  for ( int i = 0; i < keyTokens.size(); ++i )
  {
    QString keyToken = keyTokens.at( i );

    //invalid chars in XML are found at http://www.w3.org/TR/REC-xml/#NT-NameChar
    //note : it seems \x10000-\xEFFFF is valid, but it when added to the regexp, a lot of unwanted characters remain
    QString nameCharRegexp = QString( "[^:A-Z_a-z\\xC0-\\xD6\\xD8-\\xF6\\xF8-\\x2FF\\x370-\\x37D\\x37F-\\x1FFF\\x200C-\\x200D\\x2070-\\x218F\\x2C00-\\x2FEF\\x3001-\\xD7FF\\xF900-\\xFDCF\\xFDF0-\\xFFFD\\-\\.0-9\\xB7\\x0300-\\x036F\\x203F-\\x2040]" );
    QString nameStartCharRegexp = QString( "^[^:A-Z_a-z\\xC0-\\xD6\\xD8-\\xF6\\xF8-\\x2FF\\x370-\\x37D\\x37F-\\x1FFF\\x200C-\\x200D\\x2070-\\x218F\\x2C00-\\x2FEF\\x3001-\\xD7FF\\xF900-\\xFDCF\\xFDF0-\\xFFFD]" );

    if ( keyToken.contains( QRegExp( nameCharRegexp ) ) || keyToken.contains( QRegExp( nameStartCharRegexp ) ) )
    {

      QString errorString = QObject::tr( "Entry token invalid : '%1'. The token will not be saved to file." ).arg( keyToken );
      QgsMessageLog::logMessage( errorString, QString::null, QgsMessageLog::CRITICAL );

    }

  }

  return keyTokens;
}



/**
   return the property that matches the given key sequence, if any

   @param scope scope of key
   @param key keyname
   @param rootProperty is likely to be the top level QgsPropertyKey in QgsProject:e:Imp.

   @return null if not found, otherwise located Property
*/
QgsProperty *findKey_( const QString& scope,
                       const QString& key,
                       QgsPropertyKey& rootProperty )
{
  QgsPropertyKey *currentProperty = &rootProperty;
  QgsProperty *nextProperty;           // link to next property down hiearchy

  QStringList keySequence = makeKeyTokens_( scope, key );

  while ( !keySequence.isEmpty() )
  {
    // if the current head of the sequence list matches the property name,
    // then traverse down the property hierarchy
    if ( keySequence.first() == currentProperty->name() )
    {
      // remove front key since we're traversing down a level
      keySequence.pop_front();

      if ( 1 == keySequence.count() )
      {
        // if we have only one key name left, then return the key found
        return currentProperty->find( keySequence.front() );
      }
      else if ( keySequence.isEmpty() )
      {
        // if we're out of keys then the current property is the one we
        // want; i.e., we're in the rate case of being at the top-most
        // property node
        return currentProperty;
      }
      else if (( nextProperty = currentProperty->find( keySequence.first() ) ) )
      {
        if ( nextProperty->isKey() )
        {
          currentProperty = static_cast<QgsPropertyKey*>( nextProperty );
        }
        else if ( nextProperty->isValue() && 1 == keySequence.count() )
        {
          // it may be that this may be one of several property value
          // nodes keyed by QDict string; if this is the last remaining
          // key token and the next property is a value node, then
          // that's the situation, so return the currentProperty
          return currentProperty;
        }
        else
        {
          // QgsPropertyValue not Key, so return null
          return nullptr;
        }
      }
      else
      {
        // if the next key down isn't found
        // then the overall key sequence doesn't exist
        return nullptr;
      }
    }
    else
    {
      return nullptr;
    }
  }

  return nullptr;
}



/** Add the given key and value

@param scope scope of key
@param key key name
@param rootProperty is the property from which to start adding
@param value the value associated with the key
*/
QgsProperty *addKey_( const QString& scope,
                      const QString& key,
                      QgsPropertyKey* rootProperty,
                      const QVariant& value )
{
  QStringList keySequence = makeKeyTokens_( scope, key );

  // cursor through property key/value hierarchy
  QgsPropertyKey *currentProperty = rootProperty;
  QgsProperty *nextProperty; // link to next property down hiearchy
  QgsPropertyKey* newPropertyKey;

  while ( ! keySequence.isEmpty() )
  {
    // if the current head of the sequence list matches the property name,
    // then traverse down the property hierarchy
    if ( keySequence.first() == currentProperty->name() )
    {
      // remove front key since we're traversing down a level
      keySequence.pop_front();

      // if key sequence has one last element, then we use that as the
      // name to store the value
      if ( 1 == keySequence.count() )
      {
        currentProperty->setValue( keySequence.front(), value );
        return currentProperty;
      }
      // we're at the top element if popping the keySequence element
      // will leave it empty; in that case, just add the key
      else if ( keySequence.isEmpty() )
      {
        currentProperty->setValue( value );

        return currentProperty;
      }
      else if (( nextProperty = currentProperty->find( keySequence.first() ) ) )
      {
        currentProperty = dynamic_cast<QgsPropertyKey*>( nextProperty );

        if ( currentProperty )
        {
          continue;
        }
        else            // QgsPropertyValue not Key, so return null
        {
          return nullptr;
        }
      }
      else                // the next subkey doesn't exist, so add it
      {
        if (( newPropertyKey = currentProperty->addKey( keySequence.first() ) ) )
        {
          currentProperty = newPropertyKey;
        }
        continue;
      }
    }
    else
    {
      return nullptr;
    }
  }

  return nullptr;

}


void removeKey_( const QString& scope,
                 const QString& key,
                 QgsPropertyKey &rootProperty )
{
  QgsPropertyKey *currentProperty = &rootProperty;

  QgsProperty *nextProperty = nullptr;   // link to next property down hiearchy
  QgsPropertyKey *previousQgsPropertyKey = nullptr; // link to previous property up hiearchy

  QStringList keySequence = makeKeyTokens_( scope, key );

  while ( ! keySequence.isEmpty() )
  {
    // if the current head of the sequence list matches the property name,
    // then traverse down the property hierarchy
    if ( keySequence.first() == currentProperty->name() )
    {
      // remove front key since we're traversing down a level
      keySequence.pop_front();

      // if we have only one key name left, then try to remove the key
      // with that name
      if ( 1 == keySequence.count() )
      {
        currentProperty->removeKey( keySequence.front() );
      }
      // if we're out of keys then the current property is the one we
      // want to remove, but we can't delete it directly; we need to
      // delete it from the parent property key container
      else if ( keySequence.isEmpty() )
      {
        previousQgsPropertyKey->removeKey( currentProperty->name() );
      }
      else if (( nextProperty = currentProperty->find( keySequence.first() ) ) )
      {
        previousQgsPropertyKey = currentProperty;
        currentProperty = dynamic_cast<QgsPropertyKey*>( nextProperty );

        if ( currentProperty )
        {
          continue;
        }
        else            // QgsPropertyValue not Key, so return null
        {
          return;
        }
      }
      else                // if the next key down isn't found
      {                   // then the overall key sequence doesn't exist
        return;
      }
    }
    else
    {
      return;
    }
  }

}


// TODO QGIS 3.0 - move to private members. This pattern is adding complexity without any current benefits.

struct QgsProject::Imp
{
  QFile file;                 // current physical project file
  QgsPropertyKey properties_; // property hierarchy
  QString title;              // project title
  bool autoTransaction;       // transaction grouped editing
  bool evaluateDefaultValues; // evaluate default values immediately
  bool dirty;                 // project has been modified since it has been read or saved

  Imp()
      : title()
      , autoTransaction( false )
      , evaluateDefaultValues( false )
      , dirty( false )
  {                             // top property node is the root
    // "properties" that contains all plug-in
    // and extra property keys and values
    properties_.name() = "properties"; // root property node always this value
  }

  /** Clear project properties when a new project is started
   */
  void clear()
  {
    // QgsDebugMsg( "Clearing project properties Impl->clear();" );

    file.setFileName( QString() );
    properties_.clearKeys();
    title.clear();
    autoTransaction = false;
    evaluateDefaultValues = false;
    dirty = false;
  }

};


QgsProject::QgsProject()
    : imp_( new QgsProject::Imp )
    , mBadLayerHandler( new QgsProjectBadLayerDefaultHandler() )
    , mRelationManager( new QgsRelationManager( this ) )
    , mRootGroup( new QgsLayerTreeGroup )
{
  clear();

  // bind the layer tree to the map layer registry.
  // whenever layers are added to or removed from the registry,
  // layer tree will be updated
  mLayerTreeRegistryBridge = new QgsLayerTreeRegistryBridge( mRootGroup, this );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersAdded( QList<QgsMapLayer*> ) ), this, SLOT( onMapLayersAdded( QList<QgsMapLayer*> ) ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersRemoved( QStringList ) ), this, SLOT( cleanTransactionGroups() ) );
}


QgsProject::~QgsProject()
{
  delete mBadLayerHandler;
  delete mRelationManager;
  delete mRootGroup;
}


QgsProject *QgsProject::instance()
{
  if ( !theProject_ )
  {
    theProject_ = new QgsProject;
  }
  return theProject_;
}

void QgsProject::setTitle( const QString &title )
{
  imp_->title = title;

  setDirty( true );
}


QString QgsProject::title() const
{
  return imp_->title;
}


bool QgsProject::isDirty() const
{
  return imp_->dirty;
}

void QgsProject::setDirty( bool b )
{
  imp_->dirty = b;
}

void QgsProject::setFileName( const QString& name )
{
  imp_->file.setFileName( name );

  setDirty( true );
}

QString QgsProject::fileName() const
{
  return imp_->file.fileName();
}

QFileInfo QgsProject::fileInfo() const
{
  return QFileInfo( imp_->file );
}

void QgsProject::clear()
{
  imp_->clear();
  mEmbeddedLayers.clear();
  mRelationManager->clear();

  mVisibilityPresetCollection.reset( new QgsVisibilityPresetCollection() );

  mRootGroup->removeAllChildren();

  // reset some default project properties
  // XXX THESE SHOULD BE MOVED TO STATUSBAR RELATED SOURCE
  writeEntry( "PositionPrecision", "/Automatic", true );
  writeEntry( "PositionPrecision", "/DecimalPlaces", 2 );
  writeEntry( "Paths", "/Absolute", false );

  //copy default units to project
  QSettings s;
  writeEntry( "Measurement", "/DistanceUnits", s.value( "/qgis/measure/displayunits" ).toString() );
  writeEntry( "Measurement", "/AreaUnits", s.value( "/qgis/measure/areaunits" ).toString() );

  setDirty( false );
}

// basically a debugging tool to dump property list values
void dump_( const QgsPropertyKey& topQgsPropertyKey )
{
  QgsDebugMsg( "current properties:" );
  topQgsPropertyKey.dump();
}


/**

Restore any optional properties found in "doc" to "properties".

properties tags for all optional properties.  Within that there will be scope
tags.  In the following example there exist one property in the "fsplugin"
scope.  "layers" is a list containing three string values.

\code{.xml}
<properties>
  <fsplugin>
    <foo type="int" >42</foo>
    <baz type="int" >1</baz>
    <layers type="QStringList" >
      <value>railroad</value>
      <value>airport</value>
    </layers>
    <xyqzzy type="int" >1</xyqzzy>
    <bar type="double" >123.456</bar>
    <feature_types type="QStringList" >
       <value>type</value>
    </feature_types>
  </fsplugin>
</properties>
\endcode

@param doc xml document
@param project_properties should be the top QgsPropertyKey node.

*/
void _getProperties( const QDomDocument& doc, QgsPropertyKey& project_properties )
{
  QDomNodeList properties = doc.elementsByTagName( "properties" );

  if ( properties.count() > 1 )
  {
    QgsDebugMsg( "there appears to be more than one ``properties'' XML tag ... bailing" );
    return;
  }
  else if ( properties.count() < 1 )  // no properties found, so we're done
  {
    return;
  }

  // item(0) because there should only be ONE "properties" node
  QDomNodeList scopes = properties.item( 0 ).childNodes();

  if ( scopes.count() < 1 )
  {
    QgsDebugMsg( "empty ``properties'' XML tag ... bailing" );
    return;
  }

  QDomNode propertyNode = properties.item( 0 );

  if ( ! project_properties.readXML( propertyNode ) )
  {
    QgsDebugMsg( "Project_properties.readXML() failed" );
  }
}


/**
   Get the project title
   @todo XXX we should go with the attribute xor title, not both.
*/
static void _getTitle( const QDomDocument& doc, QString& title )
{
  QDomNodeList nl = doc.elementsByTagName( "title" );

  title = "";                 // by default the title will be empty

  if ( !nl.count() )
  {
    QgsDebugMsg( "unable to find title element" );
    return;
  }

  QDomNode titleNode = nl.item( 0 );  // there should only be one, so zeroth element ok

  if ( !titleNode.hasChildNodes() ) // if not, then there's no actual text
  {
    QgsDebugMsg( "unable to find title element" );
    return;
  }

  QDomNode titleTextNode = titleNode.firstChild();  // should only have one child

  if ( !titleTextNode.isText() )
  {
    QgsDebugMsg( "unable to find title element" );
    return;
  }

  QDomText titleText = titleTextNode.toText();

  title = titleText.data();

}

QgsProjectVersion getVersion( const QDomDocument& doc )
{
  QDomNodeList nl = doc.elementsByTagName( "qgis" );

  if ( !nl.count() )
  {
    QgsDebugMsg( " unable to find qgis element in project file" );
    return QgsProjectVersion( 0, 0, 0, QString() );
  }

  QDomNode qgisNode = nl.item( 0 );  // there should only be one, so zeroth element ok

  QDomElement qgisElement = qgisNode.toElement(); // qgis node should be element
  QgsProjectVersion projectVersion( qgisElement.attribute( "version" ) );
  return projectVersion;
}

void QgsProject::processLayerJoins( QgsVectorLayer* layer )
{
  if ( !layer )
    return;

  layer->createJoinCaches();
  layer->updateFields();
}

bool QgsProject::_getMapLayers( const QDomDocument& doc, QList<QDomNode>& brokenNodes )
{
  // Layer order is set by the restoring the legend settings from project file.
  // This is done on the 'readProject( ... )' signal

  QDomNodeList nl = doc.elementsByTagName( "maplayer" );

  // process the map layer nodes

  if ( 0 == nl.count() )      // if we have no layers to process, bail
  {
    return true; // Decided to return "true" since it's
    // possible for there to be a project with no
    // layers; but also, more imporantly, this
    // would cause the tests/qgsproject to fail
    // since the test suite doesn't currently
    // support test layers
  }

  bool returnStatus = true;

  emit layerLoaded( 0, nl.count() );

  // order layers based on their dependencies
  QgsLayerDefinition::DependencySorter depSorter( doc );
  if ( depSorter.hasCycle() || depSorter.hasMissingDependency() )
    return false;

  QVector<QDomNode> sortedLayerNodes = depSorter.sortedLayerNodes();

  // Collect vector layers with joins.
  // They need to refresh join caches and symbology infos after all layers are loaded
  QList< QPair< QgsVectorLayer*, QDomElement > > vLayerList;
  int i = 0;
  Q_FOREACH ( const QDomNode& node, sortedLayerNodes )
  {
    QDomElement element = node.toElement();

    QString name = node.namedItem( "layername" ).toElement().text();
    if ( !name.isNull() )
      emit loadingLayer( tr( "Loading layer %1" ).arg( name ) );

    if ( element.attribute( "embedded" ) == "1" )
    {
      createEmbeddedLayer( element.attribute( "id" ), readPath( element.attribute( "project" ) ), brokenNodes, vLayerList );
      continue;
    }
    else
    {
      if ( !addLayer( element, brokenNodes, vLayerList ) )
      {
        returnStatus = false;
      }
    }
    emit layerLoaded( i + 1, nl.count() );
    i++;
  }

  // Update field map of layers with joins and create join caches if necessary
  // Needs to be done here once all dependent layers are loaded
  QList< QPair< QgsVectorLayer*, QDomElement > >::iterator vIt = vLayerList.begin();
  for ( ; vIt != vLayerList.end(); ++vIt )
  {
    processLayerJoins( vIt->first );
  }

  QSet<QgsVectorLayer *> notified;
  for ( vIt = vLayerList.begin(); vIt != vLayerList.end(); ++vIt )
  {
    if ( notified.contains( vIt->first ) )
      continue;

    notified << vIt->first;
    emit readMapLayer( vIt->first, vIt->second );
  }

  return returnStatus;
}

bool QgsProject::addLayer( const QDomElement &layerElem, QList<QDomNode> &brokenNodes, QList< QPair< QgsVectorLayer*, QDomElement > > &vectorLayerList )
{
  QString type = layerElem.attribute( "type" );
  QgsDebugMsg( "Layer type is " + type );
  QgsMapLayer *mapLayer = nullptr;

  if ( type == "vector" )
  {
    mapLayer = new QgsVectorLayer;
  }
  else if ( type == "raster" )
  {
    mapLayer = new QgsRasterLayer;
  }
  else if ( type == "plugin" )
  {
    QString typeName = layerElem.attribute( "name" );
    mapLayer = QgsPluginLayerRegistry::instance()->createLayer( typeName );
  }

  if ( !mapLayer )
  {
    QgsDebugMsg( "Unable to create layer" );

    return false;
  }

  Q_CHECK_PTR( mapLayer );

  // have the layer restore state that is stored in Dom node
  if ( mapLayer->readLayerXML( layerElem ) && mapLayer->isValid() )
  {
    // postpone readMapLayer signal for vector layers with joins
    QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer*>( mapLayer );
    if ( !vLayer || vLayer->vectorJoins().isEmpty() )
      emit readMapLayer( mapLayer, layerElem );
    else
      vectorLayerList.push_back( qMakePair( vLayer, layerElem ) );

    QList<QgsMapLayer *> myLayers;
    myLayers << mapLayer;
    QgsMapLayerRegistry::instance()->addMapLayers( myLayers );

    return true;
  }
  else
  {
    delete mapLayer;

    QgsDebugMsg( "Unable to load " + type + " layer" );
    brokenNodes.push_back( layerElem );
    return false;
  }
}

bool QgsProject::read( const QFileInfo& file )
{
  imp_->file.setFileName( file.filePath() );

  return read();
}

bool QgsProject::read()
{
  clearError();

  QScopedPointer<QDomDocument> doc( new QDomDocument( "qgis" ) );

  if ( !imp_->file.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    imp_->file.close();

    setError( tr( "Unable to open %1" ).arg( imp_->file.fileName() ) );

    return false;
  }

  // location of problem associated with errorMsg
  int line, column;
  QString errorMsg;

  if ( !doc->setContent( &imp_->file, &errorMsg, &line, &column ) )
  {
    // want to make this class as GUI independent as possible; so commented out
#if 0
    QMessageBox::critical( 0, tr( "Project File Read Error" ),
                           tr( "%1 at line %2 column %3" ).arg( errorMsg ).arg( line ).arg( column ) );
#endif

    QString errorString = tr( "Project file read error: %1 at line %2 column %3" )
                          .arg( errorMsg ).arg( line ).arg( column );

    QgsDebugMsg( errorString );

    imp_->file.close();

    setError( tr( "%1 for file %2" ).arg( errorString, imp_->file.fileName() ) );

    return false;
  }

  imp_->file.close();


  QgsDebugMsg( "Opened document " + imp_->file.fileName() );
  QgsDebugMsg( "Project title: " + imp_->title );

  // get project version string, if any
  QgsProjectVersion fileVersion =  getVersion( *doc );
  QgsProjectVersion thisVersion( QGis::QGIS_VERSION );

  if ( thisVersion > fileVersion )
  {
    QgsLogger::warning( "Loading a file that was saved with an older "
                        "version of qgis (saved in " + fileVersion.text() +
                        ", loaded in " + QGis::QGIS_VERSION +
                        "). Problems may occur." );

    QgsProjectFileTransform projectFile( *doc, fileVersion );

    //! Shows a warning when an old project file is read.
    emit oldProjectVersionWarning( fileVersion.text() );
    QgsDebugMsg( "Emitting oldProjectVersionWarning(oldVersion)." );

    projectFile.updateRevision( thisVersion );
  }

  // start new project, just keep the file name
  QString fileName = imp_->file.fileName();
  clear();
  imp_->file.setFileName( fileName );

  // now get any properties
  _getProperties( *doc, imp_->properties_ );

  QgsDebugMsg( QString::number( imp_->properties_.count() ) + " properties read" );

  dump_( imp_->properties_ );

  // now get project title
  _getTitle( *doc, imp_->title );

  QDomNodeList nl = doc->elementsByTagName( "autotransaction" );
  if ( nl.count() )
  {
    QDomElement transactionElement = nl.at( 0 ).toElement();
    if ( transactionElement.attribute( "active", "0" ).toInt() == 1 )
      imp_->autoTransaction = true;
  }

  nl = doc->elementsByTagName( "evaluateDefaultValues" );
  if ( nl.count() )
  {
    QDomElement evaluateDefaultValuesElement = nl.at( 0 ).toElement();
    if ( evaluateDefaultValuesElement.attribute( "active", "0" ).toInt() == 1 )
      imp_->evaluateDefaultValues = true;
  }

  // read the layer tree from project file

  mRootGroup->setCustomProperty( "loading", 1 );

  QDomElement layerTreeElem = doc->documentElement().firstChildElement( "layer-tree-group" );
  if ( !layerTreeElem.isNull() )
  {
    mRootGroup->readChildrenFromXML( layerTreeElem );
  }
  else
  {
    QgsLayerTreeUtils::readOldLegend( mRootGroup, doc->documentElement().firstChildElement( "legend" ) );
  }

  QgsDebugMsg( "Loaded layer tree:\n " + mRootGroup->dump() );

  mLayerTreeRegistryBridge->setEnabled( false );

  // get the map layers
  QList<QDomNode> brokenNodes;
  bool clean = _getMapLayers( *doc, brokenNodes );

  // review the integrity of the retrieved map layers
  if ( !clean )
  {
    QgsDebugMsg( "Unable to get map layers from project file." );

    if ( !brokenNodes.isEmpty() )
    {
      QgsDebugMsg( "there are " + QString::number( brokenNodes.size() ) + " broken layers" );
    }

    // we let a custom handler to decide what to do with missing layers
    // (default implementation ignores them, there's also a GUI handler that lets user choose correct path)
    mBadLayerHandler->handleBadLayers( brokenNodes, *doc );
  }

  mLayerTreeRegistryBridge->setEnabled( true );

  // load embedded groups and layers
  loadEmbeddedNodes( mRootGroup );

  // make sure the are just valid layers
  QgsLayerTreeUtils::removeInvalidLayers( mRootGroup );

  mRootGroup->removeCustomProperty( "loading" );

  mVisibilityPresetCollection.reset( new QgsVisibilityPresetCollection() );
  mVisibilityPresetCollection->readXML( *doc );


  // read the project: used by map canvas and legend
  emit readProject( *doc );

  // if all went well, we're allegedly in pristine state
  if ( clean )
    setDirty( false );

  emit nonIdentifiableLayersChanged( nonIdentifiableLayers() );

  return true;
}


void QgsProject::loadEmbeddedNodes( QgsLayerTreeGroup *group )
{
  Q_FOREACH ( QgsLayerTreeNode *child, group->children() )
  {
    if ( QgsLayerTree::isGroup( child ) )
    {
      QgsLayerTreeGroup *childGroup = QgsLayerTree::toGroup( child );
      if ( childGroup->customProperty( "embedded" ).toInt() )
      {
        // make sure to convert the path from relative to absolute
        QString projectPath = readPath( childGroup->customProperty( "embedded_project" ).toString() );
        childGroup->setCustomProperty( "embedded_project", projectPath );

        QgsLayerTreeGroup *newGroup = createEmbeddedGroup( childGroup->name(), projectPath, childGroup->customProperty( "embedded-invisible-layers" ).toStringList() );
        if ( newGroup )
        {
          QList<QgsLayerTreeNode*> clonedChildren;
          Q_FOREACH ( QgsLayerTreeNode *newGroupChild, newGroup->children() )
            clonedChildren << newGroupChild->clone();
          delete newGroup;

          childGroup->insertChildNodes( 0, clonedChildren );
        }
      }
      else
      {
        loadEmbeddedNodes( childGroup );
      }
    }
    else if ( QgsLayerTree::isLayer( child ) )
    {
      if ( child->customProperty( "embedded" ).toInt() )
      {
        QList<QDomNode> brokenNodes;
        QList< QPair< QgsVectorLayer*, QDomElement > > vectorLayerList;
        createEmbeddedLayer( QgsLayerTree::toLayer( child )->layerId(), child->customProperty( "embedded_project" ).toString(), brokenNodes, vectorLayerList );
      }
    }

  }
}

void QgsProject::onMapLayersAdded( const QList<QgsMapLayer*>& layers )
{
  Q_FOREACH ( QgsMapLayer* layer, layers )
  {
    QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer*>( layer );
    if ( vlayer )
    {
      if ( autoTransaction() )
      {
        if ( QgsTransaction::supportsTransaction( vlayer ) )
        {
          QString connString = QgsDataSourceURI( vlayer->source() ).connectionInfo();
          QString key = vlayer->providerType();

          QgsTransactionGroup* tg = mTransactionGroups.value( qMakePair( key, connString ) );

          if ( !tg )
          {
            tg = new QgsTransactionGroup();
            mTransactionGroups.insert( qMakePair( key, connString ), tg );

            connect( tg, SIGNAL( commitError( QString ) ), this, SLOT( displayMapToolMessage( QString ) ) );
          }
          tg->addLayer( vlayer );
        }
      }
      vlayer->dataProvider()->setProviderProperty( QgsVectorDataProvider::EvaluateDefaultValues, evaluateDefaultValues() );
    }

    connect( layer, SIGNAL( configChanged() ), this, SLOT( setDirty() ) );
  }
}

void QgsProject::cleanTransactionGroups( bool force )
{
  for ( QMap< QPair< QString, QString>, QgsTransactionGroup*>::Iterator tg = mTransactionGroups.begin(); tg != mTransactionGroups.end(); )
  {
    if ( tg.value()->isEmpty() || force )
    {
      delete tg.value();
      tg = mTransactionGroups.erase( tg );
    }
    else
    {
      ++tg;
    }
  }
}

bool QgsProject::read( QDomNode &layerNode )
{
  QList<QDomNode> brokenNodes;
  QList< QPair< QgsVectorLayer*, QDomElement > > vectorLayerList;
  if ( addLayer( layerNode.toElement(), brokenNodes, vectorLayerList ) )
  {
    // have to try to update joins for all layers now - a previously added layer may be dependent on this newly
    // added layer for joins
    QVector<QgsVectorLayer*> vectorLayers = QgsMapLayerRegistry::instance()->layers<QgsVectorLayer*>();
    Q_FOREACH ( QgsVectorLayer* layer, vectorLayers )
    {
      processLayerJoins( layer );
    }

    if ( !vectorLayerList.isEmpty() )
    {
      emit readMapLayer( vectorLayerList.at( 0 ).first, vectorLayerList.at( 0 ).second );
    }

    return true;
  }
  return false;
}

bool QgsProject::write( QFileInfo const &file )
{
  imp_->file.setFileName( file.filePath() );

  return write();
}

bool QgsProject::write()
{
  clearError();

  // if we have problems creating or otherwise writing to the project file,
  // let's find out up front before we go through all the hand-waving
  // necessary to create all the Dom objects
  QFileInfo myFileInfo( imp_->file );
  if ( myFileInfo.exists() && !myFileInfo.isWritable() )
  {
    setError( tr( "%1 is not writable. Please adjust permissions (if possible) and try again." )
              .arg( imp_->file.fileName() ) );
    return false;
  }

  QDomImplementation DomImplementation;
  DomImplementation.setInvalidDataPolicy( QDomImplementation::DropInvalidChars );

  QDomDocumentType documentType =
    DomImplementation.createDocumentType( "qgis", "http://mrcc.com/qgis.dtd",
                                          "SYSTEM" );
  QScopedPointer<QDomDocument> doc( new QDomDocument( documentType ) );

  QDomElement qgisNode = doc->createElement( "qgis" );
  qgisNode.setAttribute( "projectname", title() );
  qgisNode.setAttribute( "version", QString( "%1" ).arg( QGis::QGIS_VERSION ) );

  doc->appendChild( qgisNode );

  // title
  QDomElement titleNode = doc->createElement( "title" );
  qgisNode.appendChild( titleNode );

  QDomElement transactionNode = doc->createElement( "autotransaction" );
  transactionNode.setAttribute( "active", imp_->autoTransaction ? "1" : "0" );
  qgisNode.appendChild( transactionNode );

  QDomElement evaluateDefaultValuesNode = doc->createElement( "evaluateDefaultValues" );
  evaluateDefaultValuesNode.setAttribute( "active", imp_->evaluateDefaultValues ? "1" : "0" );
  qgisNode.appendChild( evaluateDefaultValuesNode );

  QDomText titleText = doc->createTextNode( title() );  // XXX why have title TWICE?
  titleNode.appendChild( titleText );

  // write layer tree - make sure it is without embedded subgroups
  QgsLayerTreeNode *clonedRoot = mRootGroup->clone();
  QgsLayerTreeUtils::replaceChildrenOfEmbeddedGroups( QgsLayerTree::toGroup( clonedRoot ) );
  QgsLayerTreeUtils::updateEmbeddedGroupsProjectPath( QgsLayerTree::toGroup( clonedRoot ) ); // convert absolute paths to relative paths if required
  clonedRoot->writeXML( qgisNode );
  delete clonedRoot;

  // let map canvas and legend write their information
  emit writeProject( *doc );

  // within top level node save list of layers
  const QMap<QString, QgsMapLayer*> &layers = QgsMapLayerRegistry::instance()->mapLayers();

  // Iterate over layers in zOrder
  // Call writeXML() on each
  QDomElement projectLayersNode = doc->createElement( "projectlayers" );

  QMap<QString, QgsMapLayer*>::ConstIterator li = layers.constBegin();
  while ( li != layers.end() )
  {
    QgsMapLayer *ml = li.value();

    if ( ml )
    {
      QHash< QString, QPair< QString, bool> >::const_iterator emIt = mEmbeddedLayers.constFind( ml->id() );
      if ( emIt == mEmbeddedLayers.constEnd() )
      {
        // general layer metadata
        QDomElement maplayerElem = doc->createElement( "maplayer" );

        ml->writeLayerXML( maplayerElem, *doc );

        emit writeMapLayer( ml, maplayerElem, *doc );

        projectLayersNode.appendChild( maplayerElem );
      }
      else
      {
        // layer defined in an external project file
        // only save embedded layer if not managed by a legend group
        if ( emIt.value().second )
        {
          QDomElement mapLayerElem = doc->createElement( "maplayer" );
          mapLayerElem.setAttribute( "embedded", 1 );
          mapLayerElem.setAttribute( "project", writePath( emIt.value().first ) );
          mapLayerElem.setAttribute( "id", ml->id() );
          projectLayersNode.appendChild( mapLayerElem );
        }
      }
    }
    li++;
  }

  qgisNode.appendChild( projectLayersNode );

  // now add the optional extra properties

  dump_( imp_->properties_ );

  QgsDebugMsg( QString( "there are %1 property scopes" ).arg( static_cast<int>( imp_->properties_.count() ) ) );

  if ( !imp_->properties_.isEmpty() ) // only worry about properties if we
    // actually have any properties
  {
    imp_->properties_.writeXML( "properties", qgisNode, *doc );
  }

  mVisibilityPresetCollection->writeXML( *doc );

  // now wrap it up and ship it to the project file
  doc->normalize();             // XXX I'm not entirely sure what this does

  // Create backup file
  if ( QFile::exists( fileName() ) )
  {
    QFile backupFile( fileName() + '~' );
    bool ok = true;
    ok &= backupFile.open( QIODevice::WriteOnly | QIODevice::Truncate );
    ok &= imp_->file.open( QIODevice::ReadOnly );

    QByteArray ba;
    while ( ok && !imp_->file.atEnd() )
    {
      ba = imp_->file.read( 10240 );
      ok &= backupFile.write( ba ) == ba.size();
    }

    imp_->file.close();
    backupFile.close();

    if ( !ok )
    {
      setError( tr( "Unable to create backup file %1" ).arg( backupFile.fileName() ) );
      return false;
    }

    QFileInfo fi( fileName() );
    struct utimbuf tb = { fi.lastRead().toTime_t(), fi.lastModified().toTime_t() };
    utime( backupFile.fileName().toUtf8().constData(), &tb );
  }

  if ( !imp_->file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    imp_->file.close();         // even though we got an error, let's make
    // sure it's closed anyway

    setError( tr( "Unable to save to file %1" ).arg( imp_->file.fileName() ) );
    return false;
  }

  QTemporaryFile tempFile;
  bool ok = tempFile.open();
  if ( ok )
  {
    QTextStream projectFileStream( &tempFile );
    doc->save( projectFileStream, 2 );  // save as utf-8
    ok &= projectFileStream.pos() > -1;

    ok &= tempFile.seek( 0 );

    QByteArray ba;
    while ( ok && !tempFile.atEnd() )
    {
      ba = tempFile.read( 10240 );
      ok &= imp_->file.write( ba ) == ba.size();
    }

    ok &= imp_->file.error() == QFile::NoError;

    imp_->file.close();
  }

  tempFile.close();

  if ( !ok )
  {
    setError( tr( "Unable to save to file %1. Your project "
                  "may be corrupted on disk. Try clearing some space on the volume and "
                  "check file permissions before pressing save again." )
              .arg( imp_->file.fileName() ) );
    return false;
  }

  setDirty( false );               // reset to pristine state

  emit projectSaved();

  return true;
}

void QgsProject::clearProperties()
{
  clear();

  setDirty( true );
}

bool QgsProject::writeEntry( const QString& scope, QString const& key, bool value )
{
  setDirty( true );

  return addKey_( scope, key, &imp_->properties_, value );
}

bool QgsProject::writeEntry( const QString& scope, const QString& key,
                             double value )
{
  setDirty( true );

  return addKey_( scope, key, &imp_->properties_, value );
}

bool QgsProject::writeEntry( const QString& scope, QString const& key, int value )
{
  setDirty( true );

  return addKey_( scope, key, &imp_->properties_, value );
}

bool QgsProject::writeEntry( const QString& scope, const QString &key,
                             const QString& value )
{
  setDirty( true );

  return addKey_( scope, key, &imp_->properties_, value );
}

bool QgsProject::writeEntry( const QString& scope, const QString& key,
                             const QStringList& value )
{
  setDirty( true );

  return addKey_( scope, key, &imp_->properties_, value );
}

QStringList QgsProject::readListEntry( const QString& scope,
                                       const QString &key,
                                       const QStringList& def,
                                       bool *ok ) const
{
  QgsProperty *property = findKey_( scope, key, imp_->properties_ );

  QVariant value;

  if ( property )
  {
    value = property->value();

    bool valid = QVariant::StringList == value.type();
    if ( ok )
      *ok = valid;

    if ( valid )
    {
      return value.toStringList();
    }
  }

  return def;
}


QString QgsProject::readEntry( const QString& scope,
                               const QString& key,
                               const QString& def,
                               bool* ok ) const
{
  QgsProperty *property = findKey_( scope, key, imp_->properties_ );

  QVariant value;

  if ( property )
  {
    value = property->value();

    bool valid = value.canConvert( QVariant::String );
    if ( ok )
      *ok = valid;

    if ( valid )
      return value.toString();
  }

  return def;
}

int QgsProject::readNumEntry( const QString& scope, const QString &key, int def,
                              bool* ok ) const
{
  QgsProperty *property = findKey_( scope, key, imp_->properties_ );

  QVariant value;

  if ( property )
  {
    value = property->value();
  }

  bool valid = value.canConvert( QVariant::String );

  if ( ok )
  {
    *ok = valid;
  }

  if ( valid )
  {
    return value.toInt();
  }

  return def;
}

double QgsProject::readDoubleEntry( const QString& scope, const QString& key,
                                    double def,
                                    bool* ok ) const
{
  QgsProperty *property = findKey_( scope, key, imp_->properties_ );
  if ( property )
  {
    QVariant value = property->value();

    bool valid = value.canConvert( QVariant::Double );
    if ( ok )
      *ok = valid;

    if ( valid )
      return value.toDouble();
  }

  return def;
}

bool QgsProject::readBoolEntry( const QString& scope, const QString &key, bool def,
                                bool* ok ) const
{
  QgsProperty *property = findKey_( scope, key, imp_->properties_ );

  if ( property )
  {
    QVariant value = property->value();

    bool valid = value.canConvert( QVariant::Bool );
    if ( ok )
      *ok = valid;

    if ( valid )
      return value.toBool();
  }

  return def;
}


bool QgsProject::removeEntry( const QString& scope, const QString& key )
{
  removeKey_( scope, key, imp_->properties_ );

  setDirty( true );

  return !findKey_( scope, key, imp_->properties_ );
}


QStringList QgsProject::entryList( const QString& scope, const QString& key ) const
{
  QgsProperty *foundProperty = findKey_( scope, key, imp_->properties_ );

  QStringList entries;

  if ( foundProperty )
  {
    QgsPropertyKey *propertyKey = dynamic_cast<QgsPropertyKey*>( foundProperty );

    if ( propertyKey )
      { propertyKey->entryList( entries ); }
  }

  return entries;
}

QStringList QgsProject::subkeyList( const QString& scope, const QString& key ) const
{
  QgsProperty *foundProperty = findKey_( scope, key, imp_->properties_ );

  QStringList entries;

  if ( foundProperty )
  {
    QgsPropertyKey *propertyKey = dynamic_cast<QgsPropertyKey*>( foundProperty );

    if ( propertyKey )
      { propertyKey->subkeyList( entries ); }
  }

  return entries;
}

void QgsProject::dumpProperties() const
{
  dump_( imp_->properties_ );
}

QString QgsProject::readPath( QString src ) const
{
  if ( readBoolEntry( "Paths", "/Absolute", false ) )
  {
    return src;
  }

  // if this is a VSIFILE, remove the VSI prefix and append to final result
  QString vsiPrefix = qgsVsiPrefix( src );
  if ( ! vsiPrefix.isEmpty() )
  {
    // unfortunately qgsVsiPrefix returns prefix also for files like "/x/y/z.gz"
    // so we need to check if we really have the prefix
    if ( src.startsWith( "/vsi", Qt::CaseInsensitive ) )
      src.remove( 0, vsiPrefix.size() );
    else
      vsiPrefix.clear();
  }

  // relative path should always start with ./ or ../
  if ( !src.startsWith( "./" ) && !src.startsWith( "../" ) )
  {
#if defined(Q_OS_WIN)
    if ( src.startsWith( "\\\\" ) ||
         src.startsWith( "//" ) ||
         ( src[0].isLetter() && src[1] == ':' ) )
    {
      // UNC or absolute path
      return vsiPrefix + src;
    }
#else
    if ( src[0] == '/' )
    {
      // absolute path
      return vsiPrefix + src;
    }
#endif

    // so this one isn't absolute, but also doesn't start // with ./ or ../.
    // That means that it was saved with an earlier version of "relative path support",
    // where the source file had to exist and only the project directory was stripped
    // from the filename.
    QString home = homePath();
    if ( home.isNull() )
      return vsiPrefix + src;

    QFileInfo fi( home + '/' + src );

    if ( !fi.exists() )
    {
      return vsiPrefix + src;
    }
    else
    {
      return vsiPrefix + fi.canonicalFilePath();
    }
  }

  QString srcPath = src;
  QString projPath = fileName();

  if ( projPath.isEmpty() )
  {
    return vsiPrefix + src;
  }

#if defined(Q_OS_WIN)
  srcPath.replace( '\\', '/' );
  projPath.replace( '\\', '/' );

  bool uncPath = projPath.startsWith( "//" );
#endif

  QStringList srcElems = srcPath.split( '/', QString::SkipEmptyParts );
  QStringList projElems = projPath.split( '/', QString::SkipEmptyParts );

#if defined(Q_OS_WIN)
  if ( uncPath )
  {
    projElems.insert( 0, "" );
    projElems.insert( 0, "" );
  }
#endif

  // remove project file element
  projElems.removeLast();

  // append source path elements
  projElems << srcElems;
  projElems.removeAll( "." );

  // resolve ..
  int pos;
  while (( pos = projElems.indexOf( ".." ) ) > 0 )
  {
    // remove preceding element and ..
    projElems.removeAt( pos - 1 );
    projElems.removeAt( pos - 1 );
  }

#if !defined(Q_OS_WIN)
  // make path absolute
  projElems.prepend( "" );
#endif

  return vsiPrefix + projElems.join( "/" );
}

QString QgsProject::writePath( const QString& src, const QString& relativeBasePath ) const
{
  if ( readBoolEntry( "Paths", "/Absolute", false ) || src.isEmpty() )
  {
    return src;
  }

  QFileInfo srcFileInfo( src );
  QFileInfo projFileInfo( fileName() );
  QString srcPath = srcFileInfo.exists() ? srcFileInfo.canonicalFilePath() : src;
  QString projPath = projFileInfo.canonicalFilePath();

  if ( !relativeBasePath.isNull() )
  {
    projPath = relativeBasePath;
  }

  if ( projPath.isEmpty() )
  {
    return src;
  }

  // if this is a VSIFILE, remove the VSI prefix and append to final result
  QString vsiPrefix = qgsVsiPrefix( src );
  if ( ! vsiPrefix.isEmpty() )
  {
    srcPath.remove( 0, vsiPrefix.size() );
  }

#if defined( Q_OS_WIN )
  const Qt::CaseSensitivity cs = Qt::CaseInsensitive;

  srcPath.replace( '\\', '/' );

  if ( srcPath.startsWith( "//" ) )
  {
    // keep UNC prefix
    srcPath = "\\\\" + srcPath.mid( 2 );
  }

  projPath.replace( '\\', '/' );
  if ( projPath.startsWith( "//" ) )
  {
    // keep UNC prefix
    projPath = "\\\\" + projPath.mid( 2 );
  }
#else
  const Qt::CaseSensitivity cs = Qt::CaseSensitive;
#endif

  QStringList projElems = projPath.split( '/', QString::SkipEmptyParts );
  QStringList srcElems = srcPath.split( '/', QString::SkipEmptyParts );

  // remove project file element
  projElems.removeLast();

  projElems.removeAll( "." );
  srcElems.removeAll( "." );

  // remove common part
  int n = 0;
  while ( !srcElems.isEmpty() &&
          !projElems.isEmpty() &&
          srcElems[0].compare( projElems[0], cs ) == 0 )
  {
    srcElems.removeFirst();
    projElems.removeFirst();
    n++;
  }

  if ( n == 0 )
  {
    // no common parts; might not even by a file
    return src;
  }

  if ( !projElems.isEmpty() )
  {
    // go up to the common directory
    for ( int i = 0; i < projElems.size(); i++ )
    {
      srcElems.insert( 0, ".." );
    }
  }
  else
  {
    // let it start with . nevertheless,
    // so relative path always start with either ./ or ../
    srcElems.insert( 0, "." );
  }

  return vsiPrefix + srcElems.join( "/" );
}

void QgsProject::setError( const QString& errorMessage )
{
  mErrorMessage = errorMessage;
}

QString QgsProject::error() const
{
  return mErrorMessage;
}

void QgsProject::clearError()
{
  setError( QString() );
}

void QgsProject::setBadLayerHandler( QgsProjectBadLayerHandler *handler )
{
  delete mBadLayerHandler;
  mBadLayerHandler = handler;
}

QString QgsProject::layerIsEmbedded( const QString &id ) const
{
  QHash< QString, QPair< QString, bool > >::const_iterator it = mEmbeddedLayers.find( id );
  if ( it == mEmbeddedLayers.constEnd() )
  {
    return QString();
  }
  return it.value().first;
}

bool QgsProject::createEmbeddedLayer( const QString &layerId, const QString &projectFilePath, QList<QDomNode> &brokenNodes,
                                      QList< QPair< QgsVectorLayer*, QDomElement > > &vectorLayerList, bool saveFlag )
{
  QgsDebugCall;

  static QString prevProjectFilePath;
  static QDateTime prevProjectFileTimestamp;
  static QDomDocument projectDocument;

  QDateTime projectFileTimestamp = QFileInfo( projectFilePath ).lastModified();

  if ( projectFilePath != prevProjectFilePath || projectFileTimestamp != prevProjectFileTimestamp )
  {
    prevProjectFilePath.clear();

    QFile projectFile( projectFilePath );
    if ( !projectFile.open( QIODevice::ReadOnly ) )
    {
      return false;
    }

    if ( !projectDocument.setContent( &projectFile ) )
    {
      return false;
    }

    prevProjectFilePath = projectFilePath;
    prevProjectFileTimestamp = projectFileTimestamp;
  }

  // does project store pathes absolute or relative?
  bool useAbsolutePathes = true;

  QDomElement propertiesElem = projectDocument.documentElement().firstChildElement( "properties" );
  if ( !propertiesElem.isNull() )
  {
    QDomElement absElem = propertiesElem.firstChildElement( "Paths" ).firstChildElement( "Absolute" );
    if ( !absElem.isNull() )
    {
      useAbsolutePathes = absElem.text().compare( "true", Qt::CaseInsensitive ) == 0;
    }
  }

  QDomElement projectLayersElem = projectDocument.documentElement().firstChildElement( "projectlayers" );
  if ( projectLayersElem.isNull() )
  {
    return false;
  }

  QDomNodeList mapLayerNodes = projectLayersElem.elementsByTagName( "maplayer" );
  for ( int i = 0; i < mapLayerNodes.size(); ++i )
  {
    // get layer id
    QDomElement mapLayerElem = mapLayerNodes.at( i ).toElement();
    QString id = mapLayerElem.firstChildElement( "id" ).text();
    if ( id == layerId )
    {
      // layer can be embedded only once
      if ( mapLayerElem.attribute( "embedded" ) == "1" )
      {
        return false;
      }

      mEmbeddedLayers.insert( layerId, qMakePair( projectFilePath, saveFlag ) );

      // change datasource path from relative to absolute if necessary
      // see also QgsMapLayer::readLayerXML
      if ( !useAbsolutePathes )
      {
        QString provider( mapLayerElem.firstChildElement( "provider" ).text() );
        QDomElement dsElem( mapLayerElem.firstChildElement( "datasource" ) );
        QString datasource( dsElem.text() );
        if ( provider == "spatialite" )
        {
          QgsDataSourceURI uri( datasource );
          QFileInfo absoluteDs( QFileInfo( projectFilePath ).absolutePath() + '/' + uri.database() );
          if ( absoluteDs.exists() )
          {
            uri.setDatabase( absoluteDs.absoluteFilePath() );
            datasource = uri.uri();
          }
        }
        else if ( provider == "ogr" )
        {
          QStringList theURIParts( datasource.split( '|' ) );
          QFileInfo absoluteDs( QFileInfo( projectFilePath ).absolutePath() + '/' + theURIParts[0] );
          if ( absoluteDs.exists() )
          {
            theURIParts[0] = absoluteDs.absoluteFilePath();
            datasource = theURIParts.join( "|" );
          }
        }
        else if ( provider == "gpx" )
        {
          QStringList theURIParts( datasource.split( '?' ) );
          QFileInfo absoluteDs( QFileInfo( projectFilePath ).absolutePath() + '/' + theURIParts[0] );
          if ( absoluteDs.exists() )
          {
            theURIParts[0] = absoluteDs.absoluteFilePath();
            datasource = theURIParts.join( "?" );
          }
        }
        else if ( provider == "delimitedtext" )
        {
          QUrl urlSource( QUrl::fromEncoded( datasource.toAscii() ) );

          if ( !datasource.startsWith( "file:" ) )
          {
            QUrl file( QUrl::fromLocalFile( datasource.left( datasource.indexOf( '?' ) ) ) );
            urlSource.setScheme( "file" );
            urlSource.setPath( file.path() );
          }

          QFileInfo absoluteDs( QFileInfo( projectFilePath ).absolutePath() + '/' + urlSource.toLocalFile() );
          if ( absoluteDs.exists() )
          {
            QUrl urlDest = QUrl::fromLocalFile( absoluteDs.absoluteFilePath() );
            urlDest.setQueryItems( urlSource.queryItems() );
            datasource = QString::fromAscii( urlDest.toEncoded() );
          }
        }
        else
        {
          QFileInfo absoluteDs( QFileInfo( projectFilePath ).absolutePath() + '/' + datasource );
          if ( absoluteDs.exists() )
          {
            datasource = absoluteDs.absoluteFilePath();
          }
        }

        dsElem.removeChild( dsElem.childNodes().at( 0 ) );
        dsElem.appendChild( projectDocument.createTextNode( datasource ) );
      }

      if ( addLayer( mapLayerElem, brokenNodes, vectorLayerList ) )
      {
        return true;
      }
      else
      {
        mEmbeddedLayers.remove( layerId );
        return false;
      }
    }
  }

  return false;
}


QgsLayerTreeGroup *QgsProject::createEmbeddedGroup( const QString &groupName, const QString &projectFilePath, const QStringList &invisibleLayers )
{
  // open project file, get layer ids in group, add the layers
  QFile projectFile( projectFilePath );
  if ( !projectFile.open( QIODevice::ReadOnly ) )
  {
    return nullptr;
  }

  QDomDocument projectDocument;
  if ( !projectDocument.setContent( &projectFile ) )
  {
    return nullptr;
  }

  // store identify disabled layers of the embedded project
  QSet<QString> embeddedIdentifyDisabledLayers;
  QDomElement disabledLayersElem = projectDocument.documentElement().firstChildElement( "properties" ).firstChildElement( "Identify" ).firstChildElement( "disabledLayers" );
  if ( !disabledLayersElem.isNull() )
  {
    QDomNodeList valueList = disabledLayersElem.elementsByTagName( "value" );
    for ( int i = 0; i < valueList.size(); ++i )
    {
      embeddedIdentifyDisabledLayers.insert( valueList.at( i ).toElement().text() );
    }
  }

  QgsLayerTreeGroup *root = new QgsLayerTreeGroup;

  QDomElement layerTreeElem = projectDocument.documentElement().firstChildElement( "layer-tree-group" );
  if ( !layerTreeElem.isNull() )
  {
    root->readChildrenFromXML( layerTreeElem );
  }
  else
  {
    QgsLayerTreeUtils::readOldLegend( root, projectDocument.documentElement().firstChildElement( "legend" ) );
  }

  QgsLayerTreeGroup *group = root->findGroup( groupName );
  if ( !group || group->customProperty( "embedded" ).toBool() )
  {
    // embedded groups cannot be embedded again
    delete root;
    return nullptr;
  }

  // clone the group sub-tree (it is used already in a tree, we cannot just tear it off)
  QgsLayerTreeGroup *newGroup = QgsLayerTree::toGroup( group->clone() );
  delete root;
  root = nullptr;

  newGroup->setCustomProperty( "embedded", 1 );
  newGroup->setCustomProperty( "embedded_project", projectFilePath );

  // set "embedded" to all children + load embedded layers
  mLayerTreeRegistryBridge->setEnabled( false );
  initializeEmbeddedSubtree( projectFilePath, newGroup );
  mLayerTreeRegistryBridge->setEnabled( true );

  QStringList thisProjectIdentifyDisabledLayers = nonIdentifiableLayers();

  // consider the layers might be identify disabled in its project
  Q_FOREACH ( const QString& layerId, newGroup->findLayerIds() )
  {
    if ( embeddedIdentifyDisabledLayers.contains( layerId ) )
    {
      thisProjectIdentifyDisabledLayers.append( layerId );
    }

    QgsLayerTreeLayer *layer = newGroup->findLayer( layerId );
    if ( layer )
    {
      layer->setVisible( invisibleLayers.contains( layerId ) ? Qt::Unchecked : Qt::Checked );
    }
  }

  setNonIdentifiableLayers( thisProjectIdentifyDisabledLayers );

  return newGroup;
}

void QgsProject::initializeEmbeddedSubtree( const QString &projectFilePath, QgsLayerTreeGroup *group )
{
  Q_FOREACH ( QgsLayerTreeNode *child, group->children() )
  {
    // all nodes in the subtree will have "embedded" custom property set
    child->setCustomProperty( "embedded", 1 );

    if ( QgsLayerTree::isGroup( child ) )
    {
      initializeEmbeddedSubtree( projectFilePath, QgsLayerTree::toGroup( child ) );
    }
    else if ( QgsLayerTree::isLayer( child ) )
    {
      // load the layer into our project
      QList<QDomNode> brokenNodes;
      QList< QPair< QgsVectorLayer*, QDomElement > > vectorLayerList;
      createEmbeddedLayer( QgsLayerTree::toLayer( child )->layerId(), projectFilePath, brokenNodes, vectorLayerList, false );
    }
  }
}

void QgsProject::setSnapSettingsForLayer( const QString &layerId, bool enabled, QgsSnapper::SnappingType  type, QgsTolerance::UnitType unit, double tolerance, bool avoidIntersection )
{
  QStringList layerIdList, enabledList, snapTypeList, toleranceUnitList, toleranceList, avoidIntersectionList;
  snapSettings( layerIdList, enabledList, snapTypeList, toleranceUnitList, toleranceList, avoidIntersectionList );
  int idx = layerIdList.indexOf( layerId );
  if ( idx != -1 )
  {
    layerIdList.removeAt( idx );
    enabledList.removeAt( idx );
    snapTypeList.removeAt( idx );
    toleranceUnitList.removeAt( idx );
    toleranceList.removeAt( idx );
    avoidIntersectionList.removeOne( layerId );
  }

  layerIdList.append( layerId );

  // enabled
  enabledList.append( enabled ? "enabled" : "disabled" );

  // snap type
  QString typeString;
  if ( type == QgsSnapper::SnapToSegment )
  {
    typeString = "to_segment";
  }
  else if ( type == QgsSnapper::SnapToVertexAndSegment )
  {
    typeString = "to_vertex_and_segment";
  }
  else
  {
    typeString = "to_vertex";
  }
  snapTypeList.append( typeString );

  // units
  toleranceUnitList.append( QString::number( unit ) );

  // tolerance
  toleranceList.append( QString::number( tolerance ) );

  // avoid intersection
  if ( avoidIntersection )
  {
    avoidIntersectionList.append( layerId );
  }

  writeEntry( "Digitizing", "/LayerSnappingList", layerIdList );
  writeEntry( "Digitizing", "/LayerSnappingEnabledList", enabledList );
  writeEntry( "Digitizing", "/LayerSnappingToleranceList", toleranceList );
  writeEntry( "Digitizing", "/LayerSnappingToleranceUnitList", toleranceUnitList );
  writeEntry( "Digitizing", "/LayerSnapToList", snapTypeList );
  writeEntry( "Digitizing", "/AvoidIntersectionsList", avoidIntersectionList );
  emit snapSettingsChanged();
}

bool QgsProject::snapSettingsForLayer( const QString &layerId, bool &enabled, QgsSnapper::SnappingType &type, QgsTolerance::UnitType &units, double &tolerance,
                                       bool &avoidIntersection ) const
{
  QStringList layerIdList, enabledList, snapTypeList, toleranceUnitList, toleranceList, avoidIntersectionList;
  snapSettings( layerIdList, enabledList, snapTypeList, toleranceUnitList, toleranceList, avoidIntersectionList );
  int idx = layerIdList.indexOf( layerId );
  if ( idx == -1 )
  {
    return false;
  }

  // make sure all lists are long enough
  int minListEntries = idx + 1;
  if ( layerIdList.size() < minListEntries || enabledList.size() < minListEntries || snapTypeList.size() < minListEntries ||
       toleranceUnitList.size() < minListEntries || toleranceList.size() < minListEntries )
  {
    return false;
  }

  // enabled
  enabled = enabledList.at( idx ) == "enabled";

  // snap type
  QString snapType = snapTypeList.at( idx );
  if ( snapType == "to_segment" )
  {
    type = QgsSnapper::SnapToSegment;
  }
  else if ( snapType == "to_vertex_and_segment" )
  {
    type = QgsSnapper::SnapToVertexAndSegment;
  }
  else // to vertex
  {
    type = QgsSnapper::SnapToVertex;
  }

  // units
  if ( toleranceUnitList.at( idx ) == "1" )
  {
    units = QgsTolerance::Pixels;
  }
  else if ( toleranceUnitList.at( idx ) == "2" )
  {
    units = QgsTolerance::ProjectUnits;
  }
  else
  {
    units = QgsTolerance::LayerUnits;
  }

  // tolerance
  tolerance = toleranceList.at( idx ).toDouble();

  // avoid intersection
  avoidIntersection = ( avoidIntersectionList.indexOf( layerId ) != -1 );

  return true;
}

void QgsProject::snapSettings( QStringList &layerIdList, QStringList &enabledList, QStringList &snapTypeList, QStringList &toleranceUnitList, QStringList &toleranceList,
                               QStringList &avoidIntersectionList ) const
{
  layerIdList = readListEntry( "Digitizing", "/LayerSnappingList" );
  enabledList = readListEntry( "Digitizing", "/LayerSnappingEnabledList" );
  toleranceList = readListEntry( "Digitizing", "/LayerSnappingToleranceList" );
  toleranceUnitList = readListEntry( "Digitizing", "/LayerSnappingToleranceUnitList" );
  snapTypeList = readListEntry( "Digitizing", "/LayerSnapToList" );
  avoidIntersectionList = readListEntry( "Digitizing", "/AvoidIntersectionsList" );
}

bool QgsProject::evaluateDefaultValues() const
{
  return imp_->evaluateDefaultValues;
}

void QgsProject::setEvaluateDefaultValues( bool evaluateDefaultValues )
{
  Q_FOREACH ( QgsMapLayer* layer, QgsMapLayerRegistry::instance()->mapLayers().values() )
  {
    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( layer );
    if ( vl )
    {
      vl->dataProvider()->setProviderProperty( QgsVectorDataProvider::EvaluateDefaultValues, evaluateDefaultValues );
    }
  }

  imp_->evaluateDefaultValues = evaluateDefaultValues;
}

void QgsProject::setTopologicalEditing( bool enabled )
{
  QgsProject::instance()->writeEntry( "Digitizing", "/TopologicalEditing", ( enabled ? 1 : 0 ) );
  emit snapSettingsChanged();
}

bool QgsProject::topologicalEditing() const
{
  return ( QgsProject::instance()->readNumEntry( "Digitizing", "/TopologicalEditing", 0 ) > 0 );
}

QGis::UnitType QgsProject::distanceUnits() const
{
  QString distanceUnitString = QgsProject::instance()->readEntry( "Measurement", "/DistanceUnits", QString() );
  if ( !distanceUnitString.isEmpty() )
    return QgsUnitTypes::decodeDistanceUnit( distanceUnitString );

  //fallback to QGIS default measurement unit
  QSettings s;
  bool ok = false;
  QGis::UnitType type = QgsUnitTypes::decodeDistanceUnit( s.value( "/qgis/measure/displayunits" ).toString(), &ok );
  return ok ? type : QGis::Meters;
}

QgsUnitTypes::AreaUnit QgsProject::areaUnits() const
{
  QString areaUnitString = QgsProject::instance()->readEntry( "Measurement", "/AreaUnits", QString() );
  if ( !areaUnitString.isEmpty() )
    return QgsUnitTypes::decodeAreaUnit( areaUnitString );

  //fallback to QGIS default area unit
  QSettings s;
  bool ok = false;
  QgsUnitTypes::AreaUnit type = QgsUnitTypes::decodeAreaUnit( s.value( "/qgis/measure/areaunits" ).toString(), &ok );
  return ok ? type : QgsUnitTypes::SquareMeters;
}

void QgsProjectBadLayerDefaultHandler::handleBadLayers( const QList<QDomNode>& /*layers*/, const QDomDocument& /*projectDom*/ )
{
  // just ignore any bad layers
}

QString QgsProject::homePath() const
{
  QFileInfo pfi( fileName() );
  if ( !pfi.exists() )
    return QString::null;

  return pfi.canonicalPath();
}

QgsRelationManager *QgsProject::relationManager() const
{
  return mRelationManager;
}

QgsLayerTreeGroup *QgsProject::layerTreeRoot() const
{
  return mRootGroup;
}

QgsVisibilityPresetCollection* QgsProject::visibilityPresetCollection()
{
  return mVisibilityPresetCollection.data();
}

void QgsProject::setNonIdentifiableLayers( QList<QgsMapLayer*> layers )
{
  QStringList currentLayers = nonIdentifiableLayers();

  QStringList newLayers;
  Q_FOREACH ( QgsMapLayer* l, layers )
  {
    newLayers << l->id();
  }

  if ( newLayers == currentLayers )
    return;

  QStringList disabledLayerIds;

  Q_FOREACH ( QgsMapLayer* l, layers )
  {
    disabledLayerIds << l->id();
  }

  setNonIdentifiableLayers( disabledLayerIds );
}

void QgsProject::setNonIdentifiableLayers( const QStringList& layerIds )
{
  writeEntry( "Identify", "/disabledLayers", layerIds );

  emit nonIdentifiableLayersChanged( layerIds );
}

QStringList QgsProject::nonIdentifiableLayers() const
{
  return QgsProject::instance()->readListEntry( "Identify", "/disabledLayers" );
}

bool QgsProject::autoTransaction() const
{
  return imp_->autoTransaction;
}

void QgsProject::setAutoTransaction( bool autoTransaction )
{
  if ( autoTransaction != imp_->autoTransaction )
  {
    imp_->autoTransaction = autoTransaction;

    if ( autoTransaction )
      onMapLayersAdded( QgsMapLayerRegistry::instance()->mapLayers().values() );
    else
      cleanTransactionGroups( true );
  }
}

QMap<QPair<QString, QString>, QgsTransactionGroup*> QgsProject::transactionGroups()
{
  return mTransactionGroups;
}
