/***************************************************************************
    qgsairunpythontool.cpp
    ---------------------
    begin                : April 2026
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

#include "qgsairunpythontool.h"

#include "qgsaiauditlog.h"
#include "qgsaipythonapprovaldialog.h"
#include "qgsaitoolschemautil.h"
#include "qgsaiworkspacetrust.h"
#include "qgsmessagelog.h"
#include "qgspythonrunner.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QTemporaryFile>
#include <QUuid>

using namespace Qt::StringLiterals;

namespace
{
  // Python source literal that performs sandboxed-ish execution: redirects
  // stdout/stderr, runs the user's code via exec(), serialises everything as
  // JSON to a temp file. The two %1/%2 placeholders are filled with Python
  // string literals (we use QString::arg with a repr-style path, see escapePath).
  // Keep this compatible with Python 3.7+.
  constexpr const char *PY_WRAPPER_TEMPLATE = R"(
import sys, traceback, json, io

__qgsai_code_path = %1
__qgsai_out_path = %2

with open(__qgsai_code_path, "r", encoding="utf-8") as __qgsai_f:
    __qgsai_code = __qgsai_f.read()

__qgsai_stdout = io.StringIO()
__qgsai_stderr = io.StringIO()
__qgsai_old_stdout = sys.stdout
__qgsai_old_stderr = sys.stderr
sys.stdout = __qgsai_stdout
sys.stderr = __qgsai_stderr
__qgsai_error = ""
try:
    exec(compile(__qgsai_code, "<ai_run_python>", "exec"), globals())
except SystemExit:
    pass
except BaseException:
    __qgsai_error = traceback.format_exc()
sys.stdout = __qgsai_old_stdout
sys.stderr = __qgsai_old_stderr

with open(__qgsai_out_path, "w", encoding="utf-8") as __qgsai_f:
    json.dump({
        "stdout": __qgsai_stdout.getvalue(),
        "stderr": __qgsai_stderr.getvalue(),
        "error": __qgsai_error,
    }, __qgsai_f)
)";

  /**
   * Encodes \a path as a Python source-level string literal: opens with single
   * quotes, escapes embedded backslashes and single-quotes. Robust on Windows
   * where the path contains backslashes that would otherwise be interpreted
   * as escape sequences inside the embedded Python.
   */
  QString escapeRunPythonPath( const QString &path )
  {
    QString escaped = path;
    escaped.replace( '\\', "\\\\"_L1 );
    escaped.replace( '\'', "\\'"_L1 );
    return u"'%1'"_s.arg( escaped );
  }

  QString truncateRunPythonOutput( const QString &text, int maxBytes )
  {
    const QByteArray utf8 = text.toUtf8();
    if ( utf8.size() <= maxBytes )
      return text;
    return QString::fromUtf8( utf8.left( maxBytes ) ) + u"\n…[truncated]"_s;
  }
} //namespace

QgsAiRunPythonTool::QgsAiRunPythonTool( QWidget *dialogParent )
  : mDialogParent( dialogParent )
{}

bool QgsAiRunPythonTool::isAvailable() const
{
  return QgsPythonRunner::isValid() && QgsAiWorkspaceTrust::isCurrentWorkspaceTrusted();
}

QString QgsAiRunPythonTool::availabilityReason() const
{
  if ( !QgsPythonRunner::isValid() )
    return u"Python runner is not available in this QGIS instance. Start QGIS with Python enabled (do not use --nopython), build with WITH_BINDINGS, and verify that the qgispython support library loads."_s;
  return u"run_python is disabled because this workspace is not trusted. Trust the workspace from the AI provider settings to enable Python execution."_s;
}

QString QgsAiRunPythonTool::description() const
{
  return QStringLiteral(
    "Executes a snippet of PyQGIS code in the running QGIS session. "
    "Captures stdout/stderr and any Python traceback. The user must approve the "
    "code via a modal dialog before it runs; refusal returns 'user_rejected'. "
    "Use this tool ONLY when the action genuinely requires Python (e.g. driving "
    "the QGIS API to add a runtime layer). Prefer propose_edit/propose_create_file "
    "for static file changes."
  );
}

QJsonObject QgsAiRunPythonTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"code"_s, prop( u"string"_s, u"The PyQGIS code to execute. Maximum 8000 characters."_s ) );
  properties.insert( u"description"_s, prop( u"string"_s, u"Short human-readable explanation of what the code does. Shown in the approval dialog."_s ) );
  return schemaObject( properties, QJsonArray { u"code"_s } );
}

QgsAiToolResult QgsAiRunPythonTool::execute( const QJsonObject &args )
{
  const QString code = args.value( u"code"_s ).toString();
  if ( code.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'code' is required and must be non-empty."_s );
  if ( code.size() > MAX_CODE_CHARS )
    return QgsAiToolResult::error( u"Refusing to run code exceeding %1 characters (got %2). Split the work into smaller calls."_s.arg( MAX_CODE_CHARS ).arg( code.size() ) );

  if ( !QgsPythonRunner::isValid() )
    return QgsAiToolResult::error( availabilityReason() );

  const QString description = args.value( u"description"_s ).toString();

  // Modal approval: user must explicitly click Run.
  QgsAiPythonApprovalDialog dialog( description, code, mDialogParent );
  const int dialogResult = dialog.exec();
  if ( dialogResult != QDialog::Accepted )
  {
    QgsMessageLog::logMessage( u"run_python rejected by user (codeChars=%1)"_s.arg( code.size() ), u"AI/Python"_s, Qgis::MessageLevel::Info, false );
    QJsonObject output;
    output.insert( u"status"_s, u"user_rejected"_s );
    return QgsAiToolResult::ok( output );
  }

  // Persist user code and capture-output paths to disk so the wrapper can read
  // them back without us worrying about Python-source quoting.
  const QString uniqueId = QUuid::createUuid().toString( QUuid::WithoutBraces );
  const QString tmpDir = QDir::tempPath();
  const QString codePath = QDir( tmpDir ).filePath( u"qgsai_pycode_%1.py"_s.arg( uniqueId ) );
  const QString outPath = QDir( tmpDir ).filePath( u"qgsai_pyout_%1.json"_s.arg( uniqueId ) );

  {
    QFile codeFile( codePath );
    if ( !codeFile.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
      return QgsAiToolResult::error( u"Cannot write temp code file: %1"_s.arg( codePath ) );
    codeFile.write( code.toUtf8() );
  }

  QgsMessageLog::logMessage( u"run_python: executing approved code (codeChars=%1)"_s.arg( code.size() ), u"AI/Python"_s, Qgis::MessageLevel::Info, false );
  QgsAiAuditLog::append( u"run_python"_s, code );

  // Build the wrapper with safely-quoted paths.
  const QString wrapper = QString::fromUtf8( PY_WRAPPER_TEMPLATE ).arg( escapeRunPythonPath( codePath ), escapeRunPythonPath( outPath ) );

  const bool ranOk = QgsPythonRunner::run( wrapper );

  QString stdoutText;
  QString stderrText;
  QString tracebackText;
  if ( QFile::exists( outPath ) )
  {
    QFile outFile( outPath );
    if ( outFile.open( QIODevice::ReadOnly ) )
    {
      const QByteArray body = outFile.readAll();
      const QJsonDocument doc = QJsonDocument::fromJson( body );
      if ( doc.isObject() )
      {
        const QJsonObject obj = doc.object();
        stdoutText = obj.value( u"stdout"_s ).toString();
        stderrText = obj.value( u"stderr"_s ).toString();
        tracebackText = obj.value( u"error"_s ).toString();
      }
    }
  }

  // Best-effort cleanup of the temp files.
  QFile::remove( codePath );
  QFile::remove( outPath );

  if ( !ranOk )
  {
    QgsMessageLog::logMessage( u"run_python: QgsPythonRunner::run() returned false (wrapper failed)."_s, u"AI/Python"_s, Qgis::MessageLevel::Warning, false );
    return QgsAiToolResult::error( u"Python wrapper failed to execute. %1"_s.arg( tracebackText.isEmpty() ? QString() : tracebackText ) );
  }

  const bool hadException = !tracebackText.isEmpty();
  QgsMessageLog::
    logMessage( u"run_python: completed (stdoutBytes=%1, stderrBytes=%2, exception=%3)"_s.arg( stdoutText.size() ).arg( stderrText.size() ).arg( hadException ), u"AI/Python"_s, hadException ? Qgis::MessageLevel::Warning : Qgis::MessageLevel::Info, false );

  QJsonObject output;
  output.insert( u"status"_s, hadException ? u"error"_s : u"ok"_s );
  output.insert( u"stdout"_s, truncateRunPythonOutput( stdoutText, MAX_CAPTURE_BYTES ) );
  output.insert( u"stderr"_s, truncateRunPythonOutput( stderrText, MAX_CAPTURE_BYTES ) );
  if ( hadException )
    output.insert( u"traceback"_s, truncateRunPythonOutput( tracebackText, MAX_CAPTURE_BYTES ) );
  return QgsAiToolResult::ok( output );
}
