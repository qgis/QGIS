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
}

void QgsProjectionSelectionDialog::setMessage( const QString &message )
{
  QString m = message;
  //short term kludge to make the layer selector default to showing
  //a layer projection selection message. If you want the selector
  if ( m.isEmpty() )
  {
    // Set up text edit pane
    QString format( QStringLiteral( "<h1>%1</h1>%2 %3" ) );
    QString header = tr( "Define this layer's coordinate reference system:" );
    QString sentence1 = tr( "This layer appears to have no projection specification." );
    QString sentence2 = tr( "By default, this layer will now have its projection set to that of the project, "
                            "but you may override this by selecting a different projection below." );
    m = format.arg( header, sentence1, sentence2 );
  }

  QString myStyle = QgsApplication::reportStyleSheet();
  m = "<head><style>" + myStyle + "</style></head><body>" + m + "</body>";
  textEdit->setHtml( m );
  textEdit->show();
}

void QgsProjectionSelectionDialog::setShowNoProjection( bool show )
{
  projectionSelector->setShowNoProjection( show );
}

bool QgsProjectionSelectionDialog::showNoProjection() const
{
  return projectionSelector->showNoProjection();
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
