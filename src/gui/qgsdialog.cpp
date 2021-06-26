/***************************************************************************
                          qgsdialog.cpp
                             -------------------
    begin                : July 2012
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny dot dev at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
  QLayout *layout = nullptr;
  if ( orientation == Qt::Horizontal )
    layout = new QVBoxLayout();
  else
    layout = new QHBoxLayout();
  mLayout = new QVBoxLayout();
  layout->addItem( mLayout );
  layout->addWidget( mButtonBox );
  setLayout( layout );
}

