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

QgsReportSectionFieldGroupWidget::QgsReportSectionFieldGroupWidget( QWidget *parent, QgsLayoutDesignerDialog *designer, QgsReportSectionFieldGroup *section )
  : QWidget( parent )
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
  mFieldComboBox->setField( section->field() );
  mSortAscendingCheckBox->setChecked( section->sortAscending() );

  mCheckShowHeader->setChecked( section->headerEnabled() );
  mCheckShowFooter->setChecked( section->footerEnabled() );
  mCheckShowBody->setChecked( section->bodyEnabled() );

  connect( mSortAscendingCheckBox, &QCheckBox::toggled, this, &QgsReportSectionFieldGroupWidget::sortAscendingToggled );
  connect( mLayerComboBox, &QgsMapLayerComboBox::layerChanged, this, &QgsReportSectionFieldGroupWidget::setLayer );
  connect( mFieldComboBox, &QgsFieldComboBox::fieldChanged, this, &QgsReportSectionFieldGroupWidget::setField );
  connect( mCheckShowHeader, &QCheckBox::toggled, this, &QgsReportSectionFieldGroupWidget::toggleHeader );
  connect( mCheckShowFooter, &QCheckBox::toggled, this, &QgsReportSectionFieldGroupWidget::toggleFooter );
  connect( mCheckShowBody, &QCheckBox::toggled, this, &QgsReportSectionFieldGroupWidget::toggleBody );
}

void QgsReportSectionFieldGroupWidget::toggleHeader( bool enabled )
{
  mSection->setHeaderEnabled( enabled );
}

void QgsReportSectionFieldGroupWidget::toggleFooter( bool enabled )
{
  mSection->setFooterEnabled( enabled );
}

void QgsReportSectionFieldGroupWidget::editHeader()
{
  if ( !mSection->header() )
  {
    std::unique_ptr< QgsLayout > header = qgis::make_unique< QgsLayout >( mSection->project() );
    header->initializeDefaults();
    mSection->setHeader( header.release() );
  }

  if ( mSection->header() )
  {
    mSection->header()->reportContext().setLayer( mSection->layer() );
    mDesigner->setCurrentLayout( mSection->header() );
    mDesigner->setSectionTitle( tr( "%1 Header" ).arg( mSection->description() ) );
  }
}

void QgsReportSectionFieldGroupWidget::editFooter()
{
  if ( !mSection->footer() )
  {
    std::unique_ptr< QgsLayout > footer = qgis::make_unique< QgsLayout >( mSection->project() );
    footer->initializeDefaults();
    mSection->setFooter( footer.release() );
  }

  if ( mSection->footer() )
  {
    mSection->footer()->reportContext().setLayer( mSection->layer() );
    mDesigner->setCurrentLayout( mSection->footer() );
    mDesigner->setSectionTitle( tr( "%1 Footer" ).arg( mSection->description() ) );
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
    std::unique_ptr< QgsLayout > body = qgis::make_unique< QgsLayout >( mSection->project() );
    body->initializeDefaults();
    mSection->setBody( body.release() );
  }

  if ( mSection->body() )
  {
    mSection->body()->reportContext().setLayer( mSection->layer() );
    mDesigner->setCurrentLayout( mSection->body() );
    mDesigner->setSectionTitle( tr( "%1 Body" ).arg( mSection->description() ) );
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
