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

#include "qtermwidget/qtermwidget.h"
#include "qgsgrass.h"

#include "qgsgrassshell.h"

extern "C"
{
#include <stdlib.h>
}

QgsGrassShell::QgsGrassShell( QgsGrassTools *tools, QTabWidget *parent, const char *name )
    : QFrame( parent ), mTools(tools), mTabWidget(parent)
{
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

  // TODO: find a better way to manage the lockfile.
  mLockFilename = QgsGrass::lockFilePath();
  QFile::remove(mLockFilename + ".qgis");
  if (!QFile::rename(mLockFilename, mLockFilename + ".qgis"))
  {
      QMessageBox::warning(this, tr("Warning"), tr("Cannot rename the lock file %1").arg(mLockFilename));
  }

  mTerminal->setSize(80, 25);
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

  // TODO: find a better way to manage the lockfile.
  if(!QFile::rename(mLockFilename + ".qgis", mLockFilename))
  {
    QMessageBox::warning(this, tr("Warning"), tr("Cannot rename the lock file %1").arg(mLockFilename));
  }

  this->deleteLater();
}

void QgsGrassShell::initTerminal( QTermWidget *terminal )
{
  QStringList env("");
  QStringList args("");

  QString shellProgram = QString("%1/etc/Init.sh").arg(::getenv("GISBASE"));

  terminal->setShellProgram(shellProgram);
  env << "TERM=vt100";
  env << "GISRC_MODE_MEMORY";

  args << "-text";
  args << QString("%1/%2/%3").arg(QgsGrass::getDefaultGisdbase()).arg(QgsGrass::getDefaultLocation()).arg(QgsGrass::getDefaultMapset());

  terminal->setArgs(args);
  terminal->setEnvironment(env);

  // Look & Feel
  terminal->setScrollBarPosition( QTermWidget::ScrollBarRight );
}
