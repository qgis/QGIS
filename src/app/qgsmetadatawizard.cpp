/***************************************************************************
                          qgsmetadatawizard.h  -  description
                             -------------------
    begin                : 17/05/2017
    copyright            : (C) 2017 by Etienne Trimaille
    email                : etienne at kartoza.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtWidgets>
#include <QIcon>
#include <QPushButton>
#include <QString>

#include "qgsmetadatawizard.h"

QgsMetadataWizard::QgsMetadataWizard( QWidget *parent, QgsMapLayer *layer )
  : QDialog( parent ), mLayer( layer )
{
  setupUi( this );

  tabWidget->setCurrentIndex( 0 );
  backButton->setEnabled( false );
  nextButton->setEnabled( true );

  connect( tabWidget, &QTabWidget::currentChanged, this, &QgsMetadataWizard::updatePanel );
  connect( cancelButton, &QPushButton::clicked, this, &QgsMetadataWizard::cancelClicked );
  connect( backButton, &QPushButton::clicked, this, &QgsMetadataWizard::backClicked );
  connect( nextButton, &QPushButton::clicked, this, &QgsMetadataWizard::nextClicked );
  connect( finishButton, &QPushButton::clicked, this, &QgsMetadataWizard::finishedClicked );
  connect( btnAddLink, &QPushButton::clicked, this, &QgsMetadataWizard::addLink );
  connect( btnRemoveLink, &QPushButton::clicked, this, &QgsMetadataWizard::removeLink );


  // Set all properties
  layerLabel->setText( mLayer->name() );
  lineEditTitle->setText( mLayer->name() );
  textEditAbstract->setText( mLayer->abstract() );
  textEditKeywords->setText( mLayer->keywordList() );
}

QgsMetadataWizard::~QgsMetadataWizard()
{
}

void QgsMetadataWizard::cancelClicked()
{
  hide();
}

void QgsMetadataWizard::backClicked()
{
  int index = tabWidget->currentIndex();
  if ( index > 0 )
    tabWidget->setCurrentIndex( index - 1 );
  updatePanel();
}

void QgsMetadataWizard::nextClicked()
{
  int index = tabWidget->currentIndex();
  if ( index < tabWidget->count() )
    tabWidget->setCurrentIndex( index + 1 );
  updatePanel();
}

void QgsMetadataWizard::finishedClicked()
{
  mLayer->setName( lineEditTitle->text() );
  mLayer->setAbstract( textEditAbstract->toPlainText() );
  mLayer->setKeywordList( textEditKeywords->toPlainText() );
  hide();
}

void QgsMetadataWizard::updatePanel()
{
  int index = tabWidget->currentIndex();
  if ( index == 0 )
  {
    backButton->setEnabled( false );
    nextButton->setEnabled( true );
  }
  else if ( index == tabWidget->count() - 1 )
  {
    backButton->setEnabled( true );
    nextButton->setEnabled( false );
  }
  else
  {
    backButton->setEnabled( true );
    nextButton->setEnabled( true );
  }
}

void QgsMetadataWizard::addLink()
{
//  int index = tabLinks->topLevelItemCount();
//  QTreeWidgetItem *link = new QTreeWidgetItem( tabLinks );
//  link->setText( 0, tr( "Link" ) + QString::number( index ) );
//  tabLinks->addTopLevelItem( link );
}

void QgsMetadataWizard::removeLink()
{
//  if ( tabLinks->currentItem() )
//    tabLinks->takeTopLevelItem( tabLinks->currentIndex() );
}
