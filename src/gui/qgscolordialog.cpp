/***************************************************************************
    qgscolordialog.cpp - color selection dialog

    ---------------------
    begin                : March 19, 2013
    copyright            : (C) 2013 by Larry Shaffer
    email                : larrys at dakcarto dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscolordialog.h"


QgsColorDialog::QgsColorDialog()
{
}

QgsColorDialog::~QgsColorDialog()
{
}

QColor QgsColorDialog::getLiveColor( const QColor& initialColor, QObject* updateObject, const char* updateSlot,
                                     QWidget* parent,
                                     const QString& title,
                                     QColorDialog::ColorDialogOptions options )
{
  QColor returnColor( initialColor );
  QColorDialog* liveDialog = new QColorDialog( initialColor, parent );
  liveDialog->setWindowTitle( title.isEmpty() ? tr( "Select Color" ) : title );
  liveDialog->setOptions( options );

  connect( liveDialog, SIGNAL( currentColorChanged( const QColor& ) ),
           updateObject, updateSlot );

  if ( liveDialog->exec() )
  {
    returnColor = liveDialog->currentColor();
  }
  delete liveDialog;
  liveDialog = 0;

  return returnColor;
}
