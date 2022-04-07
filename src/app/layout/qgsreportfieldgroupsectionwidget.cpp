/***************************************************************************
                             qgsreportfieldgroupsectionwidget.cpp
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

#include "qgsreportfieldgroupsectionwidget.h"
#include "qgsreportsectionfieldgroup.h"
#include "qgslayout.h"
#include "qgslayoutdesignerdialog.h"
#include "qgsreportorganizerwidget.h"

QgsReportSectionFieldGroupWidget::QgsReportSectionFieldGroupWidget( QgsReportOrganizerWidget *parent, QgsLayoutDesignerDialog *designer, QgsReportSectionFieldGroup *section )
  : QWidget( parent )
  , mOrganizer( parent )
  , mSection( section )
  , mDesigner( designer )
{
  setupUi( this );

  mLayerComboBox->setFilters( QgsMapLayerProxyModel::VectorLayer );
  connect( mLayerComboBox, &QgsMapLayerComboBox::layerChanged, mFieldComboBox, &QgsFieldComboBox::setLayer );
  connect( mButtonEditBody, &QPushButton::clicked, this, &QgsReportSectionFieldGroupWidget::editBody );
  connect( mButtonEditHeader, &QPushButton::clicked, this, &QgsReportSectionFieldGroupWidget::editHeader );
  connect( mButtonEditFooter, &QPushButton::clicked, this, &QgsReportSectionFieldGroupWidget::editFooter );

  mLayerComboBox->setLayer( section->layer() );
  mFieldComboBox->setLayer( section->layer() );
  mFieldComboBox->setField( section->field() );
  mSortAscendingCheckBox->setChecked( section->sortAscending() );

  mCheckShowHeader->setChecked( section->headerEnabled() );
  mButtonEditHeader->setChecked( section->headerEnabled() );
  mCheckHeaderAlwaysVisible->setChecked( section->headerVisibility() == QgsReportSectionFieldGroup::AlwaysInclude );
  mCheckHeaderAlwaysVisible->setEnabled( section->headerEnabled() );
  mCheckShowFooter->setChecked( section->footerEnabled() );
  mButtonEditFooter->setEnabled( section->footerEnabled() );
  mCheckFooterAlwaysVisible->setChecked( section->headerVisibility() == QgsReportSectionFieldGroup::AlwaysInclude );
  mCheckFooterAlwaysVisible->setEnabled( section->footerEnabled() );
  mCheckShowBody->setChecked( section->bodyEnabled() );
  mButtonEditBody->setEnabled( section->bodyEnabled() );

  connect( mSortAscendingCheckBox, &QCheckBox::toggled, this, &QgsReportSectionFieldGroupWidget::sortAscendingToggled );
  connect( mLayerComboBox, &QgsMapLayerComboBox::layerChanged, this, &QgsReportSectionFieldGroupWidget::setLayer );
  connect( mFieldComboBox, &QgsFieldComboBox::fieldChanged, this, &QgsReportSectionFieldGroupWidget::setField );
  connect( mCheckShowHeader, &QCheckBox::toggled, this, &QgsReportSectionFieldGroupWidget::toggleHeader );
  connect( mCheckHeaderAlwaysVisible, &QCheckBox::toggled, this, &QgsReportSectionFieldGroupWidget::toggleHeaderAlwaysVisible );
  connect( mCheckShowFooter, &QCheckBox::toggled, this, &QgsReportSectionFieldGroupWidget::toggleFooter );
  connect( mCheckFooterAlwaysVisible, &QCheckBox::toggled, this, &QgsReportSectionFieldGroupWidget::toggleFooterAlwaysVisible );
  connect( mCheckShowBody, &QCheckBox::toggled, this, &QgsReportSectionFieldGroupWidget::toggleBody );

  connect( mCheckShowHeader, &QCheckBox::toggled, mButtonEditHeader, &QPushButton::setEnabled );
  connect( mCheckShowFooter, &QCheckBox::toggled, mButtonEditFooter, &QPushButton::setEnabled );
  connect( mCheckShowBody, &QCheckBox::toggled, mButtonEditBody, &QPushButton::setEnabled );
}

void QgsReportSectionFieldGroupWidget::toggleHeader( bool enabled )
{
  mSection->setHeaderEnabled( enabled );
  mCheckHeaderAlwaysVisible->setEnabled( enabled );
}

void QgsReportSectionFieldGroupWidget::toggleHeaderAlwaysVisible( bool enabled )
{
  mSection->setHeaderVisibility( enabled ? QgsReportSectionFieldGroup::AlwaysInclude : QgsReportSectionFieldGroup::IncludeWhenFeaturesFound );
}

void QgsReportSectionFieldGroupWidget::toggleFooter( bool enabled )
{
  mSection->setFooterEnabled( enabled );
  mCheckFooterAlwaysVisible->setEnabled( enabled );
}

void QgsReportSectionFieldGroupWidget::toggleFooterAlwaysVisible( bool enabled )
{
  mSection->setFooterVisibility( enabled ? QgsReportSectionFieldGroup::AlwaysInclude : QgsReportSectionFieldGroup::IncludeWhenFeaturesFound );
}

void QgsReportSectionFieldGroupWidget::editHeader()
{
  if ( !mSection->header() )
  {
    std::unique_ptr< QgsLayout > header = std::make_unique< QgsLayout >( mSection->project() );
    header->initializeDefaults();
    mSection->setHeader( header.release() );
  }

  if ( mSection->header() )
  {
    mSection->header()->reportContext().setLayer( mSection->layer() );
    mDesigner->setCurrentLayout( mSection->header() );
    mDesigner->setSectionTitle( tr( "Header: %1" ).arg( mSection->description() ) );
    mOrganizer->setEditedSection( mSection );
  }
}

void QgsReportSectionFieldGroupWidget::editFooter()
{
  if ( !mSection->footer() )
  {
    std::unique_ptr< QgsLayout > footer = std::make_unique< QgsLayout >( mSection->project() );
    footer->initializeDefaults();
    mSection->setFooter( footer.release() );
  }

  if ( mSection->footer() )
  {
    mSection->footer()->reportContext().setLayer( mSection->layer() );
    mDesigner->setCurrentLayout( mSection->footer() );
    mDesigner->setSectionTitle( tr( "Footer: %1" ).arg( mSection->description() ) );
    mOrganizer->setEditedSection( mSection );
  }
}

void QgsReportSectionFieldGroupWidget::toggleBody( bool enabled )
{
  mSection->setBodyEnabled( enabled );
}

void QgsReportSectionFieldGroupWidget::editBody()
{
  if ( !mSection->body() )
  {
    std::unique_ptr< QgsLayout > body = std::make_unique< QgsLayout >( mSection->project() );
    body->initializeDefaults();
    mSection->setBody( body.release() );
  }

  if ( mSection->body() )
  {
    mSection->body()->reportContext().setLayer( mSection->layer() );
    mDesigner->setCurrentLayout( mSection->body() );
    mDesigner->setSectionTitle( tr( "Body: %1" ).arg( mSection->description() ) );
    mOrganizer->setEditedSection( mSection );
  }
}

void QgsReportSectionFieldGroupWidget::sortAscendingToggled( bool checked )
{
  mSection->setSortAscending( checked );
}

void QgsReportSectionFieldGroupWidget::setLayer( QgsMapLayer *layer )
{
  QgsVectorLayer *vl = qobject_cast< QgsVectorLayer * >( layer );
  if ( !vl )
    return;

  mSection->setLayer( vl );
  if ( mSection->body() )
    mSection->body()->reportContext().setLayer( mSection->layer() );
}

void QgsReportSectionFieldGroupWidget::setField( const QString &field )
{
  mSection->setField( field );
}
