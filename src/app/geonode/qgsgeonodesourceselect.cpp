/***************************************************************************
                              qgsgeonodesourceselect.cpp
                              -------------------
  begin                : August 25, 2006
  copyright            : (C) 2016 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeonodesourceselect.h"

#include <QDomDocument>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QPainter>

QgsGeonodeSourceSelect::QgsGeonodeSourceSelect( QWidget *parent, Qt::WindowFlags fl, bool embeddedMode )
  : QDialog( parent, fl )
{
  setupUi( this );

  if ( embeddedMode )
  {
    buttonBox->button( QDialogButtonBox::Close )->hide();
  }

  mAddButton = new QPushButton( tr( "&Add" ) );
  mAddButton->setEnabled( false );

  mBuildQueryButton = new QPushButton( tr( "&Build query" ) );
  mBuildQueryButton->setToolTip( tr( "Build query" ) );
  mBuildQueryButton->setDisabled( true );


  buttonBox->addButton( mAddButton, QDialogButtonBox::ActionRole );

  buttonBox->addButton( mBuildQueryButton, QDialogButtonBox::ActionRole );
}

QgsGeonodeSourceSelect::~QgsGeonodeSourceSelect() {}
