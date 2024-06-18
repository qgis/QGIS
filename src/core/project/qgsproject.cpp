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
#include "qgsmaplayerfactory.h"
#include "qgspluginlayerregistry.h"
#include "qgsprojectfiletransform.h"
#include "qgssnappingconfig.h"
#include "qgspathresolver.h"
#include "qgsprojectstorage.h"
#include "qgsprojectstorageregistry.h"
#include "qgsprojectversion.h"
#include "qgsrasterlayer.h"
#include "qgsreadwritecontext.h"
#include "qgsrelationmanager.h"
#include "qgsannotationmanager.h"
#include "qgsvectorlayerjoinbuffer.h"
#include "qgsmapthemecollection.h"
#include "qgslayerdefinition.h"
#include "qgsunittypes.h"
#include "qgstransaction.h"
#include "qgstransactiongroup.h"
#include "qgsvectordataprovider.h"
#include "qgsprojectbadlayerhandler.h"
#include "qgsmeshlayer.h"
#include "qgslayoutmanager.h"
#include "qgsbookmarkmanager.h"
#include "qgsmaplayerstore.h"
#include "qgsziputils.h"
#include "qgsauxiliarystorage.h"
#include "qgscolorutils.h"
#include "qgsapplication.h"
#include "qgsexpressioncontextutils.h"
#include "qgsstyleentityvisitor.h"
#include "qgsprojectviewsettings.h"
#include "qgsprojectstylesettings.h"
#include "qgsprojecttimesettings.h"
#include "qgsvectortilelayer.h"
#include "qgstiledscenelayer.h"
#include "qgsruntimeprofiler.h"
#include "qgsannotationlayer.h"
#include "qgspointcloudlayer.h"
#include "qgsattributeeditorcontainer.h"
#include "qgsgrouplayer.h"
#include "qgsmapviewsmanager.h"
#include "qgsprojectelevationproperties.h"
#include "qgscombinedstylemodel.h"
#include "qgsprojectgpssettings.h"
#include "qgsthreadingutils.h"
#include "qgssensormanager.h"
#include "qgsproviderregistry.h"
#include "qgsrunnableprovidercreator.h"
#include "qgssettingsregistrycore.h"
#include "qgspluginlayer.h"

#include <algorithm>
#include <QApplication>
#include <QFileInfo>
#include <QDomNode>
#include <QObject>
#include <QTextStream>
#include <QTemporaryFile>
#include <QDir>
#include <QUrl>
#include <QStandardPaths>
#include <QUuid>
#include <QRegularExpression>
#include <QThreadPool>

#ifdef _MSC_VER
#include <sys/utime.h>
#else
#include <utime.h>
#endif

// canonical project instance
QgsProject *QgsProject::sProject = nullptr;

/**
 * Takes the given scope and key and convert them to a string list of key
 * tokens that will be used to navigate through a Property hierarchy
 *
 * E.g., scope "someplugin" and key "/foo/bar/baz" will become a string list
 * of { "properties", "someplugin", "foo", "bar", "baz" }.  "properties" is
 * always first because that's the permanent ``root'' Property node.
 */
QStringList makeKeyTokens_( const QString &scope, const QString &key )
{
  QStringList keyTokens = QStringList( scope );
  keyTokens += key.split( '/', Qt::SkipEmptyParts );

  // be sure to include the canonical root node
  keyTokens.push_front( QStringLiteral( "properties" ) );

  //check validy of keys since an invalid xml name will will be dropped upon saving the xml file. If not valid, we print a message to the console.
  for ( int i = 0; i < keyTokens.size(); ++i )
  {
    const QString keyToken = keyTokens.at( i );

    //invalid chars in XML are found at http://www.w3.org/TR/REC-xml/#NT-NameChar
    //note : it seems \x10000-\xEFFFF is valid, but it when added to the regexp, a lot of unwanted characters remain
    const thread_local QRegularExpression sInvalidRegexp = QRegularExpression( QStringLiteral( "([^:A-Z_a-z\\x{C0}-\\x{D6}\\x{D8}-\\x{F6}\\x{F8}-\\x{2FF}\\x{370}-\\x{37D}\\x{37F}-\\x{1FFF}\\x{200C}-\\x{200D}\\x{2070}-\\x{218F}\\x{2C00}-\\x{2FEF}\\x{3001}-\\x{D7FF}\\x{F900}-\\x{FDCF}\\x{FDF0}-\\x{FFFD}\\-\\.0-9\\x{B7}\\x{0300}-\\x{036F}\\x{203F}-\\x{2040}]|^[^:A-Z_a-z\\x{C0}-\\x{D6}\\x{D8}-\\x{F6}\\x{F8}-\\x{2FF}\\x{370}-\\x{37D}\\x{37F}-\\x{1FFF}\\x{200C}-\\x{200D}\\x{2070}-\\x{218F}\\x{2C00}-\\x{2FEF}\\x{3001}-\\x{D7FF}\\x{F900}-\\x{FDCF}\\x{FDF0}-\\x{FFFD}])" ) );
    if ( keyToken.contains( sInvalidRegexp ) )
    {
      const QString errorString = QObject::tr( "Entry token invalid : '%1'. The token will not be saved to file." ).arg( keyToken );
      QgsMessageLog::logMessage( errorString, QString(), Qgis::MessageLevel::Critical );
    }
  }

  return keyTokens;
}



/**
 * Returns the property that matches the given key sequence, if any
 *
 * \param scope scope of key
 * \param key keyname
 * \param rootProperty is likely to be the top level QgsProjectPropertyKey in QgsProject:e:Imp.
 *
 * \return null if not found, otherwise located Property
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
 * Adds the given key and value.
 *
 * \param scope scope of key
 * \param key key name
 * \param rootProperty is the property from which to start adding
 * \param value the value associated with the key
 * \param propertiesModified the parameter will be set to true if the written entry modifies pre-existing properties
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
 * Removes a given key.
 *
 * \param scope scope of key
 * \param key key name
 * \param rootProperty is the property from which to start adding
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

QgsProject::QgsProject( QObject *parent, Qgis::ProjectCapabilities capabilities )
  : QObject( parent )
  , mCapabilities( capabilities )
  , mLayerStore( new QgsMapLayerStore( this ) )
  , mBadLayerHandler( new QgsProjectBadLayerHandler() )
  , mSnappingConfig( this )
  , mRelationManager( new QgsRelationManager( this ) )
  , mAnnotationManager( new QgsAnnotationManager( this ) )
  , mLayoutManager( new QgsLayoutManager( this ) )
  , m3DViewsManager( new QgsMapViewsManager( this ) )
  , mBookmarkManager( QgsBookmarkManager::createProjectBasedManager( this ) )
  , mSensorManager( new QgsSensorManager( this ) )
  , mViewSettings( new QgsProjectViewSettings( this ) )
  , mStyleSettings( new QgsProjectStyleSettings( this ) )
  , mTimeSettings( new QgsProjectTimeSettings( this ) )
  , mElevationProperties( new QgsProjectElevationProperties( this ) )
  , mDisplaySettings( new QgsProjectDisplaySettings( this ) )
  , mGpsSettings( new QgsProjectGpsSettings( this ) )
  , mRootGroup( new QgsLayerTree )
  , mLabelingEngineSettings( new QgsLabelingEngineSettings )
  , mArchive( new QgsArchive() )
  , mAuxiliaryStorage( new QgsAuxiliaryStorage() )
{
  mProperties.setName( QStringLiteral( "properties" ) );

  mMainAnnotationLayer = new QgsAnnotationLayer( QObject::tr( "Annotations" ), QgsAnnotationLayer::LayerOptions( mTransformContext ) );
  mMainAnnotationLayer->setParent( this );

  clear();

  // bind the layer tree to the map layer registry.
  // whenever layers are added to or removed from the registry,
  // layer tree will be updated
  mLayerTreeRegistryBridge = new QgsLayerTreeRegistryBridge( mRootGroup, this, this );
  connect( this, &QgsProject::layersAdded, this, &QgsProject::onMapLayersAdded );
  connect( this, &QgsProject::layersRemoved, this, [this] { cleanTransactionGroups(); } );
  connect( this, qOverload< const QList<QgsMapLayer *> & >( &QgsProject::layersWillBeRemoved ), this, &QgsProject::onMapLayersRemoved );

  // proxy map layer store signals to this
  connect( mLayerStore.get(), qOverload<const QStringList &>( &QgsMapLayerStore::layersWillBeRemoved ),
  this, [this]( const QStringList & layers ) { mProjectScope.reset(); emit layersWillBeRemoved( layers ); } );
  connect( mLayerStore.get(), qOverload< const QList<QgsMapLayer *> & >( &QgsMapLayerStore::layersWillBeRemoved ),
  this, [this]( const QList<QgsMapLayer *> &layers ) { mProjectScope.reset(); emit layersWillBeRemoved( layers ); } );
  connect( mLayerStore.get(), qOverload< const QString & >( &QgsMapLayerStore::layerWillBeRemoved ),
  this, [this]( const QString & layer ) { mProjectScope.reset(); emit layerWillBeRemoved( layer ); } );
  connect( mLayerStore.get(), qOverload< QgsMapLayer * >( &QgsMapLayerStore::layerWillBeRemoved ),
  this, [this]( QgsMapLayer * layer ) { mProjectScope.reset(); emit layerWillBeRemoved( layer ); } );
  connect( mLayerStore.get(), qOverload<const QStringList & >( &QgsMapLayerStore::layersRemoved ), this,
  [this]( const QStringList & layers ) { mProjectScope.reset(); emit layersRemoved( layers ); } );
  connect( mLayerStore.get(), &QgsMapLayerStore::layerRemoved, this,
  [this]( const QString & layer ) { mProjectScope.reset(); emit layerRemoved( layer ); } );
  connect( mLayerStore.get(), &QgsMapLayerStore::allLayersRemoved, this,
  [this]() { mProjectScope.reset(); emit removeAll(); } );
  connect( mLayerStore.get(), &QgsMapLayerStore::layersAdded, this,
  [this]( const QList< QgsMapLayer * > &layers ) { mProjectScope.reset(); emit layersAdded( layers ); } );
  connect( mLayerStore.get(), &QgsMapLayerStore::layerWasAdded, this,
  [this]( QgsMapLayer * layer ) { mProjectScope.reset(); emit layerWasAdded( layer ); } );

  if ( QgsApplication::instance() )
  {
    connect( QgsApplication::instance(), &QgsApplication::requestForTranslatableObjects, this, &QgsProject::registerTranslatableObjects );
  }

  connect( mLayerStore.get(), qOverload< const QList<QgsMapLayer *> & >( &QgsMapLayerStore::layersWillBeRemoved ), this,
           [this]( const QList<QgsMapLayer *> &layers )
  {
    for ( const auto &layer : layers )
    {
      disconnect( layer, &QgsMapLayer::dataSourceChanged, mRelationManager, &QgsRelationManager::updateRelationsStatus );
    }
  }
         );
  connect( mLayerStore.get(),  qOverload< const QList<QgsMapLayer *> & >( &QgsMapLayerStore::layersAdded ), this,
           [this]( const QList<QgsMapLayer *> &layers )
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

  mStyleSettings->combinedStyleModel()->addDefaultStyle();
}


QgsProject::~QgsProject()
{
  mIsBeingDeleted = true;

  clear();
  releaseHandlesToProjectArchive();
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

    connect( sProject, &QgsProject::projectColorsChanged, QgsStyle::defaultStyle( false ), &QgsStyle::triggerIconRebuild );
  }
  return sProject;
}

void QgsProject::setTitle( const QString &title )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( title == mMetadata.title() )
    return;

  mMetadata.setTitle( title );
  mProjectScope.reset();
  emit metadataChanged();

  setDirty( true );
}

QString QgsProject::title() const
{
  // this method is called quite extensively from other threads via QgsProject::createExpressionContextScope()
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mMetadata.title();
}

void QgsProject::setFlags( Qgis::ProjectFlags flags )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const bool oldEvaluateDefaultValues = mFlags & Qgis::ProjectFlag::EvaluateDefaultValuesOnProviderSide;
  const bool newEvaluateDefaultValues = flags & Qgis::ProjectFlag::EvaluateDefaultValuesOnProviderSide;
  if ( oldEvaluateDefaultValues != newEvaluateDefaultValues )
  {
    const QMap<QString, QgsMapLayer *> layers = mapLayers();
    for ( auto layerIt = layers.constBegin(); layerIt != layers.constEnd(); ++layerIt )
    {
      if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layerIt.value() ) )
        if ( vl->dataProvider() )
          vl->dataProvider()->setProviderProperty( QgsVectorDataProvider::EvaluateDefaultValues, newEvaluateDefaultValues );
    }
  }

  const bool oldTrustLayerMetadata = mFlags & Qgis::ProjectFlag::TrustStoredLayerStatistics;
  const bool newTrustLayerMetadata = flags & Qgis::ProjectFlag::TrustStoredLayerStatistics;
  if ( oldTrustLayerMetadata != newTrustLayerMetadata )
  {
    const QMap<QString, QgsMapLayer *> layers = mapLayers();
    for ( auto layerIt = layers.constBegin(); layerIt != layers.constEnd(); ++layerIt )
    {
      if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layerIt.value() ) )
      {
        vl->setReadExtentFromXml( newTrustLayerMetadata );
      }
    }
  }

  if ( mFlags != flags )
  {
    mFlags = flags;
    setDirty( true );
  }
}

void QgsProject::setFlag( Qgis::ProjectFlag flag, bool enabled )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Qgis::ProjectFlags newFlags = mFlags;
  if ( enabled )
    newFlags |= flag;
  else
    newFlags &= ~( static_cast< int >( flag ) );
  setFlags( newFlags );
}

QString QgsProject::saveUser() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mSaveUser;
}

QString QgsProject::saveUserFullName() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mSaveUserFull;
}

QDateTime QgsProject::lastSaveDateTime() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mSaveDateTime;
}

QgsProjectVersion QgsProject::lastSaveVersion() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mSaveVersion;
}

bool QgsProject::isDirty() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDirty;
}

void QgsProject::setDirty( const bool dirty )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( dirty && mDirtyBlockCount > 0 )
    return;

  if ( dirty )
    emit dirtySet();

  if ( mDirty == dirty )
    return;

  mDirty = dirty;
  emit isDirtyChanged( mDirty );
}

void QgsProject::setPresetHomePath( const QString &path )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QList<QgsAttributeEditorElement *> elements = parent->children();

  for ( QgsAttributeEditorElement *element : elements )
  {
    if ( element->type() == Qgis::AttributeEditorType::Container )
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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  //register layers
  const QList<QgsLayerTreeLayer *> layers = mRootGroup->findLayers();

  for ( const QgsLayerTreeLayer *layer : layers )
  {
    translationContext->registerTranslation( QStringLiteral( "project:layers:%1" ).arg( layer->layerId() ), layer->name() );

    QgsMapLayer *mapLayer = layer->layer();
    if ( mapLayer && mapLayer->type() == Qgis::LayerType::Vector )
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

        if ( field.editorWidgetSetup().type() == QLatin1String( "ValueRelation" ) )
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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mDataDefinedServerProperties = properties;
}

QgsPropertyCollection QgsProject::dataDefinedServerProperties() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDataDefinedServerProperties;
}

bool QgsProject::startEditing( QgsVectorLayer *vectorLayer )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  switch ( mTransactionMode )
  {
    case Qgis::TransactionMode::Disabled:
    case Qgis::TransactionMode::AutomaticGroups:
    {
      if ( ! vectorLayer )
        return false;
      return vectorLayer->startEditing();
    }

    case Qgis::TransactionMode::BufferedGroups:
      return mEditBufferGroup.startEditing();
  }

  return false;
}

bool QgsProject::commitChanges( QStringList &commitErrors, bool stopEditing, QgsVectorLayer *vectorLayer )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  switch ( mTransactionMode )
  {
    case Qgis::TransactionMode::Disabled:
    case Qgis::TransactionMode::AutomaticGroups:
    {
      if ( ! vectorLayer )
      {
        commitErrors.append( tr( "Trying to commit changes without a layer specified. This only works if the transaction mode is buffered" ) );
        return false;
      }
      bool success = vectorLayer->commitChanges( stopEditing );
      commitErrors = vectorLayer->commitErrors();
      return success;
    }

    case Qgis::TransactionMode::BufferedGroups:
      return mEditBufferGroup.commitChanges( commitErrors, stopEditing );
  }

  return false;
}

bool QgsProject::rollBack( QStringList &rollbackErrors, bool stopEditing, QgsVectorLayer *vectorLayer )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  switch ( mTransactionMode )
  {
    case Qgis::TransactionMode::Disabled:
    case Qgis::TransactionMode::AutomaticGroups:
    {
      if ( ! vectorLayer )
      {
        rollbackErrors.append( tr( "Trying to roll back changes without a layer specified. This only works if the transaction mode is buffered" ) );
        return false;
      }
      bool success = vectorLayer->rollBack( stopEditing );
      rollbackErrors = vectorLayer->commitErrors();
      return success;
    }

    case Qgis::TransactionMode::BufferedGroups:
      return mEditBufferGroup.rollBack( rollbackErrors, stopEditing );
  }

  return false;
}

void QgsProject::setFileName( const QString &name )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( name == mFile.fileName() )
    return;

  const QString oldHomePath = homePath();

  mFile.setFileName( name );
  mCachedHomePath.clear();
  mProjectScope.reset();

  emit fileNameChanged();

  const QString newHomePath = homePath();
  if ( newHomePath != oldHomePath )
    emit homePathChanged();

  setDirty( true );
}

QString QgsProject::fileName() const
{
  // this method is called quite extensively from other threads via QgsProject::createExpressionContextScope()
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mFile.fileName();
}

void QgsProject::setOriginalPath( const QString &path )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mOriginalPath = path;
}

QString QgsProject::originalPath() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mOriginalPath;
}

QFileInfo QgsProject::fileInfo() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QFileInfo( mFile );
}

QgsProjectStorage *QgsProject::projectStorage() const
{
  // this method is called quite extensively from other threads via QgsProject::createExpressionContextScope()
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return QgsApplication::projectStorageRegistry()->projectStorageFromUri( mFile.fileName() );
}

QDateTime QgsProject::lastModified() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( projectStorage() )
    return QString();

  if ( mFile.fileName().isEmpty() )
    return QString();  // this is to protect ourselves from getting current directory from QFileInfo::absoluteFilePath()

  return QFileInfo( mFile.fileName() ).absolutePath();
}

QString QgsProject::absoluteFilePath() const
{
  // this method is called quite extensively from other threads via QgsProject::createExpressionContextScope()
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  if ( projectStorage() )
    return QString();

  if ( mFile.fileName().isEmpty() )
    return QString();  // this is to protect ourselves from getting current directory from QFileInfo::absoluteFilePath()

  return QFileInfo( mFile.fileName() ).absoluteFilePath();
}

QString QgsProject::baseName() const
{
  // this method is called quite extensively from other threads via QgsProject::createExpressionContextScope()
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

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

Qgis::FilePathType QgsProject::filePathStorage() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const bool absolutePaths = readBoolEntry( QStringLiteral( "Paths" ), QStringLiteral( "/Absolute" ), false );
  return absolutePaths ? Qgis::FilePathType::Absolute : Qgis::FilePathType::Relative;
}

void QgsProject::setFilePathStorage( Qgis::FilePathType type )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  switch ( type )
  {
    case Qgis::FilePathType::Absolute:
      writeEntry( QStringLiteral( "Paths" ), QStringLiteral( "/Absolute" ), true );
      break;
    case Qgis::FilePathType::Relative:
      writeEntry( QStringLiteral( "Paths" ), QStringLiteral( "/Absolute" ), false );
      break;
  }
}

QgsCoordinateReferenceSystem QgsProject::crs() const
{
  // this method is called quite extensively from other threads via QgsProject::createExpressionContextScope()
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mCrs;
}

QgsCoordinateReferenceSystem QgsProject::crs3D() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mCrs3D.isValid() ? mCrs3D : mCrs;
}

void QgsProject::setCrs( const QgsCoordinateReferenceSystem &crs, bool adjustEllipsoid )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( crs != mCrs )
  {
    const QgsCoordinateReferenceSystem oldVerticalCrs = verticalCrs();
    const QgsCoordinateReferenceSystem oldCrs3D = mCrs3D;
    mCrs = crs;
    writeEntry( QStringLiteral( "SpatialRefSys" ), QStringLiteral( "/ProjectionsEnabled" ), crs.isValid() ? 1 : 0 );
    mProjectScope.reset();

    // if annotation layer doesn't have a crs (i.e. in a newly created project), it should
    // initially inherit the project CRS
    if ( !mMainAnnotationLayer->crs().isValid() || mMainAnnotationLayer->isEmpty() )
      mMainAnnotationLayer->setCrs( crs );

    rebuildCrs3D();

    setDirty( true );
    emit crsChanged();
    // Did vertical crs also change as a result of this? If so, emit signal
    if ( oldVerticalCrs != verticalCrs() )
      emit verticalCrsChanged();
    if ( oldCrs3D != mCrs3D )
      emit crs3DChanged();
  }

  if ( adjustEllipsoid )
    setEllipsoid( crs.ellipsoidAcronym() );
}

QString QgsProject::ellipsoid() const
{
  // this method is called quite extensively from other threads via QgsProject::createExpressionContextScope()
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  if ( !crs().isValid() )
    return geoNone();

  return readEntry( QStringLiteral( "Measure" ), QStringLiteral( "/Ellipsoid" ), geoNone() );
}

void QgsProject::setEllipsoid( const QString &ellipsoid )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( ellipsoid == readEntry( QStringLiteral( "Measure" ), QStringLiteral( "/Ellipsoid" ) ) )
    return;

  mProjectScope.reset();
  writeEntry( QStringLiteral( "Measure" ), QStringLiteral( "/Ellipsoid" ), ellipsoid );
  emit ellipsoidChanged( ellipsoid );
}

QgsCoordinateReferenceSystem QgsProject::verticalCrs() const
{
  // this method is called quite extensively from other threads via QgsProject::createExpressionContextScope()
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  switch ( mCrs.type() )
  {
    case Qgis::CrsType::Vertical: // would hope this never happens!
      QgsDebugError( QStringLiteral( "Project has a vertical CRS set as the horizontal CRS!" ) );
      return mCrs;

    case Qgis::CrsType::Compound:
      return mCrs.verticalCrs();

    case Qgis::CrsType::Unknown:
    case Qgis::CrsType::Geodetic:
    case Qgis::CrsType::Geocentric:
    case Qgis::CrsType::Geographic2d:
    case Qgis::CrsType::Geographic3d:
    case Qgis::CrsType::Projected:
    case Qgis::CrsType::Temporal:
    case Qgis::CrsType::Engineering:
    case Qgis::CrsType::Bound:
    case Qgis::CrsType::Other:
    case Qgis::CrsType::DerivedProjected:
      break;
  }
  return mVerticalCrs;
}

bool QgsProject::setVerticalCrs( const QgsCoordinateReferenceSystem &crs, QString *errorMessage )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  bool res = true;
  if ( crs.isValid() )
  {
    // validate that passed crs is a vertical crs
    switch ( crs.type() )
    {
      case Qgis::CrsType::Vertical:
        break;

      case Qgis::CrsType::Unknown:
      case Qgis::CrsType::Compound:
      case Qgis::CrsType::Geodetic:
      case Qgis::CrsType::Geocentric:
      case Qgis::CrsType::Geographic2d:
      case Qgis::CrsType::Geographic3d:
      case Qgis::CrsType::Projected:
      case Qgis::CrsType::Temporal:
      case Qgis::CrsType::Engineering:
      case Qgis::CrsType::Bound:
      case Qgis::CrsType::Other:
      case Qgis::CrsType::DerivedProjected:
        if ( errorMessage )
          *errorMessage = QObject::tr( "Specified CRS is a %1 CRS, not a Vertical CRS" ).arg( qgsEnumValueToKey( crs.type() ) );
        return false;
    }
  }

  if ( crs != mVerticalCrs )
  {
    const QgsCoordinateReferenceSystem oldVerticalCrs = verticalCrs();
    const QgsCoordinateReferenceSystem oldCrs3D = mCrs3D;

    switch ( mCrs.type() )
    {
      case Qgis::CrsType::Compound:
        if ( crs != oldVerticalCrs )
        {
          if ( errorMessage )
            *errorMessage = QObject::tr( "Project CRS is a Compound CRS, specified Vertical CRS will be ignored" );
          return false;
        }
        break;

      case Qgis::CrsType::Geographic3d:
        if ( crs != oldVerticalCrs )
        {
          if ( errorMessage )
            *errorMessage = QObject::tr( "Project CRS is a Geographic 3D CRS, specified Vertical CRS will be ignored" );
          return false;
        }
        break;

      case Qgis::CrsType::Geocentric:
        if ( crs != oldVerticalCrs )
        {
          if ( errorMessage )
            *errorMessage = QObject::tr( "Project CRS is a Geocentric CRS, specified Vertical CRS will be ignored" );
          return false;
        }
        break;

      case Qgis::CrsType::Projected:
        if ( mCrs.hasVerticalAxis() && crs != oldVerticalCrs )
        {
          if ( errorMessage )
            *errorMessage = QObject::tr( "Project CRS is a Projected 3D CRS, specified Vertical CRS will be ignored" );
          return false;
        }
        break;

      case Qgis::CrsType::Unknown:
      case Qgis::CrsType::Geodetic:
      case Qgis::CrsType::Geographic2d:
      case Qgis::CrsType::Temporal:
      case Qgis::CrsType::Engineering:
      case Qgis::CrsType::Bound:
      case Qgis::CrsType::Other:
      case Qgis::CrsType::Vertical:
      case Qgis::CrsType::DerivedProjected:
        break;
    }

    mVerticalCrs = crs;
    res = rebuildCrs3D( errorMessage );
    mProjectScope.reset();

    setDirty( true );
    // only emit signal if vertical crs was actually changed, so eg if mCrs is compound
    // then we haven't actually changed the vertical crs by this call!
    if ( verticalCrs() != oldVerticalCrs )
      emit verticalCrsChanged();
    if ( mCrs3D != oldCrs3D )
      emit crs3DChanged();
  }
  return res;
}

QgsCoordinateTransformContext QgsProject::transformContext() const
{
  // this method is called quite extensively from other threads via QgsProject::createExpressionContextScope()
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mTransformContext;
}

void QgsProject::setTransformContext( const QgsCoordinateTransformContext &context )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( context == mTransformContext )
    return;

  mTransformContext = context;
  mProjectScope.reset();

  mMainAnnotationLayer->setTransformContext( context );
  for ( auto &layer : mLayerStore.get()->mapLayers() )
  {
    layer->setTransformContext( context );
  }
  emit transformContextChanged();
}

void QgsProject::clear()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  ScopedIntIncrementor snapSingleBlocker( &mBlockSnappingUpdates );

  emit aboutToBeCleared();

  mProjectScope.reset();
  mFile.setFileName( QString() );
  mProperties.clearKeys();
  mSaveUser.clear();
  mSaveUserFull.clear();
  mSaveDateTime = QDateTime();
  mSaveVersion = QgsProjectVersion();
  mHomePath.clear();
  mCachedHomePath.clear();
  mTransactionMode = Qgis::TransactionMode::Disabled;
  mFlags = Qgis::ProjectFlags();
  mDirty = false;
  mCustomVariables.clear();
  mCrs = QgsCoordinateReferenceSystem();
  mVerticalCrs = QgsCoordinateReferenceSystem();
  mCrs3D = QgsCoordinateReferenceSystem();
  mMetadata = QgsProjectMetadata();
  mElevationShadingRenderer = QgsElevationShadingRenderer();
  if ( !mSettings.value( QStringLiteral( "projects/anonymize_new_projects" ), false, QgsSettings::Core ).toBool() )
  {
    mMetadata.setCreationDateTime( QDateTime::currentDateTime() );
    mMetadata.setAuthor( QgsApplication::userFullName() );
  }
  emit metadataChanged();

  QgsCoordinateTransformContext context;
  context.readSettings();
  setTransformContext( context );

  //fallback to QGIS default measurement unit
  bool ok = false;
  const Qgis::DistanceUnit distanceUnit = QgsUnitTypes::decodeDistanceUnit( mSettings.value( QStringLiteral( "/qgis/measure/displayunits" ) ).toString(), &ok );
  setDistanceUnits( ok ? distanceUnit : Qgis::DistanceUnit::Meters );
  ok = false;
  const Qgis::AreaUnit areaUnits = QgsUnitTypes::decodeAreaUnit( mSettings.value( QStringLiteral( "/qgis/measure/areaunits" ) ).toString(), &ok );
  setAreaUnits( ok ? areaUnits : Qgis::AreaUnit::SquareMeters );

  mEmbeddedLayers.clear();
  mRelationManager->clear();
  mAnnotationManager->clear();
  mLayoutManager->clear();
  m3DViewsManager->clear();
  mBookmarkManager->clear();
  mSensorManager->clear();
  mViewSettings->reset();
  mTimeSettings->reset();
  mElevationProperties->reset();
  mDisplaySettings->reset();
  mGpsSettings->reset();
  mSnappingConfig.reset();
  mAvoidIntersectionsMode = Qgis::AvoidIntersectionsMode::AllowIntersections;
  emit avoidIntersectionsModeChanged();
  emit topologicalEditingChanged();

  mMapThemeCollection.reset( new QgsMapThemeCollection( this ) );
  emit mapThemeCollectionChanged();

  mLabelingEngineSettings->clear();

  // must happen BEFORE archive reset, because we need to release the hold on any files which
  // exists within the archive. Otherwise the archive can't be removed.
  releaseHandlesToProjectArchive();

  mAuxiliaryStorage.reset( new QgsAuxiliaryStorage() );
  mArchive.reset( new QgsArchive() );

  // must happen AFTER archive reset, as it will populate a new style database within the new archive
  mStyleSettings->reset();

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

  const bool defaultRelativePaths = mSettings.value( QStringLiteral( "/qgis/defaultProjectPathsRelative" ), true ).toBool();
  setFilePathStorage( defaultRelativePaths ? Qgis::FilePathType::Relative : Qgis::FilePathType::Absolute );

  int red = mSettings.value( QStringLiteral( "qgis/default_canvas_color_red" ), 255 ).toInt();
  int green = mSettings.value( QStringLiteral( "qgis/default_canvas_color_green" ), 255 ).toInt();
  int blue = mSettings.value( QStringLiteral( "qgis/default_canvas_color_blue" ), 255 ).toInt();
  setBackgroundColor( QColor( red, green, blue ) );

  red = mSettings.value( QStringLiteral( "qgis/default_selection_color_red" ), 255 ).toInt();
  green = mSettings.value( QStringLiteral( "qgis/default_selection_color_green" ), 255 ).toInt();
  blue = mSettings.value( QStringLiteral( "qgis/default_selection_color_blue" ), 0 ).toInt();
  const int alpha = mSettings.value( QStringLiteral( "qgis/default_selection_color_alpha" ), 255 ).toInt();
  setSelectionColor( QColor( red, green, blue, alpha ) );

  mSnappingConfig.clearIndividualLayerSettings();

  removeAllMapLayers();
  mRootGroup->clear();
  if ( mMainAnnotationLayer )
    mMainAnnotationLayer->reset();

  snapSingleBlocker.release();

  if ( !mBlockSnappingUpdates )
    emit snappingConfigChanged( mSnappingConfig );

  setDirty( false );
  emit homePathChanged();
  if ( !mBlockChangeSignalsDuringClear )
  {
    emit verticalCrsChanged();
    emit crs3DChanged();
  }
  emit cleared();
}

// basically a debugging tool to dump property list values
void dump_( const QgsProjectPropertyKey &topQgsPropertyKey )
{
  QgsDebugMsgLevel( QStringLiteral( "current properties:" ), 3 );
  topQgsPropertyKey.dump();
}

/**
 * Restores any optional properties found in "doc" to "properties".
 *
 * properties tags for all optional properties.  Within that there will be scope
 * tags.  In the following example there exist one property in the "fsplugin"
 * scope.  "layers" is a list containing three string values.
 *
 * \code{.xml}
 * <properties>
 *   <fsplugin>
 *     <foo type="int" >42</foo>
 *     <baz type="int" >1</baz>
 *     <layers type="QStringList" >
 *       <value>railroad</value>
 *       <value>airport</value>
 *     </layers>
 *     <xyqzzy type="int" >1</xyqzzy>
 *     <bar type="double" >123.456</bar>
 *     <feature_types type="QStringList" >
 *        <value>type</value>
 *     </feature_types>
 *   </fsplugin>
 * </properties>
 * \endcode
 *
 * \param doc xml document
 * \param project_properties should be the top QgsProjectPropertyKey node.
*/
void _getProperties( const QDomDocument &doc, QgsProjectPropertyKey &project_properties )
{
  const QDomElement propertiesElem = doc.documentElement().firstChildElement( QStringLiteral( "properties" ) );

  if ( propertiesElem.isNull() )  // no properties found, so we're done
  {
    return;
  }

  const QDomNodeList scopes = propertiesElem.childNodes();

  if ( propertiesElem.firstChild().isNull() )
  {
    QgsDebugError( QStringLiteral( "empty ``properties'' XML tag ... bailing" ) );
    return;
  }

  if ( ! project_properties.readXml( propertiesElem ) )
  {
    QgsDebugError( QStringLiteral( "Project_properties.readXml() failed" ) );
  }
}

/**
 * Returns the data defined server properties collection found in "doc" to "dataDefinedServerProperties".
 * \param doc xml document
 * \param dataDefinedServerPropertyDefinitions property collection of the server overrides
 * \since QGIS 3.14
*/
QgsPropertyCollection getDataDefinedServerProperties( const QDomDocument &doc, const QgsPropertiesDefinition &dataDefinedServerPropertyDefinitions )
{
  QgsPropertyCollection ddServerProperties;
  // Read data defined server properties
  const QDomElement ddElem = doc.documentElement().firstChildElement( QStringLiteral( "dataDefinedServerProperties" ) );
  if ( !ddElem.isNull() )
  {
    if ( !ddServerProperties.readXml( ddElem, dataDefinedServerPropertyDefinitions ) )
    {
      QgsDebugError( QStringLiteral( "dataDefinedServerProperties.readXml() failed" ) );
    }
  }
  return ddServerProperties;
}

/**
* Get the project title
* \todo XXX we should go with the attribute xor title, not both.
*/
static void _getTitle( const QDomDocument &doc, QString &title )
{
  const QDomElement titleNode = doc.documentElement().firstChildElement( QStringLiteral( "title" ) );

  title.clear();               // by default the title will be empty

  if ( titleNode.isNull() )
  {
    QgsDebugMsgLevel( QStringLiteral( "unable to find title element" ), 2 );
    return;
  }

  if ( !titleNode.hasChildNodes() ) // if not, then there's no actual text
  {
    QgsDebugMsgLevel( QStringLiteral( "unable to find title element" ), 2 );
    return;
  }

  const QDomNode titleTextNode = titleNode.firstChild();  // should only have one child

  if ( !titleTextNode.isText() )
  {
    QgsDebugMsgLevel( QStringLiteral( "unable to find title element" ), 2 );
    return;
  }

  const QDomText titleText = titleTextNode.toText();

  title = titleText.data();

}

static void readProjectFileMetadata( const QDomDocument &doc, QString &lastUser, QString &lastUserFull, QDateTime &lastSaveDateTime )
{
  const QDomNodeList nl = doc.elementsByTagName( QStringLiteral( "qgis" ) );

  if ( !nl.count() )
  {
    QgsDebugError( QStringLiteral( "unable to find qgis element" ) );
    return;
  }

  const QDomNode qgisNode = nl.item( 0 ); // there should only be one, so zeroth element OK

  const QDomElement qgisElement = qgisNode.toElement(); // qgis node should be element
  lastUser = qgisElement.attribute( QStringLiteral( "saveUser" ), QString() );
  lastUserFull = qgisElement.attribute( QStringLiteral( "saveUserFull" ), QString() );
  lastSaveDateTime = QDateTime::fromString( qgisElement.attribute( QStringLiteral( "saveDateTime" ), QString() ), Qt::ISODate );
}

QgsProjectVersion getVersion( const QDomDocument &doc )
{
  const QDomNodeList nl = doc.elementsByTagName( QStringLiteral( "qgis" ) );

  if ( !nl.count() )
  {
    QgsDebugError( QStringLiteral( " unable to find qgis element in project file" ) );
    return QgsProjectVersion( 0, 0, 0, QString() );
  }

  const QDomNode qgisNode = nl.item( 0 );  // there should only be one, so zeroth element OK

  const QDomElement qgisElement = qgisNode.toElement(); // qgis node should be element
  QgsProjectVersion projectVersion( qgisElement.attribute( QStringLiteral( "version" ) ) );
  return projectVersion;
}

QgsSnappingConfig QgsProject::snappingConfig() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mSnappingConfig;
}

void QgsProject::setSnappingConfig( const QgsSnappingConfig &snappingConfig )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mSnappingConfig == snappingConfig )
    return;

  mSnappingConfig = snappingConfig;
  setDirty( true );
  emit snappingConfigChanged( mSnappingConfig );
}

void QgsProject::setAvoidIntersectionsMode( const Qgis::AvoidIntersectionsMode mode )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mAvoidIntersectionsMode == mode )
    return;

  mAvoidIntersectionsMode = mode;
  emit avoidIntersectionsModeChanged();
}

static  QgsMapLayer::ReadFlags projectFlagsToLayerReadFlags( Qgis::ProjectReadFlags projectReadFlags, Qgis::ProjectFlags projectFlags )
{
  QgsMapLayer::ReadFlags layerFlags = QgsMapLayer::ReadFlags();
  // Propagate don't resolve layers
  if ( projectReadFlags & Qgis::ProjectReadFlag::DontResolveLayers )
    layerFlags |= QgsMapLayer::FlagDontResolveLayers;
  // Propagate trust layer metadata flag
  // Propagate read extent from XML based trust layer metadata flag
  if ( ( projectFlags & Qgis::ProjectFlag::TrustStoredLayerStatistics ) || ( projectReadFlags & Qgis::ProjectReadFlag::TrustLayerMetadata ) )
  {
    layerFlags |= QgsMapLayer::FlagTrustLayerMetadata;
    layerFlags |= QgsMapLayer::FlagReadExtentFromXml;
  }
  // Propagate open layers in read-only mode
  if ( ( projectReadFlags & Qgis::ProjectReadFlag::ForceReadOnlyLayers ) )
    layerFlags |= QgsMapLayer::FlagForceReadOnly;

  return layerFlags;
}

struct LayerToLoad
{
  QString layerId;
  QString provider;
  QString dataSource;
  QgsDataProvider::ProviderOptions options;
  QgsDataProvider::ReadFlags flags;
  QDomElement layerElement;
};

void QgsProject::preloadProviders( const QVector<QDomNode> &parallelLayerNodes,
                                   const QgsReadWriteContext &context,
                                   QMap<QString, QgsDataProvider *> &loadedProviders,
                                   QgsMapLayer::ReadFlags layerReadFlags,
                                   int totalProviderCount )
{
  int i = 0;
  QEventLoop loop;

  QMap<QString, LayerToLoad> layersToLoad;

  for ( const QDomNode &node : parallelLayerNodes )
  {
    LayerToLoad layerToLoad;

    const QDomElement layerElement = node.toElement();
    layerToLoad.layerElement = layerElement;
    layerToLoad.layerId = layerElement.namedItem( QStringLiteral( "id" ) ).toElement().text();
    layerToLoad.provider = layerElement.namedItem( QStringLiteral( "provider" ) ).toElement().text();
    layerToLoad.dataSource = layerElement.namedItem( QStringLiteral( "datasource" ) ).toElement().text();

    layerToLoad.dataSource = QgsProviderRegistry::instance()->relativeToAbsoluteUri( layerToLoad.provider, layerToLoad.dataSource, context );

    layerToLoad.options = QgsDataProvider::ProviderOptions( {context.transformContext()} );
    layerToLoad.flags = QgsMapLayer::providerReadFlags( node, layerReadFlags );

    // Requesting credential from worker thread could lead to deadlocks because the main thread is waiting for worker thread to fininsh
    layerToLoad.flags.setFlag( QgsDataProvider::SkipCredentialsRequest, true );
    layerToLoad.flags.setFlag( QgsDataProvider::ParallelThreadLoading, true );

    layersToLoad.insert( layerToLoad.layerId, layerToLoad );
  }

  while ( !layersToLoad.isEmpty() )
  {
    const QList<LayerToLoad> layersToAttemptInParallel = layersToLoad.values();
    QString layerToAttemptInMainThread;

    QHash<QString, QgsRunnableProviderCreator *> runnables;
    QThreadPool threadPool;
    threadPool.setMaxThreadCount( QgsSettingsRegistryCore::settingsLayerParallelLoadingMaxCount->value() );

    for ( const LayerToLoad &lay : layersToAttemptInParallel )
    {
      QgsRunnableProviderCreator *run = new QgsRunnableProviderCreator( lay.layerId, lay.provider, lay.dataSource, lay.options, lay.flags );
      runnables.insert( lay.layerId, run );

      QObject::connect( run, &QgsRunnableProviderCreator::providerCreated, run, [&]( bool isValid, const QString & layId )
      {
        if ( isValid )
        {
          layersToLoad.remove( layId );
          i++;
          QgsRunnableProviderCreator *finishedRun = runnables.value( layId, nullptr );
          Q_ASSERT( finishedRun );

          std::unique_ptr<QgsDataProvider> provider( finishedRun->dataProvider() );
          Q_ASSERT( provider && provider->isValid() );

          loadedProviders.insert( layId, provider.release() );
          emit layerLoaded( i, totalProviderCount );
        }
        else
        {
          if ( layerToAttemptInMainThread.isEmpty() )
            layerToAttemptInMainThread = layId;
          threadPool.clear(); //we have to stop all loading provider to try this layer in main thread and maybe have credentials
        }

        if ( i == parallelLayerNodes.count() || !isValid )
          loop.quit();
      } );
      threadPool.start( run );
    }
    loop.exec();

    threadPool.waitForDone(); // to be sure all threads are finished

    qDeleteAll( runnables );

    // We try with the first layer returned invalid but this time in the main thread to maybe have credentials and continue with others not loaded in parallel
    auto it = layersToLoad.find( layerToAttemptInMainThread );
    if ( it != layersToLoad.end() )
    {
      std::unique_ptr<QgsDataProvider> provider;
      QString layerId;
      {
        const LayerToLoad &lay = it.value();
        QgsDataProvider::ReadFlags providerFlags = lay.flags;
        providerFlags.setFlag( QgsDataProvider::SkipCredentialsRequest, false );
        providerFlags.setFlag( QgsDataProvider::ParallelThreadLoading, false );
        QgsScopedRuntimeProfile profile( "Create data providers/" + lay.layerId, QStringLiteral( "projectload" ) );
        provider.reset( QgsProviderRegistry::instance()->createProvider( lay.provider, lay.dataSource, lay.options, providerFlags ) );
        i++;
        if ( provider && provider->isValid() )
        {
          emit layerLoaded( i, totalProviderCount );
        }
        layerId = lay.layerId;
        layersToLoad.erase( it );
        // can't access "lay" anymore -- it's now been freed
      }
      loadedProviders.insert( layerId, provider.release() );
    }

    // if there still are some not loaded providers or some invalid in parallel thread we start again
  }

}

void QgsProject::releaseHandlesToProjectArchive()
{
  mStyleSettings->removeProjectStyle();
}

bool QgsProject::rebuildCrs3D( QString *error )
{
  bool res = true;
  if ( !mCrs.isValid() )
  {
    mCrs3D = QgsCoordinateReferenceSystem();
  }
  else if ( !mVerticalCrs.isValid() )
  {
    mCrs3D = mCrs;
  }
  else
  {
    switch ( mCrs.type() )
    {
      case Qgis::CrsType::Compound:
      case Qgis::CrsType::Geographic3d:
      case Qgis::CrsType::Geocentric:
        mCrs3D = mCrs;
        break;

      case Qgis::CrsType::Projected:
      {
        QString tempError;
        mCrs3D = mCrs.hasVerticalAxis() ? mCrs : QgsCoordinateReferenceSystem::createCompoundCrs( mCrs, mVerticalCrs, error ? *error : tempError );
        res = mCrs3D.isValid();
        break;
      }

      case Qgis::CrsType::Vertical:
        // nonsense situation
        mCrs3D = QgsCoordinateReferenceSystem();
        res = false;
        break;

      case Qgis::CrsType::Unknown:
      case Qgis::CrsType::Geodetic:
      case Qgis::CrsType::Geographic2d:
      case Qgis::CrsType::Temporal:
      case Qgis::CrsType::Engineering:
      case Qgis::CrsType::Bound:
      case Qgis::CrsType::Other:
      case Qgis::CrsType::DerivedProjected:
      {
        QString tempError;
        mCrs3D = QgsCoordinateReferenceSystem::createCompoundCrs( mCrs, mVerticalCrs, error ? *error : tempError );
        res = mCrs3D.isValid();
        break;
      }
    }
  }
  return res;
}

bool QgsProject::_getMapLayers( const QDomDocument &doc, QList<QDomNode> &brokenNodes, Qgis::ProjectReadFlags flags )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  // Layer order is set by the restoring the legend settings from project file.
  // This is done on the 'readProject( ... )' signal

  QDomElement layerElement = doc.documentElement().firstChildElement( QStringLiteral( "projectlayers" ) ).firstChildElement( QStringLiteral( "maplayer" ) );

  // process the map layer nodes

  if ( layerElement.isNull() )      // if we have no layers to process, bail
  {
    return true; // Decided to return "true" since it's
    // possible for there to be a project with no
    // layers; but also, more imporantly, this
    // would cause the tests/qgsproject to fail
    // since the test suite doesn't currently
    // support test layers
  }

  bool returnStatus = true;
  int numLayers = 0;

  while ( ! layerElement.isNull() )
  {
    numLayers++;
    layerElement = layerElement.nextSiblingElement( QStringLiteral( "maplayer" ) );
  }

  // order layers based on their dependencies
  QgsScopedRuntimeProfile profile( tr( "Sorting layers" ), QStringLiteral( "projectload" ) );
  const QgsLayerDefinition::DependencySorter depSorter( doc );
  if ( depSorter.hasCycle() )
    return false;

  // Missing a dependency? We still load all the layers, otherwise the project is completely broken!
  if ( depSorter.hasMissingDependency() )
    returnStatus = false;

  emit layerLoaded( 0, numLayers );

  const QVector<QDomNode> sortedLayerNodes = depSorter.sortedLayerNodes();
  const int totalLayerCount = sortedLayerNodes.count();

  QVector<QDomNode> parallelLoading;
  QMap<QString, QgsDataProvider *> loadedProviders;

  if ( QgsSettingsRegistryCore::settingsLayerParallelLoading->value() )
  {
    profile.switchTask( tr( "Load providers in parallel" ) );
    for ( const QDomNode &node : sortedLayerNodes )
    {
      const QDomElement element = node.toElement();
      if ( element.attribute( QStringLiteral( "embedded" ) ) != QLatin1String( "1" ) )
      {
        const QString layerId = node.namedItem( QStringLiteral( "id" ) ).toElement().text();
        if ( !depSorter.isLayerDependent( layerId ) )
        {
          const QDomNode mnl = element.namedItem( QStringLiteral( "provider" ) );
          const QDomElement mne = mnl.toElement();
          const QString provider = mne.text();
          QgsProviderMetadata *meta = QgsProviderRegistry::instance()->providerMetadata( provider );
          if ( meta && meta->providerCapabilities().testFlag( QgsProviderMetadata::ParallelCreateProvider ) )
          {
            parallelLoading.append( node );
            continue;
          }
        }
      }
    }

    QgsReadWriteContext context;
    context.setPathResolver( pathResolver() );
    if ( !parallelLoading.isEmpty() )
      preloadProviders( parallelLoading, context, loadedProviders, projectFlagsToLayerReadFlags( flags, mFlags ), sortedLayerNodes.count() );
  }

  int i = loadedProviders.count();
  for ( const QDomNode &node : std::as_const( sortedLayerNodes ) )
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
      QString layerId = element.namedItem( QStringLiteral( "id" ) ).toElement().text();

      if ( !addLayer( element, brokenNodes, context, flags, loadedProviders.take( layerId ) ) )
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

bool QgsProject::addLayer( const QDomElement &layerElem,
                           QList<QDomNode> &brokenNodes,
                           QgsReadWriteContext &context,
                           Qgis::ProjectReadFlags flags,
                           QgsDataProvider *provider )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QString type = layerElem.attribute( QStringLiteral( "type" ) );
  QgsDebugMsgLevel( "Layer type is " + type, 4 );
  std::unique_ptr<QgsMapLayer> mapLayer;

  QgsScopedRuntimeProfile profile( tr( "Create layer" ), QStringLiteral( "projectload" ) );

  bool ok = false;
  const Qgis::LayerType layerType( QgsMapLayerFactory::typeFromString( type, ok ) );
  if ( !ok )
  {
    QgsDebugError( QStringLiteral( "Unknown layer type \"%1\"" ).arg( type ) );
    return false;
  }

  switch ( layerType )
  {
    case Qgis::LayerType::Vector:
      mapLayer = std::make_unique<QgsVectorLayer>();
      break;

    case Qgis::LayerType::Raster:
      mapLayer = std::make_unique<QgsRasterLayer>();
      break;

    case Qgis::LayerType::Mesh:
      mapLayer = std::make_unique<QgsMeshLayer>();
      break;

    case Qgis::LayerType::VectorTile:
      mapLayer = std::make_unique<QgsVectorTileLayer>();
      break;

    case Qgis::LayerType::PointCloud:
      mapLayer = std::make_unique<QgsPointCloudLayer>();
      break;

    case Qgis::LayerType::TiledScene:
      mapLayer = std::make_unique<QgsTiledSceneLayer>();
      break;

    case Qgis::LayerType::Plugin:
    {
      const QString typeName = layerElem.attribute( QStringLiteral( "name" ) );
      mapLayer.reset( QgsApplication::pluginLayerRegistry()->createLayer( typeName ) );
      break;
    }

    case Qgis::LayerType::Annotation:
    {
      const QgsAnnotationLayer::LayerOptions options( mTransformContext );
      mapLayer = std::make_unique<QgsAnnotationLayer>( QString(), options );
      break;
    }

    case Qgis::LayerType::Group:
    {
      const QgsGroupLayer::LayerOptions options( mTransformContext );
      mapLayer = std::make_unique<QgsGroupLayer>( QString(), options );
      break;
    }
  }

  if ( !mapLayer )
  {
    QgsDebugError( QStringLiteral( "Unable to create layer" ) );
    return false;
  }

  Q_CHECK_PTR( mapLayer ); // NOLINT

  // This is tricky: to avoid a leak we need to check if the layer was already in the store
  // because if it was, the newly created layer will not be added to the store and it would leak.
  const QString layerId { layerElem.namedItem( QStringLiteral( "id" ) ).toElement().text() };
  Q_ASSERT( ! layerId.isEmpty() );
  const bool layerWasStored { layerStore()->mapLayer( layerId ) != nullptr };

  // have the layer restore state that is stored in Dom node
  QgsMapLayer::ReadFlags layerFlags = projectFlagsToLayerReadFlags( flags, mFlags );

  profile.switchTask( tr( "Load layer source" ) );
  const bool layerIsValid = mapLayer->readLayerXml( layerElem, context, layerFlags, provider ) && mapLayer->isValid();

  // apply specific settings to vector layer
  if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( mapLayer.get() ) )
  {
    vl->setReadExtentFromXml( layerFlags & QgsMapLayer::FlagReadExtentFromXml );
    if ( vl->dataProvider() )
    {
      const bool evaluateDefaultValues = mFlags & Qgis::ProjectFlag::EvaluateDefaultValuesOnProviderSide;
      vl->dataProvider()->setProviderProperty( QgsVectorDataProvider::EvaluateDefaultValues, evaluateDefaultValues );
    }
  }

  profile.switchTask( tr( "Add layer to project" ) );
  QList<QgsMapLayer *> newLayers;
  newLayers << mapLayer.get();
  if ( layerIsValid || flags & Qgis::ProjectReadFlag::DontResolveLayers )
  {
    emit readMapLayer( mapLayer.get(), layerElem );
    addMapLayers( newLayers );
    // Try to resolve references here (this is necessary to set up joined fields that will be possibly used by
    // virtual layers that point to this layer's joined field in their query otherwise they won't be valid ),
    // a second attempt to resolve references will be done after all layers are loaded
    // see https://github.com/qgis/QGIS/issues/46834
    if ( QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( mapLayer.get() ) )
    {
      vLayer->joinBuffer()->resolveReferences( this );
    }
  }
  else
  {
    // It's a bad layer: do not add to legend (the user will decide if she wants to do so)
    addMapLayers( newLayers, false );
    newLayers.first();
    QgsDebugError( "Unable to load " + type + " layer" );
    brokenNodes.push_back( layerElem );
  }

  const bool wasEditable = layerElem.attribute( QStringLiteral( "editable" ), QStringLiteral( "0" ) ).toInt();
  if ( wasEditable )
  {
    mapLayer->setCustomProperty( QStringLiteral( "_layer_was_editable" ), true );
  }
  else
  {
    mapLayer->removeCustomProperty( QStringLiteral( "_layer_was_editable" ) );
  }

  // It should be safe to delete the layer now if layer was stored, because all the store
  // had to to was to reset the data source in case the validity changed.
  if ( ! layerWasStored )
  {
    mapLayer.release();
  }

  return layerIsValid;
}

bool QgsProject::read( const QString &filename, Qgis::ProjectReadFlags flags )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mFile.setFileName( filename );
  mCachedHomePath.clear();
  mProjectScope.reset();

  return read( flags );
}

bool QgsProject::read( Qgis::ProjectReadFlags flags )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QString filename = mFile.fileName();
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
      const QFileInfo finfo( mFile.fileName() );
      const QString attachmentsZip = finfo.absoluteDir().absoluteFilePath( QStringLiteral( "%1_attachments.zip" ).arg( finfo.completeBaseName() ) );
      if ( QFile( attachmentsZip ).exists() )
      {
        std::unique_ptr<QgsArchive> archive( new QgsArchive() );
        if ( archive->unzip( attachmentsZip ) )
        {
          releaseHandlesToProjectArchive();
          mArchive = std::move( archive );
        }
      }
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

bool QgsProject::readProjectFile( const QString &filename, Qgis::ProjectReadFlags flags )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  // avoid multiple emission of snapping updated signals
  ScopedIntIncrementor snapSignalBlock( &mBlockSnappingUpdates );

  QFile projectFile( filename );
  clearError();

  QgsApplication::profiler()->clear( QStringLiteral( "projectload" ) );
  QgsScopedRuntimeProfile profile( tr( "Setting up translations" ), QStringLiteral( "projectload" ) );

  const QString localeFileName = QStringLiteral( "%1_%2" ).arg( QFileInfo( projectFile.fileName() ).baseName(), QgsApplication::settingsLocaleUserLocale->value() );

  if ( QFile( QStringLiteral( "%1/%2.qm" ).arg( QFileInfo( projectFile.fileName() ).absolutePath(), localeFileName ) ).exists() )
  {
    mTranslator.reset( new QTranslator() );
    ( void )mTranslator->load( localeFileName, QFileInfo( projectFile.fileName() ).absolutePath() );
  }

  profile.switchTask( tr( "Reading project file" ) );
  std::unique_ptr<QDomDocument> doc( new QDomDocument( QStringLiteral( "qgis" ) ) );

  if ( !projectFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    projectFile.close();

    setError( tr( "Unable to open %1" ).arg( projectFile.fileName() ) );

    return false;
  }

  QTextStream textStream( &projectFile );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  textStream.setCodec( "UTF-8" );
#endif
  QString projectString = textStream.readAll();
  projectFile.close();

  for ( int i = 0; i < 32; i++ )
  {
    if ( i == 9 || i == 10 || i == 13 )
    {
      continue;
    }
    projectString.replace( QChar( i ), QStringLiteral( "%1%2%1" ).arg( FONTMARKER_CHR_FIX, QString::number( i ) ) );
  }

  // location of problem associated with errorMsg
  int line, column;
  QString errorMsg;
  if ( !doc->setContent( projectString, &errorMsg, &line, &column ) )
  {
    const QString errorString = tr( "Project file read error in file %1: %2 at line %3 column %4" )
                                .arg( projectFile.fileName(), errorMsg ).arg( line ).arg( column );
    QgsDebugError( errorString );
    setError( errorString );

    return false;
  }

  projectFile.close();

  QgsDebugMsgLevel( "Opened document " + projectFile.fileName(), 2 );

  // get project version string, if any
  const QgsProjectVersion fileVersion = getVersion( *doc );
  const QgsProjectVersion thisVersion( Qgis::version() );

  profile.switchTask( tr( "Updating project file" ) );
  if ( thisVersion > fileVersion )
  {
    const bool isOlderMajorVersion = fileVersion.majorVersion() < thisVersion.majorVersion();

    if ( isOlderMajorVersion )
    {
      QgsLogger::warning( "Loading a file that was saved with an older "
                          "version of qgis (saved in " + fileVersion.text() +
                          ", loaded in " + Qgis::version() +
                          "). Problems may occur." );
    }

    QgsProjectFileTransform projectFile( *doc, fileVersion );

    // Shows a warning when an old project file is read.
    Q_NOWARN_DEPRECATED_PUSH
    emit oldProjectVersionWarning( fileVersion.text() );
    Q_NOWARN_DEPRECATED_POP
    emit readVersionMismatchOccurred( fileVersion.text() );

    projectFile.updateRevision( thisVersion );
  }
  else if ( fileVersion > thisVersion )
  {
    QgsLogger::warning( "Loading a file that was saved with a newer "
                        "version of qgis (saved in " + fileVersion.text() +
                        ", loaded in " + Qgis::version() +
                        "). Problems may occur." );

    emit readVersionMismatchOccurred( fileVersion.text() );
  }

  // start new project, just keep the file name and auxiliary storage
  profile.switchTask( tr( "Creating auxiliary storage" ) );
  const QString fileName = mFile.fileName();

  const QgsCoordinateReferenceSystem oldVerticalCrs = verticalCrs();
  const QgsCoordinateReferenceSystem oldCrs3D = mCrs3D;

  // NOTE [ND] -- I suspect this is wrong, as the archive may contain any number of non-auxiliary
  // storage related files from the previously loaded project.
  std::unique_ptr<QgsAuxiliaryStorage> aStorage = std::move( mAuxiliaryStorage );
  std::unique_ptr<QgsArchive> archive = std::move( mArchive );

  // don't emit xxxChanged signals during the clear() call, as we'll be emitting
  // them again after reading the properties from the project file
  mBlockChangeSignalsDuringClear = true;
  clear();
  mBlockChangeSignalsDuringClear = false;

  // this is ugly, but clear() will have created a new archive and started populating it. We
  // need to release handles to this archive now as the subsequent call to move will need
  // to delete it, and requires free access to do so.
  releaseHandlesToProjectArchive();

  mAuxiliaryStorage = std::move( aStorage );
  mArchive = std::move( archive );

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

  const QDomNodeList homePathNl = doc->elementsByTagName( QStringLiteral( "homePath" ) );
  if ( homePathNl.count() > 0 )
  {
    const QDomElement homePathElement = homePathNl.at( 0 ).toElement();
    const QString homePath = homePathElement.attribute( QStringLiteral( "path" ) );
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


  const QString distanceUnitString = readEntry( QStringLiteral( "Measurement" ), QStringLiteral( "/DistanceUnits" ), QString() );
  if ( !distanceUnitString.isEmpty() )
    setDistanceUnits( QgsUnitTypes::decodeDistanceUnit( distanceUnitString ) );

  const QString areaUnitString = readEntry( QStringLiteral( "Measurement" ), QStringLiteral( "/AreaUnits" ), QString() );
  if ( !areaUnitString.isEmpty() )
    setAreaUnits( QgsUnitTypes::decodeAreaUnit( areaUnitString ) );

  QgsReadWriteContext context;
  context.setPathResolver( pathResolver() );
  context.setProjectTranslator( this );

  //crs
  QgsCoordinateReferenceSystem projectCrs;
  if ( readNumEntry( QStringLiteral( "SpatialRefSys" ), QStringLiteral( "/ProjectionsEnabled" ), 0 ) )
  {
    // first preference - dedicated projectCrs node
    const QDomNode srsNode = doc->documentElement().namedItem( QStringLiteral( "projectCrs" ) );
    if ( !srsNode.isNull() )
    {
      projectCrs.readXml( srsNode );
    }

    if ( !projectCrs.isValid() )
    {
      const QString projCrsString = readEntry( QStringLiteral( "SpatialRefSys" ), QStringLiteral( "/ProjectCRSProj4String" ) );
      const long currentCRS = readNumEntry( QStringLiteral( "SpatialRefSys" ), QStringLiteral( "/ProjectCRSID" ), -1 );
      const QString authid = readEntry( QStringLiteral( "SpatialRefSys" ), QStringLiteral( "/ProjectCrs" ) );

      // authid should be prioritized over all
      const bool isUserAuthId = authid.startsWith( QLatin1String( "USER:" ), Qt::CaseInsensitive );
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

  //vertical CRS
  {
    QgsCoordinateReferenceSystem verticalCrs;
    const QDomNode verticalCrsNode = doc->documentElement().namedItem( QStringLiteral( "verticalCrs" ) );
    if ( !verticalCrsNode.isNull() )
    {
      verticalCrs.readXml( verticalCrsNode );
    }
    mVerticalCrs = verticalCrs;
  }
  rebuildCrs3D();

  QStringList datumErrors;
  if ( !mTransformContext.readXml( doc->documentElement(), context, datumErrors ) && !datumErrors.empty() )
  {
    emit missingDatumTransforms( datumErrors );
  }
  emit transformContextChanged();

  // map shading
  const QDomNode elevationShadingNode = doc->documentElement().namedItem( QStringLiteral( "elevation-shading-renderer" ) );
  if ( !elevationShadingNode.isNull() )
  {
    mElevationShadingRenderer.readXml( elevationShadingNode.toElement(), context );
  }
  emit elevationShadingRendererChanged();


  //add variables defined in project file - do this early in the reading cycle, as other components
  //(e.g. layouts) may depend on these variables
  const QStringList variableNames = readListEntry( QStringLiteral( "Variables" ), QStringLiteral( "/variableNames" ) );
  const QStringList variableValues = readListEntry( QStringLiteral( "Variables" ), QStringLiteral( "/variableValues" ) );

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

  QDomElement element = doc->documentElement().firstChildElement( QStringLiteral( "projectMetadata" ) );

  if ( !element.isNull() )
  {
    mMetadata.readMetadataXml( element );
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

  // Transaction mode
  element = doc->documentElement().firstChildElement( QStringLiteral( "transaction" ) );
  if ( !element.isNull() )
  {
    mTransactionMode = qgsEnumKeyToValue( element.attribute( QStringLiteral( "mode" ) ), Qgis::TransactionMode::Disabled );
  }
  else
  {
    // maybe older project => try read autotransaction
    element = doc->documentElement().firstChildElement( QStringLiteral( "autotransaction" ) );
    if ( ! element.isNull() )
    {
      mTransactionMode = static_cast<Qgis::TransactionMode>( element.attribute( QStringLiteral( "active" ), QStringLiteral( "0" ) ).toInt() );
    }
  }

  // read the layer tree from project file
  profile.switchTask( tr( "Loading layer tree" ) );
  mRootGroup->setCustomProperty( QStringLiteral( "loading" ), 1 );

  QDomElement layerTreeElem = doc->documentElement().firstChildElement( QStringLiteral( "layer-tree-group" ) );
  if ( !layerTreeElem.isNull() )
  {
    // Use a temporary tree to read the nodes to prevent signals being delivered to the models
    QgsLayerTree tempTree;
    tempTree.readChildrenFromXml( layerTreeElem, context );
    mRootGroup->insertChildNodes( -1, tempTree.abandonChildren() );
  }
  else
  {
    QgsLayerTreeUtils::readOldLegend( mRootGroup, doc->documentElement().firstChildElement( QStringLiteral( "legend" ) ) );
  }

  mLayerTreeRegistryBridge->setEnabled( false );

  // get the map layers
  profile.switchTask( tr( "Reading map layers" ) );

  loadProjectFlags( doc.get() );

  QList<QDomNode> brokenNodes;
  const bool clean = _getMapLayers( *doc, brokenNodes, flags );

  // review the integrity of the retrieved map layers
  if ( !clean && !( flags & Qgis::ProjectReadFlag::DontResolveLayers ) )
  {
    QgsDebugError( QStringLiteral( "Unable to get map layers from project file." ) );

    if ( !brokenNodes.isEmpty() )
    {
      QgsDebugError( "there are " + QString::number( brokenNodes.size() ) + " broken layers" );
    }

    // we let a custom handler decide what to do with missing layers
    // (default implementation ignores them, there's also a GUI handler that lets user choose correct path)
    mBadLayerHandler->handleBadLayers( brokenNodes );
  }

  mMainAnnotationLayer->readLayerXml( doc->documentElement().firstChildElement( QStringLiteral( "main-annotation-layer" ) ), context );
  mMainAnnotationLayer->setTransformContext( mTransformContext );

  // load embedded groups and layers
  profile.switchTask( tr( "Loading embedded layers" ) );
  loadEmbeddedNodes( mRootGroup, flags );

  // Resolve references to other layers
  // Needs to be done here once all dependent layers are loaded
  profile.switchTask( tr( "Resolving layer references" ) );
  QMap<QString, QgsMapLayer *> layers = mLayerStore->mapLayers();
  for ( QMap<QString, QgsMapLayer *>::iterator it = layers.begin(); it != layers.end(); ++it )
  {
    it.value()->resolveReferences( this );
  }

  mLayerTreeRegistryBridge->setEnabled( true );

  // now that layers are loaded, we can resolve layer tree's references to the layers
  profile.switchTask( tr( "Resolving references" ) );
  mRootGroup->resolveReferences( this );

  // we need to migrate old fashion designed QgsSymbolLayerReference to new ones
  if ( QgsProjectVersion( 3, 28, 0 ) > mSaveVersion )
  {
    Q_NOWARN_DEPRECATED_PUSH
    QgsProjectFileTransform::fixOldSymbolLayerReferences( mapLayers() );
    Q_NOWARN_DEPRECATED_POP
  }

  if ( !layerTreeElem.isNull() )
  {
    mRootGroup->readLayerOrderFromXml( layerTreeElem );
  }

  // Load pre 3.0 configuration
  const QDomElement layerTreeCanvasElem = doc->documentElement().firstChildElement( QStringLiteral( "layer-tree-canvas" ) );
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

  // Convert pre 3.26 default styles
  if ( QgsProjectVersion( 3, 26, 0 ) > mSaveVersion )
  {
    // Convert default symbols
    QString styleName = readEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/Marker" ) );
    if ( !styleName.isEmpty() )
    {
      std::unique_ptr<QgsSymbol> symbol( QgsStyle::defaultStyle()->symbol( styleName ) );
      styleSettings()->setDefaultSymbol( Qgis::SymbolType::Marker, symbol.get() );
    }
    styleName = readEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/Line" ) );
    if ( !styleName.isEmpty() )
    {
      std::unique_ptr<QgsSymbol> symbol( QgsStyle::defaultStyle()->symbol( styleName ) );
      styleSettings()->setDefaultSymbol( Qgis::SymbolType::Line, symbol.get() );
    }
    styleName = readEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/Fill" ) );
    if ( !styleName.isEmpty() )
    {
      std::unique_ptr<QgsSymbol> symbol( QgsStyle::defaultStyle()->symbol( styleName ) );
      styleSettings()->setDefaultSymbol( Qgis::SymbolType::Fill, symbol.get() );
    }
    styleName = readEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/ColorRamp" ) );
    if ( !styleName.isEmpty() )
    {
      std::unique_ptr<QgsColorRamp> colorRamp( QgsStyle::defaultStyle()->colorRamp( styleName ) );
      styleSettings()->setDefaultColorRamp( colorRamp.get() );
    }

    // Convert randomize default symbol fill color
    styleSettings()->setRandomizeDefaultSymbolColor( readBoolEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/RandomColors" ), true ) );

    // Convert default symbol opacity
    double opacity = 1.0;
    bool ok = false;
    // upgrade old setting
    double alpha = readDoubleEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/AlphaInt" ), 255, &ok );
    if ( ok )
      opacity = alpha / 255.0;
    double newOpacity = readDoubleEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/Opacity" ), 1.0, &ok );
    if ( ok )
      opacity = newOpacity;
    styleSettings()->setDefaultSymbolOpacity( opacity );

    // Cleanup
    removeEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/Marker" ) );
    removeEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/Line" ) );
    removeEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/Fill" ) );
    removeEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/ColorRamp" ) );
    removeEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/RandomColors" ) );
    removeEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/AlphaInt" ) );
    removeEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/Opacity" ) );
  }

  // After bad layer handling we might still have invalid layers,
  // store them in case the user wanted to handle them later
  // or wanted to pass them through when saving
  if ( !( flags & Qgis::ProjectReadFlag::DontStoreOriginalStyles ) )
  {
    profile.switchTask( tr( "Storing original layer properties" ) );
    QgsLayerTreeUtils::storeOriginalLayersProperties( mRootGroup, doc.get() );
  }

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
  if ( !( flags & Qgis::ProjectReadFlag::DontLoadLayouts ) )
  {
    profile.switchTask( tr( "Loading layouts" ) );
    mLayoutManager->readXml( doc->documentElement(), *doc );
  }

  if ( !( flags & Qgis::ProjectReadFlag::DontLoad3DViews ) )
  {
    profile.switchTask( tr( "Loading 3D Views" ) );
    m3DViewsManager->readXml( doc->documentElement(), *doc );
  }

  profile.switchTask( tr( "Loading bookmarks" ) );
  mBookmarkManager->readXml( doc->documentElement(), *doc );

  profile.switchTask( tr( "Loading sensors" ) );
  mSensorManager->readXml( doc->documentElement(), *doc );

  // reassign change dependencies now that all layers are loaded
  QMap<QString, QgsMapLayer *> existingMaps = mapLayers();
  for ( QMap<QString, QgsMapLayer *>::iterator it = existingMaps.begin(); it != existingMaps.end(); ++it )
  {
    it.value()->setDependencies( it.value()->dependencies() );
  }

  profile.switchTask( tr( "Loading snapping settings" ) );
  mSnappingConfig.readProject( *doc );
  mAvoidIntersectionsMode = static_cast<Qgis::AvoidIntersectionsMode>( readNumEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/AvoidIntersectionsMode" ), static_cast<int>( Qgis::AvoidIntersectionsMode::AvoidIntersectionsLayers ) ) );

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
  const QDomElement viewSettingsElement = doc->documentElement().firstChildElement( QStringLiteral( "ProjectViewSettings" ) );
  if ( !viewSettingsElement.isNull() )
    mViewSettings->readXml( viewSettingsElement, context );

  // restore style settings
  profile.switchTask( tr( "Loading style properties" ) );
  const QDomElement styleSettingsElement = doc->documentElement().firstChildElement( QStringLiteral( "ProjectStyleSettings" ) );
  if ( !styleSettingsElement.isNull() )
  {
    mStyleSettings->removeProjectStyle();
    mStyleSettings->readXml( styleSettingsElement, context, flags );
  }

  // restore time settings
  profile.switchTask( tr( "Loading temporal settings" ) );
  const QDomElement timeSettingsElement = doc->documentElement().firstChildElement( QStringLiteral( "ProjectTimeSettings" ) );
  if ( !timeSettingsElement.isNull() )
    mTimeSettings->readXml( timeSettingsElement, context );


  profile.switchTask( tr( "Loading elevation properties" ) );
  const QDomElement elevationPropertiesElement = doc->documentElement().firstChildElement( QStringLiteral( "ElevationProperties" ) );
  if ( !elevationPropertiesElement.isNull() )
    mElevationProperties->readXml( elevationPropertiesElement, context );
  mElevationProperties->resolveReferences( this );

  profile.switchTask( tr( "Loading display settings" ) );
  {
    const QDomElement displaySettingsElement = doc->documentElement().firstChildElement( QStringLiteral( "ProjectDisplaySettings" ) );
    if ( !displaySettingsElement.isNull() )
      mDisplaySettings->readXml( displaySettingsElement, context );
  }

  profile.switchTask( tr( "Loading GPS settings" ) );
  {
    const QDomElement gpsSettingsElement = doc->documentElement().firstChildElement( QStringLiteral( "ProjectGpsSettings" ) );
    if ( !gpsSettingsElement.isNull() )
      mGpsSettings->readXml( gpsSettingsElement, context );
    mGpsSettings->resolveReferences( this );
  }

  profile.switchTask( tr( "Updating variables" ) );
  emit customVariablesChanged();
  profile.switchTask( tr( "Updating CRS" ) );
  emit crsChanged();
  if ( verticalCrs() != oldVerticalCrs )
    emit verticalCrsChanged();
  if ( mCrs3D != oldCrs3D )
    emit crs3DChanged();
  emit ellipsoidChanged( ellipsoid() );

  // read the project: used by map canvas and legend
  profile.switchTask( tr( "Reading external settings" ) );
  emit readProject( *doc );
  emit readProjectWithContext( *doc, context );

  profile.switchTask( tr( "Updating interface" ) );

  snapSignalBlock.release();
  if ( !mBlockSnappingUpdates )
    emit snappingConfigChanged( mSnappingConfig );

  emit avoidIntersectionsModeChanged();
  emit topologicalEditingChanged();
  emit projectColorsChanged();

  // if all went well, we're allegedly in pristine state
  if ( clean )
    setDirty( false );

  QgsDebugMsgLevel( QStringLiteral( "Project save user: %1" ).arg( mSaveUser ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Project save user: %1" ).arg( mSaveUserFull ), 2 );

  Q_NOWARN_DEPRECATED_PUSH
  emit nonIdentifiableLayersChanged( nonIdentifiableLayers() );
  Q_NOWARN_DEPRECATED_POP

  if ( mTranslator )
  {
    //project possibly translated -> rename it with locale postfix
    const QString newFileName( QStringLiteral( "%1/%2.qgs" ).arg( QFileInfo( projectFile.fileName() ).absolutePath(), localeFileName ) );
    setFileName( newFileName );

    if ( write() )
    {
      setTitle( localeFileName );
      QgsMessageLog::logMessage( tr( "Translated project saved with locale prefix %1" ).arg( newFileName ), QObject::tr( "Project translation" ), Qgis::MessageLevel::Success );
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Error saving translated project with locale prefix %1" ).arg( newFileName ), QObject::tr( "Project translation" ), Qgis::MessageLevel::Critical );
    }
  }

  // lastly, make any previously editable layers editable
  const QMap<QString, QgsMapLayer *> loadedLayers = mapLayers();
  for ( auto it = loadedLayers.constBegin(); it != loadedLayers.constEnd(); ++it )
  {
    if ( it.value()->isValid() && it.value()->customProperty( QStringLiteral( "_layer_was_editable" ) ).toBool() )
    {
      if ( QgsVectorLayer *vl = qobject_cast< QgsVectorLayer * >( it.value() ) )
        vl->startEditing();
      it.value()->removeCustomProperty( QStringLiteral( "_layer_was_editable" ) );
    }
  }

  return true;
}

bool QgsProject::loadEmbeddedNodes( QgsLayerTreeGroup *group, Qgis::ProjectReadFlags flags )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
        const QString projectPath = readPath( childGroup->customProperty( QStringLiteral( "embedded_project" ) ).toString() );
        childGroup->setCustomProperty( QStringLiteral( "embedded_project" ), projectPath );
        QgsLayerTreeGroup *newGroup = createEmbeddedGroup( childGroup->name(), projectPath, childGroup->customProperty( QStringLiteral( "embedded-invisible-layers" ) ).toStringList(), flags );
        if ( newGroup )
        {
          QList<QgsLayerTreeNode *> clonedChildren;
          const QList<QgsLayerTreeNode *> constChildren = newGroup->children();
          clonedChildren.reserve( constChildren.size() );
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
  // this method is called quite extensively from other threads via QgsProject::createExpressionContextScope()
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mCustomVariables;
}

void QgsProject::setCustomVariables( const QVariantMap &variables )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  *mLabelingEngineSettings = settings;
  emit labelingEngineSettingsChanged();
}

const QgsLabelingEngineSettings &QgsProject::labelingEngineSettings() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return *mLabelingEngineSettings;
}

QgsMapLayerStore *QgsProject::layerStore()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mProjectScope.reset();
  return mLayerStore.get();
}

const QgsMapLayerStore *QgsProject::layerStore() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mLayerStore.get();
}

QList<QgsVectorLayer *> QgsProject::avoidIntersectionsLayers() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QList<QgsVectorLayer *> layers;
  const QStringList layerIds = readListEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/AvoidIntersectionsList" ), QStringList() );
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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QStringList list;
  list.reserve( layers.size() );
  for ( QgsVectorLayer *layer : layers )
    list << layer->id();
  writeEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/AvoidIntersectionsList" ), list );
  emit avoidIntersectionsLayersChanged();
}

QgsExpressionContext QgsProject::createExpressionContext() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsExpressionContext context;

  context << QgsExpressionContextUtils::globalScope()
          << QgsExpressionContextUtils::projectScope( this );

  return context;
}

QgsExpressionContextScope *QgsProject::createExpressionContextScope() const
{
  // this method is called quite extensively using QgsProject::instance()
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  // MUCH cheaper to clone than build
  if ( mProjectScope )
  {
    std::unique_ptr< QgsExpressionContextScope > projectScope = std::make_unique< QgsExpressionContextScope >( *mProjectScope );

    // we can't cache these variables
    projectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_distance_units" ), QgsUnitTypes::toString( distanceUnits() ), true, true ) );
    projectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_area_units" ), QgsUnitTypes::toString( areaUnits() ), true, true ) );

    // neither this function
    projectScope->addFunction( QStringLiteral( "sensor_data" ), new GetSensorData( sensorManager()->sensorsData() ) );

    return projectScope.release();
  }

  mProjectScope = std::make_unique< QgsExpressionContextScope >( QObject::tr( "Project" ) );

  const QVariantMap vars = customVariables();

  QVariantMap::const_iterator it = vars.constBegin();

  for ( ; it != vars.constEnd(); ++it )
  {
    mProjectScope->setVariable( it.key(), it.value(), true );
  }

  QString projectPath = projectStorage() ? fileName() : absoluteFilePath();
  if ( projectPath.isEmpty() )
    projectPath = mOriginalPath;
  const QString projectFolder = QFileInfo( projectPath ).path();
  const QString projectFilename = QFileInfo( projectPath ).fileName();
  const QString projectBasename = baseName();

  //add other known project variables
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_title" ), title(), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_path" ), QDir::toNativeSeparators( projectPath ), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_folder" ), QDir::toNativeSeparators( projectFolder ), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_filename" ), projectFilename, true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_basename" ), projectBasename, true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_home" ), QDir::toNativeSeparators( homePath() ), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_last_saved" ), mSaveDateTime.isNull() ? QVariant() : QVariant( mSaveDateTime ), true, true ) );

  const QgsCoordinateReferenceSystem projectCrs = crs();
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_crs" ), projectCrs.authid(), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_crs_definition" ), projectCrs.toProj(), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_crs_description" ), projectCrs.description(), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_crs_acronym" ), projectCrs.projectionAcronym(), true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_crs_ellipsoid" ), projectCrs.ellipsoidAcronym(), true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_crs_proj4" ), projectCrs.toProj(), true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_crs_wkt" ), projectCrs.toWkt( Qgis::CrsWktVariant::Preferred ), true ) );

  const QgsCoordinateReferenceSystem projectVerticalCrs = QgsProject::verticalCrs();
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_vertical_crs" ), projectVerticalCrs.authid(), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_vertical_crs_definition" ), projectVerticalCrs.toProj(), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_vertical_crs_description" ), projectVerticalCrs.description(), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_vertical_crs_wkt" ), projectVerticalCrs.toWkt( Qgis::CrsWktVariant::Preferred ), true ) );

  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_ellipsoid" ), ellipsoid(), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "_project_transform_context" ), QVariant::fromValue<QgsCoordinateTransformContext>( transformContext() ), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_units" ), QgsUnitTypes::toString( projectCrs.mapUnits() ), true ) );

  // metadata
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_author" ), metadata().author(), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_abstract" ), metadata().abstract(), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_creation_date" ), metadata().creationDateTime(), true, true ) );
  mProjectScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "project_identifier" ), metadata().identifier(), true, true ) );

  // keywords
  QVariantMap keywords;
  const QgsAbstractMetadataBase::KeywordMap metadataKeywords = metadata().keywords();
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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QMap<QString, QgsMapLayer *> existingMaps = mapLayers();

  const auto constLayers = layers;
  for ( QgsMapLayer *layer : constLayers )
  {
    if ( ! layer->isValid() )
      return;

    if ( QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer ) )
    {
      vlayer->setReadExtentFromXml( mFlags & Qgis::ProjectFlag::TrustStoredLayerStatistics );
      if ( vlayer->dataProvider() )
        vlayer->dataProvider()->setProviderProperty( QgsVectorDataProvider::EvaluateDefaultValues,
            ( bool )( mFlags & Qgis::ProjectFlag::EvaluateDefaultValuesOnProviderSide ) );
    }

    connect( layer, &QgsMapLayer::configChanged, this, [this] { setDirty(); } );

    // check if we have to update connections for layers with dependencies
    for ( QMap<QString, QgsMapLayer *>::const_iterator it = existingMaps.cbegin(); it != existingMaps.cend(); ++it )
    {
      const QSet<QgsMapLayerDependency> deps = it.value()->dependencies();
      if ( deps.contains( layer->id() ) )
      {
        // reconnect to change signals
        it.value()->setDependencies( deps );
      }
    }
  }

  updateTransactionGroups();

  if ( !mBlockSnappingUpdates && mSnappingConfig.addLayers( layers ) )
    emit snappingConfigChanged( mSnappingConfig );
}

void QgsProject::onMapLayersRemoved( const QList<QgsMapLayer *> &layers )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !mBlockSnappingUpdates && mSnappingConfig.removeLayers( layers ) )
    emit snappingConfigChanged( mSnappingConfig );
}

void QgsProject::cleanTransactionGroups( bool force )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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

void QgsProject::updateTransactionGroups()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mEditBufferGroup.clear();

  switch ( mTransactionMode )
  {
    case Qgis::TransactionMode::Disabled:
    {
      cleanTransactionGroups( true );
      return;
    }
    break;
    case Qgis::TransactionMode::BufferedGroups:
      cleanTransactionGroups( true );
      break;
    case Qgis::TransactionMode::AutomaticGroups:
      cleanTransactionGroups( false );
      break;
  }

  bool tgChanged = false;
  const auto constLayers = mapLayers().values();
  for ( QgsMapLayer *layer : constLayers )
  {
    if ( ! layer->isValid() )
      continue;

    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
    if ( ! vlayer )
      continue;

    switch ( mTransactionMode )
    {
      case Qgis::TransactionMode::Disabled:
        Q_ASSERT( false );
        break;
      case Qgis::TransactionMode::AutomaticGroups:
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
      break;
      case Qgis::TransactionMode::BufferedGroups:
      {
        if ( vlayer->supportsEditing() )
          mEditBufferGroup.addLayer( vlayer );
      }
      break;
    }
  }

  if ( tgChanged )
    emit transactionGroupsChanged();
}

bool QgsProject::readLayer( const QDomNode &layerNode )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsReadWriteContext context;
  context.setPathResolver( pathResolver() );
  context.setProjectTranslator( this );
  context.setTransformContext( transformContext() );
  QList<QDomNode> brokenNodes;
  if ( addLayer( layerNode.toElement(), brokenNodes, context ) )
  {
    // have to try to update joins for all layers now - a previously added layer may be dependent on this newly
    // added layer for joins
    const QVector<QgsVectorLayer *> vectorLayers = layers<QgsVectorLayer *>();
    for ( QgsVectorLayer *layer : vectorLayers )
    {
      // TODO: should be only done later - and with all layers (other layers may have referenced this layer)
      layer->resolveReferences( this );

      if ( layer->isValid() && layer->customProperty( QStringLiteral( "_layer_was_editable" ) ).toBool() )
      {
        layer->startEditing();
        layer->removeCustomProperty( QStringLiteral( "_layer_was_editable" ) );
      }
    }
    return true;
  }
  return false;
}

bool QgsProject::write( const QString &filename )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mFile.setFileName( filename );
  mCachedHomePath.clear();
  return write();
}

bool QgsProject::write()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mProjectScope.reset();
  if ( QgsProjectStorage *storage = projectStorage() )
  {
    QgsReadWriteContext context;
    // for projects stored in a custom storage, we have to check for the support
    // of relative paths since the storage most likely will not be in a file system
    const QString storageFilePath { storage->filePath( mFile.fileName() ) };
    if ( storageFilePath.isEmpty() )
    {
      setFilePathStorage( Qgis::FilePathType::Absolute );
    }
    context.setPathResolver( pathResolver() );

    const QString tempPath = QStandardPaths::standardLocations( QStandardPaths::TempLocation ).at( 0 );
    const QString tmpZipFilename( tempPath + QDir::separator() + QUuid::createUuid().toString() );

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
    bool attachmentsOk = true;
    if ( !mArchive->files().isEmpty() )
    {
      const QFileInfo finfo( mFile.fileName() );
      const QString attachmentsZip = finfo.absoluteDir().absoluteFilePath( QStringLiteral( "%1_attachments.zip" ).arg( finfo.completeBaseName() ) );
      attachmentsOk = mArchive->zip( attachmentsZip );
    }

    // errors raised during writing project file are more important
    if ( ( !asOk || !attachmentsOk ) && writeOk )
    {
      QStringList errorMessage;
      if ( !asOk )
      {
        const QString err = mAuxiliaryStorage->errorString();
        errorMessage.append( tr( "Unable to save auxiliary storage ('%1')" ).arg( err ) );
      }
      if ( !attachmentsOk )
      {
        errorMessage.append( tr( "Unable to save attachments archive" ) );
      }
      setError( errorMessage.join( '\n' ) );
    }

    return asOk && writeOk && attachmentsOk;
  }
}

bool QgsProject::writeProjectFile( const QString &filename )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QFile projectFile( filename );
  clearError();

  // if we have problems creating or otherwise writing to the project file,
  // let's find out up front before we go through all the hand-waving
  // necessary to create all the Dom objects
  const QFileInfo myFileInfo( projectFile );
  if ( myFileInfo.exists() && !myFileInfo.isWritable() )
  {
    setError( tr( "%1 is not writable. Please adjust permissions (if possible) and try again." )
              .arg( projectFile.fileName() ) );
    return false;
  }

  QgsReadWriteContext context;
  context.setPathResolver( pathResolver() );
  context.setTransformContext( transformContext() );

  QDomImplementation::setInvalidDataPolicy( QDomImplementation::DropInvalidChars );

  const QDomDocumentType documentType =
    QDomImplementation().createDocumentType( QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ),
        QStringLiteral( "SYSTEM" ) );
  std::unique_ptr<QDomDocument> doc( new QDomDocument( documentType ) );

  QDomElement qgisNode = doc->createElement( QStringLiteral( "qgis" ) );
  qgisNode.setAttribute( QStringLiteral( "projectname" ), title() );
  qgisNode.setAttribute( QStringLiteral( "version" ), Qgis::version() );

  if ( !mSettings.value( QStringLiteral( "projects/anonymize_saved_projects" ), false, QgsSettings::Core ).toBool() )
  {
    const QString newSaveUser = QgsApplication::userLoginName();
    const QString newSaveUserFull = QgsApplication::userFullName();
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

  QDomElement transactionNode = doc->createElement( QStringLiteral( "transaction" ) );
  transactionNode.setAttribute( QStringLiteral( "mode" ), qgsEnumValueToKey( mTransactionMode ) );
  qgisNode.appendChild( transactionNode );

  QDomElement flagsNode = doc->createElement( QStringLiteral( "projectFlags" ) );
  flagsNode.setAttribute( QStringLiteral( "set" ), qgsFlagValueToKeys( mFlags ) );
  qgisNode.appendChild( flagsNode );

  const QDomText titleText = doc->createTextNode( title() );  // XXX why have title TWICE?
  titleNode.appendChild( titleText );

  // write project CRS
  {
    QDomElement srsNode = doc->createElement( QStringLiteral( "projectCrs" ) );
    mCrs.writeXml( srsNode, *doc );
    qgisNode.appendChild( srsNode );
  }
  {
    QDomElement verticalSrsNode = doc->createElement( QStringLiteral( "verticalCrs" ) );
    mVerticalCrs.writeXml( verticalSrsNode, *doc );
    qgisNode.appendChild( verticalSrsNode );
  }

  QDomElement elevationShadingNode = doc->createElement( QStringLiteral( "elevation-shading-renderer" ) );
  mElevationShadingRenderer.writeXml( elevationShadingNode, context );
  qgisNode.appendChild( elevationShadingNode );

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
  const QMap<QString, QgsMapLayer *> layers = mapLayers();

  QDomElement annotationLayerNode = doc->createElement( QStringLiteral( "main-annotation-layer" ) );
  mMainAnnotationLayer->writeLayerXml( annotationLayerNode, *doc, context );
  qgisNode.appendChild( annotationLayerNode );

  // Iterate over layers in zOrder
  // Call writeXml() on each
  QDomElement projectLayersNode = doc->createElement( QStringLiteral( "projectlayers" ) );

  QMap<QString, QgsMapLayer *>::ConstIterator li = layers.constBegin();
  while ( li != layers.end() )
  {
    QgsMapLayer *ml = li.value();

    if ( ml )
    {
      const QHash< QString, QPair< QString, bool> >::const_iterator emIt = mEmbeddedLayers.constFind( ml->id() );
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

          if ( ml->isEditable() && ( mFlags & Qgis::ProjectFlag::RememberLayerEditStatusBetweenSessions ) )
            maplayerElem.setAttribute( QStringLiteral( "editable" ), QStringLiteral( "1" ) );
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
            QgsDebugError( QStringLiteral( "Could not restore layer properties for layer %1" ).arg( ml->id() ) );
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

  writeEntry( QStringLiteral( "Measurement" ), QStringLiteral( "/DistanceUnits" ), QgsUnitTypes::encodeUnit( mDistanceUnits ) );
  writeEntry( QStringLiteral( "Measurement" ), QStringLiteral( "/AreaUnits" ), QgsUnitTypes::encodeUnit( mAreaUnits ) );

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

  {
    const QDomElement annotationsElem = mAnnotationManager->writeXml( *doc, context );
    qgisNode.appendChild( annotationsElem );
  }

  {
    const QDomElement layoutElem = mLayoutManager->writeXml( *doc );
    qgisNode.appendChild( layoutElem );
  }

  {
    const QDomElement views3DElem = m3DViewsManager->writeXml( *doc );
    qgisNode.appendChild( views3DElem );
  }

  {
    const QDomElement bookmarkElem = mBookmarkManager->writeXml( *doc );
    qgisNode.appendChild( bookmarkElem );
  }

  {
    const QDomElement sensorElem = mSensorManager->writeXml( *doc );
    qgisNode.appendChild( sensorElem );
  }

  {
    const QDomElement viewSettingsElem = mViewSettings->writeXml( *doc, context );
    qgisNode.appendChild( viewSettingsElem );
  }

  {
    const QDomElement styleSettingsElem = mStyleSettings->writeXml( *doc, context );
    qgisNode.appendChild( styleSettingsElem );
  }

  {
    const QDomElement timeSettingsElement = mTimeSettings->writeXml( *doc, context );
    qgisNode.appendChild( timeSettingsElement );
  }

  {
    const QDomElement elevationPropertiesElement = mElevationProperties->writeXml( *doc, context );
    qgisNode.appendChild( elevationPropertiesElement );
  }

  {
    const QDomElement displaySettingsElem = mDisplaySettings->writeXml( *doc, context );
    qgisNode.appendChild( displaySettingsElem );
  }

  {
    const QDomElement gpsSettingsElem = mGpsSettings->writeXml( *doc, context );
    qgisNode.appendChild( gpsSettingsElem );
  }

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

    const QFileInfo fi( fileName() );
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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  bool propertiesModified;
  const bool success = addKey_( scope, key, &mProperties, value, propertiesModified );

  if ( propertiesModified )
    setDirty( true );

  return success;
}

bool QgsProject::writeEntry( const QString &scope, const QString &key, double value )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  bool propertiesModified;
  const bool success = addKey_( scope, key, &mProperties, value, propertiesModified );

  if ( propertiesModified )
    setDirty( true );

  return success;
}

bool QgsProject::writeEntry( const QString &scope, QString const &key, int value )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  bool propertiesModified;
  const bool success = addKey_( scope, key, &mProperties, value, propertiesModified );

  if ( propertiesModified )
    setDirty( true );

  return success;
}

bool QgsProject::writeEntry( const QString &scope, const QString &key, const QString &value )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  bool propertiesModified;
  const bool success = addKey_( scope, key, &mProperties, value, propertiesModified );

  if ( propertiesModified )
    setDirty( true );

  return success;
}

bool QgsProject::writeEntry( const QString &scope, const QString &key, const QStringList &value )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  bool propertiesModified;
  const bool success = addKey_( scope, key, &mProperties, value, propertiesModified );

  if ( propertiesModified )
    setDirty( true );

  return success;
}

QStringList QgsProject::readListEntry( const QString &scope,
                                       const QString &key,
                                       const QStringList &def,
                                       bool *ok ) const
{
  // this method is called quite extensively from other threads via QgsProject::createExpressionContextScope()
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  QgsProjectProperty *property = findKey_( scope, key, mProperties );

  QVariant value;

  if ( property )
  {
    value = property->value();

    const bool valid = QMetaType::Type::QStringList == value.userType();
    if ( ok )
      *ok = valid;

    if ( valid )
    {
      return value.toStringList();
    }
  }
  else if ( ok )
    *ok = false;


  return def;
}

QString QgsProject::readEntry( const QString &scope,
                               const QString &key,
                               const QString &def,
                               bool *ok ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsProjectProperty *property = findKey_( scope, key, mProperties );

  QVariant value;

  if ( property )
  {
    value = property->value();

    const bool valid = value.canConvert( QMetaType::Type::QString );
    if ( ok )
      *ok = valid;

    if ( valid )
      return value.toString();
  }
  else if ( ok )
    *ok = false;

  return def;
}

int QgsProject::readNumEntry( const QString &scope, const QString &key, int def,
                              bool *ok ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsProjectProperty *property = findKey_( scope, key, mProperties );

  QVariant value;

  if ( property )
  {
    value = property->value();
  }

  const bool valid = value.canConvert( QMetaType::Type::Int );

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsProjectProperty *property = findKey_( scope, key, mProperties );
  if ( property )
  {
    const QVariant value = property->value();

    const bool valid = value.canConvert( QMetaType::Type::Double );
    if ( ok )
      *ok = valid;

    if ( valid )
      return value.toDouble();
  }
  else if ( ok )
    *ok = false;

  return def;
}

bool QgsProject::readBoolEntry( const QString &scope, const QString &key, bool def,
                                bool *ok ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsProjectProperty *property = findKey_( scope, key, mProperties );

  if ( property )
  {
    const QVariant value = property->value();

    const bool valid = value.canConvert( QMetaType::Type::Bool );
    if ( ok )
      *ok = valid;

    if ( valid )
      return value.toBool();
  }
  else if ( ok )
    *ok = false;

  return def;
}

bool QgsProject::removeEntry( const QString &scope, const QString &key )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( findKey_( scope, key, mProperties ) )
  {
    removeKey_( scope, key, mProperties );
    setDirty( true );
  }

  return !findKey_( scope, key, mProperties );
}

QStringList QgsProject::entryList( const QString &scope, const QString &key ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  dump_( mProperties );
}

QgsPathResolver QgsProject::pathResolver() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QString filePath;
  switch ( filePathStorage() )
  {
    case Qgis::FilePathType::Absolute:
      break;

    case Qgis::FilePathType::Relative:
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
      break;
    }
  }

  return QgsPathResolver( filePath, mArchive->dir() );
}

QString QgsProject::readPath( const QString &src ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return pathResolver().readPath( src );
}

QString QgsProject::writePath( const QString &src ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return pathResolver().writePath( src );
}

void QgsProject::setError( const QString &errorMessage )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mErrorMessage = errorMessage;
}

QString QgsProject::error() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mErrorMessage;
}

void QgsProject::clearError()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  setError( QString() );
}

void QgsProject::setBadLayerHandler( QgsProjectBadLayerHandler *handler )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  delete mBadLayerHandler;
  mBadLayerHandler = handler;
}

QString QgsProject::layerIsEmbedded( const QString &id ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QHash< QString, QPair< QString, bool > >::const_iterator it = mEmbeddedLayers.find( id );
  if ( it == mEmbeddedLayers.constEnd() )
  {
    return QString();
  }
  return it.value().first;
}

bool QgsProject::createEmbeddedLayer( const QString &layerId, const QString &projectFilePath, QList<QDomNode> &brokenNodes,
                                      bool saveFlag, Qgis::ProjectReadFlags flags )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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

  const QDateTime projectFileTimestamp = QFileInfo( projectFilePath ).lastModified();

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

  const QDomElement propertiesElem = sProjectDocument.documentElement().firstChildElement( QStringLiteral( "properties" ) );
  if ( !propertiesElem.isNull() )
  {
    const QDomElement absElem = propertiesElem.firstChildElement( QStringLiteral( "Paths" ) ).firstChildElement( QStringLiteral( "Absolute" ) );
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

  const QDomElement projectLayersElem = sProjectDocument.documentElement().firstChildElement( QStringLiteral( "projectlayers" ) );
  if ( projectLayersElem.isNull() )
  {
    return false;
  }

  QDomElement mapLayerElem = projectLayersElem.firstChildElement( QStringLiteral( "maplayer" ) );
  while ( ! mapLayerElem.isNull() )
  {
    // get layer id
    const QString id = mapLayerElem.firstChildElement( QStringLiteral( "id" ) ).text();
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
    mapLayerElem = mapLayerElem.nextSiblingElement( QStringLiteral( "maplayer" ) );
  }

  return false;
}

QgsLayerTreeGroup *QgsProject::createEmbeddedGroup( const QString &groupName, const QString &projectFilePath, const QStringList &invisibleLayers, Qgis::ProjectReadFlags flags )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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

void QgsProject::initializeEmbeddedSubtree( const QString &projectFilePath, QgsLayerTreeGroup *group, Qgis::ProjectReadFlags flags )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mFlags & Qgis::ProjectFlag::EvaluateDefaultValuesOnProviderSide;
}

void QgsProject::setEvaluateDefaultValues( bool evaluateDefaultValues )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  setFlag( Qgis::ProjectFlag::EvaluateDefaultValuesOnProviderSide, evaluateDefaultValues );
}

void QgsProject::setTopologicalEditing( bool enabled )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  writeEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/TopologicalEditing" ), ( enabled ? 1 : 0 ) );
  emit topologicalEditingChanged();
}

bool QgsProject::topologicalEditing() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return readNumEntry( QStringLiteral( "Digitizing" ), QStringLiteral( "/TopologicalEditing" ), 0 );
}

void QgsProject::setDistanceUnits( Qgis::DistanceUnit unit )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mDistanceUnits == unit )
    return;

  mDistanceUnits = unit;

  emit distanceUnitsChanged();
}

void QgsProject::setAreaUnits( Qgis::AreaUnit unit )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mAreaUnits == unit )
    return;

  mAreaUnits = unit;

  emit areaUnitsChanged();
}

QString QgsProject::homePath() const
{
  // this method is called quite extensively from other threads via QgsProject::createExpressionContextScope()
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  if ( !mCachedHomePath.isEmpty() )
    return mCachedHomePath;

  const QFileInfo pfi( fileName() );

  if ( !mHomePath.isEmpty() )
  {
    const QFileInfo homeInfo( mHomePath );
    if ( !homeInfo.isRelative() )
    {
      mCachedHomePath = mHomePath;
      return mHomePath;
    }
  }
  else if ( !fileName().isEmpty() )
  {

    // If it's not stored in the file system, try to get the path from the storage
    if ( QgsProjectStorage *storage = projectStorage() )
    {
      const QString storagePath { storage->filePath( fileName() ) };
      if ( ! storagePath.isEmpty() && QFileInfo::exists( storagePath ) )
      {
        mCachedHomePath = QFileInfo( storagePath ).path();
        return mCachedHomePath;
      }
    }

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mHomePath;
}

QgsRelationManager *QgsProject::relationManager() const
{
  // because relation aggregate functions are not thread safe
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mRelationManager;
}

const QgsLayoutManager *QgsProject::layoutManager() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mLayoutManager.get();
}

QgsLayoutManager *QgsProject::layoutManager()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mLayoutManager.get();
}

const QgsMapViewsManager *QgsProject::viewsManager() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return m3DViewsManager.get();
}

QgsMapViewsManager *QgsProject::viewsManager()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return m3DViewsManager.get();
}

const QgsBookmarkManager *QgsProject::bookmarkManager() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mBookmarkManager;
}

QgsBookmarkManager *QgsProject::bookmarkManager()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mBookmarkManager;
}

const QgsSensorManager *QgsProject::sensorManager() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mSensorManager;
}

QgsSensorManager *QgsProject::sensorManager()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mSensorManager;
}

const QgsProjectViewSettings *QgsProject::viewSettings() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mViewSettings;
}

QgsProjectViewSettings *QgsProject::viewSettings()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mViewSettings;
}

const QgsProjectStyleSettings *QgsProject::styleSettings() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mStyleSettings;
}

QgsProjectStyleSettings *QgsProject::styleSettings()
{
  // this method is called quite extensively from other threads via QgsProject::createExpressionContextScope()
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mStyleSettings;
}

const QgsProjectTimeSettings *QgsProject::timeSettings() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mTimeSettings;
}

QgsProjectTimeSettings *QgsProject::timeSettings()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mTimeSettings;
}

const QgsProjectElevationProperties *QgsProject::elevationProperties() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mElevationProperties;
}

QgsProjectElevationProperties *QgsProject::elevationProperties()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mElevationProperties;
}

const QgsProjectDisplaySettings *QgsProject::displaySettings() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDisplaySettings;
}

QgsProjectDisplaySettings *QgsProject::displaySettings()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDisplaySettings;
}

const QgsProjectGpsSettings *QgsProject::gpsSettings() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mGpsSettings;
}

QgsProjectGpsSettings *QgsProject::gpsSettings()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mGpsSettings;
}

QgsLayerTree *QgsProject::layerTreeRoot() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mRootGroup;
}

QgsMapThemeCollection *QgsProject::mapThemeCollection()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mMapThemeCollection.get();
}

QgsAnnotationManager *QgsProject::annotationManager()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mAnnotationManager.get();
}

const QgsAnnotationManager *QgsProject::annotationManager() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mAnnotationManager.get();
}

void QgsProject::setNonIdentifiableLayers( const QList<QgsMapLayer *> &layers )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mTransactionMode == Qgis::TransactionMode::AutomaticGroups;
}

void QgsProject::setAutoTransaction( bool autoTransaction )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( autoTransaction
       && mTransactionMode == Qgis::TransactionMode::AutomaticGroups )
    return;

  if ( ! autoTransaction
       && mTransactionMode == Qgis::TransactionMode::Disabled )
    return;

  if ( autoTransaction )
    setTransactionMode( Qgis::TransactionMode::AutomaticGroups );
  else
    setTransactionMode( Qgis::TransactionMode::Disabled );

  updateTransactionGroups();
}

Qgis::TransactionMode QgsProject::transactionMode() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mTransactionMode;
}

bool QgsProject::setTransactionMode( Qgis::TransactionMode transactionMode )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( transactionMode == mTransactionMode )
    return true;

  // Check that all layer are not in edit mode
  const auto constLayers = mapLayers().values();
  for ( QgsMapLayer *layer : constLayers )
  {
    if ( layer->isEditable() )
    {
      QgsLogger::warning( tr( "Transaction mode can be changed only if all layers are not editable." ) );
      return false;
    }
  }

  mTransactionMode = transactionMode;
  updateTransactionGroups();
  emit transactionModeChanged();
  return true;
}

QMap<QPair<QString, QString>, QgsTransactionGroup *> QgsProject::transactionGroups()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mTransactionGroups;
}


//
// QgsMapLayerStore methods
//


int QgsProject::count() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mLayerStore->count();
}

int QgsProject::validCount() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mLayerStore->validCount();
}

QgsMapLayer *QgsProject::mapLayer( const QString &layerId ) const
{
  // because QgsVirtualLayerProvider is not anywhere NEAR thread safe:
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mLayerStore->mapLayer( layerId );
}

QList<QgsMapLayer *> QgsProject::mapLayersByName( const QString &layerName ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mLayerStore->mapLayersByName( layerName );
}

QList<QgsMapLayer *> QgsProject::mapLayersByShortName( const QString &shortName ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QList<QgsMapLayer *> layers;
  const auto constMapLayers { mLayerStore->mapLayers() };
  for ( const auto &l : constMapLayers )
  {
    if ( ! l->serverProperties()->shortName().isEmpty() )
    {
      if ( l->serverProperties()->shortName() == shortName )
        layers << l;
    }
    else if ( l->name() == shortName )
    {
      layers << l;
    }
  }
  return layers;
}

bool QgsProject::unzip( const QString &filename, Qgis::ProjectReadFlags flags )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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

  // Keep the archive
  releaseHandlesToProjectArchive();
  mArchive = std::move( archive );

  // load auxiliary storage
  if ( !static_cast<QgsProjectArchive *>( mArchive.get() )->auxiliaryStorageFile().isEmpty() )
  {
    // database file is already a copy as it's been unzipped. So we don't open
    // auxiliary storage in copy mode in this case
    mAuxiliaryStorage.reset( new QgsAuxiliaryStorage( static_cast<QgsProjectArchive *>( mArchive.get() )->auxiliaryStorageFile(), false ) );
  }
  else
  {
    mAuxiliaryStorage.reset( new QgsAuxiliaryStorage( *this ) );
  }

  // read the project file
  if ( ! readProjectFile( static_cast<QgsProjectArchive *>( mArchive.get() )->projectFile(), flags ) )
  {
    setError( tr( "Cannot read unzipped qgs project file" ) + QStringLiteral( ": " ) + error() );
    return false;
  }

  // Remove the temporary .qgs file
  static_cast<QgsProjectArchive *>( mArchive.get() )->clearProjectFile();

  return true;
}

bool QgsProject::zip( const QString &filename )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  const QString asExt = QStringLiteral( ".%1" ).arg( QgsAuxiliaryStorage::extension() );
  const QString asFileName = info.path() + QDir::separator() + info.completeBaseName() + asExt;

  bool auxiliaryStorageSavedOk = true;
  if ( ! saveAuxiliaryStorage( asFileName ) )
  {
    const QString err = mAuxiliaryStorage->errorString();
    setError( tr( "Unable to save auxiliary storage file ('%1'). The project has been saved but the latest changes to auxiliary data cannot be recovered. It is recommended to reload the project." ).arg( err ) );
    auxiliaryStorageSavedOk = false;

    // fixes the current archive and keep the previous version of qgd
    if ( !mArchive->exists() )
    {
      releaseHandlesToProjectArchive();
      mArchive.reset( new QgsProjectArchive() );
      mArchive->unzip( mFile.fileName() );
      static_cast<QgsProjectArchive *>( mArchive.get() )->clearProjectFile();

      const QString auxiliaryStorageFile = static_cast<QgsProjectArchive *>( mArchive.get() )->auxiliaryStorageFile();
      if ( ! auxiliaryStorageFile.isEmpty() )
      {
        archive->addFile( auxiliaryStorageFile );
        mAuxiliaryStorage.reset( new QgsAuxiliaryStorage( auxiliaryStorageFile, false ) );
      }
    }
  }
  else
  {
    // in this case, an empty filename means that the auxiliary database is
    // empty, so we don't want to save it
    if ( QFile::exists( asFileName ) )
    {
      archive->addFile( asFileName );
    }
  }

  // create the archive
  archive->addFile( qgsFile.fileName() );

  // Add all other files
  const QStringList &files = mArchive->files();
  for ( const QString &file : files )
  {
    if ( !file.endsWith( QLatin1String( ".qgs" ), Qt::CaseInsensitive ) && !file.endsWith( asExt, Qt::CaseInsensitive ) )
    {
      archive->addFile( file );
    }
  }

  // zip
  bool zipOk = true;
  if ( !archive->zip( filename ) )
  {
    setError( tr( "Unable to perform zip" ) );
    zipOk = false;
  }

  return auxiliaryStorageSavedOk && zipOk;
}

bool QgsProject::isZipped() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsZipUtils::isZipFile( mFile.fileName() );
}

QList<QgsMapLayer *> QgsProject::addMapLayers(
  const QList<QgsMapLayer *> &layers,
  bool addToLegend,
  bool takeOwnership )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
      if ( mlayer->type() != Qgis::LayerType::Vector )
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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QList<QgsMapLayer *> addedLayers;
  addedLayers = addMapLayers( QList<QgsMapLayer *>() << layer, addToLegend, takeOwnership );
  return addedLayers.isEmpty() ? nullptr : addedLayers[0];
}

void QgsProject::removeAuxiliaryLayer( const QgsMapLayer *ml )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( ! ml || ml->type() != Qgis::LayerType::Vector )
    return;

  const QgsVectorLayer *vl = qobject_cast<const QgsVectorLayer *>( ml );
  if ( vl && vl->auxiliaryLayer() )
  {
    const QgsDataSourceUri uri( vl->auxiliaryLayer()->source() );
    QgsAuxiliaryStorage::deleteTable( uri );
  }
}

void QgsProject::removeMapLayers( const QStringList &layerIds )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  for ( const auto &layerId : layerIds )
    removeAuxiliaryLayer( mLayerStore->mapLayer( layerId ) );

  mProjectScope.reset();
  mLayerStore->removeMapLayers( layerIds );
}

void QgsProject::removeMapLayers( const QList<QgsMapLayer *> &layers )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  for ( const auto &layer : layers )
    removeAuxiliaryLayer( layer );

  mProjectScope.reset();
  mLayerStore->removeMapLayers( layers );
}

void QgsProject::removeMapLayer( const QString &layerId )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  removeAuxiliaryLayer( mLayerStore->mapLayer( layerId ) );
  mProjectScope.reset();
  mLayerStore->removeMapLayer( layerId );
}

void QgsProject::removeMapLayer( QgsMapLayer *layer )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  removeAuxiliaryLayer( layer );
  mProjectScope.reset();
  mLayerStore->removeMapLayer( layer );
}

QgsMapLayer *QgsProject::takeMapLayer( QgsMapLayer *layer )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mProjectScope.reset();
  return mLayerStore->takeMapLayer( layer );
}

QgsAnnotationLayer *QgsProject::mainAnnotationLayer()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mMainAnnotationLayer;
}

void QgsProject::removeAllMapLayers()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mLayerStore->count() == 0 )
    return;

  ScopedIntIncrementor snapSingleBlocker( &mBlockSnappingUpdates );
  mProjectScope.reset();
  mLayerStore->removeAllMapLayers();

  snapSingleBlocker.release();
  mSnappingConfig.clearIndividualLayerSettings();
  if ( !mBlockSnappingUpdates )
    emit snappingConfigChanged( mSnappingConfig );
}

void QgsProject::reloadAllLayers()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QMap<QString, QgsMapLayer *> layers = mLayerStore->mapLayers();
  QMap<QString, QgsMapLayer *>::const_iterator it = layers.constBegin();
  for ( ; it != layers.constEnd(); ++it )
  {
    it.value()->reload();
  }
}

QMap<QString, QgsMapLayer *> QgsProject::mapLayers( const bool validOnly ) const
{
  // because QgsVirtualLayerProvider is not anywhere NEAR thread safe:
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return validOnly ? mLayerStore->validMapLayers() : mLayerStore->mapLayers();
}

QgsTransactionGroup *QgsProject::transactionGroup( const QString &providerKey, const QString &connString )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mTransactionGroups.value( qMakePair( providerKey, connString ) );
}

QgsVectorLayerEditBufferGroup *QgsProject::editBufferGroup()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return &mEditBufferGroup;
}

QgsCoordinateReferenceSystem QgsProject::defaultCrsForNewLayers() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsCoordinateReferenceSystem defaultCrs;

  // TODO QGIS 4.0 -- remove this method, and place it somewhere in app (where it belongs)
  // in the meantime, we have a slightly hacky way to read the settings key using an enum which isn't available (since it lives in app)
  if ( mSettings.value( QStringLiteral( "/projections/unknownCrsBehavior" ), QStringLiteral( "NoAction" ), QgsSettings::App ).toString() == QStringLiteral( "UseProjectCrs" )
       || mSettings.value( QStringLiteral( "/projections/unknownCrsBehavior" ), 0, QgsSettings::App ).toString() == QLatin1String( "2" ) )
  {
    // for new layers if the new layer crs method is set to either prompt or use project, then we use the project crs
    defaultCrs = crs();
  }
  else
  {
    // global crs
    const QString layerDefaultCrs = mSettings.value( QStringLiteral( "/Projections/layerDefaultCrs" ), geoEpsgCrsAuthId() ).toString();
    defaultCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( layerDefaultCrs );
  }

  return defaultCrs;
}

void QgsProject::setTrustLayerMetadata( bool trust )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  setFlag( Qgis::ProjectFlag::TrustStoredLayerStatistics, trust );
}

bool QgsProject::trustLayerMetadata() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mFlags & Qgis::ProjectFlag::TrustStoredLayerStatistics;
}

bool QgsProject::saveAuxiliaryStorage( const QString &filename )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QMap<QString, QgsMapLayer *> layers = mapLayers();
  bool empty = true;
  for ( auto it = layers.constBegin(); it != layers.constEnd(); ++it )
  {
    if ( it.value()->type() != Qgis::LayerType::Vector )
      continue;

    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( it.value() );
    if ( vl && vl->auxiliaryLayer() )
    {
      vl->auxiliaryLayer()->save();
      empty &= vl->auxiliaryLayer()->auxiliaryFields().isEmpty();
    }
  }

  if ( !mAuxiliaryStorage->exists( *this ) && empty )
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
      static_cast< int >( QgsProject::DataDefinedServerProperty::WMSOnlineResource ),
      QgsPropertyDefinition( "WMSOnlineResource", QObject::tr( "WMS Online Resource" ), QgsPropertyDefinition::String )
    },
  };
  return sPropertyDefinitions;
}

void QgsProject::setElevationShadingRenderer( const QgsElevationShadingRenderer &elevationShadingRenderer )
{
  mElevationShadingRenderer = elevationShadingRenderer;
  emit elevationShadingRendererChanged();
}

const QgsAuxiliaryStorage *QgsProject::auxiliaryStorage() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mAuxiliaryStorage.get();
}

QgsAuxiliaryStorage *QgsProject::auxiliaryStorage()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mAuxiliaryStorage.get();
}

QString QgsProject::createAttachedFile( const QString &nameTemplate )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QDir archiveDir( mArchive->dir() );
  QTemporaryFile tmpFile( archiveDir.filePath( "XXXXXX_" + nameTemplate ), this );
  tmpFile.setAutoRemove( false );
  tmpFile.open();
  mArchive->addFile( tmpFile.fileName() );
  return tmpFile.fileName();
}

QStringList QgsProject::attachedFiles() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QStringList attachments;
  const QString baseName = QFileInfo( fileName() ).baseName();
  const QStringList files = mArchive->files();
  attachments.reserve( files.size() );
  for ( const QString &file : files )
  {
    if ( QFileInfo( file ).baseName() != baseName )
    {
      attachments.append( file );
    }
  }
  return attachments;
}

bool QgsProject::removeAttachedFile( const QString &path )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mArchive->removeFile( path );
}

QString QgsProject::attachmentIdentifier( const QString &attachedFile ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QStringLiteral( "attachment:///%1" ).arg( QFileInfo( attachedFile ).fileName() );
}

QString QgsProject::resolveAttachmentIdentifier( const QString &identifier ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( identifier.startsWith( QLatin1String( "attachment:///" ) ) )
  {
    return QDir( mArchive->dir() ).absoluteFilePath( identifier.mid( 14 ) );
  }
  return QString();
}

const QgsProjectMetadata &QgsProject::metadata() const
{
  // this method is called quite extensively from other threads via QgsProject::createExpressionContextScope()
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mMetadata;
}

void QgsProject::setMetadata( const QgsProjectMetadata &metadata )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( metadata == mMetadata )
    return;

  mMetadata = metadata;
  mProjectScope.reset();

  emit metadataChanged();

  setDirty( true );
}

QSet<QgsMapLayer *> QgsProject::requiredLayers() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  // save colors to project
  QStringList customColors;
  QStringList customColorLabels;

  QgsNamedColorList::const_iterator colorIt = colors.constBegin();
  for ( ; colorIt != colors.constEnd(); ++colorIt )
  {
    const QString color = QgsColorUtils::colorToString( ( *colorIt ).first );
    const QString label = ( *colorIt ).second;
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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mBackgroundColor == color )
    return;

  mBackgroundColor = color;
  emit backgroundColorChanged();
}

QColor QgsProject::backgroundColor() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mBackgroundColor;
}

void QgsProject::setSelectionColor( const QColor &color )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mSelectionColor == color )
    return;

  mSelectionColor = color;
  emit selectionColorChanged();
}

QColor QgsProject::selectionColor() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mSelectionColor;
}

void QgsProject::setMapScales( const QVector<double> &scales )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mViewSettings->setMapScales( scales );
}

QVector<double> QgsProject::mapScales() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mViewSettings->mapScales();
}

void QgsProject::setUseProjectScales( bool enabled )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mViewSettings->setUseProjectScales( enabled );
}

bool QgsProject::useProjectScales() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mViewSettings->useProjectScales();
}

void QgsProject::generateTsFile( const QString &locale )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsTranslationContext translationContext;
  translationContext.setProject( this );
  translationContext.setFileName( QStringLiteral( "%1/%2.ts" ).arg( absolutePath(), baseName() ) );

  QgsApplication::instance()->collectTranslatableObjects( &translationContext );

  translationContext.writeTsFile( locale );
}

QString QgsProject::translate( const QString &context, const QString &sourceText, const char *disambiguation, int n ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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

QgsElevationShadingRenderer QgsProject::elevationShadingRenderer() const
{
  return mElevationShadingRenderer;
}

void QgsProject::loadProjectFlags( const QDomDocument *doc )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QDomElement element = doc->documentElement().firstChildElement( QStringLiteral( "projectFlags" ) );
  Qgis::ProjectFlags flags;
  if ( !element.isNull() )
  {
    flags = qgsFlagKeysToValue( element.attribute( QStringLiteral( "set" ) ), Qgis::ProjectFlags() );
  }
  else
  {
    // older project compatibility
    element = doc->documentElement().firstChildElement( QStringLiteral( "evaluateDefaultValues" ) );
    if ( !element.isNull() )
    {
      if ( element.attribute( QStringLiteral( "active" ), QStringLiteral( "0" ) ).toInt() == 1 )
        flags |= Qgis::ProjectFlag::EvaluateDefaultValuesOnProviderSide;
    }

    // Read trust layer metadata config in the project
    element = doc->documentElement().firstChildElement( QStringLiteral( "trust" ) );
    if ( !element.isNull() )
    {
      if ( element.attribute( QStringLiteral( "active" ), QStringLiteral( "0" ) ).toInt() == 1 )
        flags |= Qgis::ProjectFlag::TrustStoredLayerStatistics;
    }
  }

  setFlags( flags );
}

/// @cond PRIVATE
GetNamedProjectColor::GetNamedProjectColor( const QgsProject *project )
  : QgsScopedExpressionFunction( QStringLiteral( "project_color" ), 1, QStringLiteral( "Color" ) )
{
  if ( !project )
    return;

  //build up color list from project. Do this in advance for speed
  QStringList colorStrings = project->readListEntry( QStringLiteral( "Palette" ), QStringLiteral( "/Colors" ) );
  const QStringList colorLabels = project->readListEntry( QStringLiteral( "Palette" ), QStringLiteral( "/Labels" ) );

  //generate list from custom colors
  int colorIndex = 0;
  for ( QStringList::iterator it = colorStrings.begin();
        it != colorStrings.end(); ++it )
  {
    const QColor color = QgsColorUtils::colorFromString( *it );
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
  const QString colorName = values.at( 0 ).toString().toLower();
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

// ----------------

GetSensorData::GetSensorData( const QMap<QString, QgsAbstractSensor::SensorData> &sensorData )
  : QgsScopedExpressionFunction( QStringLiteral( "sensor_data" ),
                                 QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "name" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "expiration" ), true, 0 ),
                                 QStringLiteral( "Sensors" ) )
  , mSensorData( sensorData )
{
}

QVariant GetSensorData::func( const QVariantList &values, const QgsExpressionContext *, QgsExpression *, const QgsExpressionNodeFunction * )
{
  const QString sensorName = values.at( 0 ).toString();
  const int expiration = values.at( 1 ).toInt();
  const qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
  if ( mSensorData.contains( sensorName ) )
  {
    if ( expiration <= 0 || ( timestamp - mSensorData[sensorName].lastTimestamp.toMSecsSinceEpoch() ) < expiration )
    {
      return mSensorData[sensorName].lastValue;
    }
  }

  return QVariant();
}

QgsScopedExpressionFunction *GetSensorData::clone() const
{
  return new GetSensorData( mSensorData );
}
///@endcond
