
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
  QColor color = QColorDialog::getColor(pbnSelectionColour->paletteBackgroundColor(),this);
  if (color.isValid())
  {
    pbnSelectionColour->setPaletteBackgroundColor(color);
  }
}


void QgsProjectPropertiesBase::pbnDigitisedLineColour_clicked()
{
  QColor color = QColorDialog::getColor(pbnDigitisedLineColour->paletteBackgroundColor(),this);
  if (color.isValid())
  {
    pbnDigitisedLineColour->setPaletteBackgroundColor(color);
  }
}
