#include "qgsagentplugin.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>

#include <algorithm>

using namespace Qt::StringLiterals;

namespace
{
  QString workflowDirectory()
  {
    return QDir( QStandardPaths::writableLocation( QStandardPaths::AppDataLocation ) ).filePath( u"qgis-agent/workflows"_s );
  }

  QString safeWorkflowName( QString name )
  {
    name = name.trimmed();
    name.replace( QRegularExpression( u"[^A-Za-z0-9._-]+"_s ), u"_"_s );
    return name.left( 120 );
  }

  QJsonValue resolveWorkflowValue( const QJsonValue &value, const QHash<QString, QJsonObject> &results, const QHash<QString, QString> &variables )
  {
    if ( value.isArray() )
    {
      QJsonArray resolved;
      for ( const QJsonValue &entry : value.toArray() )
        resolved.append( resolveWorkflowValue( entry, results, variables ) );
      return resolved;
    }
    if ( value.isObject() )
    {
      QJsonObject resolved;
      const QJsonObject object = value.toObject();
      for ( auto it = object.constBegin(); it != object.constEnd(); ++it )
        resolved.insert( it.key(), resolveWorkflowValue( it.value(), results, variables ) );
      return resolved;
    }
    if ( !value.isString() )
      return value;

    QString text = value.toString();
    const QRegularExpressionMatch resultMatch = QRegularExpression( u"^\\$\\{([A-Za-z0-9_-]+)\\.([A-Za-z0-9_-]+)\\}$"_s ).match( text );
    if ( resultMatch.hasMatch() )
      return results.value( resultMatch.captured( 1 ) ).value( u"results"_s ).toObject().value( resultMatch.captured( 2 ) );
    for ( auto it = variables.constBegin(); it != variables.constEnd(); ++it )
      text.replace( u"${%1}"_s.arg( it.key() ), it.value() );
    return text;
  }

  QJsonObject workflowReport( const QString &name, int startStep, const QJsonArray &stepResults, bool ok )
  {
    return QJsonObject{
      { u"ok"_s, ok },
      { u"name"_s, name },
      { u"start_step"_s, startStep },
      { u"completed_steps"_s, stepResults.size() },
      { u"step_results"_s, stepResults },
    };
  }

  bool writeJsonReport( const QString &path, const QJsonObject &report, QString &error )
  {
    if ( path.isEmpty() )
      return true;
    if ( !QDir::isAbsolutePath( path ) || !QFileInfo( path ).dir().exists() )
    {
      error = QObject::tr( "Report path must be absolute and its directory must exist." );
      return false;
    }
    QSaveFile file( path );
    if ( !file.open( QIODevice::WriteOnly ) )
    {
      error = file.errorString();
      return false;
    }
    file.write( QJsonDocument( report ).toJson( QJsonDocument::Indented ) );
    if ( !file.commit() )
    {
      error = file.errorString();
      return false;
    }
    return true;
  }
}

QJsonObject QgsAgentServer::saveWorkflow( const QJsonObject &arguments ) const
{
  const QString name = safeWorkflowName( arguments.value( u"name"_s ).toString() );
  const QJsonArray steps = arguments.value( u"steps"_s ).toArray();
  if ( name.isEmpty() || steps.isEmpty() )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Workflow name and non-empty steps are required." ) } };
  const QString directory = workflowDirectory();
  if ( !QDir().mkpath( directory ) )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Could not create the workflow directory." ) } };
  const QString path = QDir( directory ).filePath( name + u".json"_s );
  if ( !confirmWriteAction( tr( "Save QGIS Agent workflow?" ), tr( "Workflow: %1\nPath: %2" ).arg( name, path ) ) )
    return QJsonObject{ { u"ok"_s, false }, { u"cancelled"_s, true } };
  QSaveFile file( path );
  if ( !file.open( QIODevice::WriteOnly ) )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, file.errorString() } };
  file.write( QJsonDocument( QJsonObject{ { u"name"_s, name }, { u"steps"_s, steps } } ).toJson( QJsonDocument::Indented ) );
  const bool ok = file.commit();
  return QJsonObject{ { u"ok"_s, ok }, { u"name"_s, name }, { u"path"_s, path }, { u"step_count"_s, steps.size() } };
}

QJsonObject QgsAgentServer::listWorkflows() const
{
  QJsonArray workflows;
  const QDir directory( workflowDirectory() );
  for ( const QFileInfo &fileInfo : directory.entryInfoList( QStringList{ u"*.json"_s }, QDir::Files, QDir::Name ) )
  {
    QFile file( fileInfo.absoluteFilePath() );
    if ( !file.open( QIODevice::ReadOnly ) )
      continue;
    const QJsonObject workflow = QJsonDocument::fromJson( file.readAll() ).object();
    workflows.append( QJsonObject{
      { u"name"_s, workflow.value( u"name"_s ).toString( fileInfo.completeBaseName() ) },
      { u"path"_s, fileInfo.absoluteFilePath() },
      { u"step_count"_s, workflow.value( u"steps"_s ).toArray().size() },
    } );
  }
  return QJsonObject{ { u"ok"_s, true }, { u"workflows"_s, workflows } };
}

QJsonObject QgsAgentServer::runWorkflow( const QJsonObject &arguments )
{
  QJsonArray steps = arguments.value( u"steps"_s ).toArray();
  QString workflowName = arguments.value( u"name"_s ).toString();
  if ( steps.isEmpty() && !workflowName.isEmpty() )
  {
    const QString path = QDir( workflowDirectory() ).filePath( safeWorkflowName( workflowName ) + u".json"_s );
    QFile file( path );
    if ( !file.open( QIODevice::ReadOnly ) )
      return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Workflow not found: %1" ).arg( workflowName ) } };
    const QJsonObject workflow = QJsonDocument::fromJson( file.readAll() ).object();
    steps = workflow.value( u"steps"_s ).toArray();
    workflowName = workflow.value( u"name"_s ).toString( workflowName );
  }
  if ( steps.isEmpty() )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "No workflow steps were provided." ) } };
  const int startStep = std::clamp( arguments.value( u"start_step"_s ).toInt( 0 ), 0, static_cast<int>( steps.size() ) - 1 );
  if ( !confirmWriteAction( tr( "Run QGIS Agent workflow?" ), tr( "Workflow: %1\nSteps: %2\nStarting at: %3" ).arg( workflowName.isEmpty() ? tr( "Ad hoc workflow" ) : workflowName ).arg( steps.size() ).arg( startStep ) ) )
    return QJsonObject{ { u"ok"_s, false }, { u"cancelled"_s, true } };

  QHash<QString, QJsonObject> resultsByStep;
  const QJsonObject resumeResults = arguments.value( u"resume_results"_s ).toObject();
  for ( auto it = resumeResults.constBegin(); it != resumeResults.constEnd(); ++it )
    resultsByStep.insert( it.key(), it.value().toObject() );
  QJsonArray stepResults;
  for ( int index = startStep; index < steps.size(); ++index )
  {
    QJsonObject step = steps.at( index ).toObject();
    const QString stepId = step.value( u"id"_s ).toString( u"step_%1"_s.arg( index ) );
    step.insert( u"parameters"_s, resolveWorkflowValue( step.value( u"parameters"_s ), resultsByStep, QHash<QString, QString>() ) );
    const QJsonObject result = runProcessingAlgorithmInternal( step, false );
    resultsByStep.insert( stepId, result );
    stepResults.append( QJsonObject{ { u"id"_s, stepId }, { u"index"_s, index }, { u"result"_s, result } } );
    if ( !result.value( u"ok"_s ).toBool() )
    {
      QJsonObject report = workflowReport( workflowName, startStep, stepResults, false );
      report.insert( u"failed_step"_s, index );
      QString reportError;
      const QString reportPath = arguments.value( u"report_path"_s ).toString();
      if ( !writeJsonReport( reportPath, report, reportError ) )
        report.insert( u"report_error"_s, reportError );
      else if ( !reportPath.isEmpty() )
        report.insert( u"report_path"_s, reportPath );
      return report;
    }
  }

  QJsonObject report = workflowReport( workflowName, startStep, stepResults, true );
  QString error;
  const QString reportPath = arguments.value( u"report_path"_s ).toString();
  if ( !writeJsonReport( reportPath, report, error ) )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, error }, { u"step_results"_s, stepResults } };
  if ( !reportPath.isEmpty() )
    report.insert( u"report_path"_s, reportPath );
  return report;
}

QJsonObject QgsAgentServer::runBatchWorkflow( const QJsonObject &arguments )
{
  const QDir inputDirectory( arguments.value( u"input_directory"_s ).toString() );
  const QString outputDirectory = arguments.value( u"output_directory"_s ).toString();
  const QStringList patterns = [&arguments] {
    QStringList values;
    for ( const QJsonValue &value : arguments.value( u"patterns"_s ).toArray() )
      values.append( value.toString() );
    return values.isEmpty() ? QStringList{ u"*"_s } : values;
  }();
  const QJsonArray steps = arguments.value( u"steps"_s ).toArray();
  if ( !inputDirectory.exists() || steps.isEmpty() )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "An existing input directory and non-empty workflow steps are required." ) } };
  if ( !outputDirectory.isEmpty() && ( !QDir::isAbsolutePath( outputDirectory ) || !QDir( outputDirectory ).exists() ) )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Output directory must be absolute and already exist." ) } };
  const int maxFiles = std::clamp( arguments.value( u"max_files"_s ).toInt( 100 ), 1, 10000 );
  QFileInfoList files = inputDirectory.entryInfoList( patterns, QDir::Files, QDir::Name );
  if ( files.isEmpty() )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "No input files matched the requested patterns." ) } };
  const bool truncated = files.size() > maxFiles;
  if ( truncated )
    files = files.mid( 0, maxFiles );
  if ( !confirmWriteAction( tr( "Run QGIS Agent batch workflow?" ), tr( "Directory: %1\nFiles: %2\nSteps per file: %3" ).arg( inputDirectory.absolutePath() ).arg( files.size() ).arg( steps.size() ) ) )
    return QJsonObject{ { u"ok"_s, false }, { u"cancelled"_s, true } };

  QJsonArray fileResults;
  bool allFilesOk = true;
  for ( const QFileInfo &fileInfo : files )
  {
    const QHash<QString, QString> variables{
      { u"input"_s, fileInfo.absoluteFilePath() },
      { u"input_name"_s, fileInfo.fileName() },
      { u"input_stem"_s, fileInfo.completeBaseName() },
      { u"input_dir"_s, fileInfo.absolutePath() },
      { u"output_dir"_s, outputDirectory },
    };
    QHash<QString, QJsonObject> resultsByStep;
    QJsonArray stepResults;
    bool fileOk = true;
    for ( int index = 0; index < steps.size(); ++index )
    {
      QJsonObject step = steps.at( index ).toObject();
      const QString stepId = step.value( u"id"_s ).toString( u"step_%1"_s.arg( index ) );
      step.insert( u"parameters"_s, resolveWorkflowValue( step.value( u"parameters"_s ), resultsByStep, variables ) );
      const QJsonObject result = runProcessingAlgorithmInternal( step, false );
      resultsByStep.insert( stepId, result );
      stepResults.append( QJsonObject{ { u"id"_s, stepId }, { u"index"_s, index }, { u"result"_s, result } } );
      if ( !result.value( u"ok"_s ).toBool() )
      {
        fileOk = false;
        allFilesOk = false;
        break;
      }
    }
    fileResults.append( QJsonObject{
      { u"input"_s, fileInfo.absoluteFilePath() },
      { u"ok"_s, fileOk },
      { u"step_results"_s, stepResults },
    } );
  }

  QJsonObject report{
    { u"ok"_s, allFilesOk },
    { u"input_directory"_s, inputDirectory.absolutePath() },
    { u"file_count"_s, files.size() },
    { u"file_count_truncated"_s, truncated },
    { u"file_results"_s, fileResults },
  };
  QString error;
  const QString reportPath = arguments.value( u"report_path"_s ).toString();
  if ( !writeJsonReport( reportPath, report, error ) )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, error }, { u"file_results"_s, fileResults } };
  if ( !reportPath.isEmpty() )
    report.insert( u"report_path"_s, reportPath );
  return report;
}
