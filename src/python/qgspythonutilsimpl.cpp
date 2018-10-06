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

#include <QMessageBox>
#include <QStringList>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

PyThreadState *_mainState = nullptr;

QgsPythonUtilsImpl::QgsPythonUtilsImpl()
{
  mMainModule = nullptr;
  mMainDict = nullptr;
  mPythonEnabled = false;
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
  Q_FOREACH ( QString p, extraPluginsPaths() )
  {
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
  runString( "sys.path = [" + newpaths.join( QStringLiteral( "," ) ) + "] + sys.path" );

  // import SIP
  if ( !runString( QStringLiteral( "import sip" ),
                   QObject::tr( "Couldn't load SIP module." ) + '\n' + QObject::tr( "Python support will be disabled." ) ) )
  {
    return false;
  }

  // set PyQt api versions
  QStringList apiV2classes;
  apiV2classes << QStringLiteral( "QDate" ) << QStringLiteral( "QDateTime" ) << QStringLiteral( "QString" ) << QStringLiteral( "QTextStream" ) << QStringLiteral( "QTime" ) << QStringLiteral( "QUrl" ) << QStringLiteral( "QVariant" );
  Q_FOREACH ( const QString &clsName, apiV2classes )
  {
    if ( !runString( QStringLiteral( "sip.setapi('%1', 2)" ).arg( clsName ),
                     QObject::tr( "Couldn't set SIP API versions." ) + '\n' + QObject::tr( "Python support will be disabled." ) ) )
    {
      return false;
    }
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
  runString( "qgis.utils.plugin_paths = [" + pluginpaths.join( QStringLiteral( "," ) ) + ']' );
  runString( "qgis.utils.sys_plugin_path = \"" + pluginsPath() + '\"' );
  runString( "qgis.utils.home_plugin_path = " + homePluginsPath() );

#ifdef Q_OS_WIN
  runString( "if oldhome: os.environ['HOME']=oldhome\n" );
#endif

  return true;
}

void QgsPythonUtilsImpl::init()
{
  // initialize python
  Py_Initialize();
  // initialize threading AND acquire GIL
  PyEval_InitThreads();

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
  QString error_msg = QObject::tr( "Couldn't load qgis.user." ) + '\n' + QObject::tr( "Python support will be disabled." );
  if ( !runString( QStringLiteral( "import qgis.user" ), error_msg ) )
  {
    // Should we really bail because of this?!
    return false;
  }
  return true;
}

void QgsPythonUtilsImpl::doCustomImports()
{
  QStringList startupPaths = QStandardPaths::locateAll( QStandardPaths::AppDataLocation, QStringLiteral( "startup.py" ) );
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

void QgsPythonUtilsImpl::initPython( QgisInterface *interface )
{
  init();
  if ( !checkSystemImports() )
  {
    exitPython();
    return;
  }
  // initialize 'iface' object
  runString( "qgis.utils.initInterface(" + QString::number( ( quint64 ) interface ) + ')' );
  if ( !checkQgisUser() )
  {
    exitPython();
    return;
  }
  doCustomImports();
  installErrorHook();
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
  QString error_msg = QObject::tr( "Couldn't load PyQGIS Server." ) + '\n' + QObject::tr( "Python support will be disabled." );
  if ( !runString( QStringLiteral( "from qgis.server import *" ), error_msg ) )
  {
    return;
  }

  // This is the other main difference with initInterface() for desktop plugins
  runString( "qgis.utils.initServerInterface(" + QString::number( ( quint64 ) interface ) + ')' );

  doCustomImports();
  finish();
}

bool QgsPythonUtilsImpl::startServerPlugin( QString packageName )
{
  QString output;
  evalString( "qgis.utils.startServerPlugin('" + packageName + "')", output );
  return ( output == QLatin1String( "True" ) );
}

#endif // End HAVE_SERVER_PYTHON_PLUGINS

void QgsPythonUtilsImpl::exitPython()
{
  uninstallErrorHook();
  Py_Finalize();
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
}

void QgsPythonUtilsImpl::uninstallErrorHook()
{
  runString( QStringLiteral( "qgis.utils.uninstallErrorHook()" ) );
}

bool QgsPythonUtilsImpl::runStringUnsafe( const QString &command, bool single )
{
  // acquire global interpreter lock to ensure we are in a consistent state
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();

  // TODO: convert special characters from unicode strings u"…" to \uXXXX
  // so that they're not mangled to utf-8
  // (non-unicode strings can be mangled)
  PyObject *obj = PyRun_String( command.toUtf8().constData(), single ? Py_single_input : Py_file_input, mMainDict, mMainDict );
  bool res = nullptr == PyErr_Occurred();
  Py_XDECREF( obj );

  // we are done calling python API, release global interpreter lock
  PyGILState_Release( gstate );

  return res;
}

bool QgsPythonUtilsImpl::runString( const QString &command, QString msgOnError, bool single )
{
  bool res = runStringUnsafe( command, single );
  if ( res )
    return true;

  if ( msgOnError.isEmpty() )
  {
    // use some default message if custom hasn't been specified
    msgOnError = QObject::tr( "An error occurred during execution of following code:" ) + "\n<tt>" + command + "</tt>";
  }

  // TODO: use python implementation

  QString traceback = getTraceback();
  QString path, version;
  evalString( QStringLiteral( "str(sys.path)" ), path );
  evalString( QStringLiteral( "sys.version" ), version );

  QString str = "<font color=\"red\">" + msgOnError + "</font><br><pre>\n" + traceback + "\n</pre>"
                + QObject::tr( "Python version:" ) + "<br>" + version + "<br><br>"
                + QObject::tr( "QGIS version:" ) + "<br>" + QStringLiteral( "%1 '%2', %3" ).arg( Qgis::QGIS_VERSION, Qgis::QGIS_RELEASE_NAME, Qgis::QGIS_DEV_VERSION ) + "<br><br>"
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

  // acquire global interpreter lock to ensure we are in a consistent state
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();

  QString errMsg;
  QString result;

  PyObject *modStringIO = nullptr;
  PyObject *modTB = nullptr;
  PyObject *obStringIO = nullptr;
  PyObject *obResult = nullptr;

  PyObject *type, *value, *traceback;

  PyErr_Fetch( &type, &value, &traceback );
  PyErr_NormalizeException( &type, &value, &traceback );

  const char *iomod = "io";

  modStringIO = PyImport_ImportModule( iomod );
  if ( !modStringIO )
    TRACEBACK_FETCH_ERROR( QString( "can't import %1" ).arg( iomod ) );

  obStringIO = PyObject_CallMethod( modStringIO, ( char * ) "StringIO", nullptr );

  /* Construct a cStringIO object */
  if ( !obStringIO )
    TRACEBACK_FETCH_ERROR( "cStringIO.StringIO() failed" );

  modTB = PyImport_ImportModule( "traceback" );
  if ( !modTB )
    TRACEBACK_FETCH_ERROR( "can't import traceback" );

  obResult = PyObject_CallMethod( modTB, ( char * ) "print_exception",
                                  ( char * ) "OOOOO",
                                  type, value ? value : Py_None,
                                  traceback ? traceback : Py_None,
                                  Py_None,
                                  obStringIO );

  if ( !obResult )
    TRACEBACK_FETCH_ERROR( "traceback.print_exception() failed" );

  Py_DECREF( obResult );

  obResult = PyObject_CallMethod( obStringIO, ( char * ) "getvalue", nullptr );
  if ( !obResult )
    TRACEBACK_FETCH_ERROR( "getvalue() failed." );

  /* And it should be a string all ready to go - duplicate it. */
  if ( !PyUnicode_Check( obResult ) )
    TRACEBACK_FETCH_ERROR( "getvalue() did not return a string" );

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

  // we are done calling python API, release global interpreter lock
  PyGILState_Release( gstate );

  return result;
}

QString QgsPythonUtilsImpl::getTypeAsString( PyObject *obj )
{
  if ( !obj )
    return QString();

  if ( PyType_Check( obj ) )
  {
    QgsDebugMsg( "got type" );
    return QString( ( ( PyTypeObject * )obj )->tp_name );
  }
  else
  {
    QgsDebugMsg( "got object" );
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
  QgsDebugMsg( "unable to convert PyObject to a QString!" );
  return QStringLiteral( "(qgis error)" );
}


bool QgsPythonUtilsImpl::evalString( const QString &command, QString &result )
{
  // acquire global interpreter lock to ensure we are in a consistent state
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();

  PyObject *res = PyRun_String( command.toUtf8().constData(), Py_eval_input, mMainDict, mMainDict );
  bool success = nullptr != res;

  // TODO: error handling

  if ( success )
    result = PyObjectToQString( res );

  Py_XDECREF( res );

  // we are done calling python API, release global interpreter lock
  PyGILState_Release( gstate );

  return success;
}

QString QgsPythonUtilsImpl::pythonPath()
{
  if ( QgsApplication::isRunningFromBuildDir() )
    return QgsApplication::buildOutputPath() + "/python";
  else
    return QgsApplication::pkgDataPath() + "/python";
}

QString QgsPythonUtilsImpl::pluginsPath()
{
  return pythonPath() + "/plugins";
}

QString QgsPythonUtilsImpl::homePythonPath()
{
  QString settingsDir = QgsApplication::qgisSettingsDirPath();
  if ( QDir::cleanPath( settingsDir ) == QDir::homePath() + QStringLiteral( "/.qgis3" ) )
  {
    return QStringLiteral( "\"%1/.qgis3/python\"" ).arg( QDir::homePath() );
  }
  else
  {
    return "\"" + settingsDir.replace( '\\', QLatin1String( "\\\\" ) ) + "python\"";
  }
}

QString QgsPythonUtilsImpl::homePluginsPath()
{
  return homePythonPath() + " + \"/plugins\"";
}

QStringList QgsPythonUtilsImpl::extraPluginsPaths()
{
  const char *cpaths = getenv( "QGIS_PLUGINPATH" );
  if ( !cpaths )
    return QStringList();

  QString paths = QString::fromLocal8Bit( cpaths );
#ifndef Q_OS_WIN
  if ( paths.contains( ':' ) )
    return paths.split( ':', QString::SkipEmptyParts );
#endif
  if ( paths.contains( ';' ) )
    return paths.split( ';', QString::SkipEmptyParts );
  else
    return QStringList( paths );
}


QStringList QgsPythonUtilsImpl::pluginList()
{
  runString( QStringLiteral( "qgis.utils.updateAvailablePlugins()" ) );

  QString output;
  evalString( QStringLiteral( "'\\n'.join(qgis.utils.available_plugins)" ), output );
  return output.split( QChar( '\n' ), QString::SkipEmptyParts );
}

QString QgsPythonUtilsImpl::getPluginMetadata( const QString &pluginName, const QString &function )
{
  QString res;
  QString str = "qgis.utils.pluginMetadata('" + pluginName + "', '" + function + "')";
  evalString( str, res );
  //QgsDebugMsg("metadata "+pluginName+" - '"+function+"' = "+res);
  return res;
}

bool QgsPythonUtilsImpl::loadPlugin( const QString &packageName )
{
  QString output;
  evalString( "qgis.utils.loadPlugin('" + packageName + "')", output );
  return ( output == QLatin1String( "True" ) );
}

bool QgsPythonUtilsImpl::startPlugin( const QString &packageName )
{
  QString output;
  evalString( "qgis.utils.startPlugin('" + packageName + "')", output );
  return ( output == QLatin1String( "True" ) );
}

bool QgsPythonUtilsImpl::canUninstallPlugin( const QString &packageName )
{
  QString output;
  evalString( "qgis.utils.canUninstallPlugin('" + packageName + "')", output );
  return ( output == QLatin1String( "True" ) );
}

bool QgsPythonUtilsImpl::unloadPlugin( const QString &packageName )
{
  QString output;
  evalString( "qgis.utils.unloadPlugin('" + packageName + "')", output );
  return ( output == QLatin1String( "True" ) );
}

bool QgsPythonUtilsImpl::isPluginLoaded( const QString &packageName )
{
  QString output;
  evalString( "qgis.utils.isPluginLoaded('" + packageName + "')", output );
  return ( output == QLatin1String( "True" ) );
}

QStringList QgsPythonUtilsImpl::listActivePlugins()
{
  QString output;
  evalString( QStringLiteral( "'\\n'.join(qgis.utils.active_plugins)" ), output );
  return output.split( QChar( '\n' ), QString::SkipEmptyParts );
}
