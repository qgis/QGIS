/***************************************************************************
    qgsprocessingtoolboxwidget.cpp
    -------------------------------
    begin                : August 2026
    copyright            : (C) 2026 by Valentin Buira
    email                : valentin dot buira at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsprocessingtoolboxwidget.h"

#include "qgis.h"
#include "qgsgui.h"

#include <QVBoxLayout>
#include <qheaderview.h>

#include "moc_qgsprocessingtoolboxwidget.cpp"

QgsProcessingToolboxWidget::QgsProcessingToolboxWidget()
{
  QVBoxLayout *layout = new QVBoxLayout();

  layout->addWidget( mToolboxSearchEdit.get() );
  layout->addWidget( mToolboxTreeView.get() );

  mToolboxTreeView->header()->setVisible( false );
  mToolboxSearchEdit->setShowSearchIcon( true );
  mToolboxSearchEdit->setPlaceholderText( tr( "Search…" ) );
  connect( mToolboxSearchEdit.get(), &QgsFilterLineEdit::textChanged, mToolboxTreeView.get(), &QgsProcessingToolboxTreeView::setFilterString );
}
