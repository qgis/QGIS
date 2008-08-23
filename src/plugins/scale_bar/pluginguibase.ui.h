/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
#include <qcolordialog.h>

void QgsScaleBarPluginGuiBase::pbnOK_clicked()
{

}


void QgsScaleBarPluginGuiBase::pbnCancel_clicked()
{

}









void QgsScaleBarPluginGuiBase::btnTopLeft_toggled( bool )
{

}


void QgsScaleBarPluginGuiBase::btnTopMiddle_toggled( bool )
{

}


void QgsScaleBarPluginGuiBase::btnTopRight_toggled( bool )
{

}


void QgsScaleBarPluginGuiBase::btnMiddleLeft_toggled( bool )
{

}


void QgsScaleBarPluginGuiBase::btnMiddleRight_toggled( bool )
{

}


void QgsScaleBarPluginGuiBase::btnBottomLeft_toggled( bool )
{

}


void QgsScaleBarPluginGuiBase::btnBottomMiddle_toggled( bool )
{

}


void QgsScaleBarPluginGuiBase::btnBottomRight_toggled( bool )
{

}


void QgsScaleBarPluginGuiBase::pbnChangeColour_clicked()
{
  frameColour->setPaletteBackgroundColor( QColorDialog::getColor( QColor( Qt::black ), this ) );
}
