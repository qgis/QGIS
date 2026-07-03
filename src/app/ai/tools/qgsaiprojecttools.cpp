/***************************************************************************
    qgsaiprojecttools.cpp
    ---------------------
    begin                : July 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsaiprojecttools.h"

#include "qgsaitoolschemautil.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsproject.h"
#include "qgssnappingconfig.h"
#include "qgsvectorlayer.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <QUuid>
#include <QVariant>

using namespace Qt::StringLiterals;

namespace
{
  enum class ProjectRollbackType
  {
    RestoreFile,
    RestoreCrs,
    RestoreProperty
  };

  struct ProjectRollbackEntry
  {
      ProjectRollbackType type = ProjectRollbackType::RestoreFile;
      QString description;

      QString filePath;
      QString previousFileName;
      bool hadFile = false;
      QByteArray fileContent;

      QString crsDefinition;

      QString scope;
      QString key;
      bool hadEntry = false;
      QString oldValue;
  };

  QHash<QString, ProjectRollbackEntry> &projectRollbackStore()
  {
    static QHash<QString, ProjectRollbackEntry> store;
    return store;
  }

  QString storeProjectRollback( const ProjectRollbackEntry &entry )
  {
    const QString token = QUuid::createUuid().toString( QUuid::WithoutBraces );
    projectRollbackStore().insert( token, entry );
    return token;
  }

  QJsonObject rollbackJson( const QString &token, const QString &action )
  {
    QJsonObject rollback;
    rollback.insert( u"token"_s, token );
    rollback.insert( u"action"_s, action );
    rollback.insert( u"volatile"_s, true );
    return rollback;
  }

  QJsonObject projectSummary( const QgsProject *project )
  {
    QJsonObject output;
    if ( !project )
      return output;

    output.insert( u"file_name"_s, project->fileName() );
    output.insert( u"absolute_file_path"_s, project->absoluteFilePath() );
    output.insert( u"absolute_path"_s, project->absolutePath() );
    output.insert( u"base_name"_s, project->baseName() );
    output.insert( u"is_dirty"_s, project->isDirty() );
    output.insert( u"layer_count"_s, project->mapLayers().size() );

    const QgsCoordinateReferenceSystem crs = project->crs();
    QJsonObject crsJson;
    crsJson.insert( u"authid"_s, crs.authid() );
    crsJson.insert( u"description"_s, crs.description() );
    crsJson.insert( u"is_valid"_s, crs.isValid() );
    output.insert( u"crs"_s, crsJson );
    return output;
  }

  bool snapshotFile( const QString &path, ProjectRollbackEntry &entry, QString &error )
  {
    entry.filePath = path;
    const QFileInfo info( path );
    entry.hadFile = info.exists();
    if ( !entry.hadFile )
      return true;

    if ( !info.isFile() )
    {
      error = u"Target path exists but is not a regular file: %1"_s.arg( path );
      return false;
    }

    QFile file( path );
    if ( !file.open( QIODevice::ReadOnly ) )
    {
      error = u"Could not read existing project file before save: %1"_s.arg( file.errorString() );
      return false;
    }

    entry.fileContent = file.readAll();
    return true;
  }

  bool restoreFileSnapshot( const ProjectRollbackEntry &entry, QString &error )
  {
    if ( entry.hadFile )
    {
      QFile file( entry.filePath );
      if ( !file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
      {
        error = u"Could not reopen project file for rollback: %1"_s.arg( file.errorString() );
        return false;
      }
      if ( file.write( entry.fileContent ) != entry.fileContent.size() )
      {
        error = u"Could not restore previous project file contents: %1"_s.arg( file.errorString() );
        return false;
      }
      return true;
    }

    if ( QFileInfo::exists( entry.filePath ) && !QFile::remove( entry.filePath ) )
    {
      error = u"Could not remove project file created by AI save operation: %1"_s.arg( entry.filePath );
      return false;
    }
    return true;
  }

  QString crsRollbackDefinition( const QgsCoordinateReferenceSystem &crs )
  {
    if ( !crs.authid().isEmpty() )
      return crs.authid();
    return u"WKT:%1"_s.arg( crs.toWkt() );
  }

  bool splitProjectKey( const QString &rawKey, QString &scope, QString &entryKey, QString &error )
  {
    const QString trimmed = rawKey.trimmed();
    if ( trimmed.isEmpty() )
    {
      error = u"Argument 'key' is required."_s;
      return false;
    }

    const int slash = trimmed.indexOf( u'/' );
    if ( slash > 0 )
    {
      scope = trimmed.left( slash ).trimmed();
      entryKey = trimmed.mid( slash ).trimmed();
    }
    else
    {
      scope = u"AI"_s;
      entryKey = trimmed;
    }

    if ( scope.isEmpty() || entryKey.isEmpty() || entryKey == "/"_L1 )
    {
      error = u"Argument 'key' must identify a project entry, e.g. 'AI/last_action'."_s;
      return false;
    }

    if ( !entryKey.startsWith( u'/' ) )
      entryKey.prepend( u'/' );
    return true;
  }

  bool writeProjectEntry( QgsProject *project, const QString &scope, const QString &key, const QJsonValue &value )
  {
    if ( value.isBool() )
      return project->writeEntry( scope, key, value.toBool() );
    if ( value.isDouble() )
    {
      const double doubleValue = value.toDouble();
      const int intValue = static_cast<int>( doubleValue );
      if ( doubleValue == intValue )
        return project->writeEntry( scope, key, intValue );
      return project->writeEntry( scope, key, doubleValue );
    }
    return project->writeEntry( scope, key, value.toVariant().toString() );
  }

  QgsAiToolResult rollbackProjectChange( QgsProject *project, const QString &token )
  {
    if ( !projectRollbackStore().contains( token ) )
      return QgsAiToolResult::error( u"Unknown or expired rollback token."_s );
    if ( !project )
      return QgsAiToolResult::error( u"No active QgsProject available."_s );

    const ProjectRollbackEntry entry = projectRollbackStore().take( token );
    QJsonObject diff;
    diff.insert( u"summary"_s, entry.description );

    switch ( entry.type )
    {
      case ProjectRollbackType::RestoreFile:
      {
        QString error;
        if ( !restoreFileSnapshot( entry, error ) )
          return QgsAiToolResult::error( error );
        const QString beforeFileName = project->fileName();
        project->setFileName( entry.previousFileName );
        diff.insert( u"file_path"_s, entry.filePath );
        diff.insert( u"before_file_name"_s, beforeFileName );
        diff.insert( u"after_file_name"_s, project->fileName() );
        break;
      }

      case ProjectRollbackType::RestoreCrs:
      {
        const QgsCoordinateReferenceSystem beforeCrs = project->crs();
        const QgsCoordinateReferenceSystem oldCrs( entry.crsDefinition );
        if ( !oldCrs.isValid() )
          return QgsAiToolResult::error( u"Cannot rollback project CRS: stored CRS is invalid."_s );
        project->setCrs( oldCrs );
        diff.insert( u"before_crs"_s, beforeCrs.authid() );
        diff.insert( u"after_crs"_s, project->crs().authid() );
        break;
      }

      case ProjectRollbackType::RestoreProperty:
      {
        bool ok = false;
        const QString beforeValue = project->readEntry( entry.scope, entry.key, QString(), &ok );
        Q_UNUSED( ok )
        if ( entry.hadEntry )
        {
          if ( !project->writeEntry( entry.scope, entry.key, entry.oldValue ) )
            return QgsAiToolResult::error( u"Could not restore project property '%1%2'."_s.arg( entry.scope, entry.key ) );
        }
        else if ( !project->removeEntry( entry.scope, entry.key ) )
        {
          return QgsAiToolResult::error( u"Could not remove project property '%1%2'."_s.arg( entry.scope, entry.key ) );
        }
        diff.insert( u"scope"_s, entry.scope );
        diff.insert( u"key"_s, entry.key );
        diff.insert( u"before"_s, beforeValue );
        diff.insert( u"after"_s, entry.hadEntry ? entry.oldValue : QJsonValue() );
        break;
      }
    }

    QJsonObject output;
    output.insert( u"status"_s, u"rolled_back"_s );
    output.insert( u"rollback_token"_s, token );
    output.insert( u"diff"_s, diff );
    return QgsAiToolResult::ok( output );
  }

  QString snappingModeName( Qgis::SnappingMode mode )
  {
    switch ( mode )
    {
      case Qgis::SnappingMode::ActiveLayer:
        return u"active_layer"_s;
      case Qgis::SnappingMode::AllLayers:
        return u"all_layers"_s;
      case Qgis::SnappingMode::AdvancedConfiguration:
        return u"advanced"_s;
    }
    return u"active_layer"_s;
  }

  bool snappingModeFromString( const QString &value, Qgis::SnappingMode &mode )
  {
    const QString normalized = value.toLower().trimmed();
    if ( normalized == "active_layer"_L1 || normalized == "active"_L1 )
    {
      mode = Qgis::SnappingMode::ActiveLayer;
      return true;
    }
    if ( normalized == "all_layers"_L1 || normalized == "all"_L1 )
    {
      mode = Qgis::SnappingMode::AllLayers;
      return true;
    }
    if ( normalized == "advanced"_L1 || normalized == "per_layer"_L1 )
    {
      mode = Qgis::SnappingMode::AdvancedConfiguration;
      return true;
    }
    return false;
  }

  QString snappingTypeName( Qgis::SnappingTypes type )
  {
    if ( type == Qgis::SnappingType::Vertex )
      return u"vertex"_s;
    if ( type == Qgis::SnappingType::Segment )
      return u"segment"_s;
    if ( type == ( Qgis::SnappingType::Vertex | Qgis::SnappingType::Segment ) )
      return u"vertex_and_segment"_s;
    if ( type == Qgis::SnappingType::NoSnap )
      return u"none"_s;
    return u"custom"_s;
  }

  bool snappingTypeFromString( const QString &value, Qgis::SnappingTypes &type )
  {
    const QString normalized = value.toLower().trimmed();
    if ( normalized == "none"_L1 || normalized == "no_snap"_L1 )
    {
      type = Qgis::SnappingType::NoSnap;
      return true;
    }
    if ( normalized == "vertex"_L1 || normalized == "vertices"_L1 )
    {
      type = Qgis::SnappingType::Vertex;
      return true;
    }
    if ( normalized == "segment"_L1 || normalized == "segments"_L1 )
    {
      type = Qgis::SnappingType::Segment;
      return true;
    }
    if ( normalized == "vertex_and_segment"_L1 || normalized == "vertex_segment"_L1 || normalized == "both"_L1 )
    {
      type = Qgis::SnappingType::Vertex | Qgis::SnappingType::Segment;
      return true;
    }
    return false;
  }

  QString mapToolUnitName( Qgis::MapToolUnit unit )
  {
    switch ( unit )
    {
      case Qgis::MapToolUnit::Layer:
        return u"layer"_s;
      case Qgis::MapToolUnit::Pixels:
        return u"pixels"_s;
      case Qgis::MapToolUnit::Project:
        return u"project"_s;
    }
    return u"project"_s;
  }

  bool mapToolUnitFromString( const QString &value, Qgis::MapToolUnit &unit )
  {
    const QString normalized = value.toLower().trimmed();
    if ( normalized == "layer"_L1 || normalized == "layer_units"_L1 )
    {
      unit = Qgis::MapToolUnit::Layer;
      return true;
    }
    if ( normalized == "pixels"_L1 || normalized == "pixel"_L1 )
    {
      unit = Qgis::MapToolUnit::Pixels;
      return true;
    }
    if ( normalized == "project"_L1 || normalized == "map"_L1 || normalized == "project_units"_L1 )
    {
      unit = Qgis::MapToolUnit::Project;
      return true;
    }
    return false;
  }

  QJsonObject snappingConfigJson( const QgsSnappingConfig &config )
  {
    QJsonObject output;
    output.insert( u"enabled"_s, config.enabled() );
    output.insert( u"mode"_s, snappingModeName( config.mode() ) );
    output.insert( u"type"_s, snappingTypeName( config.typeFlag() ) );
    output.insert( u"tolerance"_s, config.tolerance() );
    output.insert( u"units"_s, mapToolUnitName( config.units() ) );
    output.insert( u"intersection_snapping"_s, config.intersectionSnapping() );
    output.insert( u"self_snapping"_s, config.selfSnapping() );

    QJsonArray layerSettings;
    const auto settings = config.individualLayerSettings();
    for ( auto it = settings.constBegin(); it != settings.constEnd(); ++it )
    {
      QgsVectorLayer *layer = it.key();
      const QgsSnappingConfig::IndividualLayerSettings layerConfig = it.value();
      if ( !layer || !layerConfig.valid() )
        continue;
      QJsonObject entry;
      entry.insert( u"layer_id"_s, layer->id() );
      entry.insert( u"enabled"_s, layerConfig.enabled() );
      entry.insert( u"type"_s, snappingTypeName( layerConfig.typeFlag() ) );
      entry.insert( u"tolerance"_s, layerConfig.tolerance() );
      entry.insert( u"units"_s, mapToolUnitName( layerConfig.units() ) );
      layerSettings.push_back( entry );
    }
    output.insert( u"layer_settings"_s, layerSettings );
    return output;
  }
} //namespace

QgsAiManageProjectTool::QgsAiManageProjectTool( QgsProject *project )
  : mProject( project )
{}

QString QgsAiManageProjectTool::description() const
{
  return QStringLiteral(
    "Manages the active QGIS project. Supported actions are save, save_as, "
    "get_properties, set_crs and set_property. Mutating actions return a volatile "
    "rollback token."
  );
}

QJsonObject QgsAiManageProjectTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"action"_s, prop( u"string"_s, u"One of: save, save_as, get_properties, set_crs, set_property."_s ) );
  properties.insert( u"path"_s, prop( u"string"_s, u"Destination path for save_as."_s ) );
  properties.insert( u"crs"_s, prop( u"string"_s, u"Project CRS definition, e.g. EPSG:3857."_s ) );
  properties.insert( u"key"_s, prop( u"string"_s, u"Project property key. Use 'Scope/name' or defaults to the AI scope."_s ) );
  properties.insert( u"value"_s, prop( u"string"_s, u"Value for set_property. Boolean and numeric JSON values are preserved."_s ) );
  properties.insert( u"overwrite"_s, prop( u"boolean"_s, u"Allow save_as to overwrite an existing file."_s ) );
  properties.insert( u"rollback_token"_s, prop( u"string"_s, u"Optional token returned by a previous mutating manage_project call."_s ) );
  return schemaObject( properties );
}

QgsAiToolResult QgsAiManageProjectTool::execute( const QJsonObject &args )
{
  QgsProject *project = mProject ? mProject : QgsProject::instance();
  if ( !project )
    return QgsAiToolResult::error( u"No active QgsProject available."_s );

  const QString rollbackToken = args.value( u"rollback_token"_s ).toString().trimmed();
  if ( !rollbackToken.isEmpty() )
    return rollbackProjectChange( project, rollbackToken );

  const QString action = args.value( u"action"_s ).toString().toLower().trimmed();
  if ( action.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'action' is required."_s );

  if ( action == "get_properties"_L1 )
  {
    QJsonObject output = projectSummary( project );
    const QString rawKey = args.value( u"key"_s ).toString();
    if ( !rawKey.trimmed().isEmpty() )
    {
      QString scope;
      QString entryKey;
      QString error;
      if ( !splitProjectKey( rawKey, scope, entryKey, error ) )
        return QgsAiToolResult::error( error );

      bool ok = false;
      const QString value = project->readEntry( scope, entryKey, QString(), &ok );
      QJsonObject property;
      property.insert( u"scope"_s, scope );
      property.insert( u"key"_s, entryKey );
      property.insert( u"exists"_s, ok );
      if ( ok )
        property.insert( u"value"_s, value );
      output.insert( u"property"_s, property );
    }
    return QgsAiToolResult::ok( output );
  }

  if ( action == "save"_L1 || action == "save_as"_L1 )
  {
    QString path = action == "save_as"_L1 ? args.value( u"path"_s ).toString().trimmed() : project->fileName();
    if ( path.isEmpty() )
      return QgsAiToolResult::error( action == "save_as"_L1 ? u"Argument 'path' is required for save_as."_s : u"Project has no file name; use save_as first."_s );

    const QFileInfo targetInfo( path );
    if ( targetInfo.exists() && targetInfo.isDir() )
      return QgsAiToolResult::error( u"Target path is a directory, not a project file: %1"_s.arg( path ) );

    const bool overwrite = args.value( u"overwrite"_s ).toBool( false );
    if ( action == "save_as"_L1 && targetInfo.exists() && !overwrite )
      return QgsAiToolResult::error( u"Refusing to overwrite existing project file without overwrite=true: %1"_s.arg( path ) );

    const QString parentPath = targetInfo.absolutePath();
    if ( !QDir().mkpath( parentPath ) )
      return QgsAiToolResult::error( u"Could not create project directory: %1"_s.arg( parentPath ) );

    ProjectRollbackEntry entry;
    entry.type = ProjectRollbackType::RestoreFile;
    entry.description = action == "save_as"_L1 ? u"Restored project file state from before save_as."_s : u"Restored project file state from before save."_s;
    entry.previousFileName = project->fileName();

    QString snapshotError;
    if ( !snapshotFile( path, entry, snapshotError ) )
      return QgsAiToolResult::error( snapshotError );

    const bool wrote = action == "save_as"_L1 ? project->write( path ) : project->write();
    if ( !wrote )
      return QgsAiToolResult::error( u"Could not write project file: %1"_s.arg( path ) );

    const QString token = storeProjectRollback( entry );
    QJsonObject diff;
    diff.insert( u"summary"_s, action == "save_as"_L1 ? u"Saved project to a new path."_s : u"Saved project to its current path."_s );
    diff.insert( u"file_path"_s, path );
    diff.insert( u"previous_file_name"_s, entry.previousFileName );
    diff.insert( u"current_file_name"_s, project->fileName() );
    diff.insert( u"overwrote_existing_file"_s, entry.hadFile );

    QJsonObject output = projectSummary( project );
    output.insert( u"status"_s, u"saved"_s );
    output.insert( u"rollback_token"_s, token );
    output.insert( u"rollback"_s, rollbackJson( token, name() ) );
    output.insert( u"diff"_s, diff );
    return QgsAiToolResult::ok( output );
  }

  if ( action == "set_crs"_L1 )
  {
    const QString crsDefinition = args.value( u"crs"_s ).toString().trimmed();
    if ( crsDefinition.isEmpty() )
      return QgsAiToolResult::error( u"Argument 'crs' is required for set_crs."_s );

    const QgsCoordinateReferenceSystem crs( crsDefinition );
    if ( !crs.isValid() )
      return QgsAiToolResult::error( u"Invalid CRS definition: %1"_s.arg( crsDefinition ) );

    const QgsCoordinateReferenceSystem oldCrs = project->crs();
    project->setCrs( crs );

    ProjectRollbackEntry entry;
    entry.type = ProjectRollbackType::RestoreCrs;
    entry.description = u"Restored previous project CRS."_s;
    entry.crsDefinition = crsRollbackDefinition( oldCrs );
    const QString token = storeProjectRollback( entry );

    QJsonObject diff;
    diff.insert( u"summary"_s, u"Changed project CRS."_s );
    diff.insert( u"before"_s, oldCrs.authid() );
    diff.insert( u"after"_s, project->crs().authid() );

    QJsonObject output = projectSummary( project );
    output.insert( u"status"_s, u"updated"_s );
    output.insert( u"rollback_token"_s, token );
    output.insert( u"rollback"_s, rollbackJson( token, name() ) );
    output.insert( u"diff"_s, diff );
    return QgsAiToolResult::ok( output );
  }

  if ( action == "set_property"_L1 )
  {
    QString scope;
    QString key;
    QString error;
    if ( !splitProjectKey( args.value( u"key"_s ).toString(), scope, key, error ) )
      return QgsAiToolResult::error( error );
    if ( !args.contains( u"value"_s ) )
      return QgsAiToolResult::error( u"Argument 'value' is required for set_property."_s );

    bool hadEntry = false;
    const QString oldValue = project->readEntry( scope, key, QString(), &hadEntry );
    if ( !writeProjectEntry( project, scope, key, args.value( u"value"_s ) ) )
      return QgsAiToolResult::error( u"Could not write project property '%1%2'."_s.arg( scope, key ) );

    bool ok = false;
    const QString newValue = project->readEntry( scope, key, QString(), &ok );
    Q_UNUSED( ok )

    ProjectRollbackEntry entry;
    entry.type = ProjectRollbackType::RestoreProperty;
    entry.description = u"Restored previous project property value."_s;
    entry.scope = scope;
    entry.key = key;
    entry.hadEntry = hadEntry;
    entry.oldValue = oldValue;
    const QString token = storeProjectRollback( entry );

    QJsonObject diff;
    diff.insert( u"summary"_s, u"Updated project property."_s );
    diff.insert( u"scope"_s, scope );
    diff.insert( u"key"_s, key );
    if ( hadEntry )
      diff.insert( u"before"_s, oldValue );
    else
      diff.insert( u"before"_s, QJsonValue() );
    diff.insert( u"after"_s, newValue );

    QJsonObject output = projectSummary( project );
    output.insert( u"status"_s, u"updated"_s );
    output.insert( u"rollback_token"_s, token );
    output.insert( u"rollback"_s, rollbackJson( token, name() ) );
    output.insert( u"diff"_s, diff );
    return QgsAiToolResult::ok( output );
  }

  return QgsAiToolResult::error( u"Unsupported project action: %1"_s.arg( action ) );
}

QgsAiConfigureSnappingTool::QgsAiConfigureSnappingTool( QgsProject *project )
  : mProject( project )
{}

QString QgsAiConfigureSnappingTool::description() const
{
  return QStringLiteral(
    "Reads or updates the active QGIS project's snapping configuration. "
    "Set supports enabled, mode, type, tolerance, units and optional per-layer ids."
  );
}

QJsonObject QgsAiConfigureSnappingTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"action"_s, prop( u"string"_s, u"One of: get, set."_s ) );
  properties.insert( u"enabled"_s, prop( u"boolean"_s, u"Whether snapping is enabled."_s ) );
  properties.insert( u"mode"_s, prop( u"string"_s, u"active_layer, all_layers or advanced."_s ) );
  properties.insert( u"type"_s, prop( u"string"_s, u"vertex, segment, vertex_and_segment or none."_s ) );
  properties.insert( u"tolerance"_s, prop( u"number"_s, u"Snapping tolerance. Must be >= 0."_s ) );
  properties.insert( u"units"_s, prop( u"string"_s, u"pixels, project or layer."_s ) );
  properties.insert( u"layer_ids"_s, prop( u"array"_s, u"Optional vector layer ids for advanced per-layer snapping."_s ) );
  return schemaObject( properties );
}

QgsAiToolResult QgsAiConfigureSnappingTool::execute( const QJsonObject &args )
{
  QgsProject *project = mProject ? mProject : QgsProject::instance();
  if ( !project )
    return QgsAiToolResult::error( u"No active QgsProject available."_s );

  const QString action = args.value( u"action"_s ).toString( u"get"_s ).toLower().trimmed();
  if ( action == "get"_L1 )
    return QgsAiToolResult::ok( snappingConfigJson( project->snappingConfig() ) );
  if ( action != "set"_L1 )
    return QgsAiToolResult::error( u"Unsupported snapping action: %1"_s.arg( action ) );

  QgsSnappingConfig config = project->snappingConfig();
  const QJsonObject before = snappingConfigJson( config );

  if ( args.contains( u"enabled"_s ) )
    config.setEnabled( args.value( u"enabled"_s ).toBool() );

  if ( args.contains( u"mode"_s ) )
  {
    Qgis::SnappingMode mode = config.mode();
    if ( !snappingModeFromString( args.value( u"mode"_s ).toString(), mode ) )
      return QgsAiToolResult::error( u"Invalid snapping mode. Use active_layer, all_layers or advanced."_s );
    config.setMode( mode );
  }

  if ( args.contains( u"type"_s ) )
  {
    Qgis::SnappingTypes type = config.typeFlag();
    if ( !snappingTypeFromString( args.value( u"type"_s ).toString(), type ) )
      return QgsAiToolResult::error( u"Invalid snapping type. Use vertex, segment, vertex_and_segment or none."_s );
    config.setTypeFlag( type );
  }

  if ( args.contains( u"tolerance"_s ) )
  {
    const double tolerance = args.value( u"tolerance"_s ).toDouble( config.tolerance() );
    if ( tolerance < 0.0 )
      return QgsAiToolResult::error( u"Invalid snapping tolerance. Value must be >= 0."_s );
    config.setTolerance( tolerance );
  }

  if ( args.contains( u"units"_s ) )
  {
    Qgis::MapToolUnit units = config.units();
    if ( !mapToolUnitFromString( args.value( u"units"_s ).toString(), units ) )
      return QgsAiToolResult::error( u"Invalid snapping units. Use pixels, project or layer."_s );
    config.setUnits( units );
  }

  if ( args.value( u"layer_ids"_s ).isArray() )
  {
    const QJsonArray layerIds = args.value( u"layer_ids"_s ).toArray();
    config.clearIndividualLayerSettings();
    config.setMode( Qgis::SnappingMode::AdvancedConfiguration );
    for ( const QJsonValue &layerValue : layerIds )
    {
      const QString layerId = layerValue.toString().trimmed();
      QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( project->mapLayer( layerId ) );
      if ( !layer )
        return QgsAiToolResult::error( u"Invalid vector layer id for snapping: %1"_s.arg( layerId ) );

      config.setIndividualLayerSettings( layer, QgsSnappingConfig::IndividualLayerSettings( config.enabled(), config.typeFlag(), config.tolerance(), config.units() ) );
    }
  }

  project->setSnappingConfig( config );

  QJsonObject diff;
  diff.insert( u"summary"_s, u"Updated project snapping configuration."_s );
  diff.insert( u"before"_s, before );
  diff.insert( u"after"_s, snappingConfigJson( project->snappingConfig() ) );

  QJsonObject output = snappingConfigJson( project->snappingConfig() );
  output.insert( u"status"_s, u"updated"_s );
  output.insert( u"diff"_s, diff );
  return QgsAiToolResult::ok( output );
}
