/***************************************************************************
                          qgsdialog.cpp
                             -------------------
    begin                : July 2012
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny dot dev at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdialog.h"

QgsDialog::QgsDialog( QWidget *parent, Qt::WindowFlags fl,
                      QDialogButtonBox::StandardButtons buttons,
                      Qt::Orientation orientation )
  : QDialog( parent, fl )
{
  // create buttonbox
  mButtonBox = new QDialogButtonBox( buttons, orientation, this );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );

  // layout
  QBoxLayout *layout = nullptr;
  if ( orientation == Qt::Horizontal )
    layout = new QVBoxLayout();
  else
    layout = new QHBoxLayout();
  mLayout = new QVBoxLayout();
  layout->addLayout( mLayout );
  layout->addWidget( mButtonBox );
  setLayout( layout );
}

