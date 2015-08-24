/***************************************************************************
                              qgsgrassmodule.cpp
                             -------------------
    begin                : March, 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsgrassmodule.h"
#include "qgsgrassmapcalc.h"
#include "qgsgrassplugin.h"
#include "qgsgrassselect.h"
#include "qgsgrasstools.h"
#include "qgsgrassutils.h"
#include "qgsgrass.h"
#include "qgsconfig.h"

#include "qgisinterface.h"
#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"

#include "qgsfeature.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"

#include <typeinfo>
#include <QDomDocument>
#include <QMessageBox>
#include <QSvgRenderer>

extern "C"
{
#include <grass/glocale.h>
}

//#include <gdal.h>         // to collect version information

//bool QgsGrassModule::mExecPathInited = 0;
//QStringList QgsGrassModule::mExecPath;

QString QgsGrassModule::findExec( QString file )
{
  QgsDebugMsg( "called." );

  // Init mExecPath
  // Windows searches first in current directory
  // TODO verify if/why applicationDirPath() is necessary
#if 0
  if ( !mExecPathInited )
  {
    QString path = getenv( "PATH" );
    QgsDebugMsg( "path = " + path );


#ifdef Q_OS_WIN
    mExecPath = path.split( ";" );
    mExecPath.prepend( QgsGrass::shortPath( QgsApplication::applicationDirPath() ) );
#elif defined(Q_OS_MACX)
    mExecPath = path.split( ":" );
    mExecPath.prepend( QgsApplication::applicationDirPath() + "/bin" );
    mExecPath.prepend( QgsApplication::applicationDirPath() + "/grass/bin" );
#else
    mExecPath = path.split( ":" );
    mExecPath.prepend( QgsApplication::applicationDirPath() );
#endif
    mExecPathInited = true;
  }
#endif

  if ( QFile::exists( file ) )
  {
    return file;  // full path
  }

#ifdef Q_OS_WIN
  // On windows try .bat first
  foreach ( QString path, QgsGrass::grassModulesPaths() )
  {
    QString full = path + "/" + file + ".bat";
    if ( QFile::exists( full ) )
    {
      return full;
    }
  }

  // .exe next
  foreach ( QString path, QgsGrass::grassModulesPaths() )
  {
    QString full = path + "/" + file + ".exe";
    if ( QFile::exists( full ) )
    {
      return full;
    }
  }

  // and then try if it's a script (w/o extension)
#endif

  // Search for module
  foreach ( QString path, QgsGrass::grassModulesPaths() )
  {
    QString full = path + "/" + file;
    if ( QFile::exists( full ) )
    {
      QgsDebugMsg( "found " + full );
      return full;
    }
    else
    {
      QgsDebugMsg( "not found " + full );
    }
  }

  return QString();
}

bool QgsGrassModule::inExecPath( QString file )
{
  return !findExec( file ).isNull();
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

#ifdef Q_OS_WIN
  if ( exe.endsWith( ".py" ) )
  {
    arguments.append( "python" );
  }
#endif

  arguments.append( exe );

  return arguments;
}

QProcessEnvironment QgsGrassModule::processEnvironment( bool direct )
{
  QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();

  QStringList paths = QgsGrass::grassModulesPaths();
  paths += environment.value( "PATH" ).split( QgsGrass::pathSeparator() );
  environment.insert( "PATH", paths.join( QgsGrass::pathSeparator() ) );

  if ( direct )
  {
    // Set path to GRASS gis fake library
    QgsGrassModule::setDirectLibraryPath( environment );
    environment.insert( "QGIS_PREFIX_PATH", QgsApplication::prefixPath() );
    // Window to avoid crash in G__gisinit
    environment.insert( "GRASS_REGION", "west:0;south:0;east:1;north:1;cols:1;rows:1;proj:0;zone:0" );
  }
  return environment;
}

QgsGrassModule::QgsGrassModule( QgsGrassTools *tools, QString moduleName, QgisInterface *iface,
                                bool direct, QWidget *parent, Qt::WindowFlags f )
    : QDialog( parent, f )
    , QgsGrassModuleBase()
    , mOptions( 0 )
    , mSuccess( false )
    , mDirect( direct )
{
  Q_UNUSED( f );
  QgsDebugMsg( "called" );

  setupUi( this );
  lblModuleName->setText( tr( "Module: %1" ).arg( moduleName ) );
  mTools = tools;
  mIface = iface;
  mCanvas = mIface->mapCanvas();
  //mParent = parent;

  /* Read module description and create options */

  // Open QGIS module description
  QString mpath = QgsGrass::modulesConfigDirPath() + "/" + moduleName + ".qgm";
  QgsDebugMsg( QString( "mpath = %1" ).arg( mpath ) );
  QFile qFile( mpath );
  if ( !qFile.exists() )
  {
    mErrors.append( tr( "The module file (%1) not found." ).arg( mpath ) );
    return;
  }
  if ( ! qFile.open( QIODevice::ReadOnly ) )
  {
    mErrors.append( tr( "Cannot open module file (%1)" ).arg( mpath ) );
    return;
  }
  QDomDocument qDoc( "qgisgrassmodule" );
  QString err;
  int line, column;
  if ( !qDoc.setContent( &qFile,  &err, &line, &column ) )
  {
    QString errmsg = tr( "Cannot read module file (%1)" ).arg( mpath )
                     + tr( "\n%1\nat line %2 column %3" ).arg( err ).arg( line ).arg( column );
    QgsDebugMsg( errmsg );
    mErrors.append( errmsg );
    qFile.close();
    return;
  }
  qFile.close();
  QDomElement qDocElem = qDoc.documentElement();

  // Read GRASS module description
  QString xName = qDocElem.attribute( "module" );
  QString xDocName = qDocElem.attribute( "manual" );
  if ( xDocName.isEmpty() )
  {
    xDocName = xName;
  }

  // Binary modules on windows has .exe extension
  // but not all modules have to be binary (can be scripts)
  // => test if the module is in path and if it is not
  // add .exe and test again
#ifdef Q_OS_WIN
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
    mErrors.append( tr( "Module %1 not found" ).arg( xName ) );
    return;
  }
#else
  mXName = xName;
#endif

  QVBoxLayout *layout = new QVBoxLayout( mTabWidget->widget( 0 ) );
  layout->setContentsMargins( 0, 0, 0, 0 );
  if ( xName == "r.mapcalc" )
  {
    mOptions = new QgsGrassMapcalc( mTools, this,
                                    mIface, mTabWidget->widget( 0 ) );
  }
  else
  {
    mOptions = new QgsGrassModuleStandardOptions( mTools, this,
        mIface, mXName, qDocElem, mDirect, mTabWidget->widget( 0 ) );
  }
  layout->addWidget( dynamic_cast<QWidget *>( mOptions ) );

  if ( !mOptions->errors().isEmpty() )
  {
    mErrors.append( mOptions->errors() );
  }

  // Hide display if there is no output
  if ( !mOptions->hasOutput( QgsGrassModuleOption::Vector )
       && !mOptions->hasOutput( QgsGrassModuleOption::Raster ) )
  {
    mViewButton->hide();
  }
  mViewButton->setEnabled( false );

  // Create manual if available
  QString gisBase = getenv( "GISBASE" );
  QString manPath = gisBase + "/docs/html/" + xDocName + ".html";
  QFile manFile( manPath );
  if ( manFile.exists() )
  {
    mManualTextBrowser->setOpenExternalLinks( true );
    mManualTextBrowser->setSource( QUrl::fromLocalFile( manPath ) );
  }
  else
  {
    mManualTextBrowser->clear();
    mManualTextBrowser->textCursor().insertImage( ":/grass/error.png" );
    mManualTextBrowser->insertPlainText( tr( "Cannot find man page %1" ).arg( manPath ) );
    mManualTextBrowser->insertPlainText( tr( "Please ensure you have the GRASS documentation installed." ) );
  }

  connect( &mProcess, SIGNAL( readyReadStandardOutput() ), this, SLOT( readStdout() ) );
  connect( &mProcess, SIGNAL( readyReadStandardError() ), this, SLOT( readStderr() ) );
  connect( &mProcess, SIGNAL( finished( int, QProcess::ExitStatus ) ), this, SLOT( finished( int, QProcess::ExitStatus ) ) );

  const char *env = "GRASS_MESSAGE_FORMAT=gui";
  char *envstr = new char[strlen( env )+1];
  strcpy( envstr, env );
  putenv( envstr );

  mOutputTextBrowser->setReadOnly( true );
}

QgsGrassModule::Description QgsGrassModule::description( QString path )
{
  QgsDebugMsg( "called." );

  // Open QGIS module description
  path.append( ".qgm" );
  QFile qFile( path );
  if ( !qFile.exists() )
  {
    return Description( tr( "Not available, description not found (%1)" ).arg( path ) );
  }
  if ( ! qFile.open( QIODevice::ReadOnly ) )
  {
    return Description( tr( "Not available, cannot open description (%1)" ).arg( path ) );
  }
  QDomDocument qDoc( "qgisgrassmodule" );
  QString err;
  int line, column;
  if ( !qDoc.setContent( &qFile,  &err, &line, &column ) )
  {
    QString errmsg = tr( "Cannot read module file (%1)" ).arg( path )
                     + tr( "\n%1\nat line %2 column %3" ).arg( err ).arg( line ).arg( column );
    QgsDebugMsg( errmsg );
    QMessageBox::warning( 0, tr( "Warning" ), errmsg );
    qFile.close();
    return Description( tr( "Not available, incorrect description (%1)" ).arg( path ) );
  }
  qFile.close();
  QDomElement qDocElem = qDoc.documentElement();

  QString label = QApplication::translate( "grasslabel", qDocElem.attribute( "label" ).trimmed().toUtf8() );
  bool direct = qDocElem.attribute( "direct" ) == "1";
  return Description( label, direct );
}

QString QgsGrassModule::label( QString path )
{
  return description( path ).label;
}

QPixmap QgsGrassModule::pixmap( QString path, int height )
{
  //QgsDebugMsg( QString( "path = %1" ).arg( path ) );

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
      QSvgRenderer pic;
      if ( ! pic.load( fpath ) )
        break;

      QRect br( QPoint( 0, 0 ), pic.defaultSize() );

      double scale = 1. * height / br.height();

      int width = ( int )( scale * br.width() );
      if ( width <= 0 )
        width = height; // should not happen
      QPixmap pixmap( width, height );
      pixmap.fill( Qt::transparent );
      //pixmap.fill( QColor( 255, 255, 255 ) );
      QPainter painter( &pixmap );
      painter.setRenderHint( QPainter::Antialiasing );

      pic.render( &painter );
      painter.end();

      pixmaps.push_back( pixmap );
    }
    else // PNG
    {
      fpath = path + "." + QString::number( cnt ) + ".png";
      fi.setFile( fpath );

      if ( !fi.exists() )
        break;

      QPixmap pixmap;

      if ( ! pixmap.load( fpath, "PNG" ) )
        break;

      double scale = 1. * height / pixmap.height();
      int width = ( int )( scale * pixmap.width() );

      QImage img = pixmap.toImage();
      img = img.scaled( width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
      pixmap = QPixmap::fromImage( img );

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

  if ( width <= 0 )
    width = height; //should not happen

  QString iconsPath = QgsApplication::pkgDataPath() + "/grass/modules/";
  QFileInfo iconsfi( iconsPath );

  int plusWidth = 8;
  int arrowWidth = 9;

  QString arrowPath = iconsPath + "grass_arrow.png";
  QPixmap arrowPixmap;
  iconsfi.setFile( arrowPath );
  if ( iconsfi.exists() && arrowPixmap.load( arrowPath, "PNG" ) )
  {
    double scale = 1. * height / arrowPixmap.height();
    arrowWidth = ( int )( scale * arrowPixmap.width() );

    QImage img = arrowPixmap.toImage();
    img = img.scaled( arrowWidth, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
    arrowPixmap = QPixmap::fromImage( img );
  }
#if 0
  if ( iconsfi.exists() )
  {
    QSvgRenderer pic;
    if ( pic.load( arrowPath ) )
    {
      QRect br( QPoint( 0, 0 ), pic.defaultSize() );

      double scale = 1. * height / br.height();

      arrowWidth = ( int )( scale * br.width() );
      if ( arrowWidth <= 0 )
        arrowWidth = height; // should not happen
      arrowPixmap = QPixmap( arrowWidth, height );
      arrowPixmap.fill( Qt::transparent );
      QPainter painter( &arrowPixmap );
      painter.setRenderHint( QPainter::Antialiasing );

      pic.render( &painter );
      painter.end();
    }
  }
#endif

  QString plusPath = iconsPath + "grass_plus.svg";
  QPixmap plusPixmap;
  iconsfi.setFile( plusPath );
#if 0
  if ( iconsfi.exists() && plusPixmap.load( plusPath, "PNG" ) )
  {
    double scale = 1. * height / plusPixmap.height();
    plusWidth = ( int )( scale * plusPixmap.width() );

    QImage img = plusPixmap.toImage();
    img = img.scaled( plusWidth, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
    plusPixmap = QPixmap::fromImage( img );
  }
#endif
  if ( iconsfi.exists() )
  {
    QSvgRenderer pic;
    if ( pic.load( plusPath ) )
    {
      QRect br( QPoint( 0, 0 ), pic.defaultSize() );

      double scale = 1. * height / br.height();

      plusWidth = ( int )( scale * br.width() );
      if ( plusWidth <= 0 )
        plusWidth = height; // should not happen
      plusPixmap = QPixmap( plusWidth, height );
      plusPixmap.fill( Qt::transparent );
      QPainter painter( &plusPixmap );
      painter.setRenderHint( QPainter::Antialiasing );

      pic.render( &painter );
      painter.end();
    }
  }
  int buffer = height / 3; // buffer around a sign
  if ( pixmaps.size() > 1 )
    width += arrowWidth + 2 * buffer; // ->
  if ( pixmaps.size() > 2 )
    width += plusWidth + 2 * buffer; // +

  QPixmap pixmap( width, height );
  pixmap.fill( Qt::transparent );
  //pixmap.fill( QColor( 255, 255, 255 ) );
  QPainter painter( &pixmap );

  //QColor color( 255, 255, 255 );
  //painter.setBrush( QBrush( color ) );

  painter.setRenderHint( QPainter::Antialiasing );

  int pos = 0;
  for ( unsigned int i = 0; i < pixmaps.size(); i++ )
  {
    if ( i == 1 && pixmaps.size() == 3 )   // +
    {
      pos += buffer;
      painter.drawPixmap( pos, 0, plusPixmap );
      pos += buffer + plusWidth;
    }
    if (( i == 1 && pixmaps.size() == 2 ) || ( i == 2 && pixmaps.size() == 3 ) ) // ->
    {
      pos += buffer;
      painter.drawPixmap( pos, 0, arrowPixmap );
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
    QgsCoordinateReferenceSystem crs;
    if ( mOptions->requestsRegion() ) // direct always
    {
      if ( !mOptions->inputRegion( &tempWindow, crs, false ) )
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
        QMessageBox questionBox( QMessageBox::Question, tr( "Warning" ),
                                 tr( "Input %1 outside current region!" ).arg( outsideRegion.join( "," ) ),
                                 QMessageBox::Ok | QMessageBox::Cancel );
        QPushButton *resetButton = NULL;
        if ( QgsGrass::versionMajor() > 6 || ( QgsGrass::versionMajor() == 6 && QgsGrass::versionMinor() >= 1 ) )
        {
          resetButton = questionBox.addButton( tr( "Use Input Region" ), QMessageBox::DestructiveRole );
        }
        questionBox.exec();
        QAbstractButton *clicked = questionBox.clickedButton();
        if ( clicked == questionBox.button( QMessageBox::Cancel ) )
          return;
        if ( clicked == resetButton )
          resetRegion = true;

        if ( resetRegion )
        {
          if ( !mOptions->inputRegion( &tempWindow, crs, true ) )
          {
            QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot get input region" ) );
            return;
          }
        }
      }
    }

    // In direct mode user is warned by select file dialog
    if ( !mDirect )
    {
      // Check if output exists
      QStringList outputExists = mOptions->checkOutput();
      if ( outputExists.size() > 0 )
      {
        QMessageBox::StandardButton ret = QMessageBox::question( 0, "Warning",
                                          tr( "Output %1 exists! Overwrite?" ).arg( outputExists.join( "," ) ),
                                          QMessageBox::Ok | QMessageBox::Cancel );

        if ( ret == QMessageBox::Cancel )
          return;

        // r.mapcalc does not use standard parser
        if ( typeid( *mOptions ) != typeid( QgsGrassMapcalc ) )
        {
          arguments.append( "--o" );
          //mProcess.addArgument( "--o" );
          //command.append ( " --o" );
        }
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
      if ( it->contains( QRegExp( "[ <>\\$|;&]" ) ) )
      {
        argumentsHtml.append( "\"" + *it + "\"" );
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

    QProcessEnvironment environment = processEnvironment( mDirect );
    environment.insert( "GRASS_HTML_BROWSER", QgsGrassUtils::htmlBrowserPath() );

    // Warning: it is not useful to write requested region to WIND file and
    //          reset then to original beacuse it is reset before
    //          the region is read by a module even if waitForStarted() is used
    //          -> necessary to pass region as environment variable
    //             but the feature is available in GRASS 6.1 only since 23.3.2006
    if ( resetRegion )
    {
      QString reg = QgsGrass::regionString( &tempWindow );
      QgsDebugMsg( "reg: " + reg );
      environment.insert( "GRASS_REGION", reg );
    }

    if ( mDirect )
    {
      QStringList variables;
      setDirectLibraryPath( environment );
#ifdef Q_OS_WIN
      variables << "PATH";
#elif defined(Q_OS_MAC)
      variables << "DYLD_LIBRARY_PATH";
#else
      variables << "LD_LIBRARY_PATH";
#endif
      environment.insert( "QGIS_PREFIX_PATH", QgsApplication::prefixPath() );
      if ( crs.isValid() ) // it should always be valid
      {
        environment.insert( "QGIS_GRASS_CRS", crs.toProj4() );
      }
      // Suppress debug output
      environment.insert( "QGIS_DEBUG", "-1" );

      // Print some important variables
      variables << "QGIS_PREFIX_PATH" << "QGIS_GRASS_CRS" << "GRASS_REGION";
      foreach ( QString v, variables )
      {
        mOutputTextBrowser->append( v + "=" + environment.value( v ) + "<BR>" );
      }
    }

    QString commandHtml = mXName + " " + argumentsHtml.join( " " );

    QgsDebugMsg( "command: " + commandHtml );
    commandHtml.replace( "&", "&amp;" );
    commandHtml.replace( "<", "&lt;" );
    commandHtml.replace( ">", "&gt;" );
    mOutputTextBrowser->append( "<B>" +  commandHtml + "</B>" );

    // I was not able to get scripts working on Windows
    // via QProcess and sh.exe (MinGW). g.parser runs wellQProcessEnvironment::systemE
    // and it sets parameters correctly as environment variables
    // but it fails (without error) to re-run the script with
    // execlp(). And I could not figure out why it fails.
    // Because of this problem we simulate here what g.parser
    // normaly does and that way we can avoid it.

    QStringList execArguments = QgsGrassModule::execArguments( mXName );

    if ( execArguments.size() == 0 )
    {
      QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot find module %1" ).arg( mXName ) );
      return;
    }

#ifdef Q_OS_WIN
    // we already know it exists from execArguments()
    QString exe = QgsGrassModule::findExec( mXName );
    QFileInfo fi( exe );
    if ( !fi.isExecutable() )
    {
      QStringList usedFlagNames;

      // Set environment variables
      for ( int i = 0; i < arguments.size(); i++ )
      {
        QString arg = arguments.at( i );
        //QString env;
        if ( arg.at( 0 ) == '-' ) //flag
        {
          //env = "GIS_FLAG_" + QString( arg.at( 1 ).toUpper() ) + "=1";
          environment.insert( "GIS_FLAG_" + QString( arg.at( 1 ).toUpper() ), "1" );
          usedFlagNames.append( arg.at( 1 ) );
        }
        else // option
        {
          QStringList opt = arg.split( "=" );
          //env = "GIS_OPT_" + opt.takeFirst().toUpper();
          //env += "=" + opt.join( "=" ); // rejoin rest
          environment.insert( "GIS_OPT_" + opt.takeFirst().toUpper(), opt.join( "=" ) );
        }
        //environment.append( env );
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
        if ( used )
          continue;
        //QString env = "GIS_FLAG_"
        //              + QString( allFlagNames.at( i ).toUpper() )
        //              + "=0";
        //QgsDebugMsg( "set: " + env );
        //environment.append( env );
        environment.insert( "GIS_FLAG_" + QString( allFlagNames.at( i ).toUpper() ), "0" );
      }

      arguments.clear();
      arguments.append( "@ARGS_PARSED@" );
    }
#endif

    QString cmd = execArguments.takeFirst();
    execArguments += arguments;

    // Freeze output vector on Windows
    mOptions->freezeOutput();

    mProcess.setProcessEnvironment( environment );
    mProcess.start( cmd, execArguments );
    emit moduleStarted();

    mProcess.waitForStarted();
    if ( mProcess.state() != QProcess::Running )
    {
      QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot start module: %1" ).arg( mProcess.errorString() ) );
      return;
    }

    mTabWidget->setCurrentIndex( 1 );
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
      mProgressBar->setValue( 100 );
      mSuccess = true;
      mViewButton->setEnabled( true );
      mOptions->thawOutput();
      mCanvas->refresh();
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

  emit moduleFinished();
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
    QByteArray ba = mProcess.readLine();
    //line = QString::fromUtf8( ba ).replace( '\n', "" );
    line = QString::fromLocal8Bit( ba ).replace( '\n', "" );
    //QgsDebugMsg(QString("line: '%1'").arg(line));

    // GRASS_INFO_PERCENT is catched here only because of bugs in GRASS,
    // normaly it should be printed to stderr
    if ( rxpercent.indexIn( line ) != -1 )
    {
      int progress = rxpercent.cap( 1 ).toInt();
      mProgressBar->setValue( progress );
    }
    else
    {
      mOutputTextBrowser->append( "<pre>" + line + "</pre>" );
    }
  }
}

void QgsGrassModule::readStderr()
{
  QgsDebugMsg( "called." );

  QString line;
  QRegExp rxpercent( "GRASS_INFO_PERCENT: (\\d+)" );
  QRegExp rxmessage( "GRASS_INFO_MESSAGE\\(\\d+,\\d+\\): (.*)" );
  QRegExp rxwarning( "GRASS_INFO_WARNING\\(\\d+,\\d+\\): (.*)" );
  QRegExp rxerror( "GRASS_INFO_ERROR\\(\\d+,\\d+\\): (.*)" );
  QRegExp rxend( "GRASS_INFO_END\\(\\d+,\\d+\\)" );

  mProcess.setReadChannel( QProcess::StandardError );
  while ( mProcess.canReadLine() )
  {
    QByteArray ba = mProcess.readLine();
    //line = QString::fromUtf8( ba ).replace( '\n', "" );
    line = QString::fromLocal8Bit( ba ).replace( '\n', "" );
    //QgsDebugMsg(QString("line: '%1'").arg(line));

    if ( rxpercent.indexIn( line ) != -1 )
    {
      int progress = rxpercent.cap( 1 ).toInt();
      mProgressBar->setValue( progress );
    }
    else if ( rxmessage.indexIn( line ) != -1 )
    {
      mOutputTextBrowser->append( "<pre>" + rxmessage.cap( 1 ) + "</pre>" );
    }
    else if ( rxwarning.indexIn( line ) != -1 )
    {
      QString warn = rxwarning.cap( 1 );
      QString img = QgsApplication::pkgDataPath() + "/themes/default/grass/grass_module_warning.png";
      mOutputTextBrowser->append( "<img src=\"" + img + "\">" + warn );
    }
    else if ( rxerror.indexIn( line ) != -1 )
    {
      QString error = rxerror.cap( 1 );
      QString img = QgsApplication::pkgDataPath() + "/themes/default/grass/grass_module_error.png";
      mOutputTextBrowser->append( "<img src=\"" + img + "\">" + error );
    }
    else if ( rxend.indexIn( line ) != -1 )
    {
      // Do nothing
    }
    else
    {
      mOutputTextBrowser->append( "<pre>" + line + "</pre>" );
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

  if ( !mSuccess )
    return;

  for ( int i = 0; i < mOutputVector.size(); i++ )
  {
    QString map = mOutputVector.at( i );

    if ( mDirect )
    {
      // TODO, maybe
    }
    else
    {
      QStringList layers;
      try
      {
        layers = QgsGrass::vectorLayers(
                   QgsGrass::getDefaultGisdbase(),
                   QgsGrass::getDefaultLocation(),
                   QgsGrass::getDefaultMapset(), map );
      }
      catch ( QgsGrass::Exception &e )
      {
        QgsDebugMsg( e.what() );
        continue;
      }

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

        QString name = QgsGrassUtils::vectorLayerName(
                         map, layers[j], 1 );

        mIface->addVectorLayer( uri, name, "grass" );
      }
    }
  }

  for ( int i = 0; i < mOutputRaster.size(); i++ )
  {
    QString map = mOutputRaster.at( i );

    if ( mDirect )
    {
      QString baseName = QFileInfo( map ).baseName();
      mIface->addRasterLayer( map, baseName, "gdal" );
    }
    else
    {
      QString uri = QgsGrass::getDefaultGisdbase() + "/"
                    + QgsGrass::getDefaultLocation() + "/"
                    + QgsGrass::getDefaultMapset()
                    + "/cellhd/" + map;

      mIface->addRasterLayer( uri, map, "grassraster" );
    }
  }
}

QgisInterface *QgsGrassModule::qgisIface()
{
  return mIface;
}

QgsGrassModule::~QgsGrassModule()
{
  QgsDebugMsg( "called." );
  if ( mProcess.state() == QProcess::Running )
  {
    mProcess.kill();
  }
}

QString QgsGrassModule::translate( QString msg )
{
  return QString::fromUtf8( G_gettext( "grassmods", msg.trimmed().toUtf8() ) );
}

QString QgsGrassModule::libraryPathVariable()
{
#ifdef Q_OS_WIN
  return "PATH";
#elif defined(Q_OS_MAC)
  return "DYLD_LIBRARY_PATH";
#else
  return "LD_LIBRARY_PATH";
#endif
}

void QgsGrassModule::setDirectLibraryPath( QProcessEnvironment & environment )
{
  QString pathVariable = libraryPathVariable();
  QString separator;
#ifdef Q_OS_WIN
  separator = ";";
#elif defined(Q_OS_MAC)
  separator = ":";
#else
  separator = ":";
#endif
  QString lp = environment.value( pathVariable );
  lp =  QgsApplication::pluginPath() + separator + lp;
  environment.insert( pathVariable, lp );
  QgsDebugMsg( pathVariable + "=" + lp );
}

/******************* QgsGrassModuleOption ****************************/

QgsGrassModuleOption::QgsGrassModuleOption( QgsGrassModule *module, QString key,
    QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
    bool direct, QWidget * parent )
    : QgsGrassModuleGroupBoxItem( module, key, qdesc, gdesc, gnode, direct, parent )
    , mControlType( NoControl )
    , mValueType( String )
    , mOutputType( None )
    , mHaveLimits( false )
    , mMin( INT_MAX )
    , mMax( INT_MIN )
    , mComboBox( 0 )
    , mIsOutput( false )
    , mValidator( 0 )
    , mLayout( 0 )
    , mUsesRegion( false )
{
  QgsDebugMsg( "called." );
  setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );

  if ( mHidden )
    hide();

  mLayout = new QVBoxLayout();

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
      QStringList exclude = qdesc.attribute( "exclude" ).split( ',', QString::SkipEmptyParts );

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
            QString val = e.text().trimmed();

            if ( exclude.contains( val ) == 0 )
            {
              n = valueNode.namedItem( "description" );
              QString desc;
              if ( !n.isNull() )
              {
                e = n.toElement();
                desc = e.text().trimmed();
              }
              else
              {
                desc = val;
              }
              desc.replace( 0, 1, desc.left( 1 ).toUpper() );

              if ( mControlType == ComboBox )
              {
                mComboBox->addItem( desc );
                if ( mAnswer.length() > 0 && desc == mAnswer )
                {
                  mComboBox->setCurrentIndex( mComboBox->count() - 1 );
                }
              }
              else
              {
                QgsGrassModuleCheckBox *cb = new QgsGrassModuleCheckBox( desc, this );
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
          QString val = e.text().trimmed();
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
          QString itemDesc = nodeItem.toElement().text().trimmed();
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

        // Don't enable this, it makes the group box expanding
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
    QgsDebugMsg( QString( "isOutput = %1" ).arg( isOutput() ) );
    QgsDebugMsg( QString( "mOutputType = %1" ).arg( mOutputType ) );
    if ( isOutput() && mOutputType == Raster )
      mUsesRegion = true;
  }
  QgsDebugMsg( QString( "mUsesRegion = %1" ).arg( mUsesRegion ) );
}
