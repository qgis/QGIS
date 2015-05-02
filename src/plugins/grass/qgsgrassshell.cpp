/***************************************************************************
     qgsgrassshell.cpp
     --------------------------------------
    Date                 : Thu Apr 23 08:35:43 CEST 2009
    Copyright            : (C) 2009 by Lorenzo "Il Rugginoso" Masini
    Email                : lorenxo86@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QTabWidget>
#include <QVBoxLayout>
#include <QShortcut>
#include <QKeySequence>

#include "qgslogger.h"
#include "qgsapplication.h"
#include "qtermwidget/qtermwidget.h"
#include "qgsgrass.h"
#include "qgsconfig.h"

#include "qgsgrassutils.h"
#include "qgsgrassshell.h"

extern "C"
{
#include <stdlib.h>
}

QgsGrassShell::QgsGrassShell( QgsGrassTools *tools, QTabWidget *parent, const char *name )
    : QFrame( parent )
    , mTerminal( 0 )
    , mTools( tools )
    , mTabWidget( parent )
{
  Q_UNUSED( name );
  QVBoxLayout *mainLayout = new QVBoxLayout( this );
  QTermWidget *mTerminal = new QTermWidget( 0, this );
  initTerminal( mTerminal );
  QShortcut *pasteShortcut = new QShortcut( QKeySequence( tr( "Ctrl+Shift+V" ) ), mTerminal );
  QShortcut *copyShortcut = new QShortcut( QKeySequence( tr( "Ctrl+Shift+C" ) ), mTerminal );

  mainLayout->addWidget( mTerminal );
  setLayout( mainLayout );

  connect( mTerminal, SIGNAL( finished() ), this, SLOT( closeShell() ) );
  connect( pasteShortcut, SIGNAL( activated() ), mTerminal, SLOT( pasteClipboard() ) );
  connect( copyShortcut, SIGNAL( activated() ), mTerminal, SLOT( copyClipboard() ) );

#if 0
  // TODO: find a better way to manage the lockfile.
  // Locking should not be done here, a mapset is either locked by GRASS if QGIS is started from GRASS or it is created by QgsGrass::openMapset
  mLockFilename = QgsGrass::lockFilePath();
  QFile::remove( mLockFilename + ".qgis" );
  if ( !QFile::rename( mLockFilename, mLockFilename + ".qgis" ) )
  {
    QMessageBox::warning( this, tr( "Warning" ), tr( "Cannot rename the lock file %1" ).arg( mLockFilename ) );
  }
#endif

  mTerminal->setSize( 80, 25 );
  mTerminal->setColorScheme( COLOR_SCHEME_BLACK_ON_WHITE );
  mTerminal->startShellProgram();
  mTerminal->setFocus( Qt::MouseFocusReason );
}

QgsGrassShell::~QgsGrassShell()
{
}

void QgsGrassShell::closeShell()
{
  int index = mTabWidget->indexOf( this );
  mTabWidget->removeTab( index );

#if 0
  // TODO: find a better way to manage the lockfile.
  // No locking should be done here, see above

  if ( !QFile::rename( mLockFilename + ".qgis", mLockFilename ) )
  {
    QMessageBox::warning( this, tr( "Warning" ), tr( "Cannot rename the lock file %1" ).arg( mLockFilename ) );
  }
#endif
  deleteLater();
}

void QgsGrassShell::initTerminal( QTermWidget *terminal )
{
  QStringList env( "" );
  QStringList args( "" );

  // GRASS Init.sh should not be started here, it is either run when GRASS is started if QGIS is run from GRASS shell or everything (set environment variables and lock mapset) is done in QgsGrass::openMapset
  //QString shellProgram = QString( "%1/etc/Init.sh" ).arg( ::getenv( "GISBASE" ) );

  //terminal->setShellProgram( shellProgram );
  env << "TERM=vt100";
  env << "GISRC_MODE_MEMORY";
  // TODO: we should check if these environment variable were set by user before QGIS was started
  env << "GRASS_HTML_BROWSER=" + QgsGrassUtils::htmlBrowserPath() ;
  env << "GRASS_WISH=wish";
  env << "GRASS_TCLSH=tclsh";
  env << "GRASS_PYTHON=python";

  //args << "-text";
  //args << QString( "%1/%2/%3" ).arg( QgsGrass::getDefaultGisdbase() ).arg( QgsGrass::getDefaultLocation() ).arg( QgsGrass::getDefaultMapset() );

  //terminal->setArgs( args );
  terminal->setEnvironment( env );

  // Look & Feel
  terminal->setScrollBarPosition( QTermWidget::ScrollBarRight );
}
