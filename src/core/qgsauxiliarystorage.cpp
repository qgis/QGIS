/***************************************************************************
                          qgsauxiliarystorage.cpp  -  description
                            -------------------
    begin                : Aug 28, 2017
    copyright            : (C) 2017 by Paul Blottiere
    email                : paul.blottiere@oslandia.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauxiliarystorage.h"
#include "qgslogger.h"
#include "qgssqliteutils.h"
#include "qgsproject.h"
#include "qgsvectorlayerlabeling.h"
#include "qgsdiagramrenderer.h"
#include "qgsmemoryproviderutils.h"
#include "qgssymbollayer.h"

#include <sqlite3.h>
#include <QFile>

#define AS_JOINFIELD QStringLiteral( "ASPK" )
#define AS_EXTENSION QStringLiteral( "qgd" )
#define AS_JOINPREFIX QStringLiteral( "auxiliary_storage_" )
typedef QVector<QgsPalLayerSettings::Property> PalPropertyList;
Q_GLOBAL_STATIC_WITH_ARGS( PalPropertyList, palHiddenProperties, (
{
  QgsPalLayerSettings::PositionX,
  QgsPalLayerSettings::PositionY,
  QgsPalLayerSettings::Show,
  QgsPalLayerSettings::LabelRotation,
  QgsPalLayerSettings::Family,
  QgsPalLayerSettings::FontStyle,
  QgsPalLayerSettings::Size,
  QgsPalLayerSettings::Bold,
  QgsPalLayerSettings::Italic,
  QgsPalLayerSettings::Underline,
  QgsPalLayerSettings::Color,
  QgsPalLayerSettings::Strikeout,
  QgsPalLayerSettings::MultiLineAlignment,
  QgsPalLayerSettings::BufferSize,
  QgsPalLayerSettings::BufferDraw,
  QgsPalLayerSettings::BufferColor,
  QgsPalLayerSettings::LabelDistance,
  QgsPalLayerSettings::Hali,
  QgsPalLayerSettings::Vali,
  QgsPalLayerSettings::ScaleVisibility,
  QgsPalLayerSettings::MinScale,
  QgsPalLayerSettings::MaxScale,
  QgsPalLayerSettings::AlwaysShow,
  QgsPalLayerSettings::CalloutDraw,
  QgsPalLayerSettings::LabelAllParts
} ) )

//
// QgsAuxiliaryLayer
//

QgsAuxiliaryLayer::QgsAuxiliaryLayer( const QString &pkField, const QString &filename, const QString &table, QgsVectorLayer *vlayer )
  : QgsVectorLayer( QStringLiteral( "%1|layername=%2" ).arg( filename, table ),
                    QStringLiteral( "%1_auxiliarystorage" ).arg( table ), QStringLiteral( "ogr" ) )
  , mFileName( filename )
  , mTable( table )
  , mLayer( vlayer )
{
  // init join info
  mJoinInfo.setPrefix( AS_JOINPREFIX );
  mJoinInfo.setJoinLayer( this );
  mJoinInfo.setJoinFieldName( AS_JOINFIELD );
  mJoinInfo.setTargetFieldName( pkField );
  mJoinInfo.setEditable( true );
  mJoinInfo.setUpsertOnEdit( true );
  mJoinInfo.setCascadedDelete( true );
  mJoinInfo.setJoinFieldNamesBlockList( QStringList() << QStringLiteral( "rowid" ) ); // introduced by ogr provider
}

QgsAuxiliaryLayer *QgsAuxiliaryLayer::clone( QgsVectorLayer *target ) const
{
  QgsAuxiliaryStorage::duplicateTable( source(), target->id() );
  return new QgsAuxiliaryLayer( mJoinInfo.targetFieldName(), mFileName, target->id(), target );
}

bool QgsAuxiliaryLayer::clear()
{
  const bool rc = deleteFeatures( allFeatureIds() );
  commitChanges();
  startEditing();
  return rc;
}

QgsVectorLayer *QgsAuxiliaryLayer::toSpatialLayer() const
{
  QgsVectorLayer *layer = QgsMemoryProviderUtils::createMemoryLayer( QStringLiteral( "auxiliary_layer" ), fields(), mLayer->wkbType(), mLayer->crs() );

  const QString pkField = mJoinInfo.targetFieldName();
  QgsFeature joinFeature;
  QgsFeature targetFeature;
  QgsFeatureIterator it = getFeatures();

  layer->startEditing();
  while ( it.nextFeature( joinFeature ) )
  {
    const QString filter = QgsExpression::createFieldEqualityExpression( pkField, joinFeature.attribute( AS_JOINFIELD ) );

    QgsFeatureRequest request;
    request.setFilterExpression( filter );

    mLayer->getFeatures( request ).nextFeature( targetFeature );

    if ( targetFeature.isValid() )
    {
      QgsFeature newFeature( joinFeature );
      newFeature.setGeometry( targetFeature.geometry() );
      layer->addFeature( newFeature );
    }
  }
  layer->commitChanges();

  return layer;
}

QgsVectorLayerJoinInfo QgsAuxiliaryLayer::joinInfo() const
{
  return mJoinInfo;
}

bool QgsAuxiliaryLayer::exists( const QgsPropertyDefinition &definition ) const
{
  return ( indexOfPropertyDefinition( definition ) >= 0 );
}

bool QgsAuxiliaryLayer::addAuxiliaryField( const QgsPropertyDefinition &definition )
{
  if ( ( definition.name().isEmpty() && definition.comment().isEmpty() ) || exists( definition ) )
    return false;

  const QgsField af = createAuxiliaryField( definition );
  const bool rc = addAttribute( af );
  updateFields();
  mLayer->updateFields();

  if ( rc )
  {
    const int auxIndex = indexOfPropertyDefinition( definition );
    const int index = mLayer->fields().indexOf( nameFromProperty( definition, true ) );

    if ( index >= 0 && auxIndex >= 0 )
    {
      if ( isHiddenProperty( auxIndex ) )
      {
        // update editor widget
        const QgsEditorWidgetSetup setup = QgsEditorWidgetSetup( QStringLiteral( "Hidden" ), QVariantMap() );
        setEditorWidgetSetup( auxIndex, setup );

        // column is hidden
        QgsAttributeTableConfig attrCfg = mLayer->attributeTableConfig();
        attrCfg.update( mLayer->fields() );
        QVector<QgsAttributeTableConfig::ColumnConfig> columns = attrCfg.columns();
        QVector<QgsAttributeTableConfig::ColumnConfig>::iterator it;

        for ( it = columns.begin(); it != columns.end(); ++it )
        {
          if ( it->name.compare( mLayer->fields().field( index ).name() ) == 0 )
            it->hidden = true;
        }

        attrCfg.setColumns( columns );
        mLayer->setAttributeTableConfig( attrCfg );
      }
      else if ( definition.standardTemplate() == QgsPropertyDefinition::ColorNoAlpha
                || definition.standardTemplate() == QgsPropertyDefinition::ColorWithAlpha )
      {
        const QgsEditorWidgetSetup setup = QgsEditorWidgetSetup( QStringLiteral( "Color" ), QVariantMap() );
        setEditorWidgetSetup( auxIndex, setup );
      }

      mLayer->setEditorWidgetSetup( index, editorWidgetSetup( auxIndex ) );
    }
  }

  return rc;
}

QgsFields QgsAuxiliaryLayer::auxiliaryFields() const
{
  QgsFields afields;

  for ( int i = 2; i < fields().count(); i++ ) // ignore rowid and PK field
    afields.append( createAuxiliaryField( fields().field( i ) ) );

  return afields;
}

bool QgsAuxiliaryLayer::deleteAttribute( int attr )
{
  QgsVectorLayer::deleteAttribute( attr );
  const bool rc = commitChanges();
  startEditing();
  return rc;
}

bool QgsAuxiliaryLayer::save()
{
  bool rc = false;

  if ( isEditable() )
  {
    rc = commitChanges();
  }

  startEditing();

  return rc;
}

int QgsAuxiliaryLayer::createProperty( QgsPalLayerSettings::Property property, QgsVectorLayer *layer, bool overwriteExisting )
{
  int index = -1;

  if ( layer && layer->labeling() && layer->auxiliaryLayer() )
  {
    // property definition are identical whatever the provider id
    const QgsPropertyDefinition def = QgsPalLayerSettings::propertyDefinitions()[property];
    const QString fieldName = nameFromProperty( def, true );

    layer->auxiliaryLayer()->addAuxiliaryField( def );

    if ( layer->auxiliaryLayer()->indexOfPropertyDefinition( def ) >= 0 )
    {
      const QStringList subProviderIds = layer->labeling()->subProviders();
      for ( const QString &providerId : subProviderIds )
      {
        QgsPalLayerSettings *settings = new QgsPalLayerSettings( layer->labeling()->settings( providerId ) );

        QgsPropertyCollection c = settings->dataDefinedProperties();

        // is there an existing property?
        const QgsProperty existingProperty = c.property( property );
        if ( existingProperty.propertyType() == QgsProperty::InvalidProperty
             || ( existingProperty.propertyType() == QgsProperty::FieldBasedProperty && existingProperty.field().isEmpty() )
             || ( existingProperty.propertyType() == QgsProperty::ExpressionBasedProperty && existingProperty.expressionString().isEmpty() )
             || overwriteExisting )
        {
          const QgsProperty prop = QgsProperty::fromField( fieldName );
          c.setProperty( property, prop );
        }
        else
        {
          // build a new smart expression as coalesce("new aux field", 'the' || 'old' || 'expression')
          const QgsProperty prop = QgsProperty::fromExpression( QStringLiteral( "coalesce(%1,%2)" ).arg( QgsExpression::quotedColumnRef( fieldName ),
                                   existingProperty.asExpression() ) );
          c.setProperty( property, prop );
        }
        settings->setDataDefinedProperties( c );

        layer->labeling()->setSettings( settings, providerId );
      }
    }

    index = layer->fields().lookupField( fieldName );
  }

  return index;
}

int QgsAuxiliaryLayer::createProperty( QgsDiagramLayerSettings::Property property, QgsVectorLayer *layer, bool overwriteExisting )
{
  int index = -1;

  if ( layer && layer->diagramLayerSettings() && layer->auxiliaryLayer() )
  {
    const QgsPropertyDefinition def = QgsDiagramLayerSettings::propertyDefinitions()[property];

    if ( layer->auxiliaryLayer()->addAuxiliaryField( def ) )
    {
      const QString fieldName = nameFromProperty( def, true );

      QgsDiagramLayerSettings settings( *layer->diagramLayerSettings() );

      QgsPropertyCollection c = settings.dataDefinedProperties();
      // is there an existing property?
      const QgsProperty existingProperty = c.property( property );
      if ( existingProperty.propertyType() == QgsProperty::InvalidProperty || overwriteExisting )
      {
        const QgsProperty prop = QgsProperty::fromField( fieldName );
        c.setProperty( property, prop );
      }
      else
      {
        // build a new smart expression as coalesce("new aux field", 'the' || 'old' || 'expression')
        const QgsProperty prop = QgsProperty::fromExpression( QStringLiteral( "coalesce(%1,%2)" ).arg( QgsExpression::quotedColumnRef( fieldName ),
                                 existingProperty.asExpression() ) );
        c.setProperty( property, prop );
      }
      settings.setDataDefinedProperties( c );

      layer->setDiagramLayerSettings( settings );
      index = layer->fields().lookupField( fieldName );
    }
  }

  return index;
}

int QgsAuxiliaryLayer::createProperty( QgsCallout::Property property, QgsVectorLayer *layer, bool overwriteExisting )
{
  int index = -1;

  if ( layer && layer->labeling() && layer->labeling()->settings().callout() && layer->auxiliaryLayer() )
  {
    // property definition are identical whatever the provider id
    const QgsPropertyDefinition def = QgsCallout::propertyDefinitions()[property];
    const QString fieldName = nameFromProperty( def, true );

    layer->auxiliaryLayer()->addAuxiliaryField( def );

    if ( layer->auxiliaryLayer()->indexOfPropertyDefinition( def ) >= 0 )
    {
      const QStringList subProviderIds = layer->labeling()->subProviders();
      for ( const QString &providerId : subProviderIds )
      {
        QgsPalLayerSettings *settings = new QgsPalLayerSettings( layer->labeling()->settings( providerId ) );
        if ( settings->callout() )
        {
          QgsPropertyCollection c = settings->callout()->dataDefinedProperties();
          // is there an existing property?
          const QgsProperty existingProperty = c.property( property );
          if ( existingProperty.propertyType() == QgsProperty::InvalidProperty || overwriteExisting )
          {
            const QgsProperty prop = QgsProperty::fromField( fieldName );
            c.setProperty( property, prop );
          }
          else
          {
            // build a new smart expression as coalesce("new aux field", 'the' || 'old' || 'expression')
            const QgsProperty prop = QgsProperty::fromExpression( QStringLiteral( "coalesce(%1,%2)" ).arg( QgsExpression::quotedColumnRef( fieldName ),
                                     existingProperty.asExpression() ) );
            c.setProperty( property, prop );
          }
          settings->callout()->setDataDefinedProperties( c );
        }
        layer->labeling()->setSettings( settings, providerId );
      }
    }

    index = layer->fields().lookupField( fieldName );
  }

  return index;
}

bool QgsAuxiliaryLayer::isHiddenProperty( int index ) const
{
  bool hidden = false;
  const QgsPropertyDefinition def = propertyDefinitionFromIndex( index );

  if ( def.origin().compare( QLatin1String( "labeling" ) ) == 0 )
  {
    const PalPropertyList &palProps = *palHiddenProperties();
    for ( const QgsPalLayerSettings::Property &p : palProps )
    {
      const QString propName = QgsPalLayerSettings::propertyDefinitions()[ p ].name();
      if ( propName.compare( def.name() ) == 0 )
      {
        hidden = true;
        break;
      }
    }
  }

  return hidden;
}

int QgsAuxiliaryLayer::propertyFromIndex( int index ) const
{
  int p = -1;
  const QgsPropertyDefinition aDef = propertyDefinitionFromIndex( index );

  if ( aDef.origin().compare( QLatin1String( "labeling" ) ) == 0 )
  {
    const QgsPropertiesDefinition defs = QgsPalLayerSettings::propertyDefinitions();
    QgsPropertiesDefinition::const_iterator it = defs.constBegin();
    for ( ; it != defs.constEnd(); ++it )
    {
      if ( it->name().compare( aDef.name(), Qt::CaseInsensitive ) == 0 )
      {
        p = it.key();
        break;
      }
    }
  }
  else if ( aDef.origin().compare( QLatin1String( "symbol" ) ) == 0 )
  {
    const QgsPropertiesDefinition defs = QgsSymbolLayer::propertyDefinitions();
    QgsPropertiesDefinition::const_iterator it = defs.constBegin();
    for ( ; it != defs.constEnd(); ++it )
    {
      if ( it->name().compare( aDef.name(), Qt::CaseInsensitive ) == 0 )
      {
        p = it.key();
        break;
      }
    }
  }
  else if ( aDef.origin().compare( QLatin1String( "diagram" ) ) == 0 )
  {
    const QgsPropertiesDefinition defs = QgsDiagramLayerSettings::propertyDefinitions();
    QgsPropertiesDefinition::const_iterator it = defs.constBegin();
    for ( ; it != defs.constEnd(); ++it )
    {
      if ( it->name().compare( aDef.name(), Qt::CaseInsensitive ) == 0 )
      {
        p = it.key();
        break;
      }
    }
  }

  return p;
}

QgsPropertyDefinition QgsAuxiliaryLayer::propertyDefinitionFromIndex( int index ) const
{
  return propertyDefinitionFromField( fields().field( index ) );
}

int QgsAuxiliaryLayer::indexOfPropertyDefinition( const QgsPropertyDefinition &def ) const
{
  return fields().indexOf( nameFromProperty( def ) );
}

QString QgsAuxiliaryLayer::nameFromProperty( const QgsPropertyDefinition &def, bool joined )
{
  QString fieldName = def.origin();

  if ( !def.name().isEmpty() )
    fieldName =  QStringLiteral( "%1_%2" ).arg( fieldName, def.name().toLower() );

  if ( !def.comment().isEmpty() )
    fieldName = QStringLiteral( "%1_%2" ).arg( fieldName, def.comment() );

  if ( joined )
    fieldName = QStringLiteral( "%1%2" ).arg( AS_JOINPREFIX, fieldName );

  return fieldName;
}

QgsField QgsAuxiliaryLayer::createAuxiliaryField( const QgsPropertyDefinition &def )
{
  QgsField afield;

  if ( !def.name().isEmpty() || !def.comment().isEmpty() )
  {
    QVariant::Type type = QVariant::Invalid;
    QString typeName;
    int len( 0 ), precision( 0 );
    switch ( def.dataType() )
    {
      case QgsPropertyDefinition::DataTypeString:
        type = QVariant::String;
        len = 50;
        typeName = QStringLiteral( "String" );
        break;
      case QgsPropertyDefinition::DataTypeNumeric:
        type = QVariant::Double;
        len = 0;
        precision = 0;
        typeName = QStringLiteral( "Real" );
        break;
      case QgsPropertyDefinition::DataTypeBoolean:
        type = QVariant::Int; // sqlite does not have a bool type
        typeName = QStringLiteral( "Integer" );
        break;
    }

    afield.setType( type );
    afield.setName( nameFromProperty( def ) );
    afield.setTypeName( typeName );
    afield.setLength( len );
    afield.setPrecision( precision );
  }

  return afield;
}

QgsPropertyDefinition QgsAuxiliaryLayer::propertyDefinitionFromField( const QgsField &f )
{
  QgsPropertyDefinition def;
  const QStringList parts = f.name().split( '_' );

  if ( parts.size() <= 1 )
    return def;

  const QString origin = parts[0];
  const QString propertyName = parts[1];

  if ( origin.compare( QLatin1String( "labeling" ), Qt::CaseInsensitive ) == 0 )
  {
    const QgsPropertiesDefinition props = QgsPalLayerSettings::propertyDefinitions();
    for ( auto it = props.constBegin(); it != props.constEnd(); ++it )
    {
      if ( it.value().name().compare( propertyName, Qt::CaseInsensitive ) == 0 )
      {
        def = it.value();
        if ( parts.size() >= 3 )
          def.setComment( parts.mid( 2 ).join( '_' ) );
        break;
      }
    }
  }
  else if ( origin.compare( QLatin1String( "symbol" ), Qt::CaseInsensitive ) == 0 )
  {
    const QgsPropertiesDefinition props = QgsSymbolLayer::propertyDefinitions();
    for ( auto it = props.constBegin(); it != props.constEnd(); ++it )
    {
      if ( it.value().name().compare( propertyName, Qt::CaseInsensitive ) == 0 )
      {
        def = it.value();
        if ( parts.size() >= 3 )
          def.setComment( parts.mid( 2 ).join( '_' ) );
        break;
      }
    }
  }
  else if ( origin.compare( QLatin1String( "diagram" ), Qt::CaseInsensitive ) == 0 )
  {
    const QgsPropertiesDefinition props = QgsDiagramLayerSettings::propertyDefinitions();
    for ( auto it = props.constBegin(); it != props.constEnd(); ++it )
    {
      if ( it.value().name().compare( propertyName, Qt::CaseInsensitive ) == 0 )
      {
        def = it.value();
        if ( parts.size() >= 3 )
          def.setComment( parts.mid( 2 ).join( '_' ) );
        break;
      }
    }
  }
  else
  {
    def.setOrigin( origin );
    def.setName( propertyName );
    switch ( f.type() )
    {
      case QVariant::Double:
        def.setDataType( QgsPropertyDefinition::DataTypeNumeric );
        break;

      case QVariant::Bool:
        def.setDataType( QgsPropertyDefinition::DataTypeBoolean );
        break;

      case QVariant::String:
      default:
        def.setDataType( QgsPropertyDefinition::DataTypeString );
        break;
    }

    if ( parts.size() >= 3 )
      def.setComment( parts.mid( 2 ).join( '_' ) );
  }

  return def;
}

QgsField QgsAuxiliaryLayer::createAuxiliaryField( const QgsField &field )
{
  const QgsPropertyDefinition def = propertyDefinitionFromField( field );
  QgsField afield;

  if ( !def.name().isEmpty() || !def.comment().isEmpty() )
  {
    afield = createAuxiliaryField( def );
    afield.setTypeName( field.typeName() );
  }

  return afield;
}

//
// QgsAuxiliaryStorage
//

QgsAuxiliaryStorage::QgsAuxiliaryStorage( const QgsProject &project, bool copy )
  : mCopy( copy )
{
  initTmpFileName();

  if ( !project.absoluteFilePath().isEmpty() )
  {
    mFileName = filenameForProject( project );
  }

  open( mFileName );
}

QgsAuxiliaryStorage::QgsAuxiliaryStorage( const QString &filename, bool copy )
  : mFileName( filename )
  , mCopy( copy )
{
  initTmpFileName();

  open( filename );
}

QgsAuxiliaryStorage::~QgsAuxiliaryStorage()
{
  QFile::remove( mTmpFileName );
}

bool QgsAuxiliaryStorage::isValid() const
{
  return mValid;
}

QString QgsAuxiliaryStorage::fileName() const
{
  return mFileName;
}

bool QgsAuxiliaryStorage::save() const
{
  if ( mFileName.isEmpty() )
  {
    // only a saveAs is available on a new database
    return false;
  }
  else if ( mCopy )
  {
    if ( QFile::exists( mFileName ) )
      QFile::remove( mFileName );

    return QFile::copy( mTmpFileName, mFileName );
  }
  else
  {
    // if the file is not empty the copy mode is not activated, then we're
    // directly working on the database since the beginning (no savepoints
    // /rollback for now)
    return true;
  }
}

QgsAuxiliaryLayer *QgsAuxiliaryStorage::createAuxiliaryLayer( const QgsField &field, QgsVectorLayer *layer ) const
{
  QgsAuxiliaryLayer *alayer = nullptr;

  if ( mValid && layer )
  {
    const QString table( layer->id() );
    sqlite3_database_unique_ptr database;
    database = openDB( currentFileName() );

    if ( !tableExists( table, database.get() ) )
    {
      if ( !createTable( field.typeName(), table, database.get(), mErrorString ) )
      {
        return alayer;
      }
    }

    alayer = new QgsAuxiliaryLayer( field.name(), currentFileName(), table, layer );
    alayer->startEditing();
  }

  return alayer;
}

bool QgsAuxiliaryStorage::deleteTable( const QgsDataSourceUri &ogrUri )
{
  bool rc = false;
  const QgsDataSourceUri uri = parseOgrUri( ogrUri );

  if ( !uri.database().isEmpty() && !uri.table().isEmpty() )
  {
    sqlite3_database_unique_ptr database;
    database = openDB( uri.database() );

    if ( database )
    {
      QString sql = QStringLiteral( "DROP TABLE %1" ).arg( uri.table() );
      rc = exec( sql, database.get() );

      sql = QStringLiteral( "VACUUM" );
      rc = exec( sql, database.get() );
    }
  }

  return rc;
}

bool QgsAuxiliaryStorage::duplicateTable( const QgsDataSourceUri &ogrUri, const QString &newTable )
{
  const QgsDataSourceUri uri = parseOgrUri( ogrUri );
  bool rc = false;

  if ( !uri.table().isEmpty() && !uri.database().isEmpty() )
  {
    sqlite3_database_unique_ptr database;
    database = openDB( uri.database() );

    if ( database )
    {
      const QString sql = QStringLiteral( "CREATE TABLE %1 AS SELECT * FROM %2" ).arg( newTable, uri.table() );
      rc = exec( sql, database.get() );
    }
  }

  return rc;
}

QString QgsAuxiliaryStorage::errorString() const
{
  return mErrorString;
}

bool QgsAuxiliaryStorage::saveAs( const QString &filename )
{
  mErrorString.clear();

  QFile dest( filename );
  if ( dest.exists() && !dest.remove() )
  {
    mErrorString = dest.errorString();
    return false;
  }

  QFile origin( currentFileName() );
  if ( !origin.copy( filename ) )
  {
    mErrorString = origin.errorString();
    return false;
  }

  return true;
}

bool QgsAuxiliaryStorage::saveAs( const QgsProject &project )
{
  return saveAs( filenameForProject( project ) );
}

QString QgsAuxiliaryStorage::extension()
{
  return AS_EXTENSION;
}

bool QgsAuxiliaryStorage::exists( const QgsProject &project )
{
  const QFileInfo fileinfo( filenameForProject( project ) );
  return fileinfo.exists() && fileinfo.isFile();
}

bool QgsAuxiliaryStorage::exec( const QString &sql, sqlite3 *handler )
{
  bool rc = false;

  if ( handler )
  {
    const int err = sqlite3_exec( handler, sql.toStdString().c_str(), nullptr, nullptr, nullptr );

    if ( err == SQLITE_OK )
      rc = true;
    else
      debugMsg( sql, handler );
  }

  return rc;
}

QString QgsAuxiliaryStorage::debugMsg( const QString &sql, sqlite3 *handler )
{
  const QString err = QString::fromUtf8( sqlite3_errmsg( handler ) );
  const QString msg = QObject::tr( "Unable to execute" );
  const QString errMsg = QObject::tr( "%1 '%2': %3" ).arg( msg, sql, err );
  QgsDebugMsg( errMsg );
  return errMsg;
}

bool QgsAuxiliaryStorage::createTable( const QString &type, const QString &table, sqlite3 *handler, QString &errorMsg )
{
  const QString sql = QStringLiteral( "CREATE TABLE IF NOT EXISTS '%1' ( '%2' %3  )" ).arg( table, AS_JOINFIELD, type );

  if ( !exec( sql, handler ) )
  {
    errorMsg = QgsAuxiliaryStorage::debugMsg( sql, handler );
    return false;
  }

  return true;
}

sqlite3_database_unique_ptr QgsAuxiliaryStorage::createDB( const QString &filename )
{
  sqlite3_database_unique_ptr database;

  int rc;
  rc = database.open_v2( filename, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr );
  if ( rc )
  {
    debugMsg( QStringLiteral( "sqlite3_open_v2" ), database.get() );
  }
  else
    // activating Foreign Key constraints
    exec( QStringLiteral( "PRAGMA foreign_keys = 1" ), database.get() );

  return database;
}

sqlite3_database_unique_ptr QgsAuxiliaryStorage::openDB( const QString &filename )
{
  sqlite3_database_unique_ptr database;
  const int rc = database.open_v2( filename, SQLITE_OPEN_READWRITE, nullptr );

  if ( rc )
  {
    debugMsg( QStringLiteral( "sqlite3_open_v2" ), database.get() );
  }

  return database;
}

bool QgsAuxiliaryStorage::tableExists( const QString &table, sqlite3 *handler )
{
  const QString sql = QStringLiteral( "SELECT 1 FROM sqlite_master WHERE type='table' AND name='%1'" ).arg( table );
  int rows = 0;
  int columns = 0;
  char **results = nullptr;
  const int rc = sqlite3_get_table( handler, sql.toStdString().c_str(), &results, &rows, &columns, nullptr );
  if ( rc != SQLITE_OK )
  {
    debugMsg( sql, handler );
    return false;
  }

  sqlite3_free_table( results );
  if ( rows >= 1 )
    return true;

  return false;
}

sqlite3_database_unique_ptr QgsAuxiliaryStorage::open( const QString &filename )
{
  sqlite3_database_unique_ptr database;

  if ( filename.isEmpty() )
  {
    if ( ( database = createDB( currentFileName() ) ) )
      mValid = true;
  }
  else if ( QFile::exists( filename ) )
  {
    if ( mCopy )
      QFile::copy( filename, mTmpFileName );

    if ( ( database = openDB( currentFileName() ) ) )
      mValid = true;
  }
  else
  {
    if ( ( database = createDB( currentFileName() ) ) )
      mValid = true;
  }

  return database;
}

sqlite3_database_unique_ptr QgsAuxiliaryStorage::open( const QgsProject &project )
{
  return open( filenameForProject( project ) );
}

QString QgsAuxiliaryStorage::filenameForProject( const QgsProject &project )
{
  const QFileInfo info( project.absoluteFilePath() );
  const QString path = info.path() + QDir::separator() + info.baseName();
  return path + '.' + QgsAuxiliaryStorage::extension();
}

void QgsAuxiliaryStorage::initTmpFileName()
{
  QTemporaryFile tmpFile;
  tmpFile.open();
  tmpFile.close();
  mTmpFileName = tmpFile.fileName();
}

QString QgsAuxiliaryStorage::currentFileName() const
{
  if ( mCopy || mFileName.isEmpty() )
    return mTmpFileName;
  else
    return mFileName;
}

QgsDataSourceUri QgsAuxiliaryStorage::parseOgrUri( const QgsDataSourceUri &uri )
{
  QgsDataSourceUri newUri;

  // parsing for ogr style uri :
  // " filePath|layername='tableName' table="" sql="
  QStringList uriParts = uri.uri().split( '|' );
  if ( uriParts.count() < 2 )
    return newUri;

  const QString databasePath = uriParts[0].replace( ' ', QString() );

  const QString table = uriParts[1];
  QStringList tableParts = table.split( ' ' );

  if ( tableParts.count() < 1 )
    return newUri;

  const QString tableName = tableParts[0].replace( QLatin1String( "layername=" ), QString() );

  newUri.setDataSource( QString(), tableName, QString() );
  newUri.setDatabase( databasePath );

  return newUri;
}
