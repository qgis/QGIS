/***************************************************************************
                             qgsreportsectionwidget.cpp
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

#include "qgsreportsectionwidget.h"
#include "qgsreport.h"
#include "qgslayout.h"
#include "qgslayoutdesignerdialog.h"
#include "qgsreportorganizerwidget.h"

QgsReportSectionWidget::QgsReportSectionWidget( QgsReportOrganizerWidget *parent, QgsLayoutDesignerDialog *designer, QgsReport *section )
  : QWidget( parent )
  , mOrganizer( parent )
  , mSection( section )
  , mDesigner( designer )
{
  setupUi( this );

  connect( mButtonEditHeader, &QPushButton::clicked, this, &QgsReportSectionWidget::editHeader );
  connect( mButtonEditFooter, &QPushButton::clicked, this, &QgsReportSectionWidget::editFooter );

  mCheckShowHeader->setChecked( section->headerEnabled() );
  mButtonEditHeader->setEnabled( section->headerEnabled() );
  mCheckShowFooter->setChecked( section->footerEnabled() );
  mButtonEditFooter->setEnabled( section->footerEnabled() );

  connect( mCheckShowHeader, &QCheckBox::toggled, this, &QgsReportSectionWidget::toggleHeader );
  connect( mCheckShowFooter, &QCheckBox::toggled, this, &QgsReportSectionWidget::toggleFooter );

  connect( mCheckShowHeader, &QCheckBox::toggled, mButtonEditHeader, &QPushButton::setEnabled );
  connect( mCheckShowFooter, &QCheckBox::toggled, mButtonEditFooter, &QPushButton::setEnabled );
}

void QgsReportSectionWidget::toggleHeader( bool enabled )
{
  mSection->setHeaderEnabled( enabled );
}

void QgsReportSectionWidget::toggleFooter( bool enabled )
{
  mSection->setFooterEnabled( enabled );
}

void QgsReportSectionWidget::editHeader()
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
    mDesigner->setSectionTitle( tr( "Report Header" ) );
    mOrganizer->setEditedSection( mSection );
  }
}

void QgsReportSectionWidget::editFooter()
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
    mDesigner->setSectionTitle( tr( "Report Footer" ) );
    mOrganizer->setEditedSection( mSection );
  }
}

