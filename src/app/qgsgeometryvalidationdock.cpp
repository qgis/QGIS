/***************************************************************************
                      qgsgeometryvalidationdock.cpp
                     --------------------------------------
Date                 : 7.9.2018
Copyright            : (C) 2018 by Matthias Kuhn
email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometryvalidationdock.h"
#include "qgsgeometryvalidationmodel.h"

#include <QButtonGroup>

QgsGeometryValidationDock::QgsGeometryValidationDock( const QString &title, QWidget *parent, Qt::WindowFlags flags )
  : QgsDockWidget( title, parent, flags )
{
  setupUi( this );

  connect( mNextButton, &QPushButton::clicked, this, &QgsGeometryValidationDock::gotoNextError );
  connect( mPreviousButton, &QPushButton::clicked, this, &QgsGeometryValidationDock::gotoPreviousError );
  connect( mZoomToProblemButton, &QPushButton::clicked, this, &QgsGeometryValidationDock::zoomToProblem );
  connect( mZoomToFeatureButton, &QPushButton::clicked, this, &QgsGeometryValidationDock::zoomToFeature );

  onCurrentErrorChanged( QModelIndex(), QModelIndex() );
}

QgsGeometryValidationModel *QgsGeometryValidationDock::geometryValidationModel() const
{
  return mGeometryValidationModel;
}

void QgsGeometryValidationDock::setGeometryValidationModel( QgsGeometryValidationModel *geometryValidationModel )
{
  mGeometryValidationModel = geometryValidationModel;
  mErrorListView->setModel( mGeometryValidationModel );

  connect( mErrorListView->selectionModel(), &QItemSelectionModel::currentChanged, this, &QgsGeometryValidationDock::onCurrentErrorChanged );
}

void QgsGeometryValidationDock::gotoNextError()
{
  QItemSelectionModel *selectionModel = mErrorListView->selectionModel();
  selectionModel->setCurrentIndex( mGeometryValidationModel->index( selectionModel->currentIndex().row() + 1, 0, QModelIndex() ), QItemSelectionModel::ClearAndSelect );
}

void QgsGeometryValidationDock::gotoPreviousError()
{
  QItemSelectionModel *selectionModel = mErrorListView->selectionModel();
  selectionModel->setCurrentIndex( mGeometryValidationModel->index( selectionModel->currentIndex().row() - 1, 0, QModelIndex() ), QItemSelectionModel::ClearAndSelect );
}

void QgsGeometryValidationDock::zoomToProblem()
{
  mLastZoomToAction = ZoomToProblem;
}

void QgsGeometryValidationDock::zoomToFeature()
{
  mLastZoomToAction = ZoomToFeature;
  // mErrorListView->currentIndex().data( )
}

void QgsGeometryValidationDock::onCurrentErrorChanged( const QModelIndex &current, const QModelIndex &previous )
{
  mNextButton->setEnabled( current.isValid() && current.row() < mGeometryValidationModel->rowCount() - 1 );
  mPreviousButton->setEnabled( current.isValid() && current.row() > 0 );

  mProblemDetailWidget->setVisible( current.isValid() );
  mProblemDescriptionLabel->setText( current.data().toString() );

  switch ( mLastZoomToAction )
  {
    case  ZoomToProblem:
      zoomToProblem();
      break;

    case ZoomToFeature:
      zoomToFeature();
  }
}
