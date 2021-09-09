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
#include "qgsreportsectionwidget.h"
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

  mSectionModel = new QgsReportSectionModel( mReport, this );
  mViewSections->setModel( mSectionModel );
  mViewSections->header()->hide();
  mViewSections->expandAll();

#ifdef ENABLE_MODELTEST
  new ModelTest( mSectionModel, this );
#endif

  QVBoxLayout *vLayout = new QVBoxLayout();
  vLayout->setContentsMargins( 0, 0, 0, 0 );
  vLayout->setSpacing( 0 );
  mSettingsFrame->setLayout( vLayout );

  mViewSections->setEditTriggers( QAbstractItemView::AllEditTriggers );

  QMenu *addMenu = new QMenu( mButtonAddSection );
  QAction *layoutSection = new QAction( tr( "Static Layout Section" ), addMenu );
  layoutSection->setToolTip( tr( "A static layout report section which consists of a single layout inserted into the report" ) );
  addMenu->addAction( layoutSection );
  connect( layoutSection, &QAction::triggered, this, &QgsReportOrganizerWidget::addLayoutSection );
  QAction *fieldGroupSection = new QAction( tr( "Field Group Section" ), addMenu );
  fieldGroupSection->setToolTip( tr( "A report section which is repeated for every matching feature within a layer" ) );
  addMenu->addAction( fieldGroupSection );
  connect( fieldGroupSection, &QAction::triggered, this, &QgsReportOrganizerWidget::addFieldGroupSection );

  connect( mViewSections->selectionModel(), &QItemSelectionModel::currentChanged, this, &QgsReportOrganizerWidget::selectionChanged );

  mButtonAddSection->setMenu( addMenu );
  connect( mButtonRemoveSection, &QPushButton::clicked, this, &QgsReportOrganizerWidget::removeSection );
  mButtonRemoveSection->setEnabled( false ); //disable until section clicked

  // initially select report section
  const QModelIndex reportIndex = mSectionModel->indexForSection( report );
  mViewSections->setCurrentIndex( reportIndex );
}

void QgsReportOrganizerWidget::setMessageBar( QgsMessageBar *bar )
{
  mMessageBar = bar;
}

void QgsReportOrganizerWidget::setEditedSection( QgsAbstractReportSection *section )
{
  mSectionModel->setEditedSection( section );
}

void QgsReportOrganizerWidget::addLayoutSection()
{
  std::unique_ptr< QgsReportSectionLayout > section = std::make_unique< QgsReportSectionLayout >();
  QgsAbstractReportSection *newSection = section.get();
  mSectionModel->addSection( mViewSections->currentIndex(), std::move( section ) );
  const QModelIndex newIndex = mSectionModel->indexForSection( newSection );
  mViewSections->setCurrentIndex( newIndex );
}

void QgsReportOrganizerWidget::addFieldGroupSection()
{
  std::unique_ptr< QgsReportSectionFieldGroup > section = std::make_unique< QgsReportSectionFieldGroup >();
  QgsAbstractReportSection *newSection = section.get();
  mSectionModel->addSection( mViewSections->currentIndex(), std::move( section ) );
  const QModelIndex newIndex = mSectionModel->indexForSection( newSection );
  mViewSections->setCurrentIndex( newIndex );
}

void QgsReportOrganizerWidget::removeSection()
{
  QgsAbstractReportSection *section = mSectionModel->sectionForIndex( mViewSections->currentIndex() );
  if ( !section || dynamic_cast< QgsReport * >( section ) )
    return; //report cannot be removed

  const int res = QMessageBox::question( this, tr( "Remove Section" ),
                                         tr( "Are you sure you want to remove the report section?" ),
                                         QMessageBox::Yes | QMessageBox::No, QMessageBox::No );
  if ( res == QMessageBox::No )
    return;

  std::function< void( QgsAbstractReportSection *section ) > cleanup;
  cleanup = [ =, &cleanup]( QgsAbstractReportSection * section )
  {
    if ( mDesigner->currentLayout() == section->header() || mDesigner->currentLayout() == section->footer() )
      mDesigner->setCurrentLayout( nullptr );
    if ( section->type() == QLatin1String( "SectionFieldGroup" ) )
    {
      QgsReportSectionFieldGroup *fieldGroup = static_cast< QgsReportSectionFieldGroup * >( section );
      if ( fieldGroup->body() == mDesigner->currentLayout() )
        mDesigner->setCurrentLayout( nullptr );
    }
    if ( section->type() == QLatin1String( "SectionLayout" ) )
    {
      QgsReportSectionLayout *sectionLayout = static_cast< QgsReportSectionLayout * >( section );
      if ( sectionLayout->body() == mDesigner->currentLayout() )
        mDesigner->setCurrentLayout( nullptr );
    }
    const QList< QgsAbstractReportSection * > children = section->childSections();
    for ( QgsAbstractReportSection *child : children )
      cleanup( child );
  };
  cleanup( section );

  mSectionModel->removeRow( mViewSections->currentIndex().row(), mViewSections->currentIndex().parent() );
}

void QgsReportOrganizerWidget::selectionChanged( const QModelIndex &current, const QModelIndex & )
{
  QgsAbstractReportSection *parent = mSectionModel->sectionForIndex( current );
  if ( !parent )
    parent = mReport;

  // report cannot be deleted
  mButtonRemoveSection->setEnabled( parent != mReport );

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
  else if ( QgsReport *section = dynamic_cast< QgsReport * >( parent ) )
  {
    QgsReportSectionWidget *widget = new QgsReportSectionWidget( this, mDesigner, section );
    mSettingsFrame->layout()->addWidget( widget );
    mConfigWidget = widget;
  }
  else
  {
    mConfigWidget = nullptr;
  }
}
