/****************************************************************************
** Form implementation generated from reading ui file 'qgshelpviewer.ui'
**
** Created: Fre Jan 30 07:58:59 2004
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "qgshelpviewer.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qtextbrowser.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/*
 *  Constructs a QgsHelpViewer as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
QgsHelpViewer::QgsHelpViewer( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "QgsHelpViewer" );
    setSizeGripEnabled( TRUE );
    QgsHelpViewerLayout = new QGridLayout( this, 1, 1, 2, 0, "QgsHelpViewerLayout"); 

    Layout1 = new QHBoxLayout( 0, 0, 6, "Layout1"); 
    Horizontal_Spacing2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( Horizontal_Spacing2 );

    buttonCancel = new QPushButton( this, "buttonCancel" );
    buttonCancel->setAutoDefault( TRUE );
    Layout1->addWidget( buttonCancel );

    QgsHelpViewerLayout->addLayout( Layout1, 1, 0 );

    textBrowser = new QTextBrowser( this, "textBrowser" );

    QgsHelpViewerLayout->addWidget( textBrowser, 0, 0 );
    languageChange();
    resize( QSize(511, 574).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
QgsHelpViewer::~QgsHelpViewer()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void QgsHelpViewer::languageChange()
{
    setCaption( tr( "QGIS Help" ) );
    buttonCancel->setText( tr( "&Close" ) );
    buttonCancel->setAccel( QKeySequence( tr( "Alt+C" ) ) );
}

