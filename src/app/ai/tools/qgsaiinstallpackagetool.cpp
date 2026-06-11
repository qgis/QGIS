/***************************************************************************
    qgsaiinstallpackagetool.cpp
    ---------------------
    begin                : May 2026
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

#include "qgsaiinstallpackagetool.h"

#include "qgsaiauditlog.h"
#include "qgsaipipinstallapprovaldialog.h"
#include "qgsaitoolschemautil.h"
#include "qgsaiworkspacetrust.h"
#include "qgsmessagelog.h"
#include "qgspythonrunner.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QUuid>

using namespace Qt::StringLiterals;

namespace
{
  // Python wrapper that shells out to `python -m pip install --user` and serialises
  // stdout/stderr/return code to a temp JSON file. Two %1/%2/%3 placeholders are filled
  // with: the output JSON path (escaped), the pip-args JSON file path (escaped), and the
  // timeout in seconds.
  // We write the package list to a JSON file (rather than embedding it in the wrapper)
  // so we don't have to worry about escaping arbitrary user-supplied strings into Python
  // source — schema validation already restricts them, but defence in depth is cheap.
  constexpr const char *PIP_WRAPPER_TEMPLATE = R"(
import sys, subprocess, json, traceback

__qgsai_out_path = %1
__qgsai_args_path = %2
__qgsai_timeout = %3

__qgsai_error = ""
__qgsai_stdout = ""
__qgsai_stderr = ""
__qgsai_returncode = -1
try:
    with open(__qgsai_args_path, "r", encoding="utf-8") as __qgsai_f:
        __qgsai_packages = json.load(__qgsai_f)
    __qgsai_cmd = [sys.executable, "-m", "pip", "install", "--user", "--disable-pip-version-check", *__qgsai_packages]
    __qgsai_proc = subprocess.run(__qgsai_cmd, capture_output=True, text=True, timeout=__qgsai_timeout)
    __qgsai_stdout = __qgsai_proc.stdout
    __qgsai_stderr = __qgsai_proc.stderr
    __qgsai_returncode = __qgsai_proc.returncode
except subprocess.TimeoutExpired as __qgsai_e:
    __qgsai_error = "pip install timed out after %s seconds" % __qgsai_timeout
    __qgsai_stdout = (__qgsai_e.stdout or "") if hasattr(__qgsai_e, "stdout") else ""
    __qgsai_stderr = (__qgsai_e.stderr or "") if hasattr(__qgsai_e, "stderr") else ""
except BaseException:
    __qgsai_error = traceback.format_exc()

with open(__qgsai_out_path, "w", encoding="utf-8") as __qgsai_f:
    json.dump({
        "stdout": __qgsai_stdout,
        "stderr": __qgsai_stderr,
        "returncode": __qgsai_returncode,
        "error": __qgsai_error,
    }, __qgsai_f)
)";

  /**
   * Encodes \a path as a Python source-level string literal: opens with single
   * quotes, escapes embedded backslashes and single-quotes. Same idea as the
   * helper in qgsairunpythontool.cpp.
   */
  QString escapePipPath( const QString &path )
  {
    QString escaped = path;
    escaped.replace( '\\', "\\\\"_L1 );
    escaped.replace( '\'', "\\'"_L1 );
    return u"'%1'"_s.arg( escaped );
  }

  QString truncatePipOutput( const QString &text, int maxBytes )
  {
    const QByteArray utf8 = text.toUtf8();
    if ( utf8.size() <= maxBytes )
      return text;
    return QString::fromUtf8( utf8.left( maxBytes ) ) + u"\n…[truncated]"_s;
  }

  /**
   * Strict pip-spec validator. Allowed: `name`, `name==1.2`, `name>=1.2`, `name~=1.2.3`,
   * with name limited to letters/digits/underscore/dot/hyphen and version to the same plus
   * the comparison operators. This deliberately blocks: URLs, `git+...`, `-r file`,
   * extras-with-environment-markers, paths, anything containing whitespace or shell
   * metacharacters.
   */
  bool isAllowedSpec( const QString &spec )
  {
    static const QRegularExpression re( uR"(^[A-Za-z0-9_.\-]+([<>=!~]+[A-Za-z0-9_.\-]+)?$)"_s );
    return re.match( spec ).hasMatch();
  }
} //namespace

QgsAiInstallPythonPackageTool::QgsAiInstallPythonPackageTool( QWidget *dialogParent )
  : mDialogParent( dialogParent )
{}

bool QgsAiInstallPythonPackageTool::isAvailable() const
{
  return QgsPythonRunner::isValid() && QgsAiWorkspaceTrust::isCurrentWorkspaceTrusted();
}

QString QgsAiInstallPythonPackageTool::availabilityReason() const
{
  if ( !QgsPythonRunner::isValid() )
    return u"Python package installation is not available because the QGIS Python runner is unavailable. Start QGIS with Python enabled (do not use --nopython), build with WITH_BINDINGS, and verify that the qgispython support library loads."_s;
  return u"install_python_package is disabled because this workspace is not trusted. Trust the workspace from the AI provider settings to enable package installation."_s;
}

QString QgsAiInstallPythonPackageTool::description() const
{
  return QStringLiteral(
    "Installs one or more Python packages into the user-scope site-packages of the QGIS "
    "Python interpreter via 'pip install --user'. The user must approve the exact list of "
    "packages via a modal dialog before anything is installed; refusal returns 'user_rejected'. "
    "Use this when the user's task needs a library not bundled with QGIS (e.g. geopy, osmnx, "
    "requests, shapely). After installation, call run_python to actually use the library. "
    "Each spec must be a plain pip requirement of the form name[<op>version]. URLs, git refs, "
    "and '-r requirements.txt' are not accepted. Maximum 10 packages per call."
  );
}

QJsonObject QgsAiInstallPythonPackageTool::schema() const
{
  QJsonObject packagesProp;
  packagesProp.insert( u"type"_s, u"array"_s );
  packagesProp.insert( u"description"_s, u"Pip specs to install. Each item must match ^[A-Za-z0-9_.\\-]+([<>=!~]+[A-Za-z0-9_.\\-]+)?$ — for example 'geopy', 'osmnx==1.9.3', 'requests>=2.31'."_s );
  QJsonObject itemSchema;
  itemSchema.insert( u"type"_s, u"string"_s );
  packagesProp.insert( u"items"_s, itemSchema );
  packagesProp.insert( u"minItems"_s, 1 );
  packagesProp.insert( u"maxItems"_s, MAX_PACKAGES );

  QJsonObject properties;
  properties.insert( u"packages"_s, packagesProp );
  properties.insert( u"reason"_s, prop( u"string"_s, u"Short human-readable explanation of why these packages are needed. Shown in the approval dialog."_s ) );
  return schemaObject( properties, QJsonArray { u"packages"_s } );
}

QgsAiToolResult QgsAiInstallPythonPackageTool::execute( const QJsonObject &args )
{
  const QJsonValue packagesValue = args.value( u"packages"_s );
  if ( !packagesValue.isArray() )
    return QgsAiToolResult::error( u"Argument 'packages' is required and must be an array of strings."_s );

  const QJsonArray packagesArray = packagesValue.toArray();
  if ( packagesArray.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'packages' must contain at least one entry."_s );
  if ( packagesArray.size() > MAX_PACKAGES )
    return QgsAiToolResult::error( u"Refusing to install more than %1 packages in a single call (got %2)."_s.arg( MAX_PACKAGES ).arg( packagesArray.size() ) );

  QStringList packages;
  packages.reserve( packagesArray.size() );
  for ( const QJsonValue &v : packagesArray )
  {
    if ( !v.isString() )
      return QgsAiToolResult::error( u"Every entry in 'packages' must be a string."_s );
    const QString spec = v.toString().trimmed();
    if ( spec.isEmpty() )
      return QgsAiToolResult::error( u"Empty pip spec is not allowed."_s );
    if ( spec.size() > MAX_SPEC_CHARS )
      return QgsAiToolResult::error( u"Pip spec exceeds %1 characters: '%2'."_s.arg( MAX_SPEC_CHARS ).arg( spec.left( 60 ) ) );
    if ( !isAllowedSpec( spec ) )
      return QgsAiToolResult::error( u"Pip spec '%1' is not in the allowed form name[<op>version]. URLs, git refs and '-r' are blocked."_s.arg( spec ) );
    packages << spec;
  }

  if ( !QgsPythonRunner::isValid() )
    return QgsAiToolResult::error( availabilityReason() );

  const QString reason = args.value( u"reason"_s ).toString();

  // Modal approval: user must explicitly click Install.
  QgsAiPipInstallApprovalDialog dialog( packages, reason, mDialogParent );
  const int dialogResult = dialog.exec();
  if ( dialogResult != QDialog::Accepted )
  {
    QgsMessageLog::logMessage( u"install_python_package rejected by user (packages=%1)"_s.arg( packages.size() ), u"AI/Pip"_s, Qgis::MessageLevel::Info, false );
    QJsonObject output;
    output.insert( u"status"_s, u"user_rejected"_s );
    return QgsAiToolResult::ok( output );
  }

  // Persist the package list to a JSON file so the wrapper reads it without needing
  // any string interpolation into Python source.
  const QString uniqueId = QUuid::createUuid().toString( QUuid::WithoutBraces );
  const QString tmpDir = QDir::tempPath();
  const QString argsPath = QDir( tmpDir ).filePath( u"qgsai_pipargs_%1.json"_s.arg( uniqueId ) );
  const QString outPath = QDir( tmpDir ).filePath( u"qgsai_pipout_%1.json"_s.arg( uniqueId ) );

  {
    QJsonArray jsonPackages;
    for ( const QString &p : packages )
      jsonPackages.append( p );
    QFile argsFile( argsPath );
    if ( !argsFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
      return QgsAiToolResult::error( u"Cannot write temp pip-args file: %1"_s.arg( argsPath ) );
    argsFile.write( QJsonDocument( jsonPackages ).toJson( QJsonDocument::Compact ) );
  }

  QgsMessageLog::logMessage( u"install_python_package: executing approved install (packages=%1)"_s.arg( packages.size() ), u"AI/Pip"_s, Qgis::MessageLevel::Info, false );
  QgsAiAuditLog::append( u"install_python_package"_s, packages.join( ' '_L1 ) );

  constexpr int TIMEOUT_SECONDS = 300;
  const QString wrapper = QString::fromUtf8( PIP_WRAPPER_TEMPLATE ).arg( escapePipPath( outPath ), escapePipPath( argsPath ), QString::number( TIMEOUT_SECONDS ) );

  const bool ranOk = QgsPythonRunner::run( wrapper );

  QString stdoutText;
  QString stderrText;
  QString innerError;
  int returncode = -1;
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
        innerError = obj.value( u"error"_s ).toString();
        returncode = obj.value( u"returncode"_s ).toInt( -1 );
      }
    }
  }

  // Best-effort cleanup of the temp files.
  QFile::remove( argsPath );
  QFile::remove( outPath );

  if ( !ranOk )
  {
    QgsMessageLog::logMessage( u"install_python_package: QgsPythonRunner::run() returned false."_s, u"AI/Pip"_s, Qgis::MessageLevel::Warning, false );
    return QgsAiToolResult::error( u"pip wrapper failed to execute. %1"_s.arg( innerError ) );
  }

  const bool success = innerError.isEmpty() && returncode == 0;
  QgsMessageLog::
    logMessage( u"install_python_package: completed (returncode=%1, error=%2)"_s.arg( returncode ).arg( innerError.isEmpty() ? u"none"_s : u"yes"_s ), u"AI/Pip"_s, success ? Qgis::MessageLevel::Info : Qgis::MessageLevel::Warning, false );

  QJsonArray installed;
  if ( success )
  {
    for ( const QString &p : packages )
      installed.append( p );
  }

  QJsonObject output;
  output.insert( u"status"_s, success ? u"ok"_s : u"error"_s );
  output.insert( u"exit_code"_s, returncode );
  output.insert( u"stdout"_s, truncatePipOutput( stdoutText, MAX_CAPTURE_BYTES ) );
  output.insert( u"stderr"_s, truncatePipOutput( stderrText, MAX_CAPTURE_BYTES ) );
  output.insert( u"installed"_s, installed );
  if ( !innerError.isEmpty() )
    output.insert( u"error"_s, truncatePipOutput( innerError, MAX_CAPTURE_BYTES ) );
  return QgsAiToolResult::ok( output );
}
