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

#include "qgsmetadatawizard.h"

QgsMetadataWizard::QgsMetadataWizard( QWidget *parent, QgsMapLayer *layer )
  : QDialog( parent ), mLayer( layer )
{
  setupUi( this );

  basicMetadataButton->setChecked( true );
  backButton->setVisible( false );
  nextButton->setVisible( true );

  connect( basicMetadataButton, &QPushButton::clicked, this, &QgsMetadataWizard::updatePanel );
  connect( locationButton, &QPushButton::clicked, this, &QgsMetadataWizard::updatePanel );
  connect( optionalButton, &QPushButton::clicked, this, &QgsMetadataWizard::updatePanel );

  connect( cancelButton, &QPushButton::clicked, this, &QgsMetadataWizard::cancelClicked );
  connect( backButton, &QPushButton::clicked, this, &QgsMetadataWizard::backClicked );
  connect( nextButton, &QPushButton::clicked, this, &QgsMetadataWizard::nextClicked );
  connect( finishButton, &QPushButton::clicked, this, &QgsMetadataWizard::finishedClicked );

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
  int index = stackedWidget->currentIndex();
  if ( index == 1 )
  {
    basicMetadataButton->setChecked( true );
  }
  else if ( index == 2 )
  {
    locationButton->setChecked( true );
  }
  updatePanel();
}

void QgsMetadataWizard::nextClicked()
{
  int index = stackedWidget->currentIndex();
  if ( index == 0 )
  {
    locationButton->setChecked( true );
  }
  else if ( index == 1 )
  {
    optionalButton->setChecked( true );
  }
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
  if ( basicMetadataButton->isChecked() )
  {
    backButton->setVisible( false );
    nextButton->setVisible( true );
    stackedWidget->setCurrentIndex( 0 );
  }
  else if ( locationButton->isChecked() )
  {
    backButton->setVisible( true );
    backButton->setVisible( true );
    stackedWidget->setCurrentIndex( 1 );
  }
  else
  {
    backButton->setVisible( true );
    nextButton->setVisible( false );
    stackedWidget->setCurrentIndex( 2 );
  }
}
