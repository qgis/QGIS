/*
** File: evisdatabaselayerfieldselectiongui.cpp
** Author: Peter J. Ersts ( ersts at amnh.org )
** Creation Date: 2007-03-07
**
** Copyright ( c ) 2007, American Museum of Natural History. All rights reserved.
**
** This library/program is free software; you can redistribute it
** and/or modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or ( at your option ) any later version.
**
** This library/program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** This work was made possible through a grant by the the John D. and
** Catherine T. MacArthur Foundation. Additionally, this program was prepared by
** the American Museum of Natural History under award No. NA05SEC46391002
** from the National Oceanic and Atmospheric Administration, U.S. Department
** of Commerce.  The statements, findings, conclusions, and recommendations
** are those of the author( s ) and do not necessarily reflect the views of the
** National Oceanic and Atmospheric Administration or the Department of Commerce.
**
**/
/*  $Id$ */
#include "evisdatabaselayerfieldselectiongui.h"

/**
* Constructor
* @param parent - Pointer the to parent QWidget for modality
* @param fl - Windown flags
*/
eVisDatabaseLayerFieldSelectionGui::eVisDatabaseLayerFieldSelectionGui( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );
}

/**
* Public method that will insert the values in fieldList into the x and y coordinate selection combo boxes.
* @param fileList - QStringList containing the field names to add to the combo boxes
*/
void eVisDatabaseLayerFieldSelectionGui::setFieldList( QStringList* fieldList )
{

  int xCoordinateIndex = -1;
  int yCoordinateIndex = -1;
  cboxXCoordinate->clear( );
  cboxYCoordinate->clear( );

  for ( int x = 0; x < fieldList->size( ); x++ )
  {
    cboxXCoordinate->addItem( fieldList->at( x ) );
    cboxYCoordinate->addItem( fieldList->at( x ) );

    //Take a guess in an attempt to auto select the currect field
    if ( fieldList->at( x ).contains( QRegExp( "( ^x|^lon|^east )", Qt::CaseInsensitive ) ) )
    {
      xCoordinateIndex = x;
    }

    if ( fieldList->at( x ).contains( QRegExp( "( ^y|^lat|^north )", Qt::CaseInsensitive ) ) )
    {
      yCoordinateIndex = x;
    }

  }

  cboxXCoordinate->setCurrentIndex( xCoordinateIndex );
  cboxYCoordinate->setCurrentIndex( yCoordinateIndex );
}

/*
 *
 * Public and Private Slots
 *
 */
/**
* Slot called when the ok/accept button is pressed
*/
void eVisDatabaseLayerFieldSelectionGui::on_buttonBox_accepted( )
{
  //emit the signal to draw the layer
  emit eVisDatabaseLayerFieldsSelected( leLayerName->text( ), cboxXCoordinate->currentText( ), cboxYCoordinate->currentText( ) );

  //close the gui component
  close( );

  //reset the layer name line edit
  leLayerName->setText( "" );
}

/**
* Slot called then the cancel button is pressed
*/
void eVisDatabaseLayerFieldSelectionGui::on_buttonBox_rejected( )
{
  close( );
  leLayerName->setText( "" );
}
