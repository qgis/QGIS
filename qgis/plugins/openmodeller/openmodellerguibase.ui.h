/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


void OpenModellerGuiBase::init()
{
   //hide some controls that have not yet been implemented
  cbxDefaultToLastChoices->hide();
  textLabel3->hide();
  progressBar1->hide();
  //this is really cheating - I am using the char arrays creataed in the cpp file after uic has run
  //to set the images
  pmAcmeLogo_2->setPixmap( image1);
  pmAcmeLogo_3 ->setPixmap( image1);
  pmAcmeLogo_4->setPixmap( image1);
  pmAcmeLogo_5->setPixmap( image1);
  pmAcmeLogo_6->setPixmap( image1);
  pmAcmeLogo_7->setPixmap( image1);
  pmAcmeLogo_2->setPixmap( image1);
}

void OpenModellerGuiBase::pbRun_clicked()
{

}


void OpenModellerGuiBase::pbnSelectOutputFile_clicked()
{

}


void OpenModellerGuiBase::leOutputFileName_textChanged( const QString & )
{

}


void OpenModellerGuiBase::pbnRemoveParameter_clicked()
{

}


void OpenModellerGuiBase::pbnAddParameter_clicked()
{

}


void OpenModellerGuiBase::pbnRemoveLayerFile_clicked()
{

}


void OpenModellerGuiBase::pbnSelectLayerFile_clicked()
{

}


void OpenModellerGuiBase::pbnSelectLocalitiesFile_clicked()
{

}


void OpenModellerGuiBase::leLocalitiesFileName_returnPressed()
{

}


void OpenModellerGuiBase::formSelected(const QString & )
{

}
