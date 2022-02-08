/***************************************************************************
    qgspythonutils.cpp - routines for interfacing Python
    ---------------------
    begin                : October 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// python should be first include
// otherwise issues some warnings
#ifdef _MSC_VER
#ifdef _DEBUG
#undef _DEBUG
#endif
#endif
#include <Python.h>

#include "qgis.h"
#include "qgspythonutilsimpl.h"

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmessageoutput.h"
#include "qgssettings.h"

#include <QStringList>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

PyThreadState *_mainState = nullptr;

QgsPythonUtilsImpl::QgsPythonUtilsImpl()
{
}

QgsPythonUtilsImpl::~QgsPythonUtilsImpl()
{
#if SIP_VERSION >= 0x40e06
  exitPython();
#endif
}

bool QgsPythonUtilsImpl::checkSystemImports()
{
  runString( QStringLiteral( "import sys" ) ); // import sys module (for display / exception hooks)
  runString( QStringLiteral( "import os" ) ); // import os module (for user paths)

  // support for PYTHONSTARTUP-like environment variable: PYQGIS_STARTUP
  // (unlike PYTHONHOME and PYTHONPATH, PYTHONSTARTUP is not supported for embedded interpreter by default)
  // this is different than user's 'startup.py' (below), since it is loaded just after Py_Initialize
  // it is very useful for cleaning sys.path, which may have undesirable paths, or for
  // isolating/loading the initial environ without requiring a virt env, e.g. homebrew or MacPorts installs on Mac
  runString( QStringLiteral( "pyqgstart = os.getenv('PYQGIS_STARTUP')\n" ) );
  runString( QStringLiteral( "if pyqgstart is not None and os.path.exists(pyqgstart):\n    with open(pyqgstart) as f:\n        exec(f.read())\n" ) );
  runString( QStringLiteral( "if pyqgstart is not None and os.path.exists(os.path.join('%1', pyqgstart)):\n    with open(os.path.join('%1', pyqgstart)) as f:\n        exec(f.read())\n" ).arg( pythonPath() ) );

#ifdef Q_OS_WIN
  runString( "oldhome=None" );
  runString( "if 'HOME' in os.environ: oldhome=os.environ['HOME']\n" );
  runString( "os.environ['HOME']=os.environ['USERPROFILE']\n" );
#endif

  // construct a list of plugin paths
  // plugin dirs passed in QGIS_PLUGINPATH env. variable have highest priority (usually empty)
  // locally installed plugins have priority over the system plugins
  // use os.path.expanduser to support usernames with special characters (see #2512)
  QStringList pluginpaths;
  const QStringList extraPaths = extraPluginsPaths();
  for ( const QString &path : extraPaths )
  {
    QString p = path;
    if ( !QDir( p ).exists() )
    {
      QgsMessageOutput *msg = QgsMessageOutput::createMessageOutput();
      msg->setTitle( QObject::tr( "Python error" ) );
      msg->setMessage( QObject::tr( "The extra plugin path '%1' does not exist!" ).arg( p ), QgsMessageOutput::MessageText );
      msg->showMessage();
    }
#ifdef Q_OS_WIN
    p.replace( '\\', "\\\\" );
#endif
    // we store here paths in unicode strings
    // the str constant will contain utf8 code (through runString)
    // so we call '...'.decode('utf-8') to make a unicode string
    pluginpaths << '"' + p + '"';
  }
  pluginpaths << homePluginsPath();
  pluginpaths << '"' + pluginsPath() + '"';

  // expect that bindings are installed locally, so add the path to modules
  // also add path to plugins
  QStringList newpaths;
  newpaths << '"' + pythonPath() + '"';
  newpaths << homePythonPath();
  newpaths << pluginpaths;
  runString( "sys.path = [" + newpaths.join( QLatin1Char( ',' ) ) + "] + sys.path" );

  // import SIP
  if ( !runString( QStringLiteral( "from qgis.PyQt import sip" ),
                   QObject::tr( "Couldn't load SIP module." ) + '\n' + QObject::tr( "Python support will be disabled." ) ) )
  {
    return false;
  }

  // import Qt bindings
  if ( !runString( QStringLiteral( "from PyQt5 import QtCore, QtGui" ),
                   QObject::tr( "Couldn't load PyQt." ) + '\n' + QObject::tr( "Python support will be disabled." ) ) )
  {
    return false;
  }

  // import QGIS bindings
  QString error_msg = QObject::tr( "Couldn't load PyQGIS." ) + '\n' + QObject::tr( "Python support will be disabled." );
  if ( !runString( QStringLiteral( "from qgis.core import *" ), error_msg ) || !runString( QStringLiteral( "from qgis.gui import *" ), error_msg ) )
  {
    return false;
  }

  // import QGIS utils
  error_msg = QObject::tr( "Couldn't load QGIS utils." ) + '\n' + QObject::tr( "Python support will be disabled." );
  if ( !runString( QStringLiteral( "import qgis.utils" ), error_msg ) )
  {
    return false;
  }

  // tell the utils script where to look for the plugins
  runString( QStringLiteral( "qgis.utils.plugin_paths = [%1]" ).arg( pluginpaths.join( ',' ) ) );
  runString( QStringLiteral( "qgis.utils.sys_plugin_path = \"%1\"" ).arg( pluginsPath() ) );
  runString( QStringLiteral( "qgis.utils.home_plugin_path = %1" ).arg( homePluginsPath() ) ); // note - homePluginsPath() returns a python expression, not a string literal

#ifdef Q_OS_WIN
  runString( "if oldhome: os.environ['HOME']=oldhome\n" );
#endif

  return true;
}

void QgsPythonUtilsImpl::init()
{
#if defined(PY_MAJOR_VERSION) && defined(PY_MINOR_VERSION) && ((PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 8) || PY_MAJOR_VERSION > 3)
  PyStatus status;
  PyPreConfig preconfig;
  PyPreConfig_InitPythonConfig( &preconfig );

  preconfig.utf8_mode = 1;

  status = Py_PreInitialize( &preconfig );
  if ( PyStatus_Exception( status ) )
  {
    Py_ExitStatusException( status );
  }
#endif

  // initialize python
  Py_Initialize();

  mPythonEnabled = true;

  mMainModule = PyImport_AddModule( "__main__" ); // borrowed reference
  mMainDict = PyModule_GetDict( mMainModule ); // borrowed reference
}

void QgsPythonUtilsImpl::finish()
{
  // release GIL!
  // Later on, we acquire GIL just before doing some Python calls and
  // release GIL again when the work with Python API is done.
  // (i.e. there must be PyGILState_Ensure + PyGILState_Release pair
  // around any calls to Python API, otherwise we may segfault!)
  _mainState = PyEval_SaveThread();
}

bool QgsPythonUtilsImpl::checkQgisUser()
{
  // import QGIS user
  const QString error_msg = QObject::tr( "Couldn't load qgis.user." ) + '\n' + QObject::tr( "Python support will be disabled." );
  if ( !runString( QStringLiteral( "import qgis.user" ), error_msg ) )
  {
    // Should we really bail because of this?!
    return false;
  }
  return true;
}

void QgsPythonUtilsImpl::doCustomImports()
{
  const QStringList startupPaths = QStandardPaths::locateAll( QStandardPaths::AppDataLocation, QStringLiteral( "startup.py" ) );
  if ( startupPaths.isEmpty() )
  {
    return;
  }

  runString( QStringLiteral( "import importlib.util" ) );

  QStringList::const_iterator iter = startupPaths.constBegin();
  for ( ; iter != startupPaths.constEnd(); ++iter )
  {
    runString( QStringLiteral( "spec = importlib.util.spec_from_file_location('startup','%1')" ).arg( *iter ) );
    runString( QStringLiteral( "module = importlib.util.module_from_spec(spec)" ) );
    runString( QStringLiteral( "spec.loader.exec_module(module)" ) );
  }
}

void QgsPythonUtilsImpl::initPython( QgisInterface *interface, const bool installErrorHook, const QString &faultHandlerLogPath )
{
  init();
  if ( !checkSystemImports() )
  {
    exitPython();
    return;
  }

  if ( !faultHandlerLogPath.isEmpty() )
  {
    runString( QStringLiteral( "import faulthandler" ) );
    QString escapedPath = faultHandlerLogPath;
    escapedPath.replace( '\\', QLatin1String( "\\\\" ) );
    escapedPath.replace( '\'', QLatin1String( "\\'" ) );
    runString( QStringLiteral( "fault_handler_file=open('%1', 'wt')" ).arg( escapedPath ) );
    runString( QStringLiteral( "faulthandler.enable(file=fault_handler_file)" ) );
  }

  if ( interface )
  {
    // initialize 'iface' object
    runString( QStringLiteral( "qgis.utils.initInterface(%1)" ).arg( reinterpret_cast< quint64 >( interface ) ) );
  }

  if ( !checkQgisUser() )
  {
    exitPython();
    return;
  }
  doCustomImports();
  if ( installErrorHook )
    QgsPythonUtilsImpl::installErrorHook();
  finish();
}


#ifdef HAVE_SERVER_PYTHON_PLUGINS
void QgsPythonUtilsImpl::initServerPython( QgsServerInterface *interface )
{
  init();
  if ( !checkSystemImports() )
  {
    exitPython();
    return;
  }

  // This is the main difference with initInterface() for desktop plugins
  // import QGIS Server bindings
  const QString error_msg = QObject::tr( "Couldn't load PyQGIS Server." ) + '\n' + QObject::tr( "Python support will be disabled." );
  if ( !runString( QStringLiteral( "from qgis.server import *" ), error_msg ) )
  {
    return;
  }

  // This is the other main difference with initInterface() for desktop plugins
  runString( QStringLiteral( "qgis.utils.initServerInterface(%1)" ).arg( reinterpret_cast< quint64 >( interface ) ) );

  doCustomImports();
  finish();
}

bool QgsPythonUtilsImpl::startServerPlugin( QString packageName )
{
  QString output;
  evalString( QStringLiteral( "qgis.utils.startServerPlugin('%1')" ).arg( packageName ), output );
  return ( output == QLatin1String( "True" ) );
}

#endif // End HAVE_SERVER_PYTHON_PLUGINS

void QgsPythonUtilsImpl::exitPython()
{
  if ( mErrorHookInstalled )
    uninstallErrorHook();
  // causes segfault!
  //Py_Finalize();
  mMainModule = nullptr;
  mMainDict = nullptr;
  mPythonEnabled = false;
}


bool QgsPythonUtilsImpl::isEnabled()
{
  return mPythonEnabled;
}

void QgsPythonUtilsImpl::installErrorHook()
{
  runString( QStringLiteral( "qgis.utils.installErrorHook()" ) );
  mErrorHookInstalled = true;
}

void QgsPythonUtilsImpl::uninstallErrorHook()
{
  runString( QStringLiteral( "qgis.utils.uninstallErrorHook()" ) );
}

QString QgsPythonUtilsImpl::runStringUnsafe( const QString &command, bool single )
{
  // acquire global interpreter lock to ensure we are in a consistent state
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();
  QString ret;

  // TODO: convert special characters from unicode strings u"…" to \uXXXX
  // so that they're not mangled to utf-8
  // (non-unicode strings can be mangled)
  PyObject *obj = PyRun_String( command.toUtf8().constData(), single ? Py_single_input : Py_file_input, mMainDict, mMainDict );
  PyObject *errobj = PyErr_Occurred();
  if ( nullptr != errobj )
  {
    ret = getTraceback();
  }
  Py_XDECREF( obj );

  // we are done calling python API, release global interpreter lock
  PyGILState_Release( gstate );

  return ret;
}

bool QgsPythonUtilsImpl::runString( const QString &command, QString msgOnError, bool single )
{
  bool res = true;
  const QString traceback = runStringUnsafe( command, single );
  if ( traceback.isEmpty() )
    return true;
  else
    res = false;

  if ( msgOnError.isEmpty() )
  {
    // use some default message if custom hasn't been specified
    msgOnError = QObject::tr( "An error occurred during execution of following code:" ) + "\n<tt>" + command + "</tt>";
  }

  // TODO: use python implementation

  QString path, version;
  evalString( QStringLiteral( "str(sys.path)" ), path );
  evalString( QStringLiteral( "sys.version" ), version );

  QString str = "<font color=\"red\">" + msgOnError + "</font><br><pre>\n" + traceback + "\n</pre>"
                + QObject::tr( "Python version:" ) + "<br>" + version + "<br><br>"
                + QObject::tr( "QGIS version:" ) + "<br>" + QStringLiteral( "%1 '%2', %3" ).arg( Qgis::version(), Qgis::releaseName(), Qgis::devVersion() ) + "<br><br>"
                + QObject::tr( "Python path:" ) + "<br>" + path;
  str.replace( '\n', QLatin1String( "<br>" ) ).replace( QLatin1String( "  " ), QLatin1String( "&nbsp; " ) );

  qDebug() << str;
  QgsMessageOutput *msg = QgsMessageOutput::createMessageOutput();
  msg->setTitle( QObject::tr( "Python error" ) );
  msg->setMessage( str, QgsMessageOutput::MessageHtml );
  msg->showMessage();

  return res;
}


QString QgsPythonUtilsImpl::getTraceback()
{
#define TRACEBACK_FETCH_ERROR(what) {errMsg = what; goto done;}

  QString errMsg;
  QString result;

  PyObject *modStringIO = nullptr;
  PyObject *modTB = nullptr;
  PyObject *obStringIO = nullptr;
  PyObject *obResult = nullptr;

  PyObject *type = nullptr, *value = nullptr, *traceback = nullptr;

  PyErr_Fetch( &type, &value, &traceback );
  PyErr_NormalizeException( &type, &value, &traceback );

  const char *iomod = "io";

  modStringIO = PyImport_ImportModule( iomod );
  if ( !modStringIO )
    TRACEBACK_FETCH_ERROR( QStringLiteral( "can't import %1" ).arg( iomod ) );

  obStringIO = PyObject_CallMethod( modStringIO, reinterpret_cast< const char * >( "StringIO" ), nullptr );

  /* Construct a cStringIO object */
  if ( !obStringIO )
    TRACEBACK_FETCH_ERROR( QStringLiteral( "cStringIO.StringIO() failed" ) );

  modTB = PyImport_ImportModule( "traceback" );
  if ( !modTB )
    TRACEBACK_FETCH_ERROR( QStringLiteral( "can't import traceback" ) );

  obResult = PyObject_CallMethod( modTB,  reinterpret_cast< const char * >( "print_exception" ),
                                  reinterpret_cast< const char * >( "OOOOO" ),
                                  type, value ? value : Py_None,
                                  traceback ? traceback : Py_None,
                                  Py_None,
                                  obStringIO );

  if ( !obResult )
    TRACEBACK_FETCH_ERROR( QStringLiteral( "traceback.print_exception() failed" ) );

  Py_DECREF( obResult );

  obResult = PyObject_CallMethod( obStringIO,  reinterpret_cast< const char * >( "getvalue" ), nullptr );
  if ( !obResult )
    TRACEBACK_FETCH_ERROR( QStringLiteral( "getvalue() failed." ) );

  /* And it should be a string all ready to go - duplicate it. */
  if ( !PyUnicode_Check( obResult ) )
    TRACEBACK_FETCH_ERROR( QStringLiteral( "getvalue() did not return a string" ) );

  result = QString::fromUtf8( PyUnicode_AsUTF8( obResult ) );

done:

  // All finished - first see if we encountered an error
  if ( result.isEmpty() && !errMsg.isEmpty() )
  {
    result = errMsg;
  }

  Py_XDECREF( modStringIO );
  Py_XDECREF( modTB );
  Py_XDECREF( obStringIO );
  Py_XDECREF( obResult );
  Py_XDECREF( value );
  Py_XDECREF( traceback );
  Py_XDECREF( type );

  return result;
}

QString QgsPythonUtilsImpl::getTypeAsString( PyObject *obj )
{
  if ( !obj )
    return QString();

  if ( PyType_Check( obj ) )
  {
    QgsDebugMsg( QStringLiteral( "got type" ) );
    return QString( ( ( PyTypeObject * )obj )->tp_name );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "got object" ) );
    return PyObjectToQString( obj );
  }
}

bool QgsPythonUtilsImpl::getError( QString &errorClassName, QString &errorText )
{
  // acquire global interpreter lock to ensure we are in a consistent state
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();

  if ( !PyErr_Occurred() )
  {
    PyGILState_Release( gstate );
    return false;
  }

  PyObject *err_type = nullptr;
  PyObject *err_value = nullptr;
  PyObject *err_tb = nullptr;

  // get the exception information and clear error
  PyErr_Fetch( &err_type, &err_value, &err_tb );

  // get exception's class name
  errorClassName = getTypeAsString( err_type );

  // get exception's text
  if ( nullptr != err_value && err_value != Py_None )
  {
    errorText = PyObjectToQString( err_value );
  }
  else
    errorText.clear();

  // cleanup
  Py_XDECREF( err_type );
  Py_XDECREF( err_value );
  Py_XDECREF( err_tb );

  // we are done calling python API, release global interpreter lock
  PyGILState_Release( gstate );

  return true;
}


QString QgsPythonUtilsImpl::PyObjectToQString( PyObject *obj )
{
  QString result;

  // is it None?
  if ( obj == Py_None )
  {
    return QString();
  }

  // check whether the object is already a unicode string
  if ( PyUnicode_Check( obj ) )
  {
    result = QString::fromUtf8( PyUnicode_AsUTF8( obj ) );
    return result;
  }

  // if conversion to Unicode failed, try to convert it to classic string, i.e. str(obj)
  PyObject *obj_str = PyObject_Str( obj ); // new reference
  if ( obj_str )
  {
    result = QString::fromUtf8( PyUnicode_AsUTF8( obj_str ) );
    Py_XDECREF( obj_str );
    return result;
  }

  // some problem with conversion to Unicode string
  QgsDebugMsg( QStringLiteral( "unable to convert PyObject to a QString!" ) );
  return QStringLiteral( "(qgis error)" );
}


bool QgsPythonUtilsImpl::evalString( const QString &command, QString &result )
{
  // acquire global interpreter lock to ensure we are in a consistent state
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();

  PyObject *res = PyRun_String( command.toUtf8().constData(), Py_eval_input, mMainDict, mMainDict );
  const bool success = nullptr != res;

  // TODO: error handling

  if ( success )
    result = PyObjectToQString( res );

  Py_XDECREF( res );

  // we are done calling python API, release global interpreter lock
  PyGILState_Release( gstate );

  return success;
}

QString QgsPythonUtilsImpl::pythonPath() const
{
  if ( QgsApplication::isRunningFromBuildDir() )
    return QgsApplication::buildOutputPath() + QStringLiteral( "/python" );
  else
    return QgsApplication::pkgDataPath() + QStringLiteral( "/python" );
}

QString QgsPythonUtilsImpl::pluginsPath() const
{
  return pythonPath() + QStringLiteral( "/plugins" );
}

QString QgsPythonUtilsImpl::homePythonPath() const
{
  QString settingsDir = QgsApplication::qgisSettingsDirPath();
  if ( QDir::cleanPath( settingsDir ) == QDir::homePath() + QStringLiteral( "/.qgis3" ) )
  {
    return QStringLiteral( "\"%1/.qgis3/python\"" ).arg( QDir::homePath() );
  }
  else
  {
    return QStringLiteral( "\"" ) + settingsDir.replace( '\\', QLatin1String( "\\\\" ) ) + QStringLiteral( "python\"" );
  }
}

QString QgsPythonUtilsImpl::homePluginsPath() const
{
  return homePythonPath() + QStringLiteral( " + \"/plugins\"" );
}

QStringList QgsPythonUtilsImpl::extraPluginsPaths() const
{
  const char *cpaths = getenv( "QGIS_PLUGINPATH" );
  if ( !cpaths )
    return QStringList();

  const QString paths = QString::fromLocal8Bit( cpaths );
#ifndef Q_OS_WIN
  if ( paths.contains( ':' ) )
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    return paths.split( ':', QString::SkipEmptyParts );
#else
    return paths.split( ':', Qt::SkipEmptyParts );
#endif
#endif
  if ( paths.contains( ';' ) )
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    return paths.split( ';', QString::SkipEmptyParts );
#else
    return paths.split( ';', Qt::SkipEmptyParts );
#endif
  else
    return QStringList( paths );
}


QStringList QgsPythonUtilsImpl::pluginList()
{
  runString( QStringLiteral( "qgis.utils.updateAvailablePlugins(sort_by_dependencies=True)" ) );

  QString output;
  evalString( QStringLiteral( "'\\n'.join(qgis.utils.available_plugins)" ), output );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  return output.split( QChar( '\n' ), QString::SkipEmptyParts );
#else
  return output.split( QChar( '\n' ), Qt::SkipEmptyParts );
#endif
}

QString QgsPythonUtilsImpl::getPluginMetadata( const QString &pluginName, const QString &function )
{
  QString res;
  const QString str = QStringLiteral( "qgis.utils.pluginMetadata('%1', '%2')" ).arg( pluginName, function );
  evalString( str, res );
  //QgsDebugMsg("metadata "+pluginName+" - '"+function+"' = "+res);
  return res;
}

bool QgsPythonUtilsImpl::pluginHasProcessingProvider( const QString &pluginName )
{
  return getPluginMetadata( pluginName, QStringLiteral( "hasProcessingProvider" ) ).compare( QLatin1String( "yes" ), Qt::CaseInsensitive ) == 0 || getPluginMetadata( pluginName, QStringLiteral( "hasProcessingProvider" ) ).compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0;
}

bool QgsPythonUtilsImpl::loadPlugin( const QString &packageName )
{
  QString output;
  evalString( QStringLiteral( "qgis.utils.loadPlugin('%1')" ).arg( packageName ), output );
  return ( output == QLatin1String( "True" ) );
}

bool QgsPythonUtilsImpl::startPlugin( const QString &packageName )
{
  QString output;
  evalString( QStringLiteral( "qgis.utils.startPlugin('%1')" ).arg( packageName ), output );
  return ( output == QLatin1String( "True" ) );
}

bool QgsPythonUtilsImpl::startProcessingPlugin( const QString &packageName )
{
  QString output;
  evalString( QStringLiteral( "qgis.utils.startProcessingPlugin('%1')" ).arg( packageName ), output );
  return ( output == QLatin1String( "True" ) );
}

bool QgsPythonUtilsImpl::canUninstallPlugin( const QString &packageName )
{
  QString output;
  evalString( QStringLiteral( "qgis.utils.canUninstallPlugin('%1')" ).arg( packageName ), output );
  return ( output == QLatin1String( "True" ) );
}

bool QgsPythonUtilsImpl::unloadPlugin( const QString &packageName )
{
  QString output;
  evalString( QStringLiteral( "qgis.utils.unloadPlugin('%1')" ).arg( packageName ), output );
  return ( output == QLatin1String( "True" ) );
}

bool QgsPythonUtilsImpl::isPluginEnabled( const QString &packageName ) const
{
  return QgsSettings().value( QStringLiteral( "/PythonPlugins/" ) + packageName, QVariant( false ) ).toBool();
}

bool QgsPythonUtilsImpl::isPluginLoaded( const QString &packageName )
{
  QString output;
  evalString( QStringLiteral( "qgis.utils.isPluginLoaded('%1')" ).arg( packageName ), output );
  return ( output == QLatin1String( "True" ) );
}

QStringList QgsPythonUtilsImpl::listActivePlugins()
{
  QString output;
  evalString( QStringLiteral( "'\\n'.join(qgis.utils.active_plugins)" ), output );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  return output.split( QChar( '\n' ), QString::SkipEmptyParts );
#else
  return output.split( QChar( '\n' ), Qt::SkipEmptyParts );
#endif
}
