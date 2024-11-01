/***************************************************************************
                             qgsreportlayoutsectionwidget.cpp
                             ------------------------
    begin                : December 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsreportlayoutsectionwidget.h"
#include "moc_qgsreportlayoutsectionwidget.cpp"
#include "qgsreportsectionlayout.h"
#include "qgslayout.h"
#include "qgslayoutdesignerdialog.h"
#include "qgsreportorganizerwidget.h"

QgsReportLayoutSectionWidget::QgsReportLayoutSectionWidget( QgsReportOrganizerWidget *parent, QgsLayoutDesignerDialog *designer, QgsReportSectionLayout *section )
  : QWidget( parent )
  , mOrganizer( parent )
  , mSection( section )
  , mDesigner( designer )
{
  setupUi( this );

  connect( mButtonEditBody, &QPushButton::clicked, this, &QgsReportLayoutSectionWidget::editBody );

  mCheckShowBody->setChecked( section->bodyEnabled() );
  mButtonEditBody->setEnabled( section->bodyEnabled() );

  connect( mCheckShowBody, &QCheckBox::toggled, this, &QgsReportLayoutSectionWidget::toggleBody );

  connect( mCheckShowBody, &QCheckBox::toggled, mButtonEditBody, &QPushButton::setEnabled );
}

void QgsReportLayoutSectionWidget::toggleBody( bool enabled )
{
  mSection->setBodyEnabled( enabled );
}

void QgsReportLayoutSectionWidget::editBody()
{
  if ( !mSection->body() )
  {
    std::unique_ptr<QgsLayout> body = std::make_unique<QgsLayout>( mSection->project() );
    body->initializeDefaults();
    mSection->setBody( body.release() );
  }

  mDesigner->setCurrentLayout( mSection->body() );
  mDesigner->setSectionTitle( tr( "Body: %1" ).arg( mSection->description() ) );
  mOrganizer->setEditedSection( mSection );
}
