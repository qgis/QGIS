/***************************************************************************
    qgsabstractdatasourcewidget.cpp  -  base class for source selector widgets
                             -------------------
    begin                : 10 July 2017
    original             : (C) 2017 by Alessandro Pasotti
    email                : apasotti at boundlessgeo dot com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsabstractdatasourcewidget.h"
#include <QPushButton>

QgsAbstractDataSourceWidget::QgsAbstractDataSourceWidget( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode ):
  QDialog( parent, fl ),
  mWidgetMode( widgetMode )
{
}

QgsProviderRegistry::WidgetMode QgsAbstractDataSourceWidget::widgetMode() const
{
  return mWidgetMode;
}

const QgsMapCanvas *QgsAbstractDataSourceWidget::mapCanvas() const
{
  return mMapCanvas;
}

void QgsAbstractDataSourceWidget::setupButtons( QDialogButtonBox *buttonBox )
{

  mAddButton = new QPushButton( tr( "&Add" ) );
  mAddButton->setToolTip( tr( "Add selected layers to map" ) );
  mAddButton->setEnabled( false );
  buttonBox->addButton( mAddButton, QDialogButtonBox::ApplyRole );
  connect( mAddButton, &QPushButton::clicked, this, &QgsAbstractDataSourceWidget::addButtonClicked );
  connect( this, &QgsAbstractDataSourceWidget::enableButtons, mAddButton, &QPushButton::setEnabled );

  QPushButton *closeButton = new QPushButton( tr( "&Close" ) );
  closeButton->setToolTip( tr( "Close this dialog without adding any layer" ) );
  buttonBox->addButton( closeButton, QDialogButtonBox::RejectRole );
  connect( closeButton, &QPushButton::clicked, this, &QgsAbstractDataSourceWidget::reject );

}


void QgsAbstractDataSourceWidget::setMapCanvas( const QgsMapCanvas *mapCanvas )
{
  mMapCanvas = mapCanvas;
}

