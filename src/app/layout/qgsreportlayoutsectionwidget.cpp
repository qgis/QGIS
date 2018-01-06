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
  connect( mButtonEditHeader, &QPushButton::clicked, this, &QgsReportLayoutSectionWidget::editHeader );
  connect( mButtonEditFooter, &QPushButton::clicked, this, &QgsReportLayoutSectionWidget::editFooter );

  mCheckShowHeader->setChecked( section->headerEnabled() );
  mCheckShowFooter->setChecked( section->footerEnabled() );
  mCheckShowBody->setChecked( section->bodyEnabled() );

  connect( mCheckShowHeader, &QCheckBox::toggled, this, &QgsReportLayoutSectionWidget::toggleHeader );
  connect( mCheckShowFooter, &QCheckBox::toggled, this, &QgsReportLayoutSectionWidget::toggleFooter );
  connect( mCheckShowBody, &QCheckBox::toggled, this, &QgsReportLayoutSectionWidget::toggleBody );
}

void QgsReportLayoutSectionWidget::toggleHeader( bool enabled )
{
  mSection->setHeaderEnabled( enabled );
}

void QgsReportLayoutSectionWidget::toggleFooter( bool enabled )
{
  mSection->setFooterEnabled( enabled );
}

void QgsReportLayoutSectionWidget::editHeader()
{
  if ( !mSection->header() )
  {
    std::unique_ptr< QgsLayout > header = qgis::make_unique< QgsLayout >( mSection->project() );
    header->initializeDefaults();
    mSection->setHeader( header.release() );
  }

  if ( mSection->header() )
  {
    mDesigner->setCurrentLayout( mSection->header() );
    mDesigner->setSectionTitle( tr( "Header: %1" ).arg( mSection->description() ) );
    mOrganizer->setEditedSection( mSection );
  }
}

void QgsReportLayoutSectionWidget::editFooter()
{
  if ( !mSection->footer() )
  {
    std::unique_ptr< QgsLayout > footer = qgis::make_unique< QgsLayout >( mSection->project() );
    footer->initializeDefaults();
    mSection->setFooter( footer.release() );
  }

  if ( mSection->footer() )
  {
    mDesigner->setCurrentLayout( mSection->footer() );
    mDesigner->setSectionTitle( tr( "Footer: %1" ).arg( mSection->description() ) );
    mOrganizer->setEditedSection( mSection );
  }
}

void QgsReportLayoutSectionWidget::toggleBody( bool enabled )
{
  mSection->setBodyEnabled( enabled );
}

void QgsReportLayoutSectionWidget::editBody()
{
  if ( !mSection->body() )
  {
    std::unique_ptr< QgsLayout > body = qgis::make_unique< QgsLayout >( mSection->project() );
    body->initializeDefaults();
    mSection->setBody( body.release() );
  }

  mDesigner->setCurrentLayout( mSection->body() );
  mDesigner->setSectionTitle( tr( "Body: %1" ).arg( mSection->description() ) );
  mOrganizer->setEditedSection( mSection );
}
