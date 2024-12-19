/***************************************************************************
   qgsscalevisibilitydialog.cpp
    --------------------------------------
   Date                 : 20.05.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QGridLayout>
#include <QDialogButtonBox>


#include "qgsscalevisibilitydialog.h"
#include "qgsscalerangewidget.h"


QgsScaleVisibilityDialog::QgsScaleVisibilityDialog( QWidget *parent, const QString &title, QgsMapCanvas *mapCanvas )
  : QDialog( parent )
{
  if ( !title.isEmpty() )
  {
    setWindowTitle( title );
  }

  QGridLayout *dlgLayout = new QGridLayout( this );
  //dlgLayout->setContentsMargins( 0, 0, 0, 0 );

  mGroupBox = new QGroupBox( this );
  mGroupBox->setCheckable( true );
  mGroupBox->setTitle( tr( "Scale visibility " ) );

  QGridLayout *gbLayout = new QGridLayout( mGroupBox );
  //gbLayout->setContentsMargins( 0, 0, 0, 0 );

  mScaleWidget = new QgsScaleRangeWidget( this );
  if ( mapCanvas )
  {
    mScaleWidget->setMapCanvas( mapCanvas );
  }
  gbLayout->addWidget( mScaleWidget, 0, 0 );

  QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Ok, Qt::Horizontal, this );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );

  dlgLayout->addWidget( mGroupBox, 0, 0 );
  dlgLayout->addWidget( buttonBox, 1, 0 );
}

void QgsScaleVisibilityDialog::setScaleVisibility( bool hasScaleVisibility )
{
  mGroupBox->setChecked( hasScaleVisibility );
}

bool QgsScaleVisibilityDialog::hasScaleVisibility() const
{
  return mGroupBox->isChecked();
}

void QgsScaleVisibilityDialog::setMinimumScale( double minScale )
{
  mScaleWidget->setMinimumScale( minScale );
}

double QgsScaleVisibilityDialog::minimumScale() const
{
  return mScaleWidget->minimumScale();
}

void QgsScaleVisibilityDialog::setMaximumScale( double maxScale )
{
  mScaleWidget->setMaximumScale( maxScale );
}

double QgsScaleVisibilityDialog::maximumScale() const
{
  return mScaleWidget->maximumScale();
}
