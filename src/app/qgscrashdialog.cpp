/***************************************************************************
                qgscrashdialog.cpp - QgsCrashDialog

 ---------------------
 begin                : 11.4.2017
 copyright            : (C) 2017 by Nathan Woodrow
 email                : woodrow.nathan@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgscrashdialog.h"

QgsCrashDialog::QgsCrashDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  setWindowTitle( tr( "Oh. Snap!" ) );

  mCrashHeaderMessage->setText( tr( "Ouch!" ) );
  mCrashMessage->setText( tr("Sorry it looks like QGIS crashed. \nSomething unexpected happened that we didn't handle correctly."));

  mHelpLabel->setText( tr( "Keen to help us fix it? <br> <a href=\"http://qgis.org/en/site/getinvolved/development/bugreporting.html#bugs-features-and-issues\"> Follow the steps to help devs.<a>" ) );
  mHelpLabel->setTextInteractionFlags( Qt::TextBrowserInteraction );
  mHelpLabel->setOpenExternalLinks( true );
}
