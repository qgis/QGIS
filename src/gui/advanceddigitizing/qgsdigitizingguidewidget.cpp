/***************************************************************************
    qgsdigitizingguidewidget.cpp
    ----------------------
    begin                : January 2024
    copyright            : (C) Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsdigitizingguidewidget.h"

#include "qgsdigitizingguidelayer.h"
#include "qgsdigitizingguidemodel.h"
#include "qgsdigitizingguidemaptool.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"

#include <QMessageBox>

QgsDigitizingGuideWidget::QgsDigitizingGuideWidget( QgsMapCanvas *canvas, QWidget *parent )
  : QWidget{parent}
  , mCanvas( canvas )
  , mGuideLayer( QgsProject::instance()->digitizingGuideLayer() )
{
  setupUi( this );

  mGuidesTable->setSelectionBehavior( QAbstractItemView::SelectRows );
  mGuidesTable->verticalHeader()->setVisible( false );
  mGuidesTable->horizontalHeader()->setVisible( false );
  mGuidesTable->horizontalHeader()->setStretchLastSection( true );

  mGuidesTable->setModel( mGuideLayer->model() );

  mRemoveGuideButton->setEnabled( false );
  connect( mRemoveGuideButton, &QToolButton::clicked, this, &QgsDigitizingGuideWidget::removeButtonClicked );

  connect( mGuidesTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsDigitizingGuideWidget::guideSelectionChanged );

  mGuidesMapTools.insert( mDistanceToPointsButton, new QgsDigitizingGuideMapToolDistanceToPoints( canvas ) );
  mGuidesMapTools.insert( mLineExtensionButton, new QgsDigitizingGuideMapToolLineExtension( canvas ) );
  mGuidesMapTools.insert( mLineParallelButton, new QgsDigitizingGuideMapToolLineParallel( canvas ) );
  mGuidesMapTools.insert( mLinePerpendicularButton, new QgsDigitizingGuideMapToolLinePerpendicular( canvas ) );

  for ( auto it = mGuidesMapTools.constBegin(); it != mGuidesMapTools.constEnd(); it++ )
  {
    connect( it.key(), &QToolButton::clicked, this, &QgsDigitizingGuideWidget::enableGuideMapTool );
  }

}

void QgsDigitizingGuideWidget::enableGuideMapTool( )
{
  QToolButton *bt = qobject_cast<QToolButton *>( sender() );
  QgsDigitizingGuideMapTool *mapTool = mGuidesMapTools.value( bt, nullptr );

  if ( mapTool )
    mCanvas->setMapTool( mapTool );
}

void QgsDigitizingGuideWidget::guideSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  mGuideLayer->model()->selectionChanged( selected, deselected );

  mRemoveGuideButton->setEnabled( selected.indexes().count() > 0 );
}

void QgsDigitizingGuideWidget::removeButtonClicked()
{
  mGuideLayer->setGuideHighlight();
  QItemSelectionModel *selectionModel = mGuidesTable->selectionModel();
  if ( selectionModel && selectionModel->hasSelection() )
  {
    QMessageBox::StandardButton bt = QMessageBox::question( nullptr, tr( "Digitizing Guides" ), tr( "Remove %1 guide(s) ?" ).arg( selectionModel->selectedRows().count() ) );
    if ( bt == QMessageBox::StandardButton::Yes )
      mGuideLayer->model()->removeGuides( selectionModel->selectedRows() );
  }
}
