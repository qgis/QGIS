
/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
#include "../widgets/projectionselector/qgsprojectionselector.h"
#include <qcolordialog.h>
void QgsProjectPropertiesBase::mapUnitChange( int )
{

}


void QgsProjectPropertiesBase::apply()
{

}


void QgsProjectPropertiesBase::pbnSelectionColour_clicked()
{
   pbnSelectionColour->setPaletteBackgroundColor(
     QColorDialog::getColor(pbnSelectionColour->paletteBackgroundColor(),this));
}


void QgsProjectPropertiesBase::pbnDigitisedLineColour_clicked()
{
   pbnDigitisedLineColour->setPaletteBackgroundColor(
     QColorDialog::getColor(pbnDigitisedLineColour->paletteBackgroundColor(),this));
}



