/***************************************************************************
                              qgsgrasstools.cpp
                             -------------------
    begin                : March, 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <q3cstring.h>
#include <q3groupbox.h>
#include <q3listbox.h>
#include <q3listview.h>
#include <q3painter.h>
#include <q3picture.h>
#include <q3pointarray.h>
#include <q3process.h>
#include <q3progressbar.h>
#include <q3stylesheet.h>
#include <q3table.h>
#include <q3textbrowser.h>
#include <qapplication.h>
#include <qcolordialog.h>
#include <QComboBox>
#include <qcursor.h>
#include <qdir.h>
#include <qdom.h>
#include <QDoubleValidator>
#include <qevent.h>
#include <QFileDialog>
#include <qfile.h>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <qimage.h>
#include <qinputdialog.h>
#include <QIntValidator>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <QProcess>
#include <QPushButton>
#include <qpushbutton.h>
#include <qregexp.h>
#include <QRegExpValidator>
#include <qsettings.h>
#include <qsize.h>
#include <qspinbox.h>
#include <qstatusbar.h>
#include <qstringlist.h>
#include <qtabwidget.h>
#include <QUrl>
#include <QVBoxLayout>
#include "qgis.h"
#include "qgslogger.h"
#include "qgisinterface.h"
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include <qgsrasterlayer.h>
#include "qgsdatasourceuri.h"
#include "qgsdataprovider.h"
#include "qgsfield.h"
#include "qgsfeature.h"

#include <typeinfo>

extern "C"
{
#include <grass/gis.h>
#include <grass/Vect.h>
}

#include "qgsgrass.h"
#include "qgsgrassprovider.h"
#include "qgsgrassattributes.h"
#include "qgsgrassmodule.h"
#include "qgsgrassmapcalc.h"
#include "qgsgrasstools.h"
#include "qgsgrassselect.h"
#include "qgsgrassplugin.h"

#include <gdal.h>         // to collect version information

#if defined(WIN32)
#include <windows.h>
static QString getShortPath( const QString &path )
{
  TCHAR buf[MAX_PATH];
  GetShortPathName( path.ascii(), buf, MAX_PATH );
  return buf;
}
#endif

bool QgsGrassModule::mExecPathInited = 0;
QStringList QgsGrassModule::mExecPath;






QString QgsGrassModule::findExec( QString file )
{
  QgsDebugMsg( "called." );

  // Init mExecPath
  // Windows searches first in current directory
  if ( !mExecPathInited )
  {
    QString path = getenv( "PATH" );
    QgsDebugMsg( "path = " + path );

#ifdef WIN32
    mExecPath = path.split( ";" );
    mExecPath.prepend( getShortPath( QgsApplication::applicationDirPath() ) );
#else
    mExecPath = path.split( ":" );
    mExecPath.prepend( QgsApplication::applicationDirPath() );
#endif
    mExecPathInited = true;
  }

  if ( QFile::exists( file ) ) return file;  // full path

  // Search for module
  for ( QStringList::iterator it = mExecPath.begin();
        it != mExecPath.end(); ++it )
  {
    QString full = *it + "/" + file;
    if ( QFile::exists( full ) )
    {
      return full;
    }
  }

  // Not found try with .exe
#ifdef WIN32
  for ( QStringList::iterator it = mExecPath.begin();
        it != mExecPath.end(); ++it )
  {
    QString full = *it + "/" + file + ".exe";
    if ( QFile::exists( full ) )
    {
      return full;
    }
  }
#endif

  return QString();
}

bool QgsGrassModule::inExecPath( QString file )
{
  if ( findExec( file ).isNull() ) return false;
  return true;
}

QStringList QgsGrassModule::execArguments( QString module )
{
  QString exe;
  QStringList arguments;

  exe = QgsGrassModule::findExec( module );
  if ( exe.isNull() )
  {
    return arguments;
  }

#if defined(WIN32)
  QFileInfo fi( exe );
  if ( fi.isExecutable() )
  {
    arguments.append( exe );
  }
  else // script
  {
    QString cmd = getShortPath( QgsApplication::applicationDirPath() ) + "/msys/bin/sh";
    arguments.append( cmd );

    // Important! Otherwise it does not find DLL even if it is in PATH
    arguments.append( "--login" );
    arguments.append( exe );
  }
#else
  arguments.append( exe );
#endif

  return arguments;
}

QgsGrassModule::QgsGrassModule( QgsGrassTools *tools, QString moduleName, QgisInterface *iface,
                                QString path, QWidget * parent, const char * name, Qt::WFlags f )
    : QgsGrassModuleBase( ), mSuccess( false )
{
  QgsDebugMsg( "called" );

  setupUi( this );
  lblModuleName->setText( tr( "Module" ) + ": " + moduleName );
  mPath = path;
  mTools = tools;
  mIface = iface;
  mCanvas = mIface->getMapCanvas();
  mParent = parent;

  /* Read module description and create options */

  // Open QGIS module description
  QString mpath = mPath + ".qgm";
  QFile qFile( mpath );
  if ( !qFile.exists() )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "The module file (" ) + mpath + tr( ") not found." ) );
    return;
  }
  if ( ! qFile.open( QIODevice::ReadOnly ) )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot open module file (" ) + mpath + tr( ")" ) );
    return;
  }
  QDomDocument qDoc( "qgisgrassmodule" );
  QString err;
  int line, column;
  if ( !qDoc.setContent( &qFile,  &err, &line, &column ) )
  {
    QString errmsg = tr( "Cannot read module file (" ) + mpath + tr( "):\n" ) + err + tr( "\nat line " )
                     + QString::number( line ) + " column " + QString::number( column );
    QgsDebugMsg( errmsg );
    QMessageBox::warning( 0, tr( "Warning" ), errmsg );
    qFile.close();
    return;
  }
  qFile.close();
  QDomElement qDocElem = qDoc.documentElement();

  // Read GRASS module description
  QString xName = qDocElem.attribute( "module" );

  // Binary modules on windows has .exe extension
  // but not all modules have to be binary (can be scripts)
  // => test if the module is in path and if it is not
  // add .exe and test again
#ifdef WIN32
  if ( inExecPath( xName ) )
  {
    mXName = xName;
  }
  else if ( inExecPath( xName + ".exe" ) )
  {
    mXName = xName + ".exe";
  }
  else
  {
    QgsDebugMsg( "Module " + xName + " not found" );
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Module " ) + xName + tr( " not found" ) );
    return;
  }
#else
  mXName = xName;
#endif

  if ( xName == "r.mapcalc" )
  {
    QGridLayout *layout = new QGridLayout( mTabWidget->page( 0 ), 1, 1 );

    mOptions = new QgsGrassMapcalc( mTools, this,
                                    mIface, mTabWidget->page( 0 ) );

    QWidget *w = dynamic_cast<QWidget *>( mOptions );

    layout->addWidget( w, 0, 0 );
  }
  else
  {
    mOptions = new QgsGrassModuleStandardOptions( mTools, this,
        mIface, mXName, qDocElem, mTabWidget->page( 0 ) );
  }

  // Hide display if there is no output
  if ( mOptions->output( QgsGrassModuleOption::Vector ).size() == 0
       && mOptions->output( QgsGrassModuleOption::Raster ).size() == 0 )
  {
    mViewButton->hide();
  }
  mViewButton->setEnabled( false );

  // Create manual if available
  QString gisBase = getenv( "GISBASE" );
  QString manPath = gisBase + "/docs/html/" + xName + ".html";
  QFile manFile( manPath );
  if ( manFile.exists() )
  {
    mManualTextBrowser->setSource( QUrl::fromLocalFile( manPath ) );
  }
  else
  {
    mManualTextBrowser->clear();
    mManualTextBrowser->textCursor().insertImage( ":/grass/error.png" );
    mManualTextBrowser->insertPlainText( tr( "Cannot find man page " ) + manPath );
    mManualTextBrowser->insertPlainText( tr( "Please ensure you have the GRASS documentation installed." ) );
  }

  connect( &mProcess, SIGNAL( readyReadStandardOutput() ), this, SLOT( readStdout() ) );
  connect( &mProcess, SIGNAL( readyReadStandardError() ), this, SLOT( readStderr() ) );
  connect( &mProcess, SIGNAL( finished( int, QProcess::ExitStatus ) ), this, SLOT( finished( int, QProcess::ExitStatus ) ) );

  const char *env = "GRASS_MESSAGE_FORMAT=gui";
  char *envstr = new char[strlen( env )+1];
  strcpy( envstr, env );
  putenv( envstr );

  mOutputTextBrowser->setTextFormat( Qt::RichText );
  mOutputTextBrowser->setReadOnly( TRUE );
}

/******************* QgsGrassModuleOptions *******************/

QgsGrassModuleOptions::QgsGrassModuleOptions(
  QgsGrassTools *tools, QgsGrassModule *module,
  QgisInterface *iface )
{
  QgsDebugMsg( "called." );

  mTools = tools;
  mModule = module;
  mIface = iface;
  mCanvas = mIface->getMapCanvas();
}

QgsGrassModuleOptions::~QgsGrassModuleOptions()
{
}

QStringList QgsGrassModuleOptions::arguments()
{
  return QStringList();
}

/*************** QgsGrassModuleStandardOptions ***********************/

QgsGrassModuleStandardOptions::QgsGrassModuleStandardOptions(
  QgsGrassTools *tools, QgsGrassModule *module,
  QgisInterface *iface,
  QString xname, QDomElement qDocElem,
  QWidget * parent, const char * name, Qt::WFlags f )
    : QgsGrassModuleOptions( tools, module, iface ),
    QWidget( parent, name, f )
{
  QgsDebugMsg( "called." );
  QgsDebugMsg( QString( "PATH = %1" ).arg( getenv( "PATH" ) ) );

  // Attention!: sh.exe (MSYS) sets $0 in scripts to file name
  // without full path. Strange because when run from msys.bat
  // $0 is set to full path. GRASS scripts call
  // exec g.parser "$0" "$@"
  // and it fails -> find and run with full path

  mXName = xname;
  mParent = parent;

  QStringList arguments = QgsGrassModule::execArguments( mXName );

  if ( arguments.size() == 0 )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot find module " )
                          + mXName );
    return;
  }

  QString cmd = arguments.takeFirst();

  arguments.append( "--interface-description" );

  QProcess process( this );
  process.start( cmd, arguments );

  // ? Does binary on Win need .exe extention ?
  // Return code 255 (-1) was correct in GRASS < 6.1.0
  if ( !process.waitForFinished()
       || ( process.exitCode() != 0 && process.exitCode() != 255 ) )
  {
    QgsDebugMsg( "process.exitCode() = " + QString::number( process.exitCode() ) );
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot start module " ) + mXName + "<br>"
                          + cmd + " " + arguments.join( " " ) + "<br>"
                          + QString( process.readAllStandardOutput() ) + "<br>"
                          + QString( process.readAllStandardError() ) );
    return;
  }
  QByteArray gDescArray = process.readAllStandardOutput();
  QByteArray errArray = process.readAllStandardError();

  QDomDocument gDoc( "task" );
  QString err;
  int line, column;
  if ( !gDoc.setContent(( QByteArray )gDescArray, &err, &line, &column ) )
  {
    QString errmsg = tr( "Cannot read module description (" ) + mXName + tr( "):\n" ) + err + tr( "\nat line " )
                     + QString::number( line ) + tr( " column " ) + QString::number( column );
    QgsDebugMsg( errmsg );
    QgsDebugMsg( QString( gDescArray ) );
    QgsDebugMsg( QString( errArray ) );
    QMessageBox::warning( 0, tr( "Warning" ), errmsg );
    return;
  }
  QDomElement gDocElem = gDoc.documentElement();

  // Read QGIS options and create controls
  QDomNode n = qDocElem.firstChild();
  //
  //Set up dynamic inside a scroll box
  //
  QVBoxLayout * mypOuterLayout = new QVBoxLayout( mParent );
  mypOuterLayout->setContentsMargins( 0, 0, 0, 0 );
  //transfers layout ownership so no need to call delete
  this->setLayout( mypOuterLayout );
  QScrollArea * mypScrollArea = new QScrollArea();
  //transfers scroll area ownership so no need to call delete
  mypOuterLayout->addWidget( mypScrollArea );
  QFrame * mypInnerFrame = new QFrame();
  mypInnerFrame->setFrameShape( QFrame::NoFrame );
  mypInnerFrame->setFrameShadow( QFrame::Plain );
  //transfers frame ownership so no need to call delete
  mypScrollArea->setWidget( mypInnerFrame );
  mypScrollArea->setWidgetResizable( true );
  QVBoxLayout *layout = new QVBoxLayout( mypInnerFrame );
  while ( !n.isNull() )
  {
    QDomElement e = n.toElement();
    if ( !e.isNull() )
    {
      QString optionType = e.tagName();
      QgsDebugMsg( "optionType = " + optionType );

      QString key = e.attribute( "key" );
      QgsDebugMsg( "key = " + key );

      QDomNode gnode = QgsGrassModule::nodeByKey( gDocElem, key );
      if ( gnode.isNull() )
      {
        QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot find key " ) +  key );
        return;
      }

      if ( optionType == "option" )
      {
        bool created = false;

        // Check option type and create appropriate control
        QDomNode promptNode = gnode.namedItem( "gisprompt" );
        if ( !promptNode.isNull() )
        {
          QDomElement promptElem = promptNode.toElement();
          QString element = promptElem.attribute( "element" );
          QString age = promptElem.attribute( "age" );
          //QgsDebugMsg("element = " + element + " age = " + age);
          if ( age == "old" && ( element == "vector" || element == "cell" ) )
          {
            QgsGrassModuleInput *mi = new QgsGrassModuleInput(
              mModule, this, key, e, gDocElem, gnode, mParent );

            layout->addWidget( mi );
            created = true;
            mItems.push_back( mi );
          }
        }

        if ( !created )
        {
          QgsGrassModuleOption *so = new QgsGrassModuleOption(
            mModule, key, e, gDocElem, gnode, mParent );

          layout->addWidget( so );
          created = true;
          mItems.push_back( so );
        }
      }
      else if ( optionType == "ogr" )
      {
        QgsGrassModuleGdalInput *mi = new QgsGrassModuleGdalInput(
          mModule, QgsGrassModuleGdalInput::Ogr, key, e,
          gDocElem, gnode, mParent );
        layout->addWidget( mi );
        mItems.push_back( mi );
      }
      else if ( optionType == "gdal" )
      {
        QgsGrassModuleGdalInput *mi = new QgsGrassModuleGdalInput(
          mModule, QgsGrassModuleGdalInput::Gdal, key, e,
          gDocElem, gnode, mParent );
        layout->addWidget( mi );
        mItems.push_back( mi );
      }
      else if ( optionType == "field" )
      {
        QgsGrassModuleField *mi = new QgsGrassModuleField(
          mModule, this, key, e,
          gDocElem, gnode, mParent );
        layout->addWidget( mi );
        mItems.push_back( mi );
      }
      else if ( optionType == "selection" )
      {
        QgsGrassModuleSelection *mi = new QgsGrassModuleSelection(
          mModule, this, key, e,
          gDocElem, gnode, mParent );
        layout->addWidget( mi );
        mItems.push_back( mi );
      }
      else if ( optionType == "file" )
      {
        QgsGrassModuleFile *mi = new QgsGrassModuleFile(
          mModule, key, e, gDocElem, gnode, mParent );
        layout->addWidget( mi );
        mItems.push_back( mi );
      }
      else if ( optionType == "flag" )
      {
        QgsGrassModuleFlag *flag = new QgsGrassModuleFlag(
          mModule, key, e, gDocElem, gnode, mParent );

        layout->addWidget( flag );
        mItems.push_back( flag );
      }
    }
    n = n.nextSibling();
  }

  // Create list of flags
  n = gDocElem.firstChild();
  while ( !n.isNull() )
  {
    QDomElement e = n.toElement();
    if ( !e.isNull() )
    {
      QString optionType = e.tagName();
      QgsDebugMsg( "optionType = " + optionType );

      if ( optionType == "flag" )
      {
        QString name = e.attribute( "name" ).trimmed();
        QgsDebugMsg( "name = " + name );
        mFlagNames.append( name );
      }
    }
    n = n.nextSibling();
  }

  layout->addStretch();
}

QStringList QgsGrassModuleStandardOptions::arguments()
{
  QStringList arg;

  for ( unsigned int i = 0; i < mItems.size(); i++ )
  {
    QStringList list = mItems[i]->options();

    for ( QStringList::Iterator it = list.begin();
          it != list.end(); ++it )
    {
      arg.append( *it );
    }
  }
  return arg;
}

QgsGrassModuleItem *QgsGrassModuleStandardOptions::item( QString id )
{
  QgsDebugMsg( "id = " + id );

  for ( unsigned int i = 0; i < mItems.size(); i++ )
  {
    if ( mItems[i]->id() == id )
    {
      return mItems[i];
    }
  }

  QMessageBox::warning( 0, tr( "Warning" ), tr( "Item with id " ) + id + tr( " not found" ) );
  return 0;
}

QStringList QgsGrassModuleStandardOptions::checkOutput()
{
  QgsDebugMsg( "called." );
  QStringList list;

  for ( unsigned int i = 0; i < mItems.size(); i++ )
  {
    if ( typeid( *( mItems[i] ) ) != typeid( QgsGrassModuleOption ) )
    {
      continue;
    }
    QgsGrassModuleOption *opt =
      dynamic_cast<QgsGrassModuleOption *>( mItems[i] );

    QgsDebugMsg( "opt->key() = " + opt->key() );

    if ( opt->isOutput() )
    {
      QString out = opt->outputExists();
      if ( !out.isNull() )
      {
        list.append( out );
      }
    }
  }

  return list;
}

void QgsGrassModuleStandardOptions::freezeOutput()
{
  QgsDebugMsg( "called." );

#ifdef WIN32
  for ( unsigned int i = 0; i < mItems.size(); i++ )
  {
    if ( typeid( *( mItems[i] ) ) != typeid( QgsGrassModuleOption ) )
    {
      continue;
    }
    QgsGrassModuleOption *opt =
      dynamic_cast<QgsGrassModuleOption *>( mItems[i] );

    QgsDebugMsg( "opt->key() = " + opt->key() );

    if ( opt->isOutput()
         && opt->outputType() == QgsGrassModuleOption::Vector )
    {
      QgsDebugMsg( "freeze vector layers" );

      QChar sep = '/';

      int nlayers = mCanvas->layerCount();
      for ( int i = 0; i < nlayers; i++ )
      {
        QgsMapLayer *layer = mCanvas->getZpos( i );

        if ( layer->type() != QgsMapLayer::VECTOR ) continue;

        QgsVectorLayer *vector = ( QgsVectorLayer* )layer;
        if ( vector->providerType() != "grass" ) continue;

        //TODO dynamic_cast ?
        QgsGrassProvider *provider = ( QgsGrassProvider * ) vector->dataProvider();

        // TODO add map() mapset() location() gisbase() to grass provider
        QString source = QDir::cleanPath( provider->dataSourceUri() );

        QgsDebugMsg( "source = " + source );

        // Check GISBASE and LOCATION
        QStringList split = QStringList::split( sep, source );

        if ( split.size() < 4 ) continue;
        split.pop_back(); // layer

        QString map = split.last();
        split.pop_back(); // map

        QString mapset = split.last();
        split.pop_back(); // mapset

        QString loc =  source.remove( QRegExp( "/[^/]+/[^/]+/[^/]+$" ) );
        loc = QDir( loc ).canonicalPath();

        QDir curlocDir( QgsGrass::getDefaultGisdbase() + sep + QgsGrass::getDefaultLocation() );
        QString curloc = curlocDir.canonicalPath();

        if ( loc != curloc ) continue;

        if ( mapset != QgsGrass::getDefaultMapset() ) continue;

        if ( provider->isFrozen() ) continue;

        provider->freeze();
      }
    }
  }
#endif
}

void QgsGrassModuleStandardOptions::thawOutput()
{
  QgsDebugMsg( "called." );

#ifdef WIN32
  for ( unsigned int i = 0; i < mItems.size(); i++ )
  {
    if ( typeid( *( mItems[i] ) ) != typeid( QgsGrassModuleOption ) )
    {
      continue;
    }
    QgsGrassModuleOption *opt =
      dynamic_cast<QgsGrassModuleOption *>( mItems[i] );

    QgsDebugMsg( "opt->key() = " + opt->key() );

    if ( opt->isOutput()
         && opt->outputType() == QgsGrassModuleOption::Vector )
    {
      QgsDebugMsg( "thaw vector layers" );

      QChar sep = '/';

      int nlayers = mCanvas->layerCount();
      for ( int i = 0; i < nlayers; i++ )
      {
        QgsMapLayer *layer = mCanvas->getZpos( i );

        if ( layer->type() != QgsMapLayer::VECTOR ) continue;

        QgsVectorLayer *vector = ( QgsVectorLayer* )layer;
        if ( vector->providerType() != "grass" ) continue;

        //TODO dynamic_cast ?
        QgsGrassProvider *provider = ( QgsGrassProvider * ) vector->dataProvider();

        // TODO add map() mapset() location() gisbase() to grass provider
        QString source = QDir::cleanPath( provider->dataSourceUri() );

        QgsDebugMsg( "source = " + source );

        // Check GISBASE and LOCATION
        QStringList split = QStringList::split( sep, source );

        if ( split.size() < 4 ) continue;
        split.pop_back(); // layer

        QString map = split.last();
        split.pop_back(); // map

        QString mapset = split.last();
        split.pop_back(); // mapset

        QString loc =  source.remove( QRegExp( "/[^/]+/[^/]+/[^/]+$" ) );
        loc = QDir( loc ).canonicalPath();

        QDir curlocDir( QgsGrass::getDefaultGisdbase() + sep + QgsGrass::getDefaultLocation() );
        QString curloc = curlocDir.canonicalPath();

        if ( loc != curloc ) continue;

        if ( mapset != QgsGrass::getDefaultMapset() ) continue;

        if ( !provider->isFrozen() ) continue;

        provider->thaw();
      }
    }
  }
#endif
}


QStringList QgsGrassModuleStandardOptions::output( int type )
{
  QgsDebugMsg( "called." );
  QStringList list;

  for ( unsigned int i = 0; i < mItems.size(); i++ )
  {
    if ( typeid( *( mItems[i] ) ) != typeid( QgsGrassModuleOption ) )
    {
      continue;
    }
    QgsGrassModuleOption *opt =
      dynamic_cast<QgsGrassModuleOption *>( mItems[i] );

    QgsDebugMsg( "opt->key() = " + opt->key() );

    if ( opt->isOutput() )
    {
      if ( opt->outputType() == type )
      {
        QString out = opt->value();
        list.append( out );
      }
    }
  }

  return list;
}

QStringList QgsGrassModuleStandardOptions::ready()
{
  QgsDebugMsg( "entered." );
  QStringList list;

  for ( unsigned int i = 0; i < mItems.size(); i++ )
  {
    QString err = mItems[i]->ready();
    if ( !err.isNull() )
    {
      list.append( err );
    }
  }

  return list;
}

QStringList QgsGrassModuleStandardOptions::checkRegion()
{
  QgsDebugMsg( "called." );
  QStringList list;

  struct Cell_head currentWindow;
  if ( !QgsGrass::region( QgsGrass::getDefaultGisdbase(),
                          QgsGrass::getDefaultLocation(),
                          QgsGrass::getDefaultMapset(), &currentWindow ) )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot get current region" ) );
    return list;
  }

  for ( unsigned int i = 0; i < mItems.size(); i++ )
  {
    if ( typeid( *( mItems[i] ) ) != typeid( QgsGrassModuleInput ) )
    {
      continue;
    }

    struct Cell_head window;

    QgsGrassModuleInput *item = dynamic_cast<QgsGrassModuleInput *>( mItems[i] );

    QgsGrass::MapType mapType = QgsGrass::Vector;
    switch ( item->type() )
    {
      case QgsGrassModuleInput::Raster :
        mapType = QgsGrass::Raster;
        break;
      case QgsGrassModuleInput::Vector :
        mapType = QgsGrass::Vector;
        break;
    }

    QStringList mm = item->currentMap().split( "@" );
    QString map = mm.at( 0 );
    QString mapset = QgsGrass::getDefaultMapset();
    if ( mm.size() > 1 ) mapset = mm.at( 1 );
    if ( !QgsGrass::mapRegion( mapType,
                               QgsGrass::getDefaultGisdbase(),
                               QgsGrass::getDefaultLocation(), mapset, map,
                               &window ) )
    {
      QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot check region "
                            "of map " ) + item->currentMap() );
      continue;
    }

    if ( G_window_overlap( &currentWindow ,
                           window.north, window.south, window.east, window.west ) == 0 )
    {
      list.append( item->currentMap() );
    }
  }

  return list;
}

bool QgsGrassModuleStandardOptions::inputRegion( struct Cell_head *window, bool all )
{
  QgsDebugMsg( "called." );

  // Get current resolution
  if ( !QgsGrass::region( QgsGrass::getDefaultGisdbase(),
                          QgsGrass::getDefaultLocation(),
                          QgsGrass::getDefaultMapset(), window ) )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot get current region" ) );
    return false;
  }

  int rasterCount = 0;
  int vectorCount = 0;
  for ( unsigned int i = 0; i < mItems.size(); i++ )
  {
    if ( typeid( *( mItems[i] ) ) != typeid( QgsGrassModuleInput ) )
    {
      continue;
    }

    struct Cell_head mapWindow;

    QgsGrassModuleInput *item = dynamic_cast<QgsGrassModuleInput *>( mItems[i] );

    if ( !all && !item->useRegion() ) continue;

    QgsGrass::MapType mapType = QgsGrass::Vector;

    switch ( item->type() )
    {
      case QgsGrassModuleInput::Raster :
        mapType = QgsGrass::Raster;
        break;
      case QgsGrassModuleInput::Vector :
        mapType = QgsGrass::Vector;
        break;
    }

    QStringList mm = item->currentMap().split( "@" );
    QString map = mm.at( 0 );
    QString mapset = QgsGrass::getDefaultMapset();
    if ( mm.size() > 1 ) mapset = mm.at( 1 );
    if ( !QgsGrass::mapRegion( mapType,
                               QgsGrass::getDefaultGisdbase(),
                               QgsGrass::getDefaultLocation(), mapset, map,
                               &mapWindow ) )
    {
      QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot set region "
                            "of map " ) + item->currentMap() );
      return false;
    }

    // TODO: best way to set resolution ?
    if ( item->type() == QgsGrassModuleInput::Raster
         && rasterCount == 0 )
    {
      QgsGrass::copyRegionResolution( &mapWindow, window );
    }
    if ( rasterCount + vectorCount == 0 )
    {
      QgsGrass::copyRegionExtent( &mapWindow, window );
    }
    else
    {
      QgsGrass::extendRegion( &mapWindow, window );
    }

    if ( item->type() == QgsGrassModuleInput::Raster )
      rasterCount++;
    else if ( item->type() == QgsGrassModuleInput::Vector )
      vectorCount++;
  }

  G_adjust_Cell_head3( window, 0, 0, 0 );

  return true;
}

bool QgsGrassModuleStandardOptions::requestsRegion()
{
  QgsDebugMsg( "called." );

  for ( unsigned int i = 0; i < mItems.size(); i++ )
  {
    if ( typeid( *( mItems[i] ) ) != typeid( QgsGrassModuleInput ) )
    {
      continue;
    }

    QgsGrassModuleInput *item = dynamic_cast<QgsGrassModuleInput *>
                                ( mItems[i] );

    if ( item->useRegion() ) return true;
  }
  return false;
}

bool QgsGrassModuleStandardOptions::usesRegion()
{
  QgsDebugMsg( "called." );

  for ( unsigned int i = 0; i < mItems.size(); i++ )
  {
    if ( typeid( *( mItems[i] ) ) == typeid( QgsGrassModuleInput ) )
    {
      QgsGrassModuleInput *item =
        dynamic_cast<QgsGrassModuleInput *>( mItems[i] );

      if ( item->useRegion() )
        return true;
    }

    /* It only make sense to check input, right?
     * Output has no region yet */
    if ( typeid( *( mItems[i] ) ) == typeid( QgsGrassModuleOption ) )
    {
      QgsGrassModuleOption *item =
        dynamic_cast<QgsGrassModuleOption *>( mItems[i] );

      if ( item->usesRegion() )
        return true;
    }
  }

  QgsDebugMsg( "NO usesRegion()" );
  return false;
}

QgsGrassModuleStandardOptions::~QgsGrassModuleStandardOptions()
{
}

QString QgsGrassModule::label( QString path )
{
  QgsDebugMsg( "called." );

  // Open QGIS module description
  path.append( ".qgm" );
  QFile qFile( path );
  if ( !qFile.exists() )
  {
    return QString( tr( "Not available, description not found (" ) + path + tr( ")" ) );
  }
  if ( ! qFile.open( QIODevice::ReadOnly ) )
  {
    return QString( tr( "Not available, cannot open description (" ) + path + tr( ")" ) ) ;
  }
  QDomDocument qDoc( "qgisgrassmodule" );
  QString err;
  int line, column;
  if ( !qDoc.setContent( &qFile,  &err, &line, &column ) )
  {
    QString errmsg = tr( "Cannot read module file (" ) + path + tr( "):\n" ) + err + tr( "\nat line " )
                     + QString::number( line ) + tr( " column " ) + QString::number( column );
    QgsDebugMsg( errmsg );
    QMessageBox::warning( 0, tr( "Warning" ), errmsg );
    qFile.close();
    return QString( tr( "Not available, incorrect description (" ) + path + tr( ")" ) );
  }
  qFile.close();
  QDomElement qDocElem = qDoc.documentElement();

  return ( qDocElem.attribute( "label" ) );
}

QPixmap QgsGrassModule::pixmap( QString path, int height )
{
  QgsDebugMsg( "called." );

  std::vector<QPixmap> pixmaps;

  // Create vector of available pictures
  int cnt = 1;
  while ( 1 )
  {
    // SVG
    QString fpath = path + "." + QString::number( cnt ) + ".svg";
    QFileInfo fi( fpath );
    if ( fi.exists() )
    {
      Q3Picture pic;
      if ( ! pic.load( fpath, "svg" ) ) break;

      QRect br = pic.boundingRect();

      double scale = 1. * height / br.height();

      int width = ( int )( scale * br.width() );
      if ( width <= 0 ) width = height; // should not happen
      QPixmap pixmap( width, height );
      pixmap.fill( QColor( 255, 255, 255 ) );
      QPainter painter( &pixmap );
      painter.setRenderHint( QPainter::Antialiasing );

      // I am not sure if this factor is OK
      scale *= 72.0 / pixmap.logicalDpiX() ;
      painter.scale( scale, scale );

      painter.drawPicture( -br.x(), -br.y(), pic );
      painter.end();

      pixmaps.push_back( pixmap );
    }
    else // PNG
    {
      fpath = path + "." + QString::number( cnt ) + ".png";
      fi.setFile( fpath );

      if ( !fi.exists() ) break;

      QPixmap pixmap;

      if ( ! pixmap.load( fpath, "PNG" ) ) break;

      double scale = 1. * height / pixmap.height();
      int width = ( int )( scale * pixmap.width() );

      QImage img = pixmap.convertToImage();
      img = img.smoothScale( width, height );
      pixmap.convertFromImage( img );

      pixmaps.push_back( pixmap );
    }
    cnt++;
  }

  // Get total width
  int width = 0;
  for ( unsigned int i = 0; i < pixmaps.size(); i++ )
  {
    width += pixmaps[i].width();
  }

  if ( width <= 0 ) width = height; //should not happen

  int plusWidth = 8; // +
  int arrowWidth = 9; // ->
  int buffer = 10; // buffer around a sign
  if ( pixmaps.size() > 1 ) width += arrowWidth + 2 * buffer; // ->
  if ( pixmaps.size() > 2 ) width += plusWidth + 2 * buffer; // +

  QPixmap pixmap( width, height );
  pixmap.fill( QColor( 255, 255, 255 ) );
  QPainter painter( &pixmap );

  QColor color( 200, 200, 200 );
  painter.setBrush( QBrush( color ) );

  painter.setRenderHint( QPainter::Antialiasing );

  int pos = 0;
  for ( unsigned int i = 0; i < pixmaps.size(); i++ )
  {
    if ( i == 1 && pixmaps.size() == 3 )   // +
    {
      pos += buffer;

      painter.setPen( QPen( color, 3 ) );
      painter.drawLine( pos, height / 2, pos + plusWidth, height / 2 );
      painter.drawLine( pos + plusWidth / 2, height / 2 - plusWidth / 2, pos + plusWidth / 2, height / 2 + plusWidth / 2 );
      pos += buffer + plusWidth;
    }
    if (( i == 1 && pixmaps.size() == 2 ) || ( i == 2 && pixmaps.size() == 3 ) ) // ->
    {
      pos += buffer;
      painter.setPen( QPen( color, 3 ) );
      painter.drawLine( pos, height / 2, pos + arrowWidth - arrowWidth / 2, height / 2 );

      Q3PointArray pa( 3 );
      pa.setPoint( 0, pos + arrowWidth / 2 + 1, height / 2 - arrowWidth / 2 );
      pa.setPoint( 1, pos + arrowWidth, height / 2 );
      pa.setPoint( 2, pos + arrowWidth / 2 + 1, height / 2 + arrowWidth / 2 );
      painter.setPen( QPen( color, 1 ) );
      painter.drawPolygon( pa );

      pos += buffer + arrowWidth;
    }
    painter.drawPixmap( pos, 0, pixmaps[i] );
    pos += pixmaps[i].width();
  }
  painter.end();

  return pixmap;
}

void QgsGrassModule::run()
{
  QgsDebugMsg( "called." );

  if ( mProcess.state() == QProcess::Running )
  {
    mProcess.kill();
    mRunButton->setText( tr( "Run" ) );
  }
  else
  {
    //QString command;
    QStringList arguments;

    //mProcess.clearArguments();
    //mProcess.addArgument( mXName );
    //command = mXName;

    // Check if options are ready
    QStringList readyErrors = mOptions->ready();
    if ( readyErrors.size() > 0 )
    {
      QString err;
      for ( int i = 0; i < readyErrors.size(); i++ )
      {
        err.append( readyErrors.at( i ) + "<br>" );
      }
      QMessageBox::warning( 0, tr( "Warning" ), err );
      return;
    }

    // Check/set region
    struct Cell_head tempWindow;
    bool resetRegion = false;
    if ( mOptions->requestsRegion() )
    {
      if ( !mOptions->inputRegion( &tempWindow, false ) )
      {
        QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot get input region" ) );
        return;
      }
      resetRegion = true;
    }
    else if ( mOptions->usesRegion() )
    {
      QStringList outsideRegion = mOptions->checkRegion();
      if ( outsideRegion.size() > 0 )
      {
        QMessageBox questionBox( QMessageBox::Question, "Warning",
                                 "Input " + outsideRegion.join( "," ) + " outside current region!",
                                 QMessageBox::Ok | QMessageBox::Cancel );
        QPushButton *resetButton = NULL;
        if ( QgsGrass::versionMajor() > 6 || ( QgsGrass::versionMajor() == 6 && QgsGrass::versionMinor() >= 1 ) )
        {
          resetButton = questionBox.addButton( tr( "Use Input Region" ), QMessageBox::DestructiveRole );
        }
        questionBox.exec();
        QAbstractButton *clicked = questionBox.clickedButton();
        if ( clicked == questionBox.button( QMessageBox::Cancel ) ) return;
        if ( clicked == resetButton ) resetRegion = true;

        if ( resetRegion )
        {
          if ( !mOptions->inputRegion( &tempWindow, true ) )
          {
            QMessageBox::warning( 0, tr( "Warning" ),
                                  tr( "Cannot get input region" ) );
            return;
          }
        }
      }
    }

    // Check if output exists
    QStringList outputExists = mOptions->checkOutput();
    if ( outputExists.size() > 0 )
    {
      QMessageBox::StandardButton ret = QMessageBox::question( 0, "Warning",
                                        "Output " + outputExists.join( "," )
                                        + " exists! Overwrite?",
                                        QMessageBox::Ok | QMessageBox::Cancel );

      if ( ret == QMessageBox::Cancel ) return;

      // r.mapcalc does not use standard parser
      if ( typeid( *mOptions ) != typeid( QgsGrassMapcalc ) )
      {
        arguments.append( "--o" );
        //mProcess.addArgument( "--o" );
        //command.append ( " --o" );
      }
    }

    // Remember output maps
    mOutputVector = mOptions->output( QgsGrassModuleOption::Vector );
    QgsDebugMsg( QString( "mOutputVector.size() = %1" ).arg( mOutputVector.size() ) );
    mOutputRaster = mOptions->output( QgsGrassModuleOption::Raster );
    QgsDebugMsg( QString( "mOutputRaster.size() = %1" ).arg( mOutputRaster.size() ) );
    mSuccess = false;
    mViewButton->setEnabled( false );

    QStringList list = mOptions->arguments();

    QStringList argumentsHtml;
    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
    {
      QgsDebugMsg( "option: " + ( *it ) );
      //command.append ( " " + *it );
      arguments.append( *it );
      //mProcess.addArgument( *it );

      // Quote options with special characters so that user
      // can copy-paste-run the command
      if (( *it ).contains( QRegExp( "[ <>\\$|;&]" ) ) )
      {
        argumentsHtml.append( "'" + *it + "'" );
      }
      else
      {
        argumentsHtml.append( *it );
      }
    }

    /* WARNING - TODO: there was a bug in GRASS 6.0.0 / 6.1.CVS (< 2005-04-29):
    * db_start_driver set GISRC_MODE_MEMORY eviroment variable to 1 if
    * G_get_gisrc_mode() == G_GISRC_MODE_MEMORY but the variable wasn't unset
    * if  G_get_gisrc_mode() == G_GISRC_MODE_FILE. Because QGIS GRASS provider starts drivers in
    * G_GISRC_MODE_MEMORY mode, the variable remains set in variable when a module is run
    * -> unset GISRC_MODE_MEMORY. Remove later once 6.1.x / 6.0.1 is widespread.
    */
    putenv(( char* ) "GISRC_MODE_MEMORY" );  // unset

    mOutputTextBrowser->clear();

    QString commandHtml = mXName + " " + argumentsHtml.join( " " );

    QgsDebugMsg( "command: " + commandHtml );
    commandHtml.replace( "&", "&amp;" );
    commandHtml.replace( "<", "&lt;" );
    commandHtml.replace( ">", "&gt;" );
    mOutputTextBrowser->append( "<B>" +  commandHtml + "</B>" );

    // Warning: it is not useful to write requested region to WIND file and
    //          reset then to original beacuse it is reset before
    //          the region is read by a module even if waitForStarted() is used
    //          -> necessary to pass region as enviroment variable
    //             but the feature is available in GRASS 6.1 only since 23.3.2006

    QStringList environment = QProcess::systemEnvironment();
    if ( resetRegion )
    {
      QString reg = QgsGrass::regionString( &tempWindow );
      QgsDebugMsg( "reg: " + reg );
      environment.append( "GRASS_REGION=" + reg );
    }

    // I was not able to get scripts working on Windows
    // via QProcess and sh.exe (MinGW). g.parser runs well
    // and it sets parameters correctly as enviroment variables
    // but it fails (without error) to re-run the script with
    // execlp(). And I could not figure out why it fails.
    // Because of this problem we simulate here what g.parser
    // normaly does and that way we can avoid it.

    QStringList execArguments = QgsGrassModule::execArguments( mXName );

    if ( execArguments.size() == 0 )
    {
      QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot find module " )
                            + mXName );
      return;
    }

#if defined(WIN32)
    // we already know it exists from execArguments()
    QString exe = QgsGrassModule::findExec( mXName );
    QFileInfo fi( exe );
    if ( !fi.isExecutable() )
    {
      QStringList usedFlagNames;

      // Set enviroment variables
      for ( int i = 0; i < arguments.size(); i++ )
      {
        QString arg = arguments.at( i );
        QString env;
        if ( arg.at( 0 ) == '-' ) //flag
        {
          env = "GIS_FLAG_" + QString( arg.at( 1 ).toUpper() )
                + "=1";
          usedFlagNames.append( arg.at( 1 ) );
        }
        else // option
        {
          QStringList opt = arg.split( "=" );
          env = "GIS_OPT_" + opt.takeFirst().toUpper();
          env += "=" + opt.join( "=" ); // rejoin rest
        }
        QgsDebugMsg( "set: " + env );
        environment.append( env );
      }

      // Set remaining flags
      QStringList allFlagNames = mOptions->flagNames();
      for ( int i = 0; i < allFlagNames.size(); i++ )
      {
        bool used = false;
        for ( int j = 0; j < usedFlagNames.size(); j++ )
        {
          if ( usedFlagNames.at( j ) == allFlagNames.at( i ) )
          {
            used = true;
            break;
          }
        }
        if ( used ) continue;
        QString env = "GIS_FLAG_"
                      + QString( allFlagNames.at( i ).toUpper() )
                      + "=0";
        QgsDebugMsg( "set: " + env );
        environment.append( env );
      }

      arguments.clear();
      arguments.append( "@ARGS_PARSED@" );
    }
#endif

    QString cmd = execArguments.takeFirst();
    execArguments += arguments;

    // Freeze output vector on Windows
    mOptions->freezeOutput();

    mProcess.setEnvironment( environment );
    mProcess.start( cmd, execArguments );

    mProcess.waitForStarted();
    if ( mProcess.state() != QProcess::Running )
    {
      QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot start module: " )
                            + mProcess.errorString() );
      return;
    }

    mTabWidget->setCurrentPage( 1 );
    mRunButton->setText( tr( "Stop" ) );
  }
}

void QgsGrassModule::finished( int exitCode, QProcess::ExitStatus exitStatus )
{
  QgsDebugMsg( "called." );

  QgsDebugMsg( QString( "exitCode = %1" ).arg( exitCode ) );
  if ( exitStatus == QProcess::NormalExit )
  {
    if ( exitCode == 0 )
    {
      mOutputTextBrowser->append( tr( "<B>Successfully finished</B>" ) );
      mProgressBar->setProgress( 100, 100 );
      mSuccess = true;
      mViewButton->setEnabled( true );
      mOptions->thawOutput();
    }
    else
    {
      mOutputTextBrowser->append( tr( "<B>Finished with error</B>" ) );
    }
  }
  else
  {
    mOutputTextBrowser->append( tr( "<B>Module crashed or killed</B>" ) );
  }
  mRunButton->setText( tr( "Run" ) );
}

void QgsGrassModule::readStdout()
{
  QgsDebugMsg( "called." );

  QString line;
  QRegExp rxpercent( "GRASS_INFO_PERCENT: (\\d+)" );

  mProcess.setReadChannel( QProcess::StandardOutput );
  while ( mProcess.canReadLine() )
  {
    //line = QString::fromLocal8Bit( mProcess.readLineStdout().ascii() );
    QByteArray ba = mProcess.readLine();
    line = QString::fromLocal8Bit( QString( ba ).ascii() );

    // GRASS_INFO_PERCENT is catched here only because of bugs in GRASS,
    // normaly it should be printed to stderr
    if ( rxpercent.search( line ) != -1 )
    {
      int progress = rxpercent.cap( 1 ).toInt();
      mProgressBar->setProgress( progress, 100 );
    }
    else
    {
      mOutputTextBrowser->append( line );
    }
  }
}

void QgsGrassModule::readStderr()
{
  QgsDebugMsg( "called." );

  mProcess.setReadChannel( QProcess::StandardError );

  QString line;
  QRegExp rxpercent( "GRASS_INFO_PERCENT: (\\d+)" );
  QRegExp rxmessage( "GRASS_INFO_MESSAGE\\(\\d+,\\d+\\): (.*)" );
  QRegExp rxwarning( "GRASS_INFO_WARNING\\(\\d+,\\d+\\): (.*)" );
  QRegExp rxerror( "GRASS_INFO_ERROR\\(\\d+,\\d+\\): (.*)" );
  QRegExp rxend( "GRASS_INFO_END\\(\\d+,\\d+\\)" );


  while ( mProcess.canReadLine() )
  {
    //line = QString::fromLocal8Bit( mProcess.readLineStderr().ascii() );
    QByteArray ba = mProcess.readLine();
    line = QString::fromLocal8Bit( QString( ba ).ascii() );
    //QgsDebugMsg(QString("line: '%1'").arg(line));

    if ( rxpercent.search( line ) != -1 )
    {
      int progress = rxpercent.cap( 1 ).toInt();
      mProgressBar->setProgress( progress, 100 );
    }
    else if ( rxmessage.search( line ) != -1 )
    {
      mOutputTextBrowser->append( rxmessage.cap( 1 ) );
    }
    else if ( rxwarning.search( line ) != -1 )
    {
      QString warn = rxwarning.cap( 1 );
      QString img = QgsApplication::pkgDataPath() + "/themes/default/grass/grass_module_warning.png";
      mOutputTextBrowser->append( "<img src=\"" + img + "\">" + warn );
    }
    else if ( rxerror.search( line ) != -1 )
    {
      QString error = rxerror.cap( 1 );
      QString img = QgsApplication::pkgDataPath() + "/themes/default/grass/grass_module_error.png";
      mOutputTextBrowser->append( "<img src=\"" + img + "\">" + error );
    }
    else if ( rxend.search( line ) != -1 )
    {
      // Do nothing
    }
    else
    {
      mOutputTextBrowser->append( line + "\n" );
    }
  }
}

void QgsGrassModule::close()
{
  delete this;
}

void QgsGrassModule::viewOutput()
{
  QgsDebugMsg( "called." );

  if ( !mSuccess ) return;

  for ( int i = 0; i < mOutputVector.size(); i++ )
  {
    QString map = mOutputVector.at( i );

    QStringList layers = QgsGrassSelect::vectorLayers(
                           QgsGrass::getDefaultGisdbase(),
                           QgsGrass::getDefaultLocation(),
                           QgsGrass::getDefaultMapset(), map );

    // check whether there are 1_* layers
    // if so, 0_* layers won't be added
    bool onlyLayer1 = false;
    for ( int j = 0; j < layers.count(); j++ )
    {
      if ( layers[j].left( 1 ) == "1" )
      {
        onlyLayer1 = true;
        break;
      }
    }

    // TODO common method for add all layers
    for ( int j = 0; j < layers.count(); j++ )
    {
      QString uri = QgsGrass::getDefaultGisdbase() + "/"
                    + QgsGrass::getDefaultLocation() + "/"
                    + QgsGrass::getDefaultMapset() + "/"
                    + map + "/" + layers[j];

      // skip 0_* layers
      if ( onlyLayer1 && layers[j].left( 1 ) != "1" )
        continue;

      // TODO vector layer name
      mIface->addVectorLayer( uri, layers[j], "grass" );
    }
  }

  for ( int i = 0; i < mOutputRaster.size(); i++ )
  {
    QString map = mOutputRaster.at( i );

    QString uri = QgsGrass::getDefaultGisdbase() + "/"
                  + QgsGrass::getDefaultLocation() + "/"
                  + QgsGrass::getDefaultMapset()
                  + "/cellhd/" + map;

    mIface->addRasterLayer( uri, map );
  }
}

QgisInterface *QgsGrassModule::qgisIface() { return mIface; }

QgsGrassModule::~QgsGrassModule()
{
  QgsDebugMsg( "called." );
  if ( mProcess.state() == QProcess::Running )
  {
    mProcess.kill();
  }
}

QDomNode QgsGrassModule::nodeByKey( QDomElement elem, QString key )
{
  QgsDebugMsg( "called with key=" + key );
  QDomNode n = elem.firstChild();

  while ( !n.isNull() )
  {
    QDomElement e = n.toElement();

    if ( !e.isNull() )
    {
      if ( e.tagName() == "parameter" || e.tagName() == "flag" )
      {
        if ( e.attribute( "name" ) == key )
        {
          return n;
        }
      }
    }
    n = n.nextSibling();
  }

  return QDomNode();
}

/******************* QgsGrassModuleOption ****************************/

QgsGrassModuleOption::QgsGrassModuleOption( QgsGrassModule *module, QString key,
    QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
    QWidget * parent )
    : QGroupBox( parent ),
    QgsGrassModuleItem( module, key, qdesc, gdesc, gnode ),
    mControlType( NoControl ), mValueType( String ), mOutputType( None ), mHaveLimits( false ), mIsOutput( false )
{
  QgsDebugMsg( "called." );
  setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );

  if ( mHidden ) hide();

  mLayout = new QVBoxLayout();

  QString tit;
  if ( mDescription.length() > 40 )
  {
    tit = mDescription.left( 40 ) + " ...";
  }
  else
  {
    tit = mDescription;
  }

  setTitle( " " + tit + " " );

  // Is it output?
  QDomNode promptNode = gnode.namedItem( "gisprompt" );
  if ( !promptNode.isNull() )
  {
    QDomElement promptElem = promptNode.toElement();
    QString element = promptElem.attribute( "element" );
    QString age = promptElem.attribute( "age" );

    if ( age == "new" )
    {
      mOutputElement = element;
      mIsOutput = true;

      if ( element == "vector" )
      {
        mOutputType = Vector;
      }
      else if ( element == "cell" )
      {
        mOutputType = Raster;
      }
    }
  }

  // String without options
  if ( !mHidden )
  {
    QDomElement gelem = gnode.toElement();

    // Predefined values ?
    QDomNode valuesNode = gnode.namedItem( "values" );
    QDomElement valuesElem = valuesNode.toElement(); // null if valuesNode is null

    if ( !valuesNode.isNull() && valuesNode.childNodes().count() > 1 )
    {
      setLayout( mLayout );
      // predefined values -> ComboBox or CheckBox

      // one or many?
      if ( gelem.attribute( "multiple" ) == "yes" )
      {
        mControlType = CheckBoxes;
      }
      else
      {
        mControlType = ComboBox;
        mComboBox = new QComboBox( this );
        mLayout->addWidget( mComboBox );
      }

      // List of values to be excluded
      QStringList exclude = QStringList::split( ',', qdesc.attribute( "exclude" ) );

      QDomNode valueNode = valuesElem.firstChild();

      while ( !valueNode.isNull() )
      {
        QDomElement valueElem = valueNode.toElement();

        if ( !valueElem.isNull() && valueElem.tagName() == "value" )
        {

          QDomNode n = valueNode.namedItem( "name" );
          if ( !n.isNull() )
          {
            QDomElement e = n.toElement();
            QString val = e.text().stripWhiteSpace();

            if ( exclude.contains( val ) == 0 )
            {
              n = valueNode.namedItem( "description" );
              QString desc;
              if ( !n.isNull() )
              {
                e = n.toElement();
                desc = e.text().stripWhiteSpace();
              }
              else
              {
                desc = val;
              }
              desc.replace( 0, 1, desc.left( 1 ).upper() );

              if ( mControlType == ComboBox )
              {
                mComboBox->insertItem( desc );
                if ( mAnswer.length() > 0 && desc == mAnswer )
                {
                  mComboBox->setCurrentIndex( mComboBox->count() - 1 );
                }
              }
              else
              {
                QCheckBox *cb = new QCheckBox( desc, this );
                mCheckBoxes.push_back( cb );
                mLayout->addWidget( cb );
              }

              mValues.push_back( val );
            }
          }
        }

        valueNode = valueNode.nextSibling();
      }
    }
    else // No values
    {
      // Line edit
      mControlType = LineEdit;

      if ( gelem.attribute( "type" ) == "integer" )
      {
        mValueType = Integer;
      }
      else if ( gelem.attribute( "type" ) == "float" )
      {
        mValueType = Double;
      }

      QStringList minMax;
      if ( valuesNode.childNodes().count() == 1 )
      {
        QDomNode valueNode = valuesElem.firstChild();

        QDomNode n = valueNode.namedItem( "name" );
        if ( !n.isNull() )
        {
          QDomElement e = n.toElement();
          QString val = e.text().stripWhiteSpace();
          minMax = val.split( "-" );
          if ( minMax.size() == 2 )
          {
            mHaveLimits = true;
            mMin = minMax.at( 0 ).toDouble();
            mMax = minMax.at( 1 ).toDouble();
          }
        }
      }

      QDomNode keydescNode = gnode.namedItem( "keydesc" );
      if ( !keydescNode.isNull() )
      {
        // fixed number of line edits
        // Example:
        // <keydesc>
        //    <item order="1">rows</item>
        //    <item order="2">columns</item>
        // </keydesc>

        QDomNodeList keydescs = keydescNode.childNodes();
        for ( int k = 0; k < keydescs.count(); k++ )
        {
          QDomNode nodeItem = keydescs.at( k );
          QString itemDesc = nodeItem.toElement().text().stripWhiteSpace();
          //QString itemDesc = nodeItem.firstChild().toText().data();
          QgsDebugMsg( "keydesc item = " + itemDesc );

          addLineEdit();
        }

        setLayout( mLayout );
      }
      else if ( gelem.attribute( "multiple" ) == "yes" )
      {
        // variable number of line edits
        // add/delete buttons for multiple options
        QHBoxLayout *l = new QHBoxLayout( this );
        QVBoxLayout *vl = new QVBoxLayout();
        l->insertLayout( -1, mLayout );
        l->insertLayout( -1, vl );

        // TODO: how to keep both buttons on the top?
        QPushButton *b = new QPushButton( "+", this );
        connect( b, SIGNAL( clicked() ), this, SLOT( addLineEdit() ) );
        vl->addWidget( b, 0, Qt::AlignTop );

        b = new QPushButton( "-", this );
        connect( b, SIGNAL( clicked() ), this, SLOT( removeLineEdit() ) );
        vl->addWidget( b, 0, Qt::AlignTop );

        // Dont enable this, it makes the group box expanding
        // vl->addStretch();
      }
      else
      {
        // only one line edit
        addLineEdit();
        setLayout( mLayout );
      }
    }
  }

  mUsesRegion = false;
  QString region = qdesc.attribute( "region" );
  if ( region.length() > 0 )
  {
    if ( region == "yes" )
      mUsesRegion = true;
  }
  else
  {
    QgsDebugMsg( "\n\n\n\n**************************" );
    QgsDebugMsg( "isOutput = " + isOutput() );
    QgsDebugMsg( "mOutputType = " + mOutputType );
    if ( isOutput() && mOutputType == Raster )
      mUsesRegion = true;
  }
  QgsDebugMsg( "mUsesRegion = " + mUsesRegion );
}

void QgsGrassModuleOption::addLineEdit()
{
  QgsDebugMsg( "called." );

  // TODO make the widget growing with new lines. HOW???!!!
  QLineEdit *lineEdit = new QLineEdit( this );
  mLineEdits.push_back( lineEdit );
  lineEdit->setText( mAnswer );

  if ( mValueType == Integer )
  {
    if ( mHaveLimits )
    {
      mValidator = new QIntValidator(( int )mMin, ( int )mMax, this );
    }
    else
    {
      mValidator = new QIntValidator( this );
    }
    lineEdit->setValidator( mValidator );
  }
  else if ( mValueType == Double )
  {
    if ( mHaveLimits )
    {
      mValidator = new QDoubleValidator( mMin, mMax, 10, this );
    }
    else
    {
      mValidator = new QDoubleValidator( this );
    }
    lineEdit->setValidator( mValidator );
  }
  else if ( mIsOutput )
  {
    QRegExp rx;
    if ( mOutputType == Vector )
    {
      rx.setPattern( "[A-Za-z_][A-Za-z0-9_]+" );
    }
    else
    {
      rx.setPattern( "[A-Za-z0-9_.]+" );
    }
    mValidator = new QRegExpValidator( rx, this );

    lineEdit->setValidator( mValidator );
  }

  mLayout->addWidget( lineEdit );
}

void QgsGrassModuleOption::removeLineEdit()
{
  QgsDebugMsg( "called." );

  if ( mLineEdits.size() < 2 ) return;
  delete mLineEdits.at( mLineEdits.size() - 1 );
  mLineEdits.pop_back();
}

QString QgsGrassModuleOption::outputExists()
{
  QgsDebugMsg( "called." );

  if ( !mIsOutput ) return QString();

  QLineEdit *lineEdit = mLineEdits.at( 0 );
  QString value = lineEdit->text().trimmed();
  QgsDebugMsg( "mKey = " + mKey );
  QgsDebugMsg( "value = " + value );
  QgsDebugMsg( "mOutputElement = " + mOutputElement );

  if ( value.length() == 0 ) return QString();

  QString path = QgsGrass::getDefaultGisdbase() + "/"
                 + QgsGrass::getDefaultLocation() + "/"
                 + QgsGrass::getDefaultMapset() + "/"
                 + mOutputElement + "/" + value;

  QFileInfo fi( path );

  if ( fi.exists() )
  {
    return ( lineEdit->text() );
  }

  return QString();
}

QString QgsGrassModuleOption::value()
{
  QString value;

  if ( mControlType == LineEdit )
  {
    for ( unsigned int i = 0; i < mLineEdits.size(); i++ )
    {
      QLineEdit *lineEdit = mLineEdits.at( i );
      if ( lineEdit->text().trimmed().length() > 0 )
      {
        if ( value.length() > 0 ) value.append( "," );
        value.append( lineEdit->text().trimmed() );
      }
    }
  }
  else if ( mControlType == ComboBox )
  {
    value = mValues[mComboBox->currentItem()];
  }
  else if ( mControlType == CheckBoxes )
  {
    int cnt = 0;
    for ( unsigned int i = 0; i < mCheckBoxes.size(); i++ )
    {
      if ( mCheckBoxes[i]->isChecked() )
      {
        if ( cnt > 0 ) value.append( "," );
        value.append( mValues[i] );
      }
    }
  }
  return value;
}

QStringList QgsGrassModuleOption::options()
{
  QStringList list;

  if ( mHidden )
  {
    list.push_back( mKey + "=" + mAnswer );
  }
  else
  {
    list.push_back( mKey + "=" + value() );
  }
  return list;
}

QString QgsGrassModuleOption::ready()
{
  QgsDebugMsg( "called." );

  QString error;

  if ( mControlType == LineEdit )
  {
    if ( mLineEdits.at( 0 )->text().trimmed().length() == 0 )
    {
      error.append( title() + tr( ":&nbsp;missing value" ) );
    }
  }
  return error;
}

QgsGrassModuleOption::~QgsGrassModuleOption()
{
}

QgsGrassModuleFlag::QgsGrassModuleFlag( QgsGrassModule *module, QString key,
                                        QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
                                        QWidget * parent )
    : QCheckBox( parent ), QgsGrassModuleItem( module, key, qdesc, gdesc, gnode )
{
  QgsDebugMsg( "called." );

  if ( mHidden ) hide();

  if ( mAnswer == "on" )
    setChecked( true );
  else
    setChecked( false );

  setText( mDescription );
}

QStringList QgsGrassModuleFlag::options()
{
  QStringList list;
  if ( isChecked() )
  {
    list.push_back( "-" + mKey );
  }
  return list;
}

QgsGrassModuleFlag::~QgsGrassModuleFlag()
{
}

/************************** QgsGrassModuleInput ***************************/

QgsGrassModuleInput::QgsGrassModuleInput( QgsGrassModule *module,
    QgsGrassModuleStandardOptions *options, QString key,
    QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
    QWidget * parent )
    : QGroupBox( parent ),
    QgsGrassModuleItem( module, key, qdesc, gdesc, gnode ),
    mModuleStandardOptions( options ),
    mVectorTypeOption( 0 ), mVectorLayerOption( 0 ),
    mRegionButton( 0 ), mUpdate( false )
{
  QgsDebugMsg( "called." );
  mVectorTypeMask = GV_POINT | GV_LINE | GV_AREA;

  QString tit;
  if ( mDescription.isEmpty() )
  {
    tit = "Input";
  }
  else
  {
    if ( mDescription.length() > 40 )
    {
      tit = mDescription.left( 40 ) + " ...";
    }
    else
    {
      tit = mDescription;
    }
  }

  setTitle( " " + tit + " " );

  QDomNode promptNode = gnode.namedItem( "gisprompt" );
  QDomElement promptElem = promptNode.toElement();
  QString element = promptElem.attribute( "element" );

  if ( element == "vector" )
  {
    mType = Vector;

    // Read type mask if "typeoption" is defined
    QString opt = qdesc.attribute( "typeoption" );
    if ( ! opt.isNull() )
    {

      QDomNode optNode = QgsGrassModule::nodeByKey( gdesc, opt );

      if ( optNode.isNull() )
      {
        QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot find typeoption " ) +  opt );
      }
      else
      {
        mVectorTypeOption = opt;

        QDomNode valuesNode = optNode.namedItem( "values" );
        if ( valuesNode.isNull() )
        {
          QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot find values for typeoption " ) +  opt );
        }
        else
        {
          mVectorTypeMask = 0; //GV_POINT | GV_LINE | GV_AREA;

          QDomElement valuesElem = valuesNode.toElement();
          QDomNode valueNode = valuesElem.firstChild();

          while ( !valueNode.isNull() )
          {
            QDomElement valueElem = valueNode.toElement();

            if ( !valueElem.isNull() && valueElem.tagName() == "value" )
            {
              QDomNode n = valueNode.namedItem( "name" );
              if ( !n.isNull() )
              {
                QDomElement e = n.toElement();
                QString val = e.text().stripWhiteSpace();

                if ( val == "point" )
                {
                  mVectorTypeMask |= GV_POINT;
                }
                else if ( val == "line" )
                {
                  mVectorTypeMask |= GV_LINE;
                }
                else if ( val == "area" )
                {
                  mVectorTypeMask |= GV_AREA;
                }
              }
            }

            valueNode = valueNode.nextSibling();
          }
        }
      }
    }

    // Read type mask defined in configuration
    opt = qdesc.attribute( "typemask" );
    if ( ! opt.isNull() )
    {
      int mask = 0;

      if ( opt.find( "point" ) >= 0 )
      {
        mask |= GV_POINT;
      }
      if ( opt.find( "line" ) >= 0 )
      {
        mask |= GV_LINE;
      }
      if ( opt.find( "area" ) >= 0 )
      {
        mask |= GV_AREA;
      }

      mVectorTypeMask &= mask;
    }

    // Read "layeroption" if defined
    opt = qdesc.attribute( "layeroption" );
    if ( ! opt.isNull() )
    {

      QDomNode optNode = QgsGrassModule::nodeByKey( gdesc, opt );

      if ( optNode.isNull() )
      {
        QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot find layeroption " ) +  opt );
      }
      else
      {
        mVectorLayerOption = opt;
      }
    }

    // Read "mapid"
    mMapId = qdesc.attribute( "mapid" );
  }
  else if ( element == "cell" )
  {
    mType = Raster;
  }
  else
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "GRASS element " ) + element + tr( " not supported" ) );
  }

  if ( qdesc.attribute( "update" ) == "yes" )
  {
    mUpdate = true;
  }

  QHBoxLayout *l = new QHBoxLayout( this );
  mLayerComboBox = new QComboBox();
  mLayerComboBox->setSizePolicy( QSizePolicy::Expanding,
                                 QSizePolicy:: Preferred );
  l->addWidget( mLayerComboBox );

  QString region = qdesc.attribute( "region" );
  if ( mType == Raster
       && QgsGrass::versionMajor() >= 6 && QgsGrass::versionMinor() >= 1
       && region != "no"
     )
  {

    mRegionButton = new QPushButton(
      QgsGrassPlugin::getThemeIcon( "grass_set_region.png" ), "" );

    mRegionButton->setToolTip( tr( "Use region of this map" ) );
    mRegionButton->setCheckable( true );
    mRegionButton->setSizePolicy( QSizePolicy::Minimum,
                                  QSizePolicy:: Preferred );
    l->addWidget( mRegionButton );
  }

  // Of course, activated(int) is not enough, but there is no signal BEFORE the cobo is opened
  //connect ( mLayerComboBox, SIGNAL( activated(int) ), this, SLOT(updateQgisLayers()) );

  // Connect to canvas
  QgsMapCanvas *canvas = mModule->qgisIface()->getMapCanvas();
  connect( canvas, SIGNAL( layersChanged() ), this, SLOT( updateQgisLayers() ) );

  connect( mLayerComboBox, SIGNAL( activated( int ) ), this, SLOT( changed( int ) ) );

  if ( !mMapId.isEmpty() )
  {
    QgsGrassModuleItem *item = mModuleStandardOptions->item( mMapId );
    if ( item )
    {
      QgsGrassModuleInput *mapInput =
        dynamic_cast<QgsGrassModuleInput *>( item );

      connect( mapInput, SIGNAL( valueChanged() ), this, SLOT( updateQgisLayers() ) );
    }
  }

  mUsesRegion = false;
  if ( region.length() > 0 )
  {
    if ( region == "yes" )
      mUsesRegion = true;
  }
  else
  {
    if ( type() == Raster )
      mUsesRegion = true;
  }

  // Fill in QGIS layers
  updateQgisLayers();
}

bool QgsGrassModuleInput::useRegion()
{
  QgsDebugMsg( "called." );

  if ( mUsesRegion && mType == Raster && mRegionButton &&
       mRegionButton->isChecked() )
  {
    return true;
  }

  return false;
}

void QgsGrassModuleInput::updateQgisLayers()
{
  QgsDebugMsg( "called." );

  QString current = mLayerComboBox->currentText();
  mLayerComboBox->clear();
  mMaps.resize( 0 );
  mVectorTypes.resize( 0 );
  mVectorLayerNames.resize( 0 );
  mMapLayers.resize( 0 );
  mVectorFields.resize( 0 );

  QgsMapCanvas *canvas = mModule->qgisIface()->getMapCanvas();

  // Find map option
  QString sourceMap;
  if ( !mMapId.isEmpty() )
  {
    QgsGrassModuleItem *item = mModuleStandardOptions->item( mMapId );
    if ( item )
    {
      QgsGrassModuleInput *mapInput =
        dynamic_cast<QgsGrassModuleInput *>( item );
      sourceMap = mapInput->currentMap();
    }
  }

  // Note: QDir::cleanPath is using '/' also on Windows
  //QChar sep = QDir::separator();
  QChar sep = '/';

  int nlayers = canvas->layerCount();
  for ( int i = 0; i < nlayers; i++ )
  {
    QgsMapLayer *layer = canvas->getZpos( i );

    QgsDebugMsg( "layer->type() = " + QString::number( layer->type() ) );

    if ( mType == Vector && layer->type() == QgsMapLayer::VECTOR )
    {
      QgsVectorLayer *vector = ( QgsVectorLayer* )layer;
      QgsDebugMsg( "vector->providerType() = " + vector->providerType() );
      if ( vector->providerType() != "grass" ) continue;

      //TODO dynamic_cast ?
      QgsGrassProvider *provider = ( QgsGrassProvider * ) vector->dataProvider();

      // Check type mask
      int geomType = provider->geometryType();

      if (( geomType == QGis::WKBPoint && !( mVectorTypeMask & GV_POINT ) ) ||
          ( geomType == QGis::WKBLineString && !( mVectorTypeMask & GV_LINE ) ) ||
          ( geomType == QGis::WKBPolygon && !( mVectorTypeMask & GV_AREA ) )
         )
      {
        continue;
      }

      // TODO add map() mapset() location() gisbase() to grass provider
      QString source = QDir::cleanPath( provider->dataSourceUri() );

      QgsDebugMsg( "source = " + source );

      // Check GISBASE and LOCATION
      QStringList split = QStringList::split( sep, source );

      if ( split.size() < 4 ) continue;
      split.pop_back(); // layer

      QString map = split.last();
      split.pop_back(); // map

      QString mapset = split.last();
      split.pop_back(); // mapset

      //QDir locDir ( sep + split.join ( QString(sep) ) ) ;
      //QString loc = locDir.canonicalPath();
      QString loc =  source.remove( QRegExp( "/[^/]+/[^/]+/[^/]+$" ) );
      loc = QDir( loc ).canonicalPath();

      QDir curlocDir( QgsGrass::getDefaultGisdbase() + sep + QgsGrass::getDefaultLocation() );
      QString curloc = curlocDir.canonicalPath();

      QgsDebugMsg( "loc = " + loc );
      QgsDebugMsg( "curloc = " + curloc );
      QgsDebugMsg( "mapset = " + mapset );
      QgsDebugMsg( "QgsGrass::getDefaultMapset() = " + QgsGrass::getDefaultMapset() );

      if ( loc != curloc ) continue;

      if ( mUpdate && mapset != QgsGrass::getDefaultMapset() ) continue;

      // Check if it comes from source map if necessary
      if ( !mMapId.isEmpty() )
      {
        QString cm = map + "@" + mapset;
        if ( sourceMap != cm ) continue;
      }

      mMaps.push_back( map + "@" + mapset );

      QString type;
      if ( geomType == QGis::WKBPoint )
      {
        type = "point";
      }
      else if ( geomType == QGis::WKBLineString )
      {
        type = "line";
      }
      else if ( geomType == QGis::WKBPolygon )
      {
        type = "area";
      }
      else
      {
        type = "unknown";
      }

      mVectorTypes.push_back( type );

      QString grassLayer = QString::number( provider->grassLayer() );

      QString label = layer->name() + " ( " + map + "@" + mapset
                      + " " + grassLayer + " " + type + " )";

      mLayerComboBox->insertItem( label );
      if ( label == current ) mLayerComboBox->setCurrentText( current );

      mMapLayers.push_back( vector );
      mVectorLayerNames.push_back( grassLayer );

      // convert from QgsFieldMap to std::vector<QgsField>
      QgsFieldMap flds = vector->dataProvider()->fields();
      std::vector<QgsField> fields;
      for ( QgsFieldMap::iterator it = flds.begin(); it != flds.end(); ++it )
        fields.push_back( it.value() );
      mVectorFields.push_back( fields );
    }
    else if ( mType == Raster && layer->type() == QgsMapLayer::RASTER )
    {
      // Check if it is GRASS raster
      QString source = QDir::cleanDirPath( layer->source() );

      if ( source.contains( "cellhd" ) == 0 ) continue;

      // Most probably GRASS layer, check GISBASE and LOCATION
      QStringList split = QStringList::split( sep, source );

      if ( split.size() < 4 ) continue;

      QString map = split.last();
      split.pop_back(); // map
      if ( split.last() != "cellhd" ) continue;
      split.pop_back(); // cellhd

      QString mapset = split.last();
      split.pop_back(); // mapset

      //QDir locDir ( sep + split.join ( QString(sep) ) ) ;
      //QString loc = locDir.canonicalPath();
      QString loc =  source.remove( QRegExp( "/[^/]+/[^/]+/[^/]+$" ) );
      loc = QDir( loc ).canonicalPath();

      QDir curlocDir( QgsGrass::getDefaultGisdbase() + sep + QgsGrass::getDefaultLocation() );
      QString curloc = curlocDir.canonicalPath();

      if ( loc != curloc ) continue;

      if ( mUpdate && mapset != QgsGrass::getDefaultMapset() ) continue;

      mMaps.push_back( map + "@" + mapset );

      QString label = layer->name() + " ( " + map + "@" + mapset + " )";

      mLayerComboBox->insertItem( label );
      if ( label == current ) mLayerComboBox->setCurrentText( current );
    }
  }
}

QStringList QgsGrassModuleInput::options()
{
  QStringList list;
  QString opt;

  int c = mLayerComboBox->currentItem();
  if ( c < 0 ) // not found
    return list;

  unsigned current = c;

  // TODO: this is hack for network nodes, do it somehow better
  if ( mMapId.isEmpty() )
  {
    opt = mKey + "=";

    if ( current <  mMaps.size() )
    {
      opt.append( mMaps[current] );
    }
    list.push_back( opt );
  }

  if ( !mVectorTypeOption.isNull() && current < mVectorTypes.size() )
  {
    opt = mVectorTypeOption + "=" + mVectorTypes[current] ;
    list.push_back( opt );
  }

  if ( !mVectorLayerOption.isNull() && current < mVectorLayerNames.size() )
  {
    opt = mVectorLayerOption + "=" + mVectorLayerNames[current] ;
    list.push_back( opt );
  }

  return list;
}

std::vector<QgsField> QgsGrassModuleInput::currentFields()
{
  QgsDebugMsg( "called." );

  std::vector<QgsField> fields;

  int c = mLayerComboBox->currentItem();
  if ( c < 0 )
    return fields;

  unsigned current = c;

  if ( current >= 0 && current <  mVectorFields.size() )
  {
    fields = mVectorFields[current];
  }

  return fields;
}

QgsMapLayer * QgsGrassModuleInput::currentLayer()
{
  QgsDebugMsg( "called." );

  int c = mLayerComboBox->currentItem();
  if ( c < 0 )
    return 0;

  unsigned int current = c;

  if ( current >= 0 && current <  mMapLayers.size() )
  {
    return mMapLayers[current];
  }

  return 0;
}

QString QgsGrassModuleInput::currentMap()
{
  QgsDebugMsg( "called." );

  int c = mLayerComboBox->currentItem();
  if ( c < 0 )
    return QString();

  unsigned int current = c;

  if ( current >= 0 && current <  mMaps.size() )
  {
    return mMaps[current];
  }

  return QString();
}

void QgsGrassModuleInput::changed( int i )
{
  emit valueChanged();
}

QString QgsGrassModuleInput::ready()
{
  QgsDebugMsg( "called." );

  QString error;

  QgsDebugMsg( QString( "count = %1" ).arg( mLayerComboBox->count() ) );
  if ( mLayerComboBox->count() == 0 )
  {
    error.append( title() + tr( ":&nbsp;no input" ) );
  }
  return error;
}

QgsGrassModuleInput::~QgsGrassModuleInput()
{
}

/********************** QgsGrassModuleItem *************************/

QgsGrassModuleItem::QgsGrassModuleItem( QgsGrassModule *module, QString key,
                                        QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode )
    : mModule( module ),
    mKey( key ),
    mHidden( false )
{
  //mAnswer = qdesc.attribute("answer", "");

  if ( !qdesc.attribute( "answer" ).isNull() )
  {
    mAnswer = qdesc.attribute( "answer" ).trimmed();
  }
  else
  {
    QDomNode n = gnode.namedItem( "default" );
    if ( !n.isNull() )
    {
      QDomElement e = n.toElement();
      mAnswer = e.text().trimmed();
    }
  }

  if ( qdesc.attribute( "hidden" ) == "yes" )
  {
    mHidden = true;
  }

  if ( !qdesc.attribute( "label" ).isEmpty() )
  {
    mDescription = qdesc.attribute( "label" );
  }
  else
  {
    QDomNode n = gnode.namedItem( "description" );
    if ( !n.isNull() )
    {
      QDomElement e = n.toElement();
      mDescription = e.text().stripWhiteSpace();
      mDescription.replace( 0, 1, mDescription.left( 1 ).upper() );
    }
  }

  mId = qdesc.attribute( "id" );
}

bool QgsGrassModuleItem::hidden() { return mHidden; }

QStringList QgsGrassModuleItem::options() { return QStringList(); }

QgsGrassModuleItem::~QgsGrassModuleItem() {}

/***************** QgsGrassModuleGdalInput *********************/

QgsGrassModuleGdalInput::QgsGrassModuleGdalInput(
  QgsGrassModule *module, int type, QString key, QDomElement &qdesc,
  QDomElement &gdesc, QDomNode &gnode, QWidget * parent )
    : Q3GroupBox( 1, Qt::Vertical, parent ),
    QgsGrassModuleItem( module, key, qdesc, gdesc, gnode ),
    mType( type ), mOgrLayerOption( 0 ), mOgrWhereOption( 0 )
{
  QString tit;
  if ( mDescription.isEmpty() )
  {
    tit = "OGR/PostGIS/GDAL Input";
  }
  else
  {
    if ( mDescription.length() > 40 )
    {
      tit = mDescription.left( 40 ) + " ...";
    }
    else
    {
      tit = mDescription;
    }
  }

  setTitle( " " + tit + " " );

  QDomNode promptNode = gnode.namedItem( "gisprompt" );
  QDomElement promptElem = promptNode.toElement();
  QString element = promptElem.attribute( "element" );

  // Read "layeroption" is defined
  QString opt = qdesc.attribute( "layeroption" );
  if ( ! opt.isNull() )
  {

    QDomNode optNode = QgsGrassModule::nodeByKey( gdesc, opt );

    if ( optNode.isNull() )
    {
      QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot find layeroption " ) +  opt );
    }
    else
    {
      mOgrLayerOption = opt;
    }
  }

  // Read "whereoption" if defined
  opt = qdesc.attribute( "whereoption" );
  if ( !opt.isNull() )
  {
    QDomNode optNode = QgsGrassModule::nodeByKey( gdesc, opt );
    if ( optNode.isNull() )
    {
      QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot find whereoption " ) + opt );
    }
    else
    {
      mOgrWhereOption = opt;
    }
  }

  mLayerComboBox = new QComboBox( this );

  // Of course, activated(int) is not enough, but there is no signal
  // BEFORE the cobo is opened
  // connect ( mLayerComboBox, SIGNAL( activated(int) ), this, SLOT(updateQgisLayers()) );

  // Connect to canvas
  QgsMapCanvas *canvas = mModule->qgisIface()->getMapCanvas();

  // It seems that addedLayer/removedLayer does not work
  //connect ( canvas, SIGNAL(addedLayer(QgsMapLayer *)), this, SLOT(updateQgisLayers()) );
  //connect ( canvas, SIGNAL(removedLayer(QString)), this, SLOT(updateQgisLayers()) );
  connect( canvas, SIGNAL( layersChanged() ), this, SLOT( updateQgisLayers() ) );

  // Fill in QGIS layers
  updateQgisLayers();
}

void QgsGrassModuleGdalInput::updateQgisLayers()
{
  QgsDebugMsg( "called." );

  QString current = mLayerComboBox->currentText();
  mLayerComboBox->clear();
  mUri.resize( 0 );
  mOgrLayers.resize( 0 );

  QgsMapCanvas *canvas = mModule->qgisIface()->getMapCanvas();

  int nlayers = canvas->layerCount();
  for ( int i = 0; i < nlayers; i++ )
  {
    QgsMapLayer *layer = canvas->getZpos( i );

    if ( mType == Ogr && layer->type() == QgsMapLayer::VECTOR )
    {
      QgsVectorLayer *vector = ( QgsVectorLayer* )layer;
      if ( vector->providerType() != "ogr"
           && vector->providerType() != "postgres" ) continue;

      QgsDataProvider *provider = vector->dataProvider();

      QString uri;
      QString ogrLayer;
      QString ogrWhere;
      if ( vector->providerType() == "postgres" )
      {
        // Construct OGR DSN
        QgsDataSourceURI dsUri( provider->dataSourceUri() );
        uri = "PG:" + dsUri.connInfo();

        if ( dsUri.schema() != "" )
        {
          ogrLayer = dsUri.schema() + ".";
        }

        ogrLayer += dsUri.table();
        ogrWhere = dsUri.sql();
      }
      else
      {
        uri = provider->dataSourceUri();
        ogrLayer = "";
      }

      QgsDebugMsg( "uri = " + uri );
      QgsDebugMsg( "ogrLayer = " + ogrLayer );

      mLayerComboBox->insertItem( layer->name() );
      if ( layer->name() == current ) mLayerComboBox->setCurrentText( current );

      mUri.push_back( uri );

      mOgrLayers.push_back( ogrLayer );
      mOgrWheres.push_back( ogrWhere );
    }
    else if ( mType == Gdal && layer->type() == QgsMapLayer::RASTER )
    {
      QString uri = layer->source();
      mLayerComboBox->insertItem( layer->name() );
      if ( layer->name() == current ) mLayerComboBox->setCurrentText( current );
      mUri.push_back( uri );
    }
  }
}

QStringList QgsGrassModuleGdalInput::options()
{
  QStringList list;

  int c = mLayerComboBox->currentItem();
  if ( c < 0 )
    return list;

  unsigned int current = c;

  QString opt( mKey + "=" );

  if ( current >= 0 && current <  mUri.size() )
  {
    opt.append( mUri[current] );
  }
  list.push_back( opt );

  if ( !mOgrLayerOption.isNull() && mOgrLayers[current].length() > 0 )
  {
    opt = mOgrLayerOption + "=";
    // GDAL 1.4.0 supports schemas (r9998)
#if GDAL_VERSION_NUM >= 1400
    opt += mOgrLayers[current];
#else
    // Handle older versions of gdal gracefully
    // OGR does not support schemas !!!
    if ( current >= 0 && current <  mUri.size() )
    {
      QStringList l = mOgrLayers[current].split( "." );
      opt += l.at( 1 );

      // Currently only PostGIS is using layer
      //  -> layer -> PostGIS -> warning
      if ( mOgrLayers[current].length() > 0 )
      {
        QMessageBox::warning( 0, tr( "Warning" ),
                              tr( "PostGIS driver in OGR does not support schemas!<br>"
                                  "Only the table name will be used.<br>"
                                  "It can result in wrong input if more tables of the same name<br>"
                                  "are present in the database." ) );
      }
    }
#endif //GDAL_VERSION_NUM
    list.push_back( opt );

    if ( !mOgrWhereOption.isNull() && mOgrWheres[current].length() > 0 )
    {
      list.push_back( mOgrWhereOption + "=" + mOgrWheres[current] );
    }
  }

  return list;
}

QString QgsGrassModuleGdalInput::ready()
{
  QgsDebugMsg( "called." );

  QString error;

  QgsDebugMsg( QString( "count = %1" ).arg( mLayerComboBox->count() ) );
  if ( mLayerComboBox->count() == 0 )
  {
    error.append( title() + tr( ":&nbsp;no input" ) );
  }
  return error;
}

QgsGrassModuleGdalInput::~QgsGrassModuleGdalInput()
{
}

/***************** QgsGrassModuleField *********************/

QgsGrassModuleField::QgsGrassModuleField(
  QgsGrassModule *module, QgsGrassModuleStandardOptions *options,
  QString key, QDomElement &qdesc,
  QDomElement &gdesc, QDomNode &gnode, QWidget * parent )
    : Q3GroupBox( 1, Qt::Vertical, parent ),
    QgsGrassModuleItem( module, key, qdesc, gdesc, gnode ),
    mModuleStandardOptions( options ), mLayerInput( 0 )
{
  QString tit;
  if ( mDescription.isEmpty() )
  {
    tit = tr( "Attribute field" );
  }
  else
  {
    if ( mDescription.length() > 40 )
    {
      tit = mDescription.left( 40 ) + " ...";
    }
    else
    {
      tit = mDescription;
    }
  }

  setTitle( " " + tit + " " );

  QDomNode promptNode = gnode.namedItem( "gisprompt" );
  QDomElement promptElem = promptNode.toElement();
  QString element = promptElem.attribute( "element" );

  mLayerId = qdesc.attribute( "layerid" );

  mType = qdesc.attribute( "type" );

  QgsGrassModuleItem *item = mModuleStandardOptions->item( mLayerId );
  // TODO check type
  if ( item )
  {
    mLayerInput = dynamic_cast<QgsGrassModuleInput *>( item );
    connect( mLayerInput, SIGNAL( valueChanged() ), this, SLOT( updateFields() ) );
  }

  mFieldComboBox = new QComboBox( this );

  // Fill in layer current fields
  updateFields();
}

void QgsGrassModuleField::updateFields()
{
  QgsDebugMsg( "called." );

  QString current = mFieldComboBox->currentText();
  mFieldComboBox->clear();

  //QgsMapCanvas *canvas = mModule->qgisIface()->getMapCanvas();

  if ( mLayerInput == 0 ) return;

  std::vector<QgsField> fields = mLayerInput->currentFields();

  for ( unsigned int i = 0; i < fields.size(); i++ )
  {
    if ( mType.contains( fields[i].typeName() ) )
    {
      mFieldComboBox->insertItem( fields[i].name() );
      if ( fields[i].name() == current )
      {
        mFieldComboBox->setCurrentText( current );
      }
    }
  }
}

QStringList QgsGrassModuleField::options()
{
  QStringList list;

  QString opt( mKey + "=" + mFieldComboBox->currentText() );
  list.push_back( opt );

  return list;
}

QgsGrassModuleField::~QgsGrassModuleField()
{
}

/***************** QgsGrassModuleSelection *********************/

QgsGrassModuleSelection::QgsGrassModuleSelection(
  QgsGrassModule *module, QgsGrassModuleStandardOptions *options,
  QString key, QDomElement &qdesc,
  QDomElement &gdesc, QDomNode &gnode, QWidget * parent )
    : Q3GroupBox( 1, Qt::Vertical, parent ),
    QgsGrassModuleItem( module, key, qdesc, gdesc, gnode ),
    mModuleStandardOptions( options ), mLayerInput( 0 ),
    mVectorLayer( 0 )
{
  QString tit;
  if ( mDescription.isEmpty() )
  {
    tit = tr( "Attribute field" );
  }
  else
  {
    if ( mDescription.length() > 40 )
    {
      tit = mDescription.left( 40 ) + " ...";
    }
    else
    {
      tit = mDescription;
    }
  }

  setTitle( " " + tit + " " );

  QDomNode promptNode = gnode.namedItem( "gisprompt" );
  QDomElement promptElem = promptNode.toElement();
  QString element = promptElem.attribute( "element" );

  mLayerId = qdesc.attribute( "layerid" );

  mType = qdesc.attribute( "type" );

  QgsGrassModuleItem *item = mModuleStandardOptions->item( mLayerId );
  // TODO check type
  if ( item )
  {
    mLayerInput = dynamic_cast<QgsGrassModuleInput *>( item );
    connect( mLayerInput, SIGNAL( valueChanged() ), this, SLOT( updateSelection() ) );
  }

  mLineEdit = new QLineEdit( this );

  // Fill in layer current fields
  updateSelection();
}

void QgsGrassModuleSelection::updateSelection()
{
  QgsDebugMsg( "called." );

  mLineEdit->setText( "" );
  //QgsMapCanvas *canvas = mModule->qgisIface()->getMapCanvas();
  if ( mLayerInput == 0 ) return;

  QgsMapLayer *layer = mLayerInput->currentLayer();
  if ( !layer ) return;
  QgsVectorLayer *vector = dynamic_cast<QgsVectorLayer*>( layer );

  QgsGrassProvider *provider = ( QgsGrassProvider * ) vector->dataProvider();
  QgsAttributeList allAttributes = provider->allAttributesList();
  const QgsFeatureIds& selected = vector->selectedFeaturesIds();
  int keyField = provider->keyField();

  if ( keyField < 0 ) return;

  QString cats;
  provider->select( allAttributes, QgsRect(), true );
  QgsFeature feature;

  int i = 0;
  while ( provider->getNextFeature( feature ) )
  {
    if ( !selected.contains( feature.featureId() ) )
      continue;

    QgsAttributeMap attr = feature.attributeMap();
    if ( attr.size() > keyField )
    {
      if ( i > 0 ) cats.append( "," );
      cats.append( attr[keyField].toString() );
      i++;
    }
  }
  if ( mVectorLayer != vector )
  {
    if ( mVectorLayer )
    {
      disconnect( mVectorLayer, SIGNAL( selectionChanged() ), this, SLOT( updateSelection() ) );
    }

    connect( vector, SIGNAL( selectionChanged() ), this, SLOT( updateSelection() ) );
    mVectorLayer = vector;
  }

  mLineEdit->setText( cats );
}

QStringList QgsGrassModuleSelection::options()
{
  QStringList list;

  QString opt( mKey + "=" + mLineEdit->text() );
  list.push_back( opt );

  return list;
}

QgsGrassModuleSelection::~QgsGrassModuleSelection()
{
}

/***************** QgsGrassModuleFile *********************/

QgsGrassModuleFile::QgsGrassModuleFile(
  QgsGrassModule *module,
  QString key, QDomElement &qdesc,
  QDomElement &gdesc, QDomNode &gnode, QWidget * parent )
    : QGroupBox( parent ),
    QgsGrassModuleItem( module, key, qdesc, gdesc, gnode ),
    mType( Old )
{
  QString tit;
  if ( mDescription.isEmpty() )
  {
    tit = tr( "File" );
  }
  else
  {
    if ( mDescription.length() > 40 )
    {
      tit = mDescription.left( 40 ) + " ...";
    }
    else
    {
      tit = mDescription;
    }
  }

  setTitle( " " + tit + " " );

  QDomNode promptNode = gnode.namedItem( "gisprompt" );
  QDomElement promptElem = promptNode.toElement();
  QString element = promptElem.attribute( "element" );

  if ( qdesc.attribute( "type" ).toLower() == "new" )
  {
    mType = New;
  }

  if ( !qdesc.attribute( "filters" ).isNull() )
  {
    mFilters = qdesc.attribute( "filters" ).split( ";;" );

    if ( mFilters.size() > 0 )
    {
      QRegExp rx( ".*\\( *..([^ )]*).*" );
      QString ext;
      if ( rx.search( mFilters.at( 0 ) ) == 0 )
      {
        mSuffix = rx.cap( 1 );
      }
    }
  }

  mFileOption = qdesc.attribute( "fileoption" );

  QHBoxLayout *l = new QHBoxLayout( this );
  mLineEdit = new QLineEdit();
  mBrowseButton = new QPushButton( "..." );
  l->addWidget( mLineEdit );
  l->addWidget( mBrowseButton );

  connect( mBrowseButton, SIGNAL( clicked() ),
           this, SLOT( browse() ) );
}

QStringList QgsGrassModuleFile::options()
{
  QStringList list;
  QString path = mLineEdit->text().trimmed();

  if ( mFileOption.isNull() )
  {
    QString opt( mKey + "=" + path );
    list.push_back( opt );
  }
  else
  {
    QFileInfo fi( path );

    QString opt( mKey + "=" + fi.dirPath() );
    list.push_back( opt );

    opt = mFileOption + "=" + fi.baseName();
    list.push_back( opt );
  }

  return list;
}

void QgsGrassModuleFile::browse()
{
  // TODO: unfortunately QFileDialog does not support 'new' directory
  QFileDialog *fd = new QFileDialog( this, NULL, mLineEdit->text() );

  fd->setDirectory( QDir::current() );

  fd->setMode( QFileDialog::AnyFile );

  if ( mType == New )
  {
    fd->setAcceptMode( QFileDialog::AcceptSave );
  }
  else
  {
    fd->setAcceptMode( QFileDialog::AcceptOpen );
  }

  if ( mFilters.size() > 0 )
  {
    fd->setFilters( mFilters );
  }
  fd->setDefaultSuffix( mSuffix );

  if ( fd->exec() == QDialog::Accepted )
  {
    mLineEdit->setText( fd->selectedFile() );
  }
}

QString QgsGrassModuleFile::ready()
{
  QgsDebugMsg( "called." );

  QString error;
  QString path = mLineEdit->text().trimmed();


  if ( path.length() == 0 )
  {
    error.append( title() + tr( ":&nbsp;missing value" ) );
    return error;
  }

  QFileInfo fi( path );
  if ( !fi.dir().exists() )
  {
    error.append( title() + tr( ":&nbsp;directory does not exist" ) );
  }

  return error;
}

QgsGrassModuleFile::~QgsGrassModuleFile()
{
}
