/***************************************************************************
                          qgsgenericprojectionselector.cpp
                    Set user defined CRS using projection selector widget
                             -------------------
    begin                : May 28, 2004
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsapplication.h"
#include "qgssettings.h"

#include "qgsprojectionselectiondialog.h"
#include "qgshelp.h"
#include <QApplication>
#include "qgsgui.h"
#include <QPushButton>

QgsProjectionSelectionDialog::QgsProjectionSelectionDialog( QWidget *parent,
    Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsProjectionSelectionDialog::showHelp );

  //we will show this only when a message is set
  textEdit->hide();

  //apply selected projection upon double-click on item
  connect( projectionSelector, &QgsProjectionSelectionTreeWidget::projectionDoubleClicked, this, &QgsProjectionSelectionDialog::accept );

  QgsSettings settings;
  mSplitter->restoreState( settings.value( QStringLiteral( "Windows/ProjectionSelectorDialog/splitterState" ) ).toByteArray() );
}

QgsProjectionSelectionDialog::~QgsProjectionSelectionDialog()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/ProjectionSelectorDialog/splitterState" ), mSplitter->saveState() );
}

void QgsProjectionSelectionDialog::setMessage( const QString &message )
{
  textEdit->setHtml( QStringLiteral( "<head><style>%1</style></head><body>%2</body>" ).arg( QgsApplication::reportStyleSheet(),
                     message ) );
  textEdit->show();
}

void QgsProjectionSelectionDialog::showNoCrsForLayerMessage()
{
  setMessage( tr( "This layer appears to have no projection specification." )
              + ' '
              + tr( "By default, this layer will now have its projection set to that of the project, "
                    "but you may override this by selecting a different projection below." ) );
}

void QgsProjectionSelectionDialog::setShowNoProjection( bool show )
{
  projectionSelector->setShowNoProjection( show );
}

bool QgsProjectionSelectionDialog::showNoProjection() const
{
  return projectionSelector->showNoProjection();
}

void QgsProjectionSelectionDialog::setNotSetText( const QString &text )
{
  projectionSelector->setNotSetText( text );
}

void QgsProjectionSelectionDialog::setRequireValidSelection()
{
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( projectionSelector->hasValidSelection() );

  connect( projectionSelector, &QgsProjectionSelectionTreeWidget::hasValidSelectionChanged, this, [ = ]( bool isValid )
  {
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( isValid );
  } );
}

QgsCoordinateReferenceSystem QgsProjectionSelectionDialog::crs() const
{
  return projectionSelector->crs();
}

void QgsProjectionSelectionDialog::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  projectionSelector->setCrs( crs );
}

void QgsProjectionSelectionDialog::setOgcWmsCrsFilter( const QSet<QString> &crsFilter )
{
  projectionSelector->setOgcWmsCrsFilter( crsFilter );
}

void QgsProjectionSelectionDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_projections/working_with_projections.html" ) );
}
