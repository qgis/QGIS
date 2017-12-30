/***************************************************************************
                             qgsreportorganizerwidget.cpp
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

#include "qgsreportorganizerwidget.h"
#include "qgsreport.h"
#include "qgsreportsectionmodel.h"
#include "qgsreportsectionlayout.h"
#include "qgsreportsectionfieldgroup.h"
#include "qgslayout.h"
#include "qgslayoutdesignerdialog.h"
#include "qgsreportlayoutsectionwidget.h"
#include "qgsreportfieldgroupsectionwidget.h"
#include <QMenu>
#include <QMessageBox>

#ifdef ENABLE_MODELTEST
#include "modeltest.h"
#endif

QgsReportOrganizerWidget::QgsReportOrganizerWidget( QWidget *parent, QgsLayoutDesignerDialog *designer, QgsReport *report )
  : QgsPanelWidget( parent )
  , mReport( report )
  , mDesigner( designer )
{
  setupUi( this );
  setPanelTitle( tr( "Report" ) );

  mSectionModel = new QgsReportSectionModel( mReport, mViewSections );
  mViewSections->setModel( mSectionModel );
  mViewSections->expandAll();

#ifdef ENABLE_MODELTEST
  new ModelTest( mSectionModel, this );
#endif

  QVBoxLayout *vLayout = new QVBoxLayout();
  vLayout->setMargin( 0 );
  vLayout->setSpacing( 0 );
  mSettingsFrame->setLayout( vLayout );

  mViewSections->setEditTriggers( QAbstractItemView::AllEditTriggers );

  QMenu *addMenu = new QMenu( mButtonAddSection );
  QAction *layoutSection = new QAction( tr( "Single section" ), addMenu );
  addMenu->addAction( layoutSection );
  connect( layoutSection, &QAction::triggered, this, &QgsReportOrganizerWidget::addLayoutSection );
  QAction *fieldGroupSection = new QAction( tr( "Field group" ), addMenu );
  addMenu->addAction( fieldGroupSection );
  connect( fieldGroupSection, &QAction::triggered, this, &QgsReportOrganizerWidget::addFieldGroupSection );

  connect( mCheckShowHeader, &QCheckBox::toggled, this, &QgsReportOrganizerWidget::toggleHeader );
  connect( mCheckShowFooter, &QCheckBox::toggled, this, &QgsReportOrganizerWidget::toggleFooter );
  connect( mButtonEditHeader, &QPushButton::clicked, this, &QgsReportOrganizerWidget::editHeader );
  connect( mButtonEditFooter, &QPushButton::clicked, this, &QgsReportOrganizerWidget::editFooter );
  connect( mViewSections->selectionModel(), &QItemSelectionModel::currentChanged, this, &QgsReportOrganizerWidget::selectionChanged );

  mButtonAddSection->setMenu( addMenu );
  connect( mButtonRemoveSection, &QPushButton::clicked, this, &QgsReportOrganizerWidget::removeSection );
}

void QgsReportOrganizerWidget::setMessageBar( QgsMessageBar *bar )
{
  mMessageBar = bar;
}

void QgsReportOrganizerWidget::addLayoutSection()
{
  std::unique_ptr< QgsReportSectionLayout > section = qgis::make_unique< QgsReportSectionLayout >();
  mSectionModel->addSection( mViewSections->currentIndex(), std::move( section ) );
}

void QgsReportOrganizerWidget::addFieldGroupSection()
{
  std::unique_ptr< QgsReportSectionFieldGroup > section = qgis::make_unique< QgsReportSectionFieldGroup >();
  mSectionModel->addSection( mViewSections->currentIndex(), std::move( section ) );
}

void QgsReportOrganizerWidget::removeSection()
{
  QgsAbstractReportSection *section = mSectionModel->sectionForIndex( mViewSections->currentIndex() );
  if ( dynamic_cast< QgsReport * >( section ) )
    return; //report cannot be removed

  int res = QMessageBox::question( this, tr( "Remove Section" ),
                                   tr( "Are you sure you want to remove the report section?" ),
                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::No );
  if ( res == QMessageBox::No )
    return;

  mSectionModel->removeRow( mViewSections->currentIndex().row(), mViewSections->currentIndex().parent() );
}

void QgsReportOrganizerWidget::toggleHeader( bool enabled )
{
  QgsAbstractReportSection *parent = mSectionModel->sectionForIndex( mViewSections->currentIndex() );
  if ( !parent )
    parent = mReport;
  parent->setHeaderEnabled( enabled );
}

void QgsReportOrganizerWidget::toggleFooter( bool enabled )
{
  QgsAbstractReportSection *parent = mSectionModel->sectionForIndex( mViewSections->currentIndex() );
  if ( !parent )
    parent = mReport;
  parent->setFooterEnabled( enabled );
}

void QgsReportOrganizerWidget::editHeader()
{
  QgsAbstractReportSection *parent = mSectionModel->sectionForIndex( mViewSections->currentIndex() );
  if ( !parent )
    parent = mReport;

  if ( !parent->header() )
  {
    std::unique_ptr< QgsLayout > header = qgis::make_unique< QgsLayout >( mReport->layoutProject() );
    header->initializeDefaults();
    parent->setHeader( header.release() );
  }

  mDesigner->setCurrentLayout( parent->header() );
}

void QgsReportOrganizerWidget::editFooter()
{
  QgsAbstractReportSection *parent = mSectionModel->sectionForIndex( mViewSections->currentIndex() );
  if ( !parent )
    parent = mReport;

  if ( !parent->footer() )
  {
    std::unique_ptr< QgsLayout > footer = qgis::make_unique< QgsLayout >( mReport->layoutProject() );
    footer->initializeDefaults();
    parent->setFooter( footer.release() );
  }

  mDesigner->setCurrentLayout( parent->footer() );
}

void QgsReportOrganizerWidget::selectionChanged( const QModelIndex &current, const QModelIndex & )
{
  QgsAbstractReportSection *parent = mSectionModel->sectionForIndex( current );
  if ( !parent )
    parent = mReport;

  whileBlocking( mCheckShowHeader )->setChecked( parent->headerEnabled() );
  whileBlocking( mCheckShowFooter )->setChecked( parent->footerEnabled() );

  delete mConfigWidget;
  if ( QgsReportSectionLayout *section = dynamic_cast< QgsReportSectionLayout * >( parent ) )
  {
    QgsReportLayoutSectionWidget *widget = new QgsReportLayoutSectionWidget( this, mDesigner, section );
    mSettingsFrame->layout()->addWidget( widget );
    mConfigWidget = widget;
  }
  else if ( QgsReportSectionFieldGroup *section = dynamic_cast< QgsReportSectionFieldGroup * >( parent ) )
  {
    QgsReportSectionFieldGroupWidget *widget = new QgsReportSectionFieldGroupWidget( this, mDesigner, section );
    mSettingsFrame->layout()->addWidget( widget );
    mConfigWidget = widget;
  }
  else
  {
    mConfigWidget = nullptr;
  }
}
