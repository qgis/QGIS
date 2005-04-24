

/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
#include <qcolordialog.h> 
#include <qcolor.h>
#include <qfontdialog.h>
#include <qfont.h>

void QgsCopyrightLabelPluginGuiBase::pbnOK_clicked()
{

}


void QgsCopyrightLabelPluginGuiBase::pbnCancel_clicked()
{

}


void QgsCopyrightLabelPluginGuiBase::btnFontColour_clicked()
{
    QColor myQColor = QColorDialog::getColor();
    txtCopyrightText->setPaletteForegroundColor(myQColor);
}


void QgsCopyrightLabelPluginGuiBase::btnFontFace_clicked()
{
    bool ok;
    QFont myFont = QFontDialog::getFont(
                    &ok, QFont( "Helvetica", 10 ), this );
    if ( ok ) {
        // font is set to the font the user selected
	txtCopyrightText->setCurrentFont(myFont);
    } else {
        // the user canceled the dialog; font is set to the initial
        // value, in this case Helvetica [Cronyx], 10
    }
    
    
}


void QgsCopyrightLabelPluginGuiBase::cboxEnabled_toggled( bool )
{

}


void QgsCopyrightLabelPluginGuiBase::cboOrientation_textChanged( const QString & )
{

}


void QgsCopyrightLabelPluginGuiBase::cboPlacement_highlighted( const QString & )
{

}
