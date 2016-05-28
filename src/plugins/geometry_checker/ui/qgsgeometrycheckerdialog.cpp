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

#include <QCloseEvent>
#include <QVBoxLayout>
#include <QSettings>

QgsGeometryCheckerDialog::QgsGeometryCheckerDialog( QgisInterface *iface, QWidget *parent )
    : QDialog( parent )
{
  mIface = iface;

  setWindowTitle( tr( "Check Geometries" ) );

  QSettings s;
  restoreGeometry( s.value( "/Plugin-GeometryChecker/Window/geometry" ).toByteArray() );

  mTabWidget = new QTabWidget();
  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Close, Qt::Horizontal );

  QVBoxLayout* layout = new QVBoxLayout( this );
  layout->addWidget( mTabWidget );
  layout->addWidget( mButtonBox );

  mTabWidget->addTab( new QgsGeometryCheckerSetupTab( iface ), tr( "Settings" ) );
  mTabWidget->addTab( new QWidget(), tr( "Result" ) );
  mTabWidget->setTabEnabled( 1, false );

  connect( mButtonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );
  connect( mTabWidget->widget( 0 ), SIGNAL( checkerStarted( QgsGeometryChecker*, QgsFeaturePool* ) ), this, SLOT( onCheckerStarted( QgsGeometryChecker*, QgsFeaturePool* ) ) );
  connect( mTabWidget->widget( 0 ), SIGNAL( checkerFinished( bool ) ), this, SLOT( onCheckerFinished( bool ) ) );
}

QgsGeometryCheckerDialog::~QgsGeometryCheckerDialog()
{
  QSettings s;
  s.setValue( "/Plugin-GeometryChecker/Window/geometry", saveGeometry() );
}

void QgsGeometryCheckerDialog::onCheckerStarted( QgsGeometryChecker *checker, QgsFeaturePool *featurePool )
{
  delete mTabWidget->widget( 1 );
  mTabWidget->removeTab( 1 );
  mTabWidget->addTab( new QgsGeometryCheckerResultTab( mIface, checker, featurePool, mTabWidget ), tr( "Result" ) );
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
    static_cast<QgsGeometryCheckerResultTab*>( mTabWidget->widget( 1 ) )->finalize();
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

void QgsGeometryCheckerDialog::closeEvent( QCloseEvent* ev )
{
  if ( qobject_cast<QgsGeometryCheckerResultTab*>( mTabWidget->widget( 1 ) ) &&
       !static_cast<QgsGeometryCheckerResultTab*>( mTabWidget->widget( 1 ) )->isCloseable() )
  {
    ev->ignore();
  }
  else
  {
    QDialog::closeEvent( ev );
  }
}
