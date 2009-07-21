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
#include <QPushButton>
#include <QShortcut>
#include <QKeySequence>
#include <QSizePolicy>

#include "qgsgrasstools.h"
#include "qtermwidget/qtermwidget.h"
#include "qgsapplication.h"

#include "qgsgrassshell.h"

extern "C"
{
#include <stdlib.h>
}

QgsGrassShell::QgsGrassShell( QgsGrassTools *tools, QTabWidget *parent, const char *name )
    : QFrame( parent )
{
  mTools = tools;
  mTabWidget = parent;

  QVBoxLayout *mainLayout = new QVBoxLayout( this );
  QTermWidget *mTerminal = new QTermWidget( 0, this );
  initTerminal( mTerminal );
  QPushButton *closeButton = new QPushButton( tr( "Close" ), this );
  QShortcut *pasteShortcut = new QShortcut( QKeySequence( tr( "Ctrl+Shift+V" ) ), mTerminal );
  QShortcut *copyShortcut = new QShortcut( QKeySequence( tr( "Ctrl+Shift+C" ) ), mTerminal );

  mainLayout->addWidget( mTerminal );
  mainLayout->addWidget( closeButton );
  setLayout( mainLayout );

  connect( closeButton, SIGNAL( clicked() ), this, SLOT( closeShell() ) );
  connect( mTerminal, SIGNAL( finished() ), this, SLOT( closeShell() ) );
  connect( pasteShortcut, SIGNAL( activated() ), mTerminal, SLOT( pasteClipboard() ) );
  connect( copyShortcut, SIGNAL( activated() ), mTerminal, SLOT( copyClipboard() ) );

  mTerminal->startShellProgram();
  mTerminal->setFocus( Qt::MouseFocusReason );
}

QgsGrassShell::~QgsGrassShell()
{
}


/* TODO: Implement something that resizes the terminal without
 *       crashes.
void QgsGrassShell::resizeTerminal()
{
    //mTerminal->setSize(80, 25);
    //mTerminal->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}
*/

void QgsGrassShell::closeShell()
{
  int index = mTabWidget->indexOf( this );
  mTabWidget->removeTab( index );
  delete this;
}

void QgsGrassShell::initTerminal( QTermWidget *terminal )
{
  QStringList args( "" );
  QStringList env( "" );
  // Set the shell program
  QString shell = ::getenv( "SHELL" );
  if ( shell.isEmpty() || shell.isNull() )
  {
    // if the shell isn't specified use the default one (/bin/bash)
    terminal->setShellProgram( shell );
  }

  // Set shell program arguments
  QFileInfo shellInfo( shell );
  if ( shellInfo.fileName() == "bash" || shellInfo.fileName() == "sh" )
  {
    args << "--norc";
  }
  else if ( shellInfo.fileName() == "tcsh" || shellInfo.fileName() == "csh" )
  {
    args << "-f";
  }
  terminal->setArgs( args );

  // Set shell program enviroment variables
  env << "GRASS_MESSAGE_FORMAT=";
  env << "GRASS_UI_TERM=1";
  env << "GISRC_MODE_MEMORY";
  env << "PS1=GRASS > ";
  env << "TERM=vt100";
  terminal->setEnvironment( env );

  // Look & Feel
  terminal->setScrollBarPosition( QTermWidget::ScrollBarRight );
}
