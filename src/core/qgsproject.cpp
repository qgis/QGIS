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
#include "qgslabelingenginesettings.h"
#include "qgslayertree.h"
#include "qgslayertreeutils.h"
#include "qgslayertreeregistrybridge.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgspluginlayer.h"
#include "qgspluginlayerregistry.h"
#include "qgsprojectfiletransform.h"
#include "qgssnappingconfig.h"
#include "qgspathresolver.h"
#include "qgsprojectstorage.h"
#include "qgsprojectstorageregistry.h"
#include "qgsprojectversion.h"
#include "qgsrasterlayer.h"
#include "qgsreadwritecontext.h"
#include "qgsrectangle.h"
#include "qgsrelationmanager.h"
#include "qgsannotationmanager.h"
#include "qgsvectorlayerjoininfo.h"
#include "qgsmapthemecollection.h"
#include "qgslayerdefinition.h"
#include "qgsunittypes.h"
#include "qgstransaction.h"
#include "qgstransactiongroup.h"
#include "qgsvectordataprovider.h"
#include "qgsprojectbadlayerhandler.h"
#include "qgssettings.h"
#include "qgsmaplayerlistutils.h"
#include "qgsmeshlayer.h"
#include "qgslayoutmanager.h"
#include "qgsbookmarkmanager.h"
#include "qgsmaplayerstore.h"
#include "qgsziputils.h"
#include "qgsauxiliarystorage.h"
#include "qgssymbollayerutils.h"
#include "qgsapplication.h"
#include "qgsexpressioncontextutils.h"
#include "qgsstyleentityvisitor.h"
#include "qgsprojectviewsettings.h"
#include "qgsprojectdisplaysettings.h"
#include "qgsprojecttimesettings.h"
#include "qgsvectortilelayer.h"
#include "qgsruntimeprofiler.h"

#include <algorithm>
#include <QApplication>
#include <QFileInfo>
#include <QDomNode>
#include <QObject>
#include <QTextStream>
#include <QTemporaryFile>
#include <QDir>
#include <QUrl>


#ifdef _MSC_VER
#include <sys/utime.h>
#else
#include <utime.h>
#endif

// canonical project instance
QgsProject *QgsProject::sProject = nullptr;

/**
    Take the given scope and key and convert them to a string list of key
    tokens that will be used to navigate through a Property hierarchy

    E.g., scope "someplugin" and key "/foo/bar/baz" will become a string list
    of { "properties", "someplugin", "foo", "bar", "baz" }.  "properties" is
    always first because that's the permanent ``root'' Property node.
 */
QStringList makeKeyTokens_( const QString &scope, const QString &key )
{
  QStringList keyTokens = QStringList( scope );
  keyTokens += key.split( '/', QString::SkipEmptyParts );

  // be sure to include the canonical root node
  keyTokens.push_front( QStringLiteral( "properties" ) );

  //check validy of keys since an invalid xml name will will be dropped upon saving the xml file. If not valid, we print a message to the console.
  for ( int i = 0; i < keyTokens.size(); ++i )
  {
    QString keyToken = keyTokens.at( i );

    //invalid chars in XML are found at http://www.w3.org/TR/REC-xml/#NT-NameChar
    //note : it seems \x10000-\xEFFFF is valid, but it when added to the regexp, a lot of unwanted characters remain
    QString nameCharRegexp = QStringLiteral( "[^:A-Z_a-z\\xC0-\\xD6\\xD8-\\xF6\\xF8-\\x2FF\\x370-\\x37D\\x37F-\\x1FFF\\x200C-\\x200D\\x2070-\\x218F\\x2C00-\\x2FEF\\x3001-\\xD7FF\\xF900-\\xFDCF\\xFDF0-\\xFFFD\\-\\.0-9\\xB7\\x0300-\\x036F\\x203F-\\x2040]" );
    QString nameStartCharRegexp = QStringLiteral( "^[^:A-Z_a-z\\xC0-\\xD6\\xD8-\\xF6\\xF8-\\x2FF\\x370-\\x37D\\x37F-\\x1FFF\\x200C-\\x200D\\x2070-\\x218F\\x2C00-\\x2FEF\\x3001-\\xD7FF\\xF900-\\xFDCF\\xFDF0-\\xFFFD]" );

    if ( keyToken.contains( QRegExp( nameCharRegexp ) ) || keyToken.contains( QRegExp( nameStartCharRegexp ) ) )
    {

      QString errorString = QObject::tr( "Entry token invalid : '%1'. The token will not be saved to file." ).arg( keyToken );
      QgsMessageLog::logMessage( errorString, QString(), Qgis::Critical );

    }

  }

  return keyTokens;
}



/**
   return the property that matches the given key sequence, if any

   \param scope scope of key
   \param key keyname
   \param rootProperty is likely to be the top level QgsProjectPropertyKey in QgsProject:e:Imp.

   \return null if not found, otherwise located Property
*/
QgsProjectProperty *findKey_( const QString &scope,
                              const QString &key,
                              QgsProjectPropertyKey &rootProperty )
{
  QgsProjectPropertyKey *currentProperty = &rootProperty;
  QgsProjectProperty *nextProperty;           // link to next property down hierarchy

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
      else if ( ( nextProperty = currentProperty->find( keySequence.first() ) ) )
      {
        if ( nextProperty->isKey() )
        {
          currentProperty = static_cast<QgsProjectPropertyKey *>( nextProperty );
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
          // QgsProjectPropertyValue not Key, so return null
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



/**
 * Add the given key and value

\param scope scope of key
\param key key name
\param rootProperty is the property from which to start adding
\param value the value associated with the key
\param propertiesModified the parameter will be set to true if the written entry modifies pre-existing properties
*/
QgsProjectProperty *addKey_( const QString &scope,
                             const QString &key,
                             QgsProjectPropertyKey *rootProperty,
                             const QVariant &value,
                             bool &propertiesModified )
{
  QStringList keySequence = makeKeyTokens_( scope, key );

  // cursor through property key/value hierarchy
  QgsProjectPropertyKey *currentProperty = rootProperty;
  QgsProjectProperty *nextProperty; // link to next property down hierarchy
  QgsProjectPropertyKey *newPropertyKey = nullptr;

  propertiesModified = false;
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
        QgsProjectProperty *property = currentProperty->find( keySequence.front() );
        if ( !property || property->value() != value )
        {
          currentProperty->setValue( keySequence.front(), value );
          propertiesModified = true;
        }

        return currentProperty;
      }
      // we're at the top element if popping the keySequence element
      // will leave it empty; in that case, just add the key
      else if ( keySequence.isEmpty() )
      {
        if ( currentProperty->value() != value )
        {
          currentProperty->setValue( value );
          propertiesModified = true;
        }

        return currentProperty;
      }
      else if ( ( nextProperty = currentProperty->find( keySequence.first() ) ) )
      {
        currentProperty = dynamic_cast<QgsProjectPropertyKey *>( nextProperty );

        if ( currentProperty )
        {
          continue;
        }
        else            // QgsProjectPropertyValue not Key, so return null
        {
          return nullptr;
        }
      }
      else                // the next subkey doesn't exist, so add it
      {
        if ( ( newPropertyKey = currentProperty->addKey( keySequence.first() ) ) )
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

/**
 * Remove a given key

\param scope scope of key
\param key key name
\param rootProperty is the property from which to start adding
*/

void removeKey_( const QString &scope,
                 const QString &key,
                 QgsProjectPropertyKey &rootProperty )
{
  QgsProjectPropertyKey *currentProperty = &rootProperty;

  QgsProjectProperty *nextProperty = nullptr;   // link to next property down hierarchy
  QgsProjectPropertyKey *previousQgsPropertyKey = nullptr; // link to previous property up hierarchy

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
      else if ( ( nextProperty = currentProperty->find( keySequence.first() ) ) )
      {
        previousQgsPropertyKey = currentProperty;
        currentProperty = dynamic_cast<QgsProjectPropertyKey *>( nextProperty );

        if ( currentProperty )
        {
          continue;
        }
        else            // QgsProjectPropertyValue not Key, so return null
        {
          return;
        }
      }
      else                // if the next key down isn't found
      {
        // then the overall key sequence doesn't exist
        return;
      }
    }
    else
    {
      return;
    }
  }
}

QgsProject::QgsProject( QObject *parent )
  : QObject( parent )
  , mLayerStore( new QgsMapLayerStore( this ) )
  , mBadLayerHandler( new QgsProjectBadLayerHandler() )
  , mSnappingConfig( this )
  , mRelationManager( new QgsRelationManager( this ) )
  , mAnnotationManager( new QgsAnnotationManager( this ) )
  , mLayoutManager( new QgsLayoutManager( this ) )
  , mBookmarkManager( QgsBookmarkManager::createProjectBasedManager( this ) )
  , mViewSettings( new QgsProjectViewSettings( this ) )
  , mTimeSettings( new QgsProjectTimeSettings( this ) )
  , mDisplaySettings( new QgsProjectDisplaySettings( this ) )
  , mRootGroup( new QgsLayerTree )
  , mLabelingEngineSettings( new QgsLabelingEngineSettings )
  , mArchive( new QgsProjectArchive() )
  , mAuxiliaryStorage( new QgsAuxiliaryStorage() )
{
  mProperties.setName( QStringLiteral( "properties" ) );
  clear();

  // bind the layer tree to the map layer registry.
  // whenever layers are added to or removed from the registry,
  // layer tree will be updated
  mLayerTreeRegistryBridge = new QgsLayerTreeRegistryBridge( mRootGroup, this, this );
  connect( this, &QgsProject::layersAdded, this, &QgsProject::onMapLayersAdded );
  connect( this, &QgsProject::layersRemoved, this, [ = ] { cleanTransactionGroups(); } );
  connect( this, qgis::overload< const QList<QgsMapLayer *> & >::of( &QgsProject::layersWillBeRemoved ), this, &QgsProject::onMapLayersRemoved );

  // proxy map layer store signals to this
  connect( mLayerStore.get(), qgis::overload<const QStringList &>::of( &QgsMapLayerStore::layersWillBeRemoved ),
  this, [ = ]( const QStringList & layers ) { mProjectScope.reset(); emit layersWillBeRemoved( layers ); } );
  connect( mLayerStore.get(), qgis::overload< const QList<QgsMapLayer *> & >::of( &QgsMapLayerStore::layersWillBeRemoved ),
  this, [ = ]( const QList<QgsMapLayer *> &layers ) { mProjectScope.reset(); emit layersWillBeRemoved( layers ); } );
  connect( mLayerStore.get(), qgis::overload< const QString & >::of( &QgsMapLayerStore::layerWillBeRemoved ),
  this, [ = ]( const QString & layer ) { mProjectScope.reset(); emit layerWillBeRemoved( layer ); } );
  connect( mLayerStore.get(), qgis::overload< QgsMapLayer * >::of( &QgsMapLayerStore::layerWillBeRemoved ),
  this, [ = ]( QgsMapLayer * layer ) { mProjectScope.reset(); emit layerWillBeRemoved( layer ); } );
  connect( mLayerStore.get(), qgis::overload<const QStringList & >::of( &QgsMapLayerStore::layersRemoved ), this,
  [ = ]( const QStringList & layers ) { mProjectScope.reset(); emit layersRemoved( layers ); } );
  connect( mLayerStore.get(), &QgsMapLayerStore::layerRemoved, this,
  [ = ]( const QString & layer ) { mProjectScope.reset(); emit layerRemoved( layer ); } );
  connect( mLayerStore.get(), &QgsMapLayerStore::allLayersRemoved, this,
  [ = ]() { mProjectScope.reset(); emit removeAll(); } );
  connect( mLayerStore.get(), &QgsMapLayerStore::layersAdded, this,
  [ = ]( const QList< QgsMapLayer * > &layers ) { mProjectScope.reset(); emit layersAdded( layers ); } );
  connect( mLayerStore.get(), &QgsMapLayerStore::layerWasAdded, this,
  [ = ]( QgsMapLayer * layer ) { mProjectScope.reset(); emit layerWasAdded( layer ); } );

  if ( QgsApplication::instance() )
  {
    connect( QgsApplication::instance(), &QgsApplication::requestForTranslatableObjects, this, &QgsProject::registerTranslatableObjects );
  }

  connect( mLayerStore.get(), qgis::overload< const QList<QgsMapLayer *> & >::of( &QgsMapLayerStore::layersWillBeRemoved ), this,
           [ = ]( const QList<QgsMapLayer *> &layers )
  {
    for ( const auto &layer : layers )
    {
      disconnect( layer, &QgsMapLayer::dataSourceChanged, mRelationManager, &QgsRelationManager::updateRelationsStatus );
    }
  }
         );
  connect( mLayerStore.get(),  qgis::overload< const QList<QgsMapLayer *> & >::of( &QgsMapLayerStore::layersAdded ), this,
           [ = ]( const QList<QgsMapLayer *> &layers )
  {
    for ( const auto &layer : layers )
    {
      connect( layer, &QgsMapLayer::dataSourceChanged, mRelationManager, &QgsRelationManager::updateRelationsStatus );
    }
  }
         );

  Q_NOWARN_DEPRECATED_PUSH
  connect( mViewSettings, &QgsProjectViewSettings::mapScalesChanged, this, &QgsProject::mapScalesChanged );
  Q_NOWARN_DEPRECATED_POP
}


QgsProject::~QgsProject()
{
  mIsBeingDeleted = true;

  clear();
  delete mBadLayerHandler;
  delete mRelationManager;
  delete mLayerTreeRegistryBridge;
  delete mRootGroup;
  if ( this == sProject )
  {
    sProject = nullptr;
  }
}

void QgsProject::setInstance( QgsProject *project )
{
  sProject = project;
}


QgsProject *QgsProject::instance()
{
  if ( !sProject )
  {
    sProject = new QgsProject;
  }
  return sProject;
}

void QgsProject::setTitle( const QString &title )
{
  if ( title == mMetadata.title() )
    return;

  mMetadata.setTitle( title );
  mProjectScope.reset();
  emit metadataChanged();

  setDirty( true );
}

QString QgsProject::title() const
{
  return mMetadata.title();
}

QString QgsProject::saveUser() const
{
  return mSaveUser;
}

QString QgsProject::saveUserFullName() const
{
  return mSaveUserFull;
}

QDateTime QgsProject::lastSaveDateTime() const
{
  return mSaveDateTime;
}

QgsProjectVersion QgsProject::lastSaveVersion() const
{
  return mSaveVersion;
}

bool QgsProject::isDirty() const
{
  return mDirty;
}

void QgsProject::setDirty( const bool dirty )
{
  if ( dirty && mDirtyBlockCount > 0 )
    return;

  if ( mDirty == dirty )
    return;

  mDirty = dirty;
  emit isDirtyChanged( mDirty );
}

void QgsProject::setPresetHomePath( const QString &path )
{
  if ( path == mHomePath )
    return;

  mHomePath = path;
  mCachedHomePath.clear();
  mProjectScope.reset();

  emit homePathChanged();

  setDirty( true );
}

void QgsProject::registerTranslatableContainers( QgsTranslationContext *translationContext, QgsAttributeEditorContainer *parent, const QString &layerId )
{
  const QList<QgsAttributeEditorElement *> elements = parent->children();

  for ( QgsAttributeEditorElement *element : elements )
  {
    if ( element->type() == QgsAttributeEditorElement::AeTypeContainer )
    {
      QgsAttributeEditorContainer *container = dynamic_cast<QgsAttributeEditorContainer *>( element );

      translationContext->registerTranslation( QStringLiteral( "project:layers:%1:formcontainers" ).arg( layerId ), container->name() );

      if ( !container->children().empty() )
        registerTranslatableContainers( translationContext, container, layerId );
    }
  }
}

void QgsProject::registerTranslatableObjects( QgsTranslationContext *translationContext )
{
  //register layers
  const QList<QgsLayerTreeLayer *> layers = mRootGroup->findLayers();

  for ( const QgsLayerTreeLayer *layer : layers )
  {
    translationContext->registerTranslation( QStringLiteral( "project:layers:%1" ).arg( layer->layerId() ), layer->name() );

    QgsMapLayer *mapLayer = layer->layer();
    if ( mapLayer && mapLayer->type() == QgsMapLayerType::VectorLayer )
    {
      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mapLayer );

      //register aliases and fields
      const QgsFields fields = vlayer->fields();
      for ( const QgsField &field : fields )
      {
        QString fieldName;
        if ( field.alias().isEmpty() )
          fieldName = field.name();
        else
          fieldName = field.alias();

        translationContext->registerTranslation( QStringLiteral( "project:layers:%1:fieldaliases" ).arg( vlayer->id() ), fieldName );

        if ( field.editorWidgetSetup().type() == QStringLiteral( "ValueRelation" ) )
        {
          translationContext->registerTranslation( QStringLiteral( "project:layers:%1:fields:%2:valuerelationvalue" ).arg( vlayer->id(), field.name() ), field.editorWidgetSetup().config().value( QStringLiteral( "Value" ) ).toString() );
        }
      }

      //register formcontainers
      registerTranslatableContainers( translationContext, vlayer->editFormConfig().invisibleRootContainer(), vlayer->id() );

    }
  }

  //register layergroups
  const QList<QgsLayerTreeGroup *> groupLayers = mRootGroup->findGroups();
  for ( const QgsLayerTreeGroup *groupLayer : groupLayers )
  {
    translationContext->registerTranslation( QStringLiteral( "project:layergroups" ), groupLayer->name() );
  }

  //register relations
  const QList<QgsRelation> &relations = mRelationManager->relations().values();
  for ( const QgsRelation &relation : relations )
  {
    translationContext->registerTranslation( QStringLiteral( "project:relations" ), relation.name() );
  }
}

void QgsProject::setDataDefinedServerProperties( const QgsPropertyCollection &properties )
{
  mDataDefinedServerProperties = properties;
}

QgsPropertyCollection QgsProject::dataDefinedServerProperties() const
{
  return mDataDefinedServerProperties;
}

void QgsProject::setFileName( const QString &name )
{
  if ( name == mFile.fileName() )
    return;

  QString oldHomePath = homePath();

  mFile.setFileName( name );
  mCachedHomePath.clear();
  mProjectScope.reset();

  emit fileNameChanged();

  QString newHomePath = homePath();
  if ( newHomePath != oldHomePath )
    emit homePathChanged();

  setDirty( true );
}

QString QgsProject::fileName() const
{
  return mFile.fileName();
}

void QgsProject::setOriginalPath( const QString &path )
{
  mOriginalPath = path;
}

QString QgsProject::originalPath() const
{
  return mOriginalPath;
}

QFileInfo QgsProject::fileInfo() const
{
  return QFileInfo( mFile );
}

QgsProjectStorage *QgsProject::projectStorage() const
{
  return QgsApplication::projectStorageRegistry()->projectStorageFromUri( mFile.fileName() );
}

QDateTime QgsProject::lastModified() const
{
  if ( QgsProjectStorage *storage = projectStorage() )
  {
    QgsProjectStorage::Metadata metadata;
    storage->readProjectStorageMetadata( mFile.fileName(), metadata );
    return metadata.lastModified;
  }
  else
  {
    return QFileInfo( mFile.fileName() ).lastModified();
  }
}

QString QgsProject::absolutePath() const
{
  if ( projectStorage() )
    return QString();

  if ( mFile.fileName().isEmpty() )
    return QString();  // this is to protect ourselves from getting current directory from QFileInfo::absoluteFilePath()

  return QFileInfo( mFile.fileName() ).absolutePath();
}

QString QgsProject::absoluteFilePath() const
{
  if ( projectStorage() )
    return QString();

  if ( mFile.fileName().isEmpty() )
    return QString();  // this is to protect ourselves from getting current directory from QFileInfo::absoluteFilePath()

  return QFileInfo( mFile.fileName() ).absoluteFilePath();
}

QString QgsProject::baseName() const
{
  if ( QgsProjectStorage *storage = projectStorage() )
  {
    QgsProjectStorage::Metadata metadata;
    storage->readProjectStorageMetadata( mFile.fileName(), metadata );
    return metadata.name;
  }
  else
  {
    return QFileInfo( mFile.fileName() ).completeBaseName();
  }
}

QgsCoordinateReferenceSystem QgsProject::crs() const
{
  return mCrs;
}

void QgsProject::setCrs( const QgsCoordinateReferenceSystem &crs, bool adjustEllipsoid )
{
  if ( crs != mCrs )
  {
    mCrs = crs;
    writeEntry( QStringLiteral( "SpatialRefSys" ), QStringLiteral( "/ProjectionsEnabled" ), crs.isValid() ? 1 : 0 );
    mProjectScope.reset();
    setDirty( true );
    emit crsChanged();
  }

  if ( adjustEllipsoid )
    setEllipsoid( crs.ellipsoidAcronym() );
}

QString QgsProject::ellipsoid() const
{
  if ( !crs().isValid() )
    return geoNone();

  return readEntry( QStringLiteral( "Measure" ), QStringLiteral( "/Ellipsoid" ), geoNone() );
}

void QgsProject::setEllipsoid( const QString &ellipsoid )
{
  if ( ellipsoid == readEntry( QStringLiteral( "Measure" ), QStringLiteral( "/Ellipsoid" ) ) )
    return;

  mProjectScope.reset();
  writeEntry( QStringLiteral( "Measure" ), QStringLiteral( "/Ellipsoid" ), ellipsoid );
  emit ellipsoidChanged( ellipsoid );
}

QgsCoordinateTransformContext QgsProject::transformContext() const
{
  return mTransformContext;
}

void QgsProject::setTransformContext( const QgsCoordinateTransformContext &context )
{
  if ( context == mTransformContext )
    return;

  mTransformContext = context;
  mProjectScope.reset();

  for ( auto &layer : mLayerStore.get()->mapLayers() )
  {
    layer->setTransformContext( context );
  }
  emit transformContextChanged();
}

void QgsProject::clear()
{
  QgsSettings settings;

  mProjectScope.reset();
  mFile.setFileName( QString() );
  mProperties.clearKeys();
  mSaveUser.clear();
  mSaveUserFull.clear();
  mSaveDateTime = QDateTime();
  mSaveVersion = QgsProjectVersion();
  mHomePath.clear();
  mCachedHomePath.clear();
  mAutoTransaction = false;
  mEvaluateDefaultValues = false;
  mDirty = false;
  mTrustLayerMetadata = false;
  mCustomVariables.clear();
  mMetadata = QgsProjectMetadata();
  if ( !settings.value( QStringLiteral( "projects/anonymize_new_projects" ), false, QgsSettings::Core ).toBool() )
  {
    mMetadata.setCreationDateTime( QDateTime::currentDateTime() );
    mMetadata.setAuthor( QgsApplication::userFullName() );
  }
  emit metadataChanged();

  QgsCoordinateTransformContext context;
  context.readSettings();
  setTransformContext( context );

  mEmbeddedLayers.clear();
  mRelationManager->clear();
  mAnnotationManager->clear();
  mLayoutManager->clear();
  mBookmarkManager->clear();
  mViewSettings->reset();
  mTimeSettings->reset();
  mDisplaySettings->reset();
  mSnappingConfig.reset();
  emit snappingConfigChanged( mSnappingConfig );
  emit avoidIntersectionsModeChanged();
  emit topologicalEditingChanged();

  mMapThemeCollection.reset( new QgsMapThemeCollection( this ) );
  emit mapThemeCollectionChanged();

  mLabelingEngineSettings->clear();

  mAuxiliaryStorage.reset( new QgsAuxiliaryStorage() );
  mArchive->clear();

  emit labelingEngineSettingsChanged();

  if ( !mIsBeingDeleted )
  {
    // possibly other signals should also not be thrown on destruction -- e.g. labelEngineSettingsChanged, etc.
    emit projectColorsChanged();
  }

  // reset some default project properties
  // XXX THESE SHOULD BE MOVED TO STATUSBAR RELATED SOURCE
  writeEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/Automatic" ), true );
  writeEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/DecimalPlaces" ), 2 );
  writeEntry( QStringLiteral( "Paths" ), QStringLiteral( "/Absolute" ), false );

  //copy default units to project
  writeEntry( QStringLiteral( "Measurement" ), QStringLiteral( "/DistanceUnits" ), settings.value( QStringLiteral( "/qgis/measure/displayunits" ) ).toString() );
  writeEntry( QStringLiteral( "Measurement" ), QStringLiteral( "/AreaUnits" ), settings.value( QStringLiteral( "/qgis/measure/areaunits" ) ).toString() );

  int red = settings.value( QStringLiteral( "qgis/default_canvas_color_red" ), 255 ).toInt();
  int green = settings.value( QStringLiteral( "qgis/default_canvas_color_green" ), 255 ).toInt();
  int blue = settings.value( QStringLiteral( "qgis/default_canvas_color_blue" ), 255 ).toInt();
  setBackgroundColor( QColor( red, green, blue ) );

  red = settings.value( QStringLiteral( "qgis/default_selection_color_red" ), 255 ).toInt();
  green = settings.value( QStringLiteral( "qgis/default_selection_color_green" ), 255 ).toInt();
  blue = settings.value( QStringLiteral( "qgis/default_selection_color_blue" ), 0 ).toInt();
  int alpha = settings.value( QStringLiteral( "qgis/default_selection_color_alpha" ), 255 ).toInt();
  setSelectionColor( QColor( red, green, blue, alpha ) );

  removeAllMapLayers();
  mRootGroup->clear();

  setDirty( false );
  emit homePathChanged();
  emit cleared();
}

// basically a debugging tool to dump property list values
void dump_( const QgsProjectPropertyKey &topQgsPropertyKey )
{
  QgsDebugMsgLevel( QStringLiteral( "current properties:" ), 3 );
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

\param doc xml document
\param project_properties should be the top QgsProjectPropertyKey node.

*/
void _getProperties( const QDomDocument &doc, QgsProjectPropertyKey &project_properties )
{
  QDomElement propertiesElem = doc.documentElement().firstChildElement( QStringLiteral( "properties" ) );

  if ( propertiesElem.isNull() )  // no properties found, so we're done
  {
    return;
  }

  QDomNodeList scopes = propertiesElem.childNodes();

  if ( scopes.count() < 1 )
  {
    QgsDebugMsg( QStringLiteral( "empty ``properties'' XML tag ... bailing" ) );
    return;
  }

  if ( ! project_properties.readXml( propertiesElem ) )
  {
    QgsDebugMsg( QStringLiteral( "Project_properties.readXml() failed" ) );
  }
}

/**
 * Returns the data defined server properties collection found in "doc" to "dataDefinedServerProperties".
 * \param doc xml document
 * \param dataDefinedServerPropertyDefinitions property collection of the server overrides
 * \since QGIS 3.14
**/
QgsPropertyCollection getDataDefinedServerProperties( const QDomDocument &doc, const QgsPropertiesDefinition &dataDefinedServerPropertyDefinitions )
{
  QgsPropertyCollection ddServerProperties;
  // Read data defined server properties
  QDomElement ddElem = doc.documentElement().firstChildElement( QStringLiteral( "dataDefinedServerProperties" ) );
  if ( !ddElem.isNull() )
  {
    if ( !ddServerProperties.readXml( ddElem, dataDefinedServerPropertyDefinitions ) )
    {
      QgsDebugMsg( QStringLiteral( "dataDefinedServerProperties.readXml() failed" ) );
    }
  }
  return ddServerProperties;
}

/**
   Get the project title
   \todo XXX we should go with the attribute xor title, not both.
*/
static void _getTitle( const QDomDocument &doc, QString &title )
{
  QDomNodeList nl = doc.elementsByTagName( QStringLiteral( "title" ) );

  title.clear();               // by default the title will be empty

  if ( !nl.count() )
  {
    QgsDebugMsgLevel( QStringLiteral( "unable to find title element" ), 2 );
    return;
  }

  QDomNode titleNode = nl.item( 0 );  // there should only be one, so zeroth element OK

  if ( !titleNode.hasChildNodes() ) // if not, then there's no actual text
  {
    QgsDebugMsgLevel( QStringLiteral( "unable to find title element" ), 2 );
    return;
  }

  QDomNode titleTextNode = titleNode.firstChild();  // should only have one child

  if ( !titleTextNode.isText() )
  {
    QgsDebugMsgLevel( QStringLiteral( "unable to find title element" ), 2 );
    return;
  }

  QDomText titleText = titleTextNode.toText();

  title = titleText.data();

}

static void readProjectFileMetadata( const QDomDocument &doc, QString &lastUser, QString &lastUserFull, QDateTime &lastSaveDateTime )
{
  QDomNodeList nl = doc.elementsByTagName( QStringLiteral( "qgis" ) );

  if ( !nl.count() )
  {
    QgsDebugMsg( "unable to find qgis element" );
    return;
  }

  QDomNode qgisNode = nl.item( 0 ); // there should only be one, so zeroth element OK

  QDomElement qgisElement = qgisNode.toElement(); // qgis node should be element
  lastUser = qgisElement.attribute( QStringLiteral( "saveUser" ), QString() );
  lastUserFull = qgisElement.attribute( QStringLiteral( "saveUserFull" ), QString() );
  lastSaveDateTime = QDateTime::fromString( qgisElement.attribute( QStringLiteral( "saveDateTime" ), QString() ), Qt::ISODate );
}


QgsProjectVersion getVersion( const QDomDocument &doc )
{
  QDomNodeList nl = doc.elementsByTagName( QStringLiteral( "qgis" ) );

  if ( !nl.count() )
  {
    QgsDebugMsg( QStringLiteral( " unable to find qgis element in project file" ) );
    return QgsProjectVersion( 0, 0, 0, QString() );
  }

  QDomNode qgisNode = nl.item( 0 );  // there should only be one, so zeroth element OK

  QDomElement qgisElement = qgisNode.toElement(); // qgis node should be element
  QgsProjectVersion projectVersion( qgisElement.attribute( QStringLiteral( "version" ) ) );
  return projectVersion;
}


QgsSnappingConfig QgsProject::snappingConfig() const
{
  return mSnappingConfig;
}

void QgsProject::setSnappingConfig( const QgsSnappingConfig &snappingConfig )
{
  if ( mSnappingConfig == snappingConfig )
    return;

  mSnappingConfig = snappingConfig;
  setDirty( true );
  emit snappingConfigChanged( mSnappingConfig );
}

void QgsProject::setAvoidIntersectionsMode( const AvoidIntersectionsMode mode )
{
  if ( mAvoidIntersectionsMode == mode )
    return;

  mAvoidIntersectionsMode = mode;
  emit avoidIntersectionsModeChanged();
}

bool QgsProject::_getMapLayers( const QDomDocument &doc, QList<QDomNode> &brokenNodes, QgsProject::ReadFlags flags )
{
  // Layer order is set by the restoring the legend settings from project file.
  // This is done on the 'readProject( ... )' signal

  QDomNodeList nl = doc.elementsByTagName( QStringLiteral( "maplayer" ) );

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
  QgsScopedRuntimeProfile profile( tr( "Sorting layers" ), QStringLiteral( "projectload" ) );
  QgsLayerDefinition::DependencySorter depSorter( doc );
  if ( depSorter.hasCycle() || depSorter.hasMissingDependency() )
    return false;

  const QVector<QDomNode> sortedLayerNodes = depSorter.sortedLayerNodes();
  const int totalLayerCount = sortedLayerNodes.count();

  int i = 0;
  for ( const QDomNode &node : sortedLayerNodes )
  {
    const QDomElement element = node.toElement();

    const QString name = translate( QStringLiteral( "project:layers:%1" ).arg( node.namedItem( QStringLiteral( "id" ) ).toElement().text() ), node.namedItem( QStringLiteral( "layername" ) ).toElement().text() );
    if ( !name.isNull() )
      emit loadingLayer( tr( "Loading layer %1" ).arg( name ) );

    profile.switchTask( name );

    if ( element.attribute( QStringLiteral( "embedded" ) ) == QLatin1String( "1" ) )
    {
      createEmbeddedLayer( element.attribute( QStringLiteral( "id" ) ), readPath( element.attribute( QStringLiteral( "project" ) ) ), brokenNodes, true, flags );
    }
    else
    {
      QgsReadWriteContext context;
      context.setPathResolver( pathResolver() );
      context.setProjectTranslator( this );
      context.setTransformContext( transformContext() );

      if ( !addLayer( element, brokenNodes, context, flags ) )
      {
        returnStatus = false;
      }
      const auto messages = context.takeMessages();
      if ( !messages.isEmpty() )
      {
        emit loadingLayerMessageReceived( tr( "Loading layer %1" ).arg( name ), messages );
      }
    }
    emit layerLoaded( i + 1, totalLayerCount );
    i++;
  }

  return returnStatus;
}

bool QgsProject::addLayer( const QDomElement &layerElem, QList<QDomNode> &brokenNodes, QgsReadWriteContext &context, QgsProject::ReadFlags flags )
{
  QString type = layerElem.attribute( QStringLiteral( "type" ) );
  QgsDebugMsgLevel( "Layer type is " + type, 4 );
  std::unique_ptr<QgsMapLayer> mapLayer;

  QgsScopedRuntimeProfile profile( tr( "Create layer" ), QStringLiteral( "projectload" ) );
  if ( type == QLatin1String( "vector" ) )
  {
    mapLayer = qgis::make_unique<QgsVectorLayer>();
    // apply specific settings to vector layer
    if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( mapLayer.get() ) )
    {
      vl->setReadExtentFromXml( mTrustLayerMetadata );
    }
  }
  else if ( type == QLatin1String( "raster" ) )
  {
    mapLayer =  qgis::make_unique<QgsRasterLayer>();
  }
  else if ( type == QLatin1String( "mesh" ) )
  {
    mapLayer = qgis::make_unique<QgsMeshLayer>();
  }
  else if ( type == QLatin1String( "vector-tile" ) )
  {
    mapLayer = qgis::make_unique<QgsVectorTileLayer>();
  }
  else if ( type == QLatin1String( "plugin" ) )
  {
    QString typeName = layerElem.attribute( QStringLiteral( "name" ) );
    mapLayer.reset( QgsApplication::pluginLayerRegistry()->createLayer( typeName ) );
  }

  if ( !mapLayer )
  {
    QgsDebugMsg( QStringLiteral( "Unable to create layer" ) );
    return false;
  }

  Q_CHECK_PTR( mapLayer ); // NOLINT

  // This is tricky: to avoid a leak we need to check if the layer was already in the store
  // because if it was, the newly created layer will not be added to the store and it would leak.
  const QString layerId { layerElem.namedItem( QStringLiteral( "id" ) ).toElement().text() };
  Q_ASSERT( ! layerId.isEmpty() );
  const bool layerWasStored { layerStore()->mapLayer( layerId ) != nullptr };

  // have the layer restore state that is stored in Dom node
  QgsMapLayer::ReadFlags layerFlags = QgsMapLayer::ReadFlags();
  if ( flags & QgsProject::ReadFlag::FlagDontResolveLayers )
    layerFlags |= QgsMapLayer::FlagDontResolveLayers;

  profile.switchTask( tr( "Load layer source" ) );
  bool layerIsValid = mapLayer->readLayerXml( layerElem, context, layerFlags ) && mapLayer->isValid();

  profile.switchTask( tr( "Add layer to project" ) );
  QList<QgsMapLayer *> newLayers;
  newLayers << mapLayer.get();
  if ( layerIsValid || flags & QgsProject::ReadFlag::FlagDontResolveLayers )
  {
    emit readMapLayer( mapLayer.get(), layerElem );
    addMapLayers( newLayers );
  }
  else
  {
    // It's a bad layer: do not add to legend (the user will decide if she wants to do so)
    addMapLayers( newLayers, false );
    newLayers.first();
    QgsDebugMsg( "Unable to load " + type + " layer" );
    brokenNodes.push_back( layerElem );
  }

  // It should be safe to delete the layer now if layer was stored, because all the store
  // had to to was to reset the data source in case the validity changed.
  if ( ! layerWasStored )
  {
    mapLayer.release();
  }

  return layerIsValid;
}

bool QgsProject::read( const QString &filename, QgsProject::ReadFlags flags )
{
  mFile.setFileName( filename );
  mCachedHomePath.clear();
  mProjectScope.reset();

  return read( flags );
}

bool QgsProject::read( QgsProject::ReadFlags flags )
{
  QString filename = mFile.fileName();
  bool returnValue;

  if ( QgsProjectStorage *storage = projectStorage() )
  {
    QTemporaryFile inDevice;
    if ( !inDevice.open() )
    {
      setError( tr( "Unable to open %1" ).arg( inDevice.fileName() ) );
      return false;
    }

    QgsReadWriteContext context;
    context.setProjectTranslator( this );
    if ( !storage->readProject( filename, &inDevice, context ) )
    {
      QString err = tr( "Unable to open %1" ).arg( filename );
      QList<QgsReadWriteContext::ReadWriteMessage> messages = context.takeMessages();
      if ( !messages.isEmpty() )
        err += QStringLiteral( "\n\n" ) + messages.last().message();
      setError( err );
      return false;
    }
    returnValue = unzip( inDevice.fileName(), flags );  // calls setError() if returning false
  }
  else
  {
    if ( QgsZipUtils::isZipFile( mFile.fileName() ) )
    {
      returnValue = unzip( mFile.fileName(), flags );
    }
    else
    {
      mAuxiliaryStorage.reset( new QgsAuxiliaryStorage( *this ) );
      returnValue = readProjectFile( mFile.fileName(), flags );
    }

    //on translation we should not change the filename back
    if ( !mTranslator )
    {
      mFile.setFileName( filename );
      mCachedHomePath.clear();
      mProjectScope.reset();
    }
    else
    {
      //but delete the translator
      mTranslator.reset( nullptr );
    }
  }
  emit homePathChanged();
  return returnValue;
}

bool QgsProject::readProjectFile( const QString &filename, QgsProject::ReadFlags flags )
{
  QFile projectFile( filename );
  clearError();

  QgsApplication::profiler()->clear( QStringLiteral( "projectload" ) );
  QgsScopedRuntimeProfile profile( tr( "Setting up translations" ), QStringLiteral( "projectload" ) );

  QgsSettings settings;

  QString localeFileName = QStringLiteral( "%1_%2" ).arg( QFileInfo( projectFile.fileName() ).baseName(), settings.value( QStringLiteral( "locale/userLocale" ), QString() ).toString() );

  if ( QFile( QStringLiteral( "%1/%2.qm" ).arg( QFileInfo( projectFile.fileName() ).absolutePath(), localeFileName ) ).exists() )
  {
    mTranslator.reset( new QTranslator() );
    mTranslator->load( localeFileName, QFileInfo( projectFile.fileName() ).absolutePath() );
  }

  profile.switchTask( tr( "Reading project file" ) );
  std::unique_ptr<QDomDocument> doc( new QDomDocument( QStringLiteral( "qgis" ) ) );

  if ( !projectFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    projectFile.close();

    setError( tr( "Unable to open %1" ).arg( projectFile.fileName() ) );

    return false;
  }

  // location of problem associated with errorMsg
  int line, column;
  QString errorMsg;

  if ( !doc->setContent( &projectFile, &errorMsg, &line, &column ) )
  {
    // want to make this class as GUI independent as possible; so commented out
#if 0
    QMessageBox::critical( 0, tr( "Read Project File" ),
                           tr( "%1 at line %2 column %3" ).arg( errorMsg ).arg( line ).arg( column ) );
#endif

    QString errorString = tr( "Project file read error in file %1: %2 at line %3 column %4" )
                          .arg( projectFile.fileName(), errorMsg ).arg( line ).arg( column );

    QgsDebugMsg( errorString );

    projectFile.close();

    setError( tr( "%1 for file %2" ).arg( errorString, projectFile.fileName() ) );

    return false;
  }

  projectFile.close();

  QgsDebugMsgLevel( "Opened document " + projectFile.fileName(), 2 );

  // get project version string, if any
  QgsProjectVersion fileVersion = getVersion( *doc );
  const QgsProjectVersion thisVersion( Qgis::version() );

  profile.switchTask( tr( "Updating project file" ) );
  if ( thisVersion > fileVersion )
  {
    QgsLogger::warning( "Loading a file that was saved with an older "
                        "version of qgis (saved in " + fileVersion.text() +
                        ", loaded in " + Qgis::version() +
                        "). Problems may occur." );

    QgsProjectFileTransform projectFile( *doc, fileVersion );

    // Shows a warning when an old project file is read.
    emit oldProjectVersionWarning( fileVersion.text() );

    projectFile.updateRevision( thisVersion );
  }

  // start new project, just keep the file name and auxiliary storage
  profile.switchTask( tr( "Creating auxiliary storage" ) );
  QString fileName = mFile.fileName();
  std::unique_ptr<QgsAuxiliaryStorage> aStorage = std::move( mAuxiliaryStorage );
  clear();
  mAuxiliaryStorage = std::move( aStorage );
  mFile.setFileName( fileName );
  mCachedHomePath.clear();
  mProjectScope.reset();
  mSaveVersion = fileVersion;

  // now get any properties
  profile.switchTask( tr( "Reading properties" ) );
  _getProperties( *doc, mProperties );

  // now get the data defined server properties
  mDataDefinedServerProperties = getDataDefinedServerProperties( *doc, dataDefinedServerPropertyDefinitions() );

  QgsDebugMsgLevel( QString::number( mProperties.count() ) + " properties read", 2 );

#if 0
  dump_( mProperties );
#endif

  // get older style project title
  QString oldTitle;
  _getTitle( *doc, oldTitle );

  readProjectFileMetadata( *doc, mSaveUser, mSaveUserFull, mSaveDateTime );

  QDomNodeList homePathNl = doc->elementsByTagName( QStringLiteral( "homePath" ) );
  if ( homePathNl.count() > 0 )
  {
    QDomElement homePathElement = homePathNl.at( 0 ).toElement();
    QString homePath = homePathElement.attribute( QStringLiteral( "path" ) );
    if ( !homePath.isEmpty() )
      setPresetHomePath( homePath );
  }
  else
  {
    emit homePathChanged();
  }

  const QColor backgroundColor( readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorRedPart" ), 255 ),
                                readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorGreenPart" ), 255 ),
                                readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorBluePart" ), 255 ) );
  setBackgroundColor( backgroundColor );
  const QColor selectionColor( readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/SelectionColorRedPart" ), 255 ),
                               readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/SelectionColorGreenPart" ), 255 ),
                               readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/SelectionColorBluePart" ), 255 ),
                               readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/SelectionColorAlphaPart" ), 255 ) );
  setSelectionColor( selectionColor );

  QgsReadWriteContext context;
  context.setPathResolver( pathResolver() );
  context.setProjectTranslator( this );

  //crs
  QgsCoordinateReferenceSystem projectCrs;
  if ( readNumEntry( QStringLiteral( "SpatialRefSys" ), QStringLiteral( "/ProjectionsEnabled" ), 0 ) )
  {
    // first preference - dedicated projectCrs node
    QDomNode srsNode = doc->documentElement().namedItem( QStringLiteral( "projectCrs" ) );
    if ( !srsNode.isNull() )
    {
      projectCrs.readXml( srsNode );
    }

    if ( !projectCrs.isValid() )
    {
      QString projCrsString = readEntry( QStringLiteral( "SpatialRefSys" ), QStringLiteral( "/ProjectCRSProj4String" ) );
      long currentCRS = readNumEntry( QStringLiteral( "SpatialRefSys" ), QStringLiteral( "/ProjectCRSID" ), -1 );
      const QString authid = readEntry( QStringLiteral( "SpatialRefSys" ), QStringLiteral( "/ProjectCrs" ) );

      // authid should be prioritized over all
      bool isUserAuthId = authid.startsWith( QLatin1String( "USER:" ), Qt::CaseInsensitive );
      if ( !authid.isEmpty() && !isUserAuthId )
        projectCrs = QgsCoordinateReferenceSystem( authid );

      // try the CRS
      if ( !projectCrs.isValid() && currentCRS >= 0 )
      {
        projectCrs = QgsCoordinateReferenceSystem::fromSrsId( currentCRS );
      }

      // if that didn't produce a match, try the proj.4 string
      if ( !projCrsString.isEmpty() && ( authid.isEmpty() || isUserAuthId ) && ( !projectCrs.isValid() || projectCrs.toProj() != projCrsString ) )
      {
        projectCrs = QgsCoordinateReferenceSystem::fromProj( projCrsString );
      }

      // last just take the given id
      if ( !projectCrs.isValid() )
      {
        projectCrs = QgsCoordinateReferenceSystem::fromSrsId( currentCRS );
      }
    }
  }
  mCrs = projectCrs;

  QStringList datumErrors;
  if ( !mTransformContext.readXml( doc->documentElement(), context, datumErrors ) && !datumErrors.empty() )
  {
    emit missingDatumTransforms( datumErrors );
  }
  emit transformContextChanged();

  //add variables defined in project file - do this early in the reading cycle, as other components
  //(e.g. layouts) may depend on these variables
  QStringList variableNames = readListEntry( QStringLiteral( "Variables" ), QStringLiteral( "/variableNames" ) );
  QStringList variableValues = readListEntry( QStringLiteral( "Variables" ), QStringLiteral( "/variableValues" ) );

  mCustomVariables.clear();
  if ( variableNames.length() == variableValues.length() )
  {
    for ( int i = 0; i < variableNames.length(); ++i )
    {
      mCustomVariables.insert( variableNames.at( i ), variableValues.at( i ) );
    }
  }
  else
  {
    QgsMessageLog::logMessage( tr( "Project Variables Invalid" ), tr( "The project contains invalid variable settings." ) );
  }

  QDomNodeList nl = doc->elementsByTagName( QStringLiteral( "projectMetadata" ) );
  if ( !nl.isEmpty() )
  {
    QDomElement metadataElement = nl.at( 0 ).toElement();
    mMetadata.readMetadataXml( metadataElement );
  }
  else
  {
    // older project, no metadata => remove auto generated metadata which is populated on QgsProject::clear()
    mMetadata = QgsProjectMetadata();
  }
  if ( mMetadata.title().isEmpty() && !oldTitle.isEmpty() )
  {
    // upgrade older title storage to storing within project metadata.
    mMetadata.setTitle( oldTitle );
  }
  emit metadataChanged();

  nl = doc->elementsByTagName( QStringLiteral( "autotransaction" ) );
  if ( nl.count() )
  {
    QDomElement transactionElement = nl.at( 0 ).toElement();
    if ( transactionElement.attribute( QStringLiteral( "active" ), QStringLiteral( "0" ) ).toInt() == 1 )
      mAutoTransaction = true;
  }

  nl = doc->elementsByTagName( QStringLiteral( "evaluateDefaultValues" ) );
  if ( nl.count() )
  {
    QDomElement evaluateDefaultValuesElement = nl.at( 0 ).toElement();
    if ( evaluateDefaultValuesElement.attribute( QStringLiteral( "active" ), QStringLiteral( "0" ) ).toInt() == 1 )
      mEvaluateDefaultValues = true;
  }

  nl = doc->elementsByTagName( QStringLiteral( "trust" ) );
  if ( nl.count() )
  {
    QDomElement trustElement = nl.at( 0 ).toElement();
    if ( trustElement.attribute( QStringLiteral( "active" ), QStringLiteral( "0" ) ).toInt() == 1 )
      mTrustLayerMetadata = true;
  }

  // read the layer tree from project file
  profile.switchTask( tr( "Loading layer tree" ) );
  mRootGroup->setCustomProperty( QStringLiteral( "loading" ), 1 );

  QDomElement layerTreeElem = doc->documentElement().firstChildElement( QStringLiteral( "layer-tree-group" ) );
  if ( !layerTreeElem.isNull() )
  {
    mRootGroup->readChildrenFromXml( layerTreeElem, context );
  }
  else
  {
    QgsLayerTreeUtils::readOldLegend( mRootGroup, doc->documentElement().firstChildElement( QStringLiteral( "legend" ) ) );
  }

  mLayerTreeRegistryBridge->setEnabled( false );

  // get the map layers
  profile.switchTask( tr( "Reading map layers" ) );

  QList<QDomNode> brokenNodes;
  bool clean = _getMapLayers( *doc, brokenNodes, flags );

  // review the integrity of the retrieved map layers
  if ( !clean )
  {
    QgsDebugMsg( QStringLiteral( "Unable to get map layers from project file." ) );

    if ( !brokenNodes.isEmpty() )
    {
      QgsDebugMsg( "there are " + QString::number( brokenNodes.size() ) + " broken layers" );
    }

    // we let a custom handler decide what to do with missing layers
    // (default implementation ignores them, there's also a GUI handler that lets user choose correct path)
    mBadLayerHandler->handleBadLayers( brokenNodes );
  }

  // Resolve references to other layers
  // Needs to be done here once all dependent layers are loaded
  profile.switchTask( tr( "Resolving layer references" ) );
  QMap<QString, QgsMapLayer *> layers = mLayerStore->mapLayers();
  for ( QMap<QString, QgsMapLayer *>::iterator it = layers.begin(); it != layers.end(); ++it )
  {
    it.value()->resolveReferences( this );
  }

  mLayerTreeRegistryBridge->setEnabled( true );

  // load embedded groups and layers
  profile.switchTask( tr( "Loading embedded layers" ) );
  loadEmbeddedNodes( mRootGroup, flags );

  // now that layers are loaded, we can resolve layer tree's references to the layers
  profile.switchTask( tr( "Resolving references" ) );
  mRootGroup->resolveReferences( this );

  if ( !layerTreeElem.isNull() )
  {
    mRootGroup->readLayerOrderFromXml( layerTreeElem );
  }

  // Load pre 3.0 configuration
  QDomElement layerTreeCanvasElem = doc->documentElement().firstChildElement( QStringLiteral( "layer-tree-canvas" ) );
  if ( !layerTreeCanvasElem.isNull( ) )
  {
    mRootGroup->readLayerOrderFromXml( layerTreeCanvasElem );
  }

  // Convert pre 3.4 to create layers flags
  if ( QgsProjectVersion( 3, 4, 0 ) > mSaveVersion )
  {
    const QStringList requiredLayerIds = readListEntry( QStringLiteral( "RequiredLayers" ), QStringLiteral( "Layers" ) );
    for ( const QString &layerId : requiredLayerIds )
    {
      if ( QgsMapLayer *layer = mapLayer( layerId ) )
      {
        layer->setFlags( layer->flags() & ~QgsMapLayer::Removable );
      }
    }
    const QStringList disabledLayerIds = readListEntry( QStringLiteral( "Identify" ), QStringLiteral( "/disabledLayers" ) );
    for ( const QString &layerId : disabledLayerIds )
    {
      if ( QgsMapLayer *layer = mapLayer( layerId ) )
      {
        layer->setFlags( layer->flags() & ~QgsMapLayer::Identifiable );
      }
    }
  }

  // After bad layer handling we might still have invalid layers,
  // store them in case the user wanted to handle them later
  // or wanted to pass them through when saving
  QgsLayerTreeUtils::storeOriginalLayersProperties( mRootGroup, doc.get() );

  mRootGroup->removeCustomProperty( QStringLiteral( "loading" ) );

  profile.switchTask( tr( "Loading map themes" ) );
  mMapThemeCollection.reset( new QgsMapThemeCollection( this ) );
  emit mapThemeCollectionChanged();
  mMapThemeCollection->readXml( *doc );

  profile.switchTask( tr( "Loading label settings" ) );
  mLabelingEngineSettings->readSettingsFromProject( this );
  emit labelingEngineSettingsChanged();

  profile.switchTask( tr( "Loading annotations" ) );
  mAnnotationManager->readXml( doc->documentElement(), context );
  if ( !( flags & QgsProject::ReadFlag::FlagDontLoadLayouts ) )
  {
    profile.switchTask( tr( "Loading layouts" ) );
    mLayoutManager->readXml( doc->documentElement(), *doc );
  }
  profile.switchTask( tr( "Loading bookmarks" ) );
  mBookmarkManager->readXml( doc->documentElement(), *doc );

  // reassign change dependencies now that all layers are loaded
  QMap<QString, QgsMapLayer *> existingMaps = mapLayers();
  for ( QMap<QString, QgsMapLayer *>::iterator it = existingMaps.begin(); it != existingMaps.end(); ++it )
  {
    it.value()->setDependencies( it.value()->dependencies() );
  }

  profile.switchTask( tr( "Loading snapping settings" ) );
  mSnappingConfig.readProject( *doc );
  mAvoidIntersectionsMode = static_cast<AvoidIntersectionsMode>( readNumEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/AvoidIntersectionsMode" ), static_cast<int>( AvoidIntersectionsMode::AvoidIntersectionsLayers ) ) );

  profile.switchTask( tr( "Loading view settings" ) );
  // restore older project scales settings
  mViewSettings->setUseProjectScales( readBoolEntry( QStringLiteral( "Scales" ), QStringLiteral( "/useProjectScales" ) ) );
  const QStringList scales = readListEntry( QStringLiteral( "Scales" ), QStringLiteral( "/ScalesList" ) );
  QVector<double> res;
  for ( const QString &scale : scales )
  {
    const QStringList parts = scale.split( ':' );
    if ( parts.size() != 2 )
      continue;

    bool ok = false;
    const double denominator = QLocale().toDouble( parts[1], &ok );
    if ( ok )
    {
      res << denominator;
    }
  }
  mViewSettings->setMapScales( res );
  QDomElement viewSettingsElement = doc->documentElement().firstChildElement( QStringLiteral( "ProjectViewSettings" ) );
  if ( !viewSettingsElement.isNull() )
    mViewSettings->readXml( viewSettingsElement, context );

  // restore time settings
  profile.switchTask( tr( "Loading temporal settings" ) );
  QDomElement timeSettingsElement = doc->documentElement().firstChildElement( QStringLiteral( "ProjectTimeSettings" ) );
  if ( !timeSettingsElement.isNull() )
    mTimeSettings->readXml( timeSettingsElement, context );

  profile.switchTask( tr( "Loading display settings" ) );
  QDomElement displaySettingsElement = doc->documentElement().firstChildElement( QStringLiteral( "ProjectDisplaySettings" ) );
  if ( !displaySettingsElement.isNull() )
    mDisplaySettings->readXml( displaySettingsElement, context );

  profile.switchTask( tr( "Updating variables" ) );
  emit customVariablesChanged();
  profile.switchTask( tr( "Updating CRS" ) );
  emit crsChanged();
  emit ellipsoidChanged( ellipsoid() );

  // read the project: used by map canvas and legend
  profile.switchTask( tr( "Reading external settings" ) );
  emit readProject( *doc );
  emit readProjectWithContext( *doc, context );

  profile.switchTask( tr( "Updating interface" ) );
  emit snappingConfigChanged( mSnappingConfig );
  emit avoidIntersectionsModeChanged();
  emit topologicalEditingChanged();
  emit projectColorsChanged();

  // if all went well, we're allegedly in pristine state
  if ( clean )
    setDirty( false );

  QgsDebugMsgLevel( QString( "Project save user: %1" ).arg( mSaveUser ), 2 );
  QgsDebugMsgLevel( QString( "Project save user: %1" ).arg( mSaveUserFull ), 2 );

  Q_NOWARN_DEPRECATED_PUSH
  emit nonIdentifiableLayersChanged( nonIdentifiableLayers() );
  Q_NOWARN_DEPRECATED_POP

  if ( mTranslator )
  {
    //project possibly translated -> rename it with locale postfix
    QString newFileName( QStringLiteral( "%1/%2.qgs" ).arg( QFileInfo( projectFile.fileName() ).absolutePath(), localeFileName ) );
    setFileName( newFileName );

    if ( write() )
    {
      setTitle( localeFileName );
      QgsMessageLog::logMessage( tr( "Translated project saved with locale prefix %1" ).arg( newFileName ), QObject::tr( "Project translation" ), Qgis::Success );
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Error saving translated project with locale prefix %1" ).arg( newFileName ), QObject::tr( "Project translation" ), Qgis::Critical );
    }
  }
  return true;
}


bool QgsProject::loadEmbeddedNodes( QgsLayerTreeGroup *group, QgsProject::ReadFlags flags )
{
  bool valid = true;
  const auto constChildren = group->children();
  for ( QgsLayerTreeNode *child : constChildren )
  {
    if ( QgsLayerTree::isGroup( child ) )
    {
      QgsLayerTreeGroup *childGroup = QgsLayerTree::toGroup( child );
      if ( childGroup->customProperty( QStringLiteral( "embedded" ) ).toInt() )
      {
        // make sure to convert the path from relative to absolute
        QString projectPath = readPath( childGroup->customProperty( QStringLiteral( "embedded_project" ) ).toString() );
        childGroup->setCustomProperty( QStringLiteral( "embedded_project" ), projectPath );
        QgsLayerTreeGroup *newGroup = createEmbeddedGroup( childGroup->name(), projectPath, childGroup->customProperty( QStringLiteral( "embedded-invisible-layers" ) ).toStringList(), flags );
        if ( newGroup )
        {
          QList<QgsLayerTreeNode *> clonedChildren;
          const auto constChildren = newGroup->children();
          for ( QgsLayerTreeNode *newGroupChild : constChildren )
            clonedChildren << newGroupChild->clone();
          delete newGroup;

          childGroup->insertChildNodes( 0, clonedChildren );
        }
      }
      else
      {
        loadEmbeddedNodes( childGroup, flags );
      }
    }
    else if ( QgsLayerTree::isLayer( child ) )
    {
      if ( child->customProperty( QStringLiteral( "embedded" ) ).toInt() )
      {
        QList<QDomNode> brokenNodes;
        if ( ! createEmbeddedLayer( QgsLayerTree::toLayer( child )->layerId(), readPath( child->customProperty( QStringLiteral( "embedded_project" ) ).toString() ), brokenNodes, true, flags ) )
        {
          valid = valid && false;
        }
      }
    }

  }

  return valid;
}

QVariantMap QgsProject::customVariables() const
{
  return mCustomVariables;
}

void QgsProject::setCustomVariables( const QVariantMap &variables )
{
  if ( variables == mCustomVariables )
    return;

  //write variable to project
  QStringList variableNames;
  QStringList variableValues;

  QVariantMap::const_iterator it = variables.constBegin();
  for ( ; it != variables.constEnd(); ++it )
  {
    variableNames << it.key();
    variableValues << it.value().toString();
  }

  writeEntry( QStringLiteral( "Variables" ), QStringLiteral( "/variableNames" ), variableNames );
  writeEntry( QStringLiteral( "Variables" ), QStringLiteral( "/variableValues" ), variableValues );

  mCustomVariables = variables;
  mProjectScope.reset();

  emit customVariablesChanged();
}

void QgsProject::setLabelingEngineSettings( const QgsLabelingEngineSettings &settings )
{
  *mLabelingEngineSettings = settings;
  emit labelingEngineSettingsChanged();
}

const QgsLabelingEngineSettings &QgsProject::labelingEngineSettings() const
{
  return *mLabelingEngineSettings;
}

QgsMapLayerStore *QgsProject::layerStore()
{
  mProjectScope.reset();
  return mLayerStore.get();
}

const QgsMapLayerStore *QgsProject::layerStore() const
{
  return mLayerStore.get();
}

QList<QgsVectorLayer *> QgsProject::avoidIntersectionsLayers() const
{
  QList<QgsVectorLayer *> layers;
  QStringList layerIds = readListEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/AvoidIntersectionsList" ), QStringList() );
  const auto constLayerIds = layerIds;
  for ( const QString &layerId : constLayerIds )
  {
    if ( QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mapLayer( layerId ) ) )
      layers << vlayer;
  }
  return layers;
}

void QgsProject::setAvoidIntersectionsLayers( const QList<QgsVectorLayer *> &layers )
{
  QStringList list;
  const auto constLayers = layers;
  for ( QgsVectorLayer *layer : constLayers )
    list << layer->id();
  writeEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/AvoidIntersectionsList" ), list );
  emit avoidIntersectionsLayersChanged();
}

QgsExpressionContext QgsProject::createExpressionContext() const
{
  QgsExpressionContext context;

  context << QgsExpressionContextUtils::globalScope()
          << QgsExpressionContextUtils::projectScope( this );

  return context;
}

QgsExpressionContextScope *QgsProject::createExpressionContextScope() const
{
  // MUCH cheaper to clone than build
  if ( mProjectScope )
  {
    std::unique_ptr< QgsExpressionContextScope > projectScope = qgis::make_unique< QgsExpressionContextScope >( *mProjectScope );
    // we can't cache these
    projectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_distance_units" ), QgsUnitTypes::toString( distanceUnits() ), true, true ) );
    projectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_area_units" ), QgsUnitTypes::toString( areaUnits() ), true, true ) );
    return projectScope.release();
  }

  mProjectScope = qgis::make_unique< QgsExpressionContextScope >( QObject::tr( "Project" ) );

  const QVariantMap vars = customVariables();

  QVariantMap::const_iterator it = vars.constBegin();

  for ( ; it != vars.constEnd(); ++it )
  {
    mProjectScope->setVariable( it.key(), it.value(), true );
  }

  QString projectPath = projectStorage() ? fileName() : absoluteFilePath();
  if ( projectPath.isEmpty() )
    projectPath = mOriginalPath;
  QString projectFolder = QFileInfo( projectPath ).path();
  QString projectFilename = QFileInfo( projectPath ).fileName();
  QString projectBasename = baseName();

  //add other known project variables
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_title" ), title(), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_path" ), QDir::toNativeSeparators( projectPath ), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_folder" ), QDir::toNativeSeparators( projectFolder ), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_filename" ), projectFilename, true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_basename" ), projectBasename, true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_home" ), QDir::toNativeSeparators( homePath() ), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_last_saved" ), mSaveDateTime.isNull() ? QVariant() : QVariant( mSaveDateTime ), true, true ) );
  QgsCoordinateReferenceSystem projectCrs = crs();
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_crs" ), projectCrs.authid(), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_crs_definition" ), projectCrs.toProj(), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_crs_description" ), projectCrs.description(), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_ellipsoid" ), ellipsoid(), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "_project_transform_context" ), QVariant::fromValue<QgsCoordinateTransformContext>( transformContext() ), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_units" ), QgsUnitTypes::toString( projectCrs.mapUnits() ), true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_crs_acronym" ), projectCrs.projectionAcronym(), true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_crs_ellipsoid" ), projectCrs.ellipsoidAcronym(), true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_crs_proj4" ), projectCrs.toProj(), true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_crs_wkt" ), projectCrs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED ), true ) );

  // metadata
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_author" ), metadata().author(), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_abstract" ), metadata().abstract(), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_creation_date" ), metadata().creationDateTime(), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_identifier" ), metadata().identifier(), true, true ) );

  // keywords
  QVariantMap keywords;
  QgsAbstractMetadataBase::KeywordMap metadataKeywords = metadata().keywords();
  for ( auto it = metadataKeywords.constBegin(); it != metadataKeywords.constEnd(); ++it )
  {
    keywords.insert( it.key(), it.value() );
  }
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_keywords" ), keywords, true, true ) );

  // layers
  QVariantList layersIds;
  QVariantList layers;
  const QMap<QString, QgsMapLayer *> layersInProject = mLayerStore->mapLayers();
  layersIds.reserve( layersInProject.count() );
  layers.reserve( layersInProject.count() );
  for ( auto it = layersInProject.constBegin(); it != layersInProject.constEnd(); ++it )
  {
    layersIds << it.value()->id();
    layers << QVariant::fromValue<QgsWeakMapLayerPointer>( QgsWeakMapLayerPointer( it.value() ) );
  }
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "layer_ids" ), layersIds, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "layers" ), layers, true ) );

  mProjectScope->addFunction( QStringLiteral( "project_color" ), new GetNamedProjectColor( this ) );

  return createExpressionContextScope();
}

void QgsProject::onMapLayersAdded( const QList<QgsMapLayer *> &layers )
{
  QMap<QString, QgsMapLayer *> existingMaps = mapLayers();

  bool tgChanged = false;

  const auto constLayers = layers;
  for ( QgsMapLayer *layer : constLayers )
  {
    if ( layer->isValid() )
    {
      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
      if ( vlayer )
      {
        if ( autoTransaction() )
        {
          if ( QgsTransaction::supportsTransaction( vlayer ) )
          {
            const QString connString = QgsTransaction::connectionString( vlayer->source() );
            const QString key = vlayer->providerType();

            QgsTransactionGroup *tg = mTransactionGroups.value( qMakePair( key, connString ) );

            if ( !tg )
            {
              tg = new QgsTransactionGroup();
              mTransactionGroups.insert( qMakePair( key, connString ), tg );
              tgChanged = true;
            }
            tg->addLayer( vlayer );
          }
        }
        vlayer->dataProvider()->setProviderProperty( QgsVectorDataProvider::EvaluateDefaultValues, evaluateDefaultValues() );
      }

      if ( tgChanged )
        emit transactionGroupsChanged();

      connect( layer, &QgsMapLayer::configChanged, this, [ = ] { setDirty(); } );

      // check if we have to update connections for layers with dependencies
      for ( QMap<QString, QgsMapLayer *>::iterator it = existingMaps.begin(); it != existingMaps.end(); ++it )
      {
        QSet<QgsMapLayerDependency> deps = it.value()->dependencies();
        if ( deps.contains( layer->id() ) )
        {
          // reconnect to change signals
          it.value()->setDependencies( deps );
        }
      }
    }
  }

  if ( mSnappingConfig.addLayers( layers ) )
    emit snappingConfigChanged( mSnappingConfig );
}

void QgsProject::onMapLayersRemoved( const QList<QgsMapLayer *> &layers )
{
  if ( mSnappingConfig.removeLayers( layers ) )
    emit snappingConfigChanged( mSnappingConfig );
}

void QgsProject::cleanTransactionGroups( bool force )
{
  bool changed = false;
  for ( QMap< QPair< QString, QString>, QgsTransactionGroup *>::Iterator tg = mTransactionGroups.begin(); tg != mTransactionGroups.end(); )
  {
    if ( tg.value()->isEmpty() || force )
    {
      delete tg.value();
      tg = mTransactionGroups.erase( tg );
      changed = true;
    }
    else
    {
      ++tg;
    }
  }
  if ( changed )
    emit transactionGroupsChanged();
}

bool QgsProject::readLayer( const QDomNode &layerNode )
{
  QgsReadWriteContext context;
  context.setPathResolver( pathResolver() );
  context.setProjectTranslator( this );
  context.setTransformContext( transformContext() );
  QList<QDomNode> brokenNodes;
  if ( addLayer( layerNode.toElement(), brokenNodes, context ) )
  {
    // have to try to update joins for all layers now - a previously added layer may be dependent on this newly
    // added layer for joins
    QVector<QgsVectorLayer *> vectorLayers = layers<QgsVectorLayer *>();
    const auto constVectorLayers = vectorLayers;
    for ( QgsVectorLayer *layer : constVectorLayers )
    {
      // TODO: should be only done later - and with all layers (other layers may have referenced this layer)
      layer->resolveReferences( this );
    }

    return true;
  }
  return false;
}

bool QgsProject::write( const QString &filename )
{
  mFile.setFileName( filename );
  mCachedHomePath.clear();
  return write();
}

bool QgsProject::write()
{
  mProjectScope.reset();
  if ( QgsProjectStorage *storage = projectStorage() )
  {
    QgsReadWriteContext context;
    // for projects stored in a custom storage, we have to check for the support
    // of relative paths since the storage most likely will not be in a file system
    QString storageFilePath { storage->filePath( mFile.fileName() ) };
    if ( storageFilePath.isEmpty() )
    {
      writeEntry( QStringLiteral( "Paths" ), QStringLiteral( "/Absolute" ), true );
    }
    context.setPathResolver( pathResolver() );

    QString tempPath = QStandardPaths::standardLocations( QStandardPaths::TempLocation ).at( 0 );
    QString tmpZipFilename( tempPath + QDir::separator() + QUuid::createUuid().toString() );

    if ( !zip( tmpZipFilename ) )
      return false;  // zip() already calls setError() when returning false

    QFile tmpZipFile( tmpZipFilename );
    if ( !tmpZipFile.open( QIODevice::ReadOnly ) )
    {
      setError( tr( "Unable to read file %1" ).arg( tmpZipFilename ) );
      return false;
    }

    context.setTransformContext( transformContext() );
    if ( !storage->writeProject( mFile.fileName(), &tmpZipFile, context ) )
    {
      QString err = tr( "Unable to save project to storage %1" ).arg( mFile.fileName() );
      QList<QgsReadWriteContext::ReadWriteMessage> messages = context.takeMessages();
      if ( !messages.isEmpty() )
        err += QStringLiteral( "\n\n" ) + messages.last().message();
      setError( err );
      return false;
    }

    tmpZipFile.close();
    QFile::remove( tmpZipFilename );

    return true;
  }

  if ( QgsZipUtils::isZipFile( mFile.fileName() ) )
  {
    return zip( mFile.fileName() );
  }
  else
  {
    // write project file even if the auxiliary storage is not correctly
    // saved
    const bool asOk = saveAuxiliaryStorage();
    const bool writeOk = writeProjectFile( mFile.fileName() );

    // errors raised during writing project file are more important
    if ( !asOk && writeOk )
    {
      const QString err = mAuxiliaryStorage->errorString();
      setError( tr( "Unable to save auxiliary storage ('%1')" ).arg( err ) );
    }

    return asOk && writeOk;
  }
}

bool QgsProject::writeProjectFile( const QString &filename )
{
  QFile projectFile( filename );
  clearError();

  // if we have problems creating or otherwise writing to the project file,
  // let's find out up front before we go through all the hand-waving
  // necessary to create all the Dom objects
  QFileInfo myFileInfo( projectFile );
  if ( myFileInfo.exists() && !myFileInfo.isWritable() )
  {
    setError( tr( "%1 is not writable. Please adjust permissions (if possible) and try again." )
              .arg( projectFile.fileName() ) );
    return false;
  }

  QgsReadWriteContext context;
  context.setPathResolver( pathResolver() );
  context.setTransformContext( transformContext() );

  QDomImplementation DomImplementation;
  DomImplementation.setInvalidDataPolicy( QDomImplementation::DropInvalidChars );

  QDomDocumentType documentType =
    DomImplementation.createDocumentType( QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ),
                                          QStringLiteral( "SYSTEM" ) );
  std::unique_ptr<QDomDocument> doc( new QDomDocument( documentType ) );

  QDomElement qgisNode = doc->createElement( QStringLiteral( "qgis" ) );
  qgisNode.setAttribute( QStringLiteral( "projectname" ), title() );
  qgisNode.setAttribute( QStringLiteral( "version" ), QStringLiteral( "%1" ).arg( Qgis::version() ) );

  QgsSettings settings;
  if ( !settings.value( QStringLiteral( "projects/anonymize_saved_projects" ), false, QgsSettings::Core ).toBool() )
  {
    QString newSaveUser = QgsApplication::userLoginName();
    QString newSaveUserFull = QgsApplication::userFullName();
    qgisNode.setAttribute( QStringLiteral( "saveUser" ), newSaveUser );
    qgisNode.setAttribute( QStringLiteral( "saveUserFull" ), newSaveUserFull );
    mSaveUser = newSaveUser;
    mSaveUserFull = newSaveUserFull;
    mSaveDateTime = QDateTime::currentDateTime();
    qgisNode.setAttribute( QStringLiteral( "saveDateTime" ), mSaveDateTime.toString( Qt::ISODate ) );
  }
  else
  {
    mSaveUser.clear();
    mSaveUserFull.clear();
    mSaveDateTime = QDateTime();
  }
  doc->appendChild( qgisNode );
  mSaveVersion = QgsProjectVersion( Qgis::version() );

  QDomElement homePathNode = doc->createElement( QStringLiteral( "homePath" ) );
  homePathNode.setAttribute( QStringLiteral( "path" ), mHomePath );
  qgisNode.appendChild( homePathNode );

  // title
  QDomElement titleNode = doc->createElement( QStringLiteral( "title" ) );
  qgisNode.appendChild( titleNode );

  QDomElement transactionNode = doc->createElement( QStringLiteral( "autotransaction" ) );
  transactionNode.setAttribute( QStringLiteral( "active" ), mAutoTransaction ? 1 : 0 );
  qgisNode.appendChild( transactionNode );

  QDomElement evaluateDefaultValuesNode = doc->createElement( QStringLiteral( "evaluateDefaultValues" ) );
  evaluateDefaultValuesNode.setAttribute( QStringLiteral( "active" ), mEvaluateDefaultValues ? 1 : 0 );
  qgisNode.appendChild( evaluateDefaultValuesNode );

  QDomElement trustNode = doc->createElement( QStringLiteral( "trust" ) );
  trustNode.setAttribute( QStringLiteral( "active" ), mTrustLayerMetadata ? 1 : 0 );
  qgisNode.appendChild( trustNode );

  QDomText titleText = doc->createTextNode( title() );  // XXX why have title TWICE?
  titleNode.appendChild( titleText );

  // write project CRS
  QDomElement srsNode = doc->createElement( QStringLiteral( "projectCrs" ) );
  mCrs.writeXml( srsNode, *doc );
  qgisNode.appendChild( srsNode );

  // write layer tree - make sure it is without embedded subgroups
  QgsLayerTreeNode *clonedRoot = mRootGroup->clone();
  QgsLayerTreeUtils::replaceChildrenOfEmbeddedGroups( QgsLayerTree::toGroup( clonedRoot ) );
  QgsLayerTreeUtils::updateEmbeddedGroupsProjectPath( QgsLayerTree::toGroup( clonedRoot ), this ); // convert absolute paths to relative paths if required

  clonedRoot->writeXml( qgisNode, context );
  delete clonedRoot;

  mSnappingConfig.writeProject( *doc );
  writeEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/AvoidIntersectionsMode" ), static_cast<int>( mAvoidIntersectionsMode ) );

  // let map canvas and legend write their information
  emit writeProject( *doc );

  // within top level node save list of layers
  const QMap<QString, QgsMapLayer *> &layers = mapLayers();

  // Iterate over layers in zOrder
  // Call writeXml() on each
  QDomElement projectLayersNode = doc->createElement( QStringLiteral( "projectlayers" ) );

  QMap<QString, QgsMapLayer *>::ConstIterator li = layers.constBegin();
  while ( li != layers.end() )
  {
    QgsMapLayer *ml = li.value();

    if ( ml )
    {
      QHash< QString, QPair< QString, bool> >::const_iterator emIt = mEmbeddedLayers.constFind( ml->id() );
      if ( emIt == mEmbeddedLayers.constEnd() )
      {
        QDomElement maplayerElem;
        // If layer is not valid, prefer to restore saved properties from invalidLayerProperties. But if that's
        // not available, just write what we DO have
        if ( ml->isValid() || ml->originalXmlProperties().isEmpty() )
        {
          // general layer metadata
          maplayerElem = doc->createElement( QStringLiteral( "maplayer" ) );
          ml->writeLayerXml( maplayerElem, *doc, context );
        }
        else if ( ! ml->originalXmlProperties().isEmpty() )
        {
          QDomDocument document;
          if ( document.setContent( ml->originalXmlProperties() ) )
          {
            maplayerElem = document.firstChildElement();
          }
          else
          {
            QgsDebugMsg( QStringLiteral( "Could not restore layer properties for layer %1" ).arg( ml->id() ) );
          }
        }

        emit writeMapLayer( ml, maplayerElem, *doc );

        projectLayersNode.appendChild( maplayerElem );
      }
      else
      {
        // layer defined in an external project file
        // only save embedded layer if not managed by a legend group
        if ( emIt.value().second )
        {
          QDomElement mapLayerElem = doc->createElement( QStringLiteral( "maplayer" ) );
          mapLayerElem.setAttribute( QStringLiteral( "embedded" ), 1 );
          mapLayerElem.setAttribute( QStringLiteral( "project" ), writePath( emIt.value().first ) );
          mapLayerElem.setAttribute( QStringLiteral( "id" ), ml->id() );
          projectLayersNode.appendChild( mapLayerElem );
        }
      }
    }
    li++;
  }

  qgisNode.appendChild( projectLayersNode );

  QDomElement layerOrderNode = doc->createElement( QStringLiteral( "layerorder" ) );
  const auto constCustomLayerOrder = mRootGroup->customLayerOrder();
  for ( QgsMapLayer *layer : constCustomLayerOrder )
  {
    QDomElement mapLayerElem = doc->createElement( QStringLiteral( "layer" ) );
    mapLayerElem.setAttribute( QStringLiteral( "id" ), layer->id() );
    layerOrderNode.appendChild( mapLayerElem );
  }
  qgisNode.appendChild( layerOrderNode );

  mLabelingEngineSettings->writeSettingsToProject( this );

  writeEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorRedPart" ), mBackgroundColor.red() );
  writeEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorGreenPart" ), mBackgroundColor.green() );
  writeEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorBluePart" ), mBackgroundColor.blue() );

  writeEntry( QStringLiteral( "Gui" ), QStringLiteral( "/SelectionColorRedPart" ), mSelectionColor.red() );
  writeEntry( QStringLiteral( "Gui" ), QStringLiteral( "/SelectionColorGreenPart" ), mSelectionColor.green() );
  writeEntry( QStringLiteral( "Gui" ), QStringLiteral( "/SelectionColorBluePart" ), mSelectionColor.blue() );
  writeEntry( QStringLiteral( "Gui" ), QStringLiteral( "/SelectionColorAlphaPart" ), mSelectionColor.alpha() );

  // now add the optional extra properties
#if 0
  dump_( mProperties );
#endif

  QgsDebugMsgLevel( QStringLiteral( "there are %1 property scopes" ).arg( static_cast<int>( mProperties.count() ) ), 2 );

  if ( !mProperties.isEmpty() ) // only worry about properties if we
    // actually have any properties
  {
    mProperties.writeXml( QStringLiteral( "properties" ), qgisNode, *doc );
  }

  QDomElement ddElem = doc->createElement( QStringLiteral( "dataDefinedServerProperties" ) );
  mDataDefinedServerProperties.writeXml( ddElem, dataDefinedServerPropertyDefinitions() );
  qgisNode.appendChild( ddElem );

  mMapThemeCollection->writeXml( *doc );

  mTransformContext.writeXml( qgisNode, context );

  QDomElement metadataElem = doc->createElement( QStringLiteral( "projectMetadata" ) );
  mMetadata.writeMetadataXml( metadataElem, *doc );
  qgisNode.appendChild( metadataElem );

  QDomElement annotationsElem = mAnnotationManager->writeXml( *doc, context );
  qgisNode.appendChild( annotationsElem );

  QDomElement layoutElem = mLayoutManager->writeXml( *doc );
  qgisNode.appendChild( layoutElem );

  QDomElement bookmarkElem = mBookmarkManager->writeXml( *doc );
  qgisNode.appendChild( bookmarkElem );

  QDomElement viewSettingsElem = mViewSettings->writeXml( *doc, context );
  qgisNode.appendChild( viewSettingsElem );

  QDomElement timeSettingsElement = mTimeSettings->writeXml( *doc, context );
  qgisNode.appendChild( timeSettingsElement );

  QDomElement displaySettingsElem = mDisplaySettings->writeXml( *doc, context );
  qgisNode.appendChild( displaySettingsElem );

  // now wrap it up and ship it to the project file
  doc->normalize();             // XXX I'm not entirely sure what this does

  // Create backup file
  if ( QFile::exists( fileName() ) )
  {
    QFile backupFile( QStringLiteral( "%1~" ).arg( filename ) );
    bool ok = true;
    ok &= backupFile.open( QIODevice::WriteOnly | QIODevice::Truncate );
    ok &= projectFile.open( QIODevice::ReadOnly );

    QByteArray ba;
    while ( ok && !projectFile.atEnd() )
    {
      ba = projectFile.read( 10240 );
      ok &= backupFile.write( ba ) == ba.size();
    }

    projectFile.close();
    backupFile.close();

    if ( !ok )
    {
      setError( tr( "Unable to create backup file %1" ).arg( backupFile.fileName() ) );
      return false;
    }

    QFileInfo fi( fileName() );
    struct utimbuf tb = { static_cast<time_t>( fi.lastRead().toSecsSinceEpoch() ), static_cast<time_t>( fi.lastModified().toSecsSinceEpoch() ) };
    utime( backupFile.fileName().toUtf8().constData(), &tb );
  }

  if ( !projectFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    projectFile.close();         // even though we got an error, let's make
    // sure it's closed anyway

    setError( tr( "Unable to save to file %1" ).arg( projectFile.fileName() ) );
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
      ok &= projectFile.write( ba ) == ba.size();
    }

    ok &= projectFile.error() == QFile::NoError;

    projectFile.close();
  }

  tempFile.close();

  if ( !ok )
  {
    setError( tr( "Unable to save to file %1. Your project "
                  "may be corrupted on disk. Try clearing some space on the volume and "
                  "check file permissions before pressing save again." )
              .arg( projectFile.fileName() ) );
    return false;
  }

  setDirty( false );               // reset to pristine state

  emit projectSaved();
  return true;
}

bool QgsProject::writeEntry( const QString &scope, QString const &key, bool value )
{
  bool propertiesModified;
  bool success = addKey_( scope, key, &mProperties, value, propertiesModified );

  if ( propertiesModified )
    setDirty( true );

  return success;
}

bool QgsProject::writeEntry( const QString &scope, const QString &key, double value )
{
  bool propertiesModified;
  bool success = addKey_( scope, key, &mProperties, value, propertiesModified );

  if ( propertiesModified )
    setDirty( true );

  return success;
}

bool QgsProject::writeEntry( const QString &scope, QString const &key, int value )
{
  bool propertiesModified;
  bool success = addKey_( scope, key, &mProperties, value, propertiesModified );

  if ( propertiesModified )
    setDirty( true );

  return success;
}

bool QgsProject::writeEntry( const QString &scope, const QString &key, const QString &value )
{
  bool propertiesModified;
  bool success = addKey_( scope, key, &mProperties, value, propertiesModified );

  if ( propertiesModified )
    setDirty( true );

  return success;
}

bool QgsProject::writeEntry( const QString &scope, const QString &key, const QStringList &value )
{
  bool propertiesModified;
  bool success = addKey_( scope, key, &mProperties, value, propertiesModified );

  if ( propertiesModified )
    setDirty( true );

  return success;
}

QStringList QgsProject::readListEntry( const QString &scope,
                                       const QString &key,
                                       const QStringList &def,
                                       bool *ok ) const
{
  QgsProjectProperty *property = findKey_( scope, key, mProperties );

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


QString QgsProject::readEntry( const QString &scope,
                               const QString &key,
                               const QString &def,
                               bool *ok ) const
{
  QgsProjectProperty *property = findKey_( scope, key, mProperties );

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

int QgsProject::readNumEntry( const QString &scope, const QString &key, int def,
                              bool *ok ) const
{
  QgsProjectProperty *property = findKey_( scope, key, mProperties );

  QVariant value;

  if ( property )
  {
    value = property->value();
  }

  bool valid = value.canConvert( QVariant::Int );

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

double QgsProject::readDoubleEntry( const QString &scope, const QString &key,
                                    double def,
                                    bool *ok ) const
{
  QgsProjectProperty *property = findKey_( scope, key, mProperties );
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

bool QgsProject::readBoolEntry( const QString &scope, const QString &key, bool def,
                                bool *ok ) const
{
  QgsProjectProperty *property = findKey_( scope, key, mProperties );

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

bool QgsProject::removeEntry( const QString &scope, const QString &key )
{
  if ( findKey_( scope, key, mProperties ) )
  {
    removeKey_( scope, key, mProperties );
    setDirty( true );
  }

  return !findKey_( scope, key, mProperties );
}


QStringList QgsProject::entryList( const QString &scope, const QString &key ) const
{
  QgsProjectProperty *foundProperty = findKey_( scope, key, mProperties );

  QStringList entries;

  if ( foundProperty )
  {
    QgsProjectPropertyKey *propertyKey = dynamic_cast<QgsProjectPropertyKey *>( foundProperty );

    if ( propertyKey )
    { propertyKey->entryList( entries ); }
  }

  return entries;
}

QStringList QgsProject::subkeyList( const QString &scope, const QString &key ) const
{
  QgsProjectProperty *foundProperty = findKey_( scope, key, mProperties );

  QStringList entries;

  if ( foundProperty )
  {
    QgsProjectPropertyKey *propertyKey = dynamic_cast<QgsProjectPropertyKey *>( foundProperty );

    if ( propertyKey )
    { propertyKey->subkeyList( entries ); }
  }

  return entries;
}

void QgsProject::dumpProperties() const
{
  dump_( mProperties );
}

QgsPathResolver QgsProject::pathResolver() const
{
  bool absolutePaths = readBoolEntry( QStringLiteral( "Paths" ), QStringLiteral( "/Absolute" ), false );
  QString filePath;
  if ( ! absolutePaths )
  {
    // for projects stored in a custom storage, we need to ask to the
    // storage for the path, if the storage returns an empty path
    // relative paths are not supported
    if ( QgsProjectStorage *storage = projectStorage() )
    {
      filePath = storage->filePath( mFile.fileName() );
    }
    else
    {
      filePath = fileName();
    }
  }
  return QgsPathResolver( filePath );
}

QString QgsProject::readPath( const QString &src ) const
{
  return pathResolver().readPath( src );
}

QString QgsProject::writePath( const QString &src ) const
{
  return pathResolver().writePath( src );
}

void QgsProject::setError( const QString &errorMessage )
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
                                      bool saveFlag, QgsProject::ReadFlags flags )
{
  QgsDebugCall;

  static QString sPrevProjectFilePath;
  static QDateTime sPrevProjectFileTimestamp;
  static QDomDocument sProjectDocument;

  QString qgsProjectFile = projectFilePath;
  QgsProjectArchive archive;
  if ( projectFilePath.endsWith( QLatin1String( ".qgz" ), Qt::CaseInsensitive ) )
  {
    archive.unzip( projectFilePath );
    qgsProjectFile = archive.projectFile();
  }

  QDateTime projectFileTimestamp = QFileInfo( projectFilePath ).lastModified();

  if ( projectFilePath != sPrevProjectFilePath || projectFileTimestamp != sPrevProjectFileTimestamp )
  {
    sPrevProjectFilePath.clear();

    QFile projectFile( qgsProjectFile );
    if ( !projectFile.open( QIODevice::ReadOnly ) )
    {
      return false;
    }

    if ( !sProjectDocument.setContent( &projectFile ) )
    {
      return false;
    }

    sPrevProjectFilePath = projectFilePath;
    sPrevProjectFileTimestamp = projectFileTimestamp;
  }

  // does project store paths absolute or relative?
  bool useAbsolutePaths = true;

  QDomElement propertiesElem = sProjectDocument.documentElement().firstChildElement( QStringLiteral( "properties" ) );
  if ( !propertiesElem.isNull() )
  {
    QDomElement absElem = propertiesElem.firstChildElement( QStringLiteral( "Paths" ) ).firstChildElement( QStringLiteral( "Absolute" ) );
    if ( !absElem.isNull() )
    {
      useAbsolutePaths = absElem.text().compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0;
    }
  }

  QgsReadWriteContext embeddedContext;
  if ( !useAbsolutePaths )
    embeddedContext.setPathResolver( QgsPathResolver( projectFilePath ) );
  embeddedContext.setProjectTranslator( this );
  embeddedContext.setTransformContext( transformContext() );

  QDomElement projectLayersElem = sProjectDocument.documentElement().firstChildElement( QStringLiteral( "projectlayers" ) );
  if ( projectLayersElem.isNull() )
  {
    return false;
  }

  QDomNodeList mapLayerNodes = projectLayersElem.elementsByTagName( QStringLiteral( "maplayer" ) );
  for ( int i = 0; i < mapLayerNodes.size(); ++i )
  {
    // get layer id
    QDomElement mapLayerElem = mapLayerNodes.at( i ).toElement();
    QString id = mapLayerElem.firstChildElement( QStringLiteral( "id" ) ).text();
    if ( id == layerId )
    {
      // layer can be embedded only once
      if ( mapLayerElem.attribute( QStringLiteral( "embedded" ) ) == QLatin1String( "1" ) )
      {
        return false;
      }

      mEmbeddedLayers.insert( layerId, qMakePair( projectFilePath, saveFlag ) );

      if ( addLayer( mapLayerElem, brokenNodes, embeddedContext, flags ) )
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


QgsLayerTreeGroup *QgsProject::createEmbeddedGroup( const QString &groupName, const QString &projectFilePath, const QStringList &invisibleLayers, QgsProject::ReadFlags flags )
{
  QString qgsProjectFile = projectFilePath;
  QgsProjectArchive archive;
  if ( projectFilePath.endsWith( QLatin1String( ".qgz" ), Qt::CaseInsensitive ) )
  {
    archive.unzip( projectFilePath );
    qgsProjectFile = archive.projectFile();
  }

  // open project file, get layer ids in group, add the layers
  QFile projectFile( qgsProjectFile );
  if ( !projectFile.open( QIODevice::ReadOnly ) )
  {
    return nullptr;
  }

  QDomDocument projectDocument;
  if ( !projectDocument.setContent( &projectFile ) )
  {
    return nullptr;
  }

  QgsReadWriteContext context;
  context.setPathResolver( pathResolver() );
  context.setProjectTranslator( this );
  context.setTransformContext( transformContext() );

  QgsLayerTreeGroup *root = new QgsLayerTreeGroup;

  QDomElement layerTreeElem = projectDocument.documentElement().firstChildElement( QStringLiteral( "layer-tree-group" ) );
  if ( !layerTreeElem.isNull() )
  {
    root->readChildrenFromXml( layerTreeElem, context );
  }
  else
  {
    QgsLayerTreeUtils::readOldLegend( root, projectDocument.documentElement().firstChildElement( QStringLiteral( "legend" ) ) );
  }

  QgsLayerTreeGroup *group = root->findGroup( groupName );
  if ( !group || group->customProperty( QStringLiteral( "embedded" ) ).toBool() )
  {
    // embedded groups cannot be embedded again
    delete root;
    return nullptr;
  }

  // clone the group sub-tree (it is used already in a tree, we cannot just tear it off)
  QgsLayerTreeGroup *newGroup = QgsLayerTree::toGroup( group->clone() );
  delete root;
  root = nullptr;

  newGroup->setCustomProperty( QStringLiteral( "embedded" ), 1 );
  newGroup->setCustomProperty( QStringLiteral( "embedded_project" ), projectFilePath );

  // set "embedded" to all children + load embedded layers
  mLayerTreeRegistryBridge->setEnabled( false );
  initializeEmbeddedSubtree( projectFilePath, newGroup, flags );
  mLayerTreeRegistryBridge->setEnabled( true );

  // consider the layers might be identify disabled in its project
  const auto constFindLayerIds = newGroup->findLayerIds();
  for ( const QString &layerId : constFindLayerIds )
  {
    QgsLayerTreeLayer *layer = newGroup->findLayer( layerId );
    if ( layer )
    {
      layer->resolveReferences( this );
      layer->setItemVisibilityChecked( !invisibleLayers.contains( layerId ) );
    }
  }

  return newGroup;
}

void QgsProject::initializeEmbeddedSubtree( const QString &projectFilePath, QgsLayerTreeGroup *group, QgsProject::ReadFlags flags )
{
  const auto constChildren = group->children();
  for ( QgsLayerTreeNode *child : constChildren )
  {
    // all nodes in the subtree will have "embedded" custom property set
    child->setCustomProperty( QStringLiteral( "embedded" ), 1 );

    if ( QgsLayerTree::isGroup( child ) )
    {
      initializeEmbeddedSubtree( projectFilePath, QgsLayerTree::toGroup( child ), flags );
    }
    else if ( QgsLayerTree::isLayer( child ) )
    {
      // load the layer into our project
      QList<QDomNode> brokenNodes;
      createEmbeddedLayer( QgsLayerTree::toLayer( child )->layerId(), projectFilePath, brokenNodes, false, flags );
    }
  }
}

bool QgsProject::evaluateDefaultValues() const
{
  return mEvaluateDefaultValues;
}

void QgsProject::setEvaluateDefaultValues( bool evaluateDefaultValues )
{
  if ( evaluateDefaultValues == mEvaluateDefaultValues )
    return;

  const QMap<QString, QgsMapLayer *> layers = mapLayers();
  QMap<QString, QgsMapLayer *>::const_iterator layerIt = layers.constBegin();
  for ( ; layerIt != layers.constEnd(); ++layerIt )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layerIt.value() );
    if ( vl )
    {
      vl->dataProvider()->setProviderProperty( QgsVectorDataProvider::EvaluateDefaultValues, evaluateDefaultValues );
    }
  }

  mEvaluateDefaultValues = evaluateDefaultValues;
}

void QgsProject::setTopologicalEditing( bool enabled )
{
  writeEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/TopologicalEditing" ), ( enabled ? 1 : 0 ) );
  emit topologicalEditingChanged();
}

bool QgsProject::topologicalEditing() const
{
  return readNumEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/TopologicalEditing" ), 0 );
}

QgsUnitTypes::DistanceUnit QgsProject::distanceUnits() const
{
  QString distanceUnitString = readEntry( QStringLiteral( "Measurement" ), QStringLiteral( "/DistanceUnits" ), QString() );
  if ( !distanceUnitString.isEmpty() )
    return QgsUnitTypes::decodeDistanceUnit( distanceUnitString );

  //fallback to QGIS default measurement unit
  QgsSettings s;
  bool ok = false;
  QgsUnitTypes::DistanceUnit type = QgsUnitTypes::decodeDistanceUnit( s.value( QStringLiteral( "/qgis/measure/displayunits" ) ).toString(), &ok );
  return ok ? type : QgsUnitTypes::DistanceMeters;
}

void QgsProject::setDistanceUnits( QgsUnitTypes::DistanceUnit unit )
{
  writeEntry( QStringLiteral( "Measurement" ), QStringLiteral( "/DistanceUnits" ), QgsUnitTypes::encodeUnit( unit ) );
}

QgsUnitTypes::AreaUnit QgsProject::areaUnits() const
{
  QString areaUnitString = readEntry( QStringLiteral( "Measurement" ), QStringLiteral( "/AreaUnits" ), QString() );
  if ( !areaUnitString.isEmpty() )
    return QgsUnitTypes::decodeAreaUnit( areaUnitString );

  //fallback to QGIS default area unit
  QgsSettings s;
  bool ok = false;
  QgsUnitTypes::AreaUnit type = QgsUnitTypes::decodeAreaUnit( s.value( QStringLiteral( "/qgis/measure/areaunits" ) ).toString(), &ok );
  return ok ? type : QgsUnitTypes::AreaSquareMeters;
}

void QgsProject::setAreaUnits( QgsUnitTypes::AreaUnit unit )
{
  writeEntry( QStringLiteral( "Measurement" ), QStringLiteral( "/AreaUnits" ), QgsUnitTypes::encodeUnit( unit ) );
}

QString QgsProject::homePath() const
{
  if ( !mCachedHomePath.isEmpty() )
    return mCachedHomePath;

  QFileInfo pfi( fileName() );

  if ( !mHomePath.isEmpty() )
  {
    QFileInfo homeInfo( mHomePath );
    if ( !homeInfo.isRelative() )
    {
      mCachedHomePath = mHomePath;
      return mHomePath;
    }
  }
  else if ( !fileName().isEmpty() )
  {
    mCachedHomePath = pfi.path();

    return mCachedHomePath;
  }

  if ( !pfi.exists() )
  {
    mCachedHomePath = mHomePath;
    return mHomePath;
  }

  if ( !mHomePath.isEmpty() )
  {
    // path is relative to project file
    mCachedHomePath = QDir::cleanPath( pfi.path() + '/' + mHomePath );
  }
  else
  {
    mCachedHomePath = pfi.canonicalPath();
  }
  return mCachedHomePath;
}

QString QgsProject::presetHomePath() const
{
  return mHomePath;
}

QgsRelationManager *QgsProject::relationManager() const
{
  return mRelationManager;
}

const QgsLayoutManager *QgsProject::layoutManager() const
{
  return mLayoutManager.get();
}

QgsLayoutManager *QgsProject::layoutManager()
{
  return mLayoutManager.get();
}

const QgsBookmarkManager *QgsProject::bookmarkManager() const
{
  return mBookmarkManager;
}

QgsBookmarkManager *QgsProject::bookmarkManager()
{
  return mBookmarkManager;
}

const QgsProjectViewSettings *QgsProject::viewSettings() const
{
  return mViewSettings;
}

QgsProjectViewSettings *QgsProject::viewSettings()
{
  return mViewSettings;
}

const QgsProjectTimeSettings *QgsProject::timeSettings() const
{
  return mTimeSettings;
}

QgsProjectTimeSettings *QgsProject::timeSettings()
{
  return mTimeSettings;
}

const QgsProjectDisplaySettings *QgsProject::displaySettings() const
{
  return mDisplaySettings;
}

QgsProjectDisplaySettings *QgsProject::displaySettings()
{
  return mDisplaySettings;
}

QgsLayerTree *QgsProject::layerTreeRoot() const
{
  return mRootGroup;
}

QgsMapThemeCollection *QgsProject::mapThemeCollection()
{
  return mMapThemeCollection.get();
}

QgsAnnotationManager *QgsProject::annotationManager()
{
  return mAnnotationManager.get();
}

const QgsAnnotationManager *QgsProject::annotationManager() const
{
  return mAnnotationManager.get();
}

void QgsProject::setNonIdentifiableLayers( const QList<QgsMapLayer *> &layers )
{
  const QMap<QString, QgsMapLayer *> &projectLayers = mapLayers();
  for ( QMap<QString, QgsMapLayer *>::const_iterator it = projectLayers.constBegin(); it != projectLayers.constEnd(); ++it )
  {
    if ( layers.contains( it.value() ) == !it.value()->flags().testFlag( QgsMapLayer::Identifiable ) )
      continue;

    if ( layers.contains( it.value() ) )
      it.value()->setFlags( it.value()->flags() & ~QgsMapLayer::Identifiable );
    else
      it.value()->setFlags( it.value()->flags() | QgsMapLayer::Identifiable );
  }

  Q_NOWARN_DEPRECATED_PUSH
  emit nonIdentifiableLayersChanged( nonIdentifiableLayers() );
  Q_NOWARN_DEPRECATED_POP
}

void QgsProject::setNonIdentifiableLayers( const QStringList &layerIds )
{
  QList<QgsMapLayer *> nonIdentifiableLayers;
  nonIdentifiableLayers.reserve( layerIds.count() );
  for ( const QString &layerId : layerIds )
  {
    QgsMapLayer *layer = mapLayer( layerId );
    if ( layer )
      nonIdentifiableLayers << layer;
  }
  Q_NOWARN_DEPRECATED_PUSH
  setNonIdentifiableLayers( nonIdentifiableLayers );
  Q_NOWARN_DEPRECATED_POP
}

QStringList QgsProject::nonIdentifiableLayers() const
{
  QStringList nonIdentifiableLayers;

  const QMap<QString, QgsMapLayer *> &layers = mapLayers();
  for ( QMap<QString, QgsMapLayer *>::const_iterator it = layers.constBegin(); it != layers.constEnd(); ++it )
  {
    if ( !it.value()->flags().testFlag( QgsMapLayer::Identifiable ) )
    {
      nonIdentifiableLayers.append( it.value()->id() );
    }
  }
  return nonIdentifiableLayers;
}

bool QgsProject::autoTransaction() const
{
  return mAutoTransaction;
}

void QgsProject::setAutoTransaction( bool autoTransaction )
{
  if ( autoTransaction != mAutoTransaction )
  {
    mAutoTransaction = autoTransaction;

    if ( autoTransaction )
      onMapLayersAdded( mapLayers().values() );
    else
      cleanTransactionGroups( true );
  }
}

QMap<QPair<QString, QString>, QgsTransactionGroup *> QgsProject::transactionGroups()
{
  return mTransactionGroups;
}


//
// QgsMapLayerStore methods
//


int QgsProject::count() const
{
  return mLayerStore->count();
}

int QgsProject::validCount() const
{
  return mLayerStore->validCount();
}

QgsMapLayer *QgsProject::mapLayer( const QString &layerId ) const
{
  return mLayerStore->mapLayer( layerId );
}

QList<QgsMapLayer *> QgsProject::mapLayersByName( const QString &layerName ) const
{
  return mLayerStore->mapLayersByName( layerName );
}

QList<QgsMapLayer *> QgsProject::mapLayersByShortName( const QString &shortName ) const
{
  QList<QgsMapLayer *> layers;
  const auto constMapLayers { mLayerStore->mapLayers() };
  for ( const auto &l : constMapLayers )
  {
    if ( ! l->shortName().isEmpty() )
    {
      if ( l->shortName() == shortName )
        layers << l;
    }
    else if ( l->name() == shortName )
    {
      layers << l;
    }
  }
  return layers;
}

bool QgsProject::unzip( const QString &filename, QgsProject::ReadFlags flags )
{
  clearError();
  std::unique_ptr<QgsProjectArchive> archive( new QgsProjectArchive() );

  // unzip the archive
  if ( !archive->unzip( filename ) )
  {
    setError( tr( "Unable to unzip file '%1'" ).arg( filename ) );
    return false;
  }

  // test if zip provides a .qgs file
  if ( archive->projectFile().isEmpty() )
  {
    setError( tr( "Zip archive does not provide a project file" ) );
    return false;
  }

  // load auxiliary storage
  if ( !archive->auxiliaryStorageFile().isEmpty() )
  {
    // database file is already a copy as it's been unzipped. So we don't open
    // auxiliary storage in copy mode in this case
    mAuxiliaryStorage.reset( new QgsAuxiliaryStorage( archive->auxiliaryStorageFile(), false ) );
  }
  else
  {
    mAuxiliaryStorage.reset( new QgsAuxiliaryStorage( *this ) );
  }

  // read the project file
  if ( ! readProjectFile( archive->projectFile(), flags ) )
  {
    setError( tr( "Cannot read unzipped qgs project file" ) );
    return false;
  }

  // keep the archive and remove the temporary .qgs file
  mArchive = std::move( archive );
  mArchive->clearProjectFile();

  return true;
}

bool QgsProject::zip( const QString &filename )
{
  clearError();

  // save the current project in a temporary .qgs file
  std::unique_ptr<QgsProjectArchive> archive( new QgsProjectArchive() );
  const QString baseName = QFileInfo( filename ).baseName();
  const QString qgsFileName = QStringLiteral( "%1.qgs" ).arg( baseName );
  QFile qgsFile( QDir( archive->dir() ).filePath( qgsFileName ) );

  bool writeOk = false;
  if ( qgsFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    writeOk = writeProjectFile( qgsFile.fileName() );
    qgsFile.close();
  }

  // stop here with an error message
  if ( ! writeOk )
  {
    setError( tr( "Unable to write temporary qgs file" ) );
    return false;
  }

  // save auxiliary storage
  const QFileInfo info( qgsFile );
  const QString asFileName = info.path() + QDir::separator() + info.completeBaseName() + "." + QgsAuxiliaryStorage::extension();

  if ( ! saveAuxiliaryStorage( asFileName ) )
  {
    const QString err = mAuxiliaryStorage->errorString();
    setError( tr( "Unable to save auxiliary storage ('%1')" ).arg( err ) );
    return false;
  }

  // create the archive
  archive->addFile( qgsFile.fileName() );
  archive->addFile( asFileName );

  // zip
  if ( !archive->zip( filename ) )
  {
    setError( tr( "Unable to perform zip" ) );
    return false;
  }

  return true;
}

bool QgsProject::isZipped() const
{
  return QgsZipUtils::isZipFile( mFile.fileName() );
}

QList<QgsMapLayer *> QgsProject::addMapLayers(
  const QList<QgsMapLayer *> &layers,
  bool addToLegend,
  bool takeOwnership )
{
  const QList<QgsMapLayer *> myResultList { mLayerStore->addMapLayers( layers, takeOwnership ) };
  if ( !myResultList.isEmpty() )
  {
    // Update transform context
    for ( auto &l : myResultList )
    {
      l->setTransformContext( transformContext() );
    }
    if ( addToLegend )
    {
      emit legendLayersAdded( myResultList );
    }
  }

  if ( mAuxiliaryStorage )
  {
    for ( QgsMapLayer *mlayer : myResultList )
    {
      if ( mlayer->type() != QgsMapLayerType::VectorLayer )
        continue;

      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( mlayer );
      if ( vl )
      {
        vl->loadAuxiliaryLayer( *mAuxiliaryStorage );
      }
    }
  }

  mProjectScope.reset();

  return myResultList;
}

QgsMapLayer *
QgsProject::addMapLayer( QgsMapLayer *layer,
                         bool addToLegend,
                         bool takeOwnership )
{
  QList<QgsMapLayer *> addedLayers;
  addedLayers = addMapLayers( QList<QgsMapLayer *>() << layer, addToLegend, takeOwnership );
  return addedLayers.isEmpty() ? nullptr : addedLayers[0];
}

void QgsProject::removeMapLayers( const QStringList &layerIds )
{
  mProjectScope.reset();
  mLayerStore->removeMapLayers( layerIds );
}

void QgsProject::removeMapLayers( const QList<QgsMapLayer *> &layers )
{
  mProjectScope.reset();
  mLayerStore->removeMapLayers( layers );
}

void QgsProject::removeMapLayer( const QString &layerId )
{
  mProjectScope.reset();
  mLayerStore->removeMapLayer( layerId );
}

void QgsProject::removeMapLayer( QgsMapLayer *layer )
{
  mProjectScope.reset();
  mLayerStore->removeMapLayer( layer );
}

QgsMapLayer *QgsProject::takeMapLayer( QgsMapLayer *layer )
{
  mProjectScope.reset();
  return mLayerStore->takeMapLayer( layer );
}

void QgsProject::removeAllMapLayers()
{
  mProjectScope.reset();
  mLayerStore->removeAllMapLayers();
}

void QgsProject::reloadAllLayers()
{
  QMap<QString, QgsMapLayer *> layers = mLayerStore->mapLayers();
  QMap<QString, QgsMapLayer *>::const_iterator it = layers.constBegin();
  for ( ; it != layers.constEnd(); ++it )
  {
    it.value()->reload();
  }
}

QMap<QString, QgsMapLayer *> QgsProject::mapLayers( const bool validOnly ) const
{
  return validOnly ? mLayerStore->validMapLayers() : mLayerStore->mapLayers();
}

QgsTransactionGroup *QgsProject::transactionGroup( const QString &providerKey, const QString &connString )
{
  return mTransactionGroups.value( qMakePair( providerKey, connString ) );
}

QgsCoordinateReferenceSystem QgsProject::defaultCrsForNewLayers() const
{
  QgsSettings settings;
  QgsCoordinateReferenceSystem defaultCrs;

  // TODO QGIS 4.0 -- remove this method, and place it somewhere in app (where it belongs)
  // in the meantime, we have a slightly hacky way to read the settings key using an enum which isn't available (since it lives in app)
  if ( settings.value( QStringLiteral( "/projections/unknownCrsBehavior" ), QStringLiteral( "NoAction" ), QgsSettings::App ).toString() == QStringLiteral( "UseProjectCrs" )
       || settings.value( QStringLiteral( "/projections/unknownCrsBehavior" ), 0, QgsSettings::App ).toString() == 2 )
  {
    // for new layers if the new layer crs method is set to either prompt or use project, then we use the project crs
    defaultCrs = crs();
  }
  else
  {
    // global crs
    QString layerDefaultCrs = settings.value( QStringLiteral( "/Projections/layerDefaultCrs" ), geoEpsgCrsAuthId() ).toString();
    defaultCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( layerDefaultCrs );
  }

  return defaultCrs;
}

void QgsProject::setTrustLayerMetadata( bool trust )
{
  mTrustLayerMetadata = trust;

  auto layers = mapLayers();
  for ( auto it = layers.constBegin(); it != layers.constEnd(); ++it )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( it.value() );
    if ( vl )
    {
      vl->setReadExtentFromXml( trust );
    }
  }
}

bool QgsProject::saveAuxiliaryStorage( const QString &filename )
{
  const QMap<QString, QgsMapLayer *> layers = mapLayers();
  bool empty = true;
  for ( auto it = layers.constBegin(); it != layers.constEnd(); ++it )
  {
    if ( it.value()->type() != QgsMapLayerType::VectorLayer )
      continue;

    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( it.value() );
    if ( vl && vl->auxiliaryLayer() )
    {
      vl->auxiliaryLayer()->save();
      empty &= vl->auxiliaryLayer()->auxiliaryFields().isEmpty();
    }
  }

  if ( !mAuxiliaryStorage->exists( *this ) && filename.isEmpty() && empty )
  {
    return true; // it's not an error
  }
  else if ( !filename.isEmpty() )
  {
    return mAuxiliaryStorage->saveAs( filename );
  }
  else
  {
    return mAuxiliaryStorage->saveAs( *this );
  }
}

QgsPropertiesDefinition &QgsProject::dataDefinedServerPropertyDefinitions()
{
  static QgsPropertiesDefinition sPropertyDefinitions
  {
    {
      QgsProject::DataDefinedServerProperty::WMSOnlineResource,
      QgsPropertyDefinition( "WMSOnlineResource", QObject::tr( "WMS Online Resource" ), QgsPropertyDefinition::String )
    },
  };
  return sPropertyDefinitions;
}

const QgsAuxiliaryStorage *QgsProject::auxiliaryStorage() const
{
  return mAuxiliaryStorage.get();
}

QgsAuxiliaryStorage *QgsProject::auxiliaryStorage()
{
  return mAuxiliaryStorage.get();
}

const QgsProjectMetadata &QgsProject::metadata() const
{
  return mMetadata;
}

void QgsProject::setMetadata( const QgsProjectMetadata &metadata )
{
  if ( metadata == mMetadata )
    return;

  mMetadata = metadata;
  mProjectScope.reset();

  emit metadataChanged();

  setDirty( true );
}

QSet<QgsMapLayer *> QgsProject::requiredLayers() const
{
  QSet<QgsMapLayer *> requiredLayers;

  const QMap<QString, QgsMapLayer *> &layers = mapLayers();
  for ( QMap<QString, QgsMapLayer *>::const_iterator it = layers.constBegin(); it != layers.constEnd(); ++it )
  {
    if ( !it.value()->flags().testFlag( QgsMapLayer::Removable ) )
    {
      requiredLayers.insert( it.value() );
    }
  }
  return requiredLayers;
}

void QgsProject::setRequiredLayers( const QSet<QgsMapLayer *> &layers )
{
  const QMap<QString, QgsMapLayer *> &projectLayers = mapLayers();
  for ( QMap<QString, QgsMapLayer *>::const_iterator it = projectLayers.constBegin(); it != projectLayers.constEnd(); ++it )
  {
    if ( layers.contains( it.value() ) == !it.value()->flags().testFlag( QgsMapLayer::Removable ) )
      continue;

    if ( layers.contains( it.value() ) )
      it.value()->setFlags( it.value()->flags() & ~QgsMapLayer::Removable );
    else
      it.value()->setFlags( it.value()->flags() | QgsMapLayer::Removable );
  }
}

void QgsProject::setProjectColors( const QgsNamedColorList &colors )
{
  // save colors to project
  QStringList customColors;
  QStringList customColorLabels;

  QgsNamedColorList::const_iterator colorIt = colors.constBegin();
  for ( ; colorIt != colors.constEnd(); ++colorIt )
  {
    QString color = QgsSymbolLayerUtils::encodeColor( ( *colorIt ).first );
    QString label = ( *colorIt ).second;
    customColors.append( color );
    customColorLabels.append( label );
  }
  writeEntry( QStringLiteral( "Palette" ), QStringLiteral( "/Colors" ), customColors );
  writeEntry( QStringLiteral( "Palette" ), QStringLiteral( "/Labels" ), customColorLabels );
  mProjectScope.reset();
  emit projectColorsChanged();
}

void QgsProject::setBackgroundColor( const QColor &color )
{
  if ( mBackgroundColor == color )
    return;

  mBackgroundColor = color;
  emit backgroundColorChanged();
}

QColor QgsProject::backgroundColor() const
{
  return mBackgroundColor;
}

void QgsProject::setSelectionColor( const QColor &color )
{
  if ( mSelectionColor == color )
    return;

  mSelectionColor = color;
  emit selectionColorChanged();
}

QColor QgsProject::selectionColor() const
{
  return mSelectionColor;
}

void QgsProject::setMapScales( const QVector<double> &scales )
{
  mViewSettings->setMapScales( scales );
}

QVector<double> QgsProject::mapScales() const
{
  return mViewSettings->mapScales();
}

void QgsProject::setUseProjectScales( bool enabled )
{
  mViewSettings->setUseProjectScales( enabled );
}

bool QgsProject::useProjectScales() const
{
  return mViewSettings->useProjectScales();
}

void QgsProject::generateTsFile( const QString &locale )
{
  QgsTranslationContext translationContext;
  translationContext.setProject( this );
  translationContext.setFileName( QStringLiteral( "%1/%2.ts" ).arg( absolutePath(), baseName() ) );

  QgsApplication::instance()->collectTranslatableObjects( &translationContext );

  translationContext.writeTsFile( locale );
}

QString QgsProject::translate( const QString &context, const QString &sourceText, const char *disambiguation, int n ) const
{
  if ( !mTranslator )
  {
    return sourceText;
  }

  QString result = mTranslator->translate( context.toUtf8(), sourceText.toUtf8(), disambiguation, n );

  if ( result.isEmpty() )
  {
    return sourceText;
  }
  return result;
}

bool QgsProject::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  const QMap<QString, QgsMapLayer *> layers = mapLayers( false );
  if ( !layers.empty() )
  {
    for ( auto it = layers.constBegin(); it != layers.constEnd(); ++it )
    {
      // NOTE: if visitEnter returns false it means "don't visit this layer", not "abort all further visitations"
      if ( visitor->visitEnter( QgsStyleEntityVisitorInterface::Node( QgsStyleEntityVisitorInterface::NodeType::Layer, ( *it )->id(), ( *it )->name() ) ) )
      {
        if ( !( ( *it )->accept( visitor ) ) )
          return false;

        if ( !visitor->visitExit( QgsStyleEntityVisitorInterface::Node( QgsStyleEntityVisitorInterface::NodeType::Layer, ( *it )->id(), ( *it )->name() ) ) )
          return false;
      }
    }
  }

  if ( !mLayoutManager->accept( visitor ) )
    return false;

  if ( !mAnnotationManager->accept( visitor ) )
    return false;

  return true;
}

/// @cond PRIVATE
GetNamedProjectColor::GetNamedProjectColor( const QgsProject *project )
  : QgsScopedExpressionFunction( QStringLiteral( "project_color" ), 1, QStringLiteral( "Color" ) )
{
  if ( !project )
    return;

  //build up color list from project. Do this in advance for speed
  QStringList colorStrings = project->readListEntry( QStringLiteral( "Palette" ), QStringLiteral( "/Colors" ) );
  QStringList colorLabels = project->readListEntry( QStringLiteral( "Palette" ), QStringLiteral( "/Labels" ) );

  //generate list from custom colors
  int colorIndex = 0;
  for ( QStringList::iterator it = colorStrings.begin();
        it != colorStrings.end(); ++it )
  {
    QColor color = QgsSymbolLayerUtils::decodeColor( *it );
    QString label;
    if ( colorLabels.length() > colorIndex )
    {
      label = colorLabels.at( colorIndex );
    }

    mColors.insert( label.toLower(), color );
    colorIndex++;
  }
}

GetNamedProjectColor::GetNamedProjectColor( const QHash<QString, QColor> &colors )
  : QgsScopedExpressionFunction( QStringLiteral( "project_color" ), 1, QStringLiteral( "Color" ) )
  , mColors( colors )
{
}

QVariant GetNamedProjectColor::func( const QVariantList &values, const QgsExpressionContext *, QgsExpression *, const QgsExpressionNodeFunction * )
{
  QString colorName = values.at( 0 ).toString().toLower();
  if ( mColors.contains( colorName ) )
  {
    return QStringLiteral( "%1,%2,%3" ).arg( mColors.value( colorName ).red() ).arg( mColors.value( colorName ).green() ).arg( mColors.value( colorName ).blue() );
  }
  else
    return QVariant();
}

QgsScopedExpressionFunction *GetNamedProjectColor::clone() const
{
  return new GetNamedProjectColor( mColors );
}
///@endcond
