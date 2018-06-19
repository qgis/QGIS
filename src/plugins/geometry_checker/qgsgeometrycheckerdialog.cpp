/***************************************************************************
 *  qgsgeometrycheckerdialog.cpp                                           *
 *  -------------------                                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometrycheckerdialog.h"
#include "qgsgeometrycheckersetuptab.h"
#include "qgsgeometrycheckerresulttab.h"
#include "qgsgeometrychecker.h"
#include "qgsfeaturepool.h"
#include "qgssettings.h"

#include <QCloseEvent>
#include <QVBoxLayout>

QgsGeometryCheckerDialog::QgsGeometryCheckerDialog( QgisInterface *iface, QWidget *parent )
  : QDialog( parent )
{
  mIface = iface;

  setWindowTitle( tr( "Check Geometries" ) );

  QgsSettings s;
  restoreGeometry( s.value( QStringLiteral( "/Plugin-GeometryChecker/Window/geometry" ) ).toByteArray() );

  mTabWidget = new QTabWidget();
  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Close | QDialogButtonBox::Help, Qt::Horizontal );

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->addWidget( mTabWidget );
  layout->addWidget( mButtonBox );

  mTabWidget->addTab( new QgsGeometryCheckerSetupTab( iface, this ), tr( "Setup" ) );
  mTabWidget->addTab( new QWidget(), tr( "Result" ) );
  mTabWidget->setTabEnabled( 1, false );
  resize( 640, 640 );

  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsGeometryCheckerDialog::showHelp );
  connect( dynamic_cast< QgsGeometryCheckerSetupTab * >( mTabWidget->widget( 0 ) ), &QgsGeometryCheckerSetupTab::checkerStarted, this, &QgsGeometryCheckerDialog::onCheckerStarted );
  connect( dynamic_cast< QgsGeometryCheckerSetupTab * >( mTabWidget->widget( 0 ) ), &QgsGeometryCheckerSetupTab::checkerFinished, this, &QgsGeometryCheckerDialog::onCheckerFinished );
}

QgsGeometryCheckerDialog::~QgsGeometryCheckerDialog()
{
  QgsSettings s;
  s.setValue( QStringLiteral( "/Plugin-GeometryChecker/Window/geometry" ), saveGeometry() );
}

void QgsGeometryCheckerDialog::onCheckerStarted( QgsGeometryChecker *checker )
{
  delete mTabWidget->widget( 1 );
  mTabWidget->removeTab( 1 );
  mTabWidget->addTab( new QgsGeometryCheckerResultTab( mIface, checker, mTabWidget ), tr( "Result" ) );
  mTabWidget->setTabEnabled( 1, false );
  mButtonBox->button( QDialogButtonBox::Close )->setEnabled( false );
}

void QgsGeometryCheckerDialog::onCheckerFinished( bool successful )
{
  mButtonBox->button( QDialogButtonBox::Close )->setEnabled( true );
  if ( successful )
  {
    mTabWidget->setTabEnabled( 1, true );
    mTabWidget->setCurrentIndex( 1 );
    static_cast<QgsGeometryCheckerResultTab *>( mTabWidget->widget( 1 ) )->finalize();
  }
}

void QgsGeometryCheckerDialog::done( int r )
{
  QDialog::done( r );
  delete mTabWidget->widget( 1 );
  mTabWidget->removeTab( 1 );
  mTabWidget->addTab( new QWidget(), tr( "Result" ) );
  mTabWidget->setTabEnabled( 1, false );
}

void QgsGeometryCheckerDialog::closeEvent( QCloseEvent *ev )
{
  if ( qobject_cast<QgsGeometryCheckerResultTab *>( mTabWidget->widget( 1 ) ) &&
       !static_cast<QgsGeometryCheckerResultTab *>( mTabWidget->widget( 1 ) )->isCloseable() )
  {
    ev->ignore();
  }
  else
  {
    QDialog::closeEvent( ev );
  }
}

void QgsGeometryCheckerDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "plugins/plugins_geometry_checker.html" ) );
}
