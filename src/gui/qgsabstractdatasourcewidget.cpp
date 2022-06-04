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

QgsBrowserModel *QgsAbstractDataSourceWidget::browserModel()
{
  return mBrowserModel;
}

void QgsAbstractDataSourceWidget::setupButtons( QDialogButtonBox *buttonBox )
{
  buttonBox->setStandardButtons( QDialogButtonBox::Apply | QDialogButtonBox::Close | QDialogButtonBox::Help );
#ifdef Q_OS_MACX
  buttonBox->setStyleSheet( "* { button-layout: 2 }" );
#endif
  mAddButton = buttonBox->button( QDialogButtonBox::Apply );
  mAddButton->setText( tr( "&Add" ) );
  mAddButton->setToolTip( tr( "Add selected layers to map" ) );
  mAddButton->setEnabled( false );
  connect( mAddButton, &QPushButton::clicked, this, &QgsAbstractDataSourceWidget::addButtonClicked );
  connect( this, &QgsAbstractDataSourceWidget::enableButtons, mAddButton, &QPushButton::setEnabled );

  QPushButton *closeButton = buttonBox->button( QDialogButtonBox::Close );
  closeButton->setToolTip( tr( "Close this dialog without adding any layer" ) );
  connect( closeButton, &QPushButton::clicked, this, &QgsAbstractDataSourceWidget::reject );
}

void QgsAbstractDataSourceWidget::setBrowserModel( QgsBrowserModel *model )
{
  mBrowserModel = model;
}

void QgsAbstractDataSourceWidget::addButtonClicked()
{
}

void QgsAbstractDataSourceWidget::reset()
{
}
