/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/



void QgsGridMakerPluginGuiBase::pbnOK_clicked()
{

}


void QgsGridMakerPluginGuiBase::pbnSelectInputFile_clicked()
{

}


void QgsGridMakerPluginGuiBase::pbnSelectOutputFile_clicked()
{

}


void QgsGridMakerPluginGuiBase::pbnCancel_clicked()
{

}


void QgsGridMakerPluginGuiBase::leInputFile_textChanged( const QString & theQString)
{
  if (theQString != "") 
    {
      pbnOK->setEnabled(true);
  }
  else
  {
   pbnOK->setEnabled(false);   
  }
}


void QgsGridMakerPluginGuiBase::leOutputShapeFile_textChanged( const QString & theQString )
{
  if (theQString != "") 
    {
      pbnOK->setEnabled(true);
  }
  else
  {
   pbnOK->setEnabled(false);   
  }
}
