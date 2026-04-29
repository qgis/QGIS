#include "qgsairunpythontool.h"

#include "qgsaipythonapprovaldialog.h"
#include "qgsmessagelog.h"
#include "qgspythonrunner.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryFile>
#include <QUuid>

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
  QString escapePath( const QString &path )
  {
    QString escaped = path;
    escaped.replace( '\\', QStringLiteral( "\\\\" ) );
    escaped.replace( '\'', QStringLiteral( "\\'" ) );
    return QStringLiteral( "'%1'" ).arg( escaped );
  }

  QJsonObject schemaObject( const QJsonObject &properties, const QJsonArray &required = QJsonArray() )
  {
    QJsonObject schema;
    schema.insert( QStringLiteral( "type" ), QStringLiteral( "object" ) );
    schema.insert( QStringLiteral( "properties" ), properties );
    if ( !required.isEmpty() )
      schema.insert( QStringLiteral( "required" ), required );
    return schema;
  }

  QJsonObject prop( const QString &type, const QString &description )
  {
    QJsonObject p;
    p.insert( QStringLiteral( "type" ), type );
    p.insert( QStringLiteral( "description" ), description );
    return p;
  }

  QString truncate( const QString &text, int maxBytes )
  {
    const QByteArray utf8 = text.toUtf8();
    if ( utf8.size() <= maxBytes )
      return text;
    return QString::fromUtf8( utf8.left( maxBytes ) ) + QStringLiteral( "\n…[truncated]" );
  }
}

QgsAiRunPythonTool::QgsAiRunPythonTool( QWidget *dialogParent )
  : mDialogParent( dialogParent )
{}

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
  properties.insert( QStringLiteral( "code" ), prop( QStringLiteral( "string" ), QStringLiteral( "The PyQGIS code to execute. Maximum 8000 characters." ) ) );
  properties.insert( QStringLiteral( "description" ), prop( QStringLiteral( "string" ), QStringLiteral( "Short human-readable explanation of what the code does. Shown in the approval dialog." ) ) );
  return schemaObject( properties, QJsonArray { QStringLiteral( "code" ) } );
}

QgsAiToolResult QgsAiRunPythonTool::execute( const QJsonObject &args )
{
  const QString code = args.value( QStringLiteral( "code" ) ).toString();
  if ( code.isEmpty() )
    return QgsAiToolResult::error( QStringLiteral( "Argument 'code' is required and must be non-empty." ) );
  if ( code.size() > MAX_CODE_CHARS )
    return QgsAiToolResult::error( QStringLiteral( "Refusing to run code exceeding %1 characters (got %2). Split the work into smaller calls." ).arg( MAX_CODE_CHARS ).arg( code.size() ) );

  if ( !QgsPythonRunner::isValid() )
    return QgsAiToolResult::error( QStringLiteral( "Python runner is not available in this QGIS instance." ) );

  const QString description = args.value( QStringLiteral( "description" ) ).toString();

  // Modal approval: user must explicitly click Run.
  QgsAiPythonApprovalDialog dialog( description, code, mDialogParent );
  const int dialogResult = dialog.exec();
  if ( dialogResult != QDialog::Accepted )
  {
    QgsMessageLog::logMessage(
      QStringLiteral( "run_python rejected by user (codeChars=%1)" ).arg( code.size() ),
      QStringLiteral( "AI/Python" ), Qgis::MessageLevel::Info, false );
    QJsonObject output;
    output.insert( QStringLiteral( "status" ), QStringLiteral( "user_rejected" ) );
    return QgsAiToolResult::ok( output );
  }

  // Persist user code and capture-output paths to disk so the wrapper can read
  // them back without us worrying about Python-source quoting.
  const QString uniqueId = QUuid::createUuid().toString( QUuid::WithoutBraces );
  const QString tmpDir = QDir::tempPath();
  const QString codePath = QDir( tmpDir ).filePath( QStringLiteral( "qgsai_pycode_%1.py" ).arg( uniqueId ) );
  const QString outPath = QDir( tmpDir ).filePath( QStringLiteral( "qgsai_pyout_%1.json" ).arg( uniqueId ) );

  {
    QFile codeFile( codePath );
    if ( !codeFile.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
      return QgsAiToolResult::error( QStringLiteral( "Cannot write temp code file: %1" ).arg( codePath ) );
    codeFile.write( code.toUtf8() );
  }

  QgsMessageLog::logMessage(
    QStringLiteral( "run_python: executing approved code (codeChars=%1, codePath=%2)" ).arg( code.size() ).arg( codePath ),
    QStringLiteral( "AI/Python" ), Qgis::MessageLevel::Info, false );

  // Build the wrapper with safely-quoted paths.
  const QString wrapper = QString::fromUtf8( PY_WRAPPER_TEMPLATE )
                            .arg( escapePath( codePath ), escapePath( outPath ) );

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
        stdoutText = obj.value( QStringLiteral( "stdout" ) ).toString();
        stderrText = obj.value( QStringLiteral( "stderr" ) ).toString();
        tracebackText = obj.value( QStringLiteral( "error" ) ).toString();
      }
    }
  }

  // Best-effort cleanup of the temp files.
  QFile::remove( codePath );
  QFile::remove( outPath );

  if ( !ranOk )
  {
    QgsMessageLog::logMessage(
      QStringLiteral( "run_python: QgsPythonRunner::run() returned false (wrapper failed). traceback='%1'" ).arg( tracebackText.left( 500 ) ),
      QStringLiteral( "AI/Python" ), Qgis::MessageLevel::Warning, false );
    return QgsAiToolResult::error(
      QStringLiteral( "Python wrapper failed to execute. %1" ).arg( tracebackText.isEmpty() ? QString() : tracebackText ) );
  }

  const bool hadException = !tracebackText.isEmpty();
  QgsMessageLog::logMessage(
    QStringLiteral( "run_python: completed (stdoutBytes=%1, stderrBytes=%2, exception=%3)" )
      .arg( stdoutText.size() ).arg( stderrText.size() ).arg( hadException ),
    QStringLiteral( "AI/Python" ), hadException ? Qgis::MessageLevel::Warning : Qgis::MessageLevel::Info, false );

  QJsonObject output;
  output.insert( QStringLiteral( "status" ), hadException ? QStringLiteral( "error" ) : QStringLiteral( "ok" ) );
  output.insert( QStringLiteral( "stdout" ), truncate( stdoutText, MAX_CAPTURE_BYTES ) );
  output.insert( QStringLiteral( "stderr" ), truncate( stderrText, MAX_CAPTURE_BYTES ) );
  if ( hadException )
    output.insert( QStringLiteral( "traceback" ), truncate( tracebackText, MAX_CAPTURE_BYTES ) );
  return QgsAiToolResult::ok( output );
}
