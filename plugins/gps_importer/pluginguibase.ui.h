/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/



void PluginGuiBase::pbnOK_clicked()
{

}


void PluginGuiBase::pbnSelectInputFile_clicked()
{

}


void PluginGuiBase::pbnSelectOutputFile_clicked()
{

}


void PluginGuiBase::pbnCancel_clicked()
{

}


void PluginGuiBase::leInputFile_textChanged( const QString & theQString)
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


void PluginGuiBase::leOutputShapeFile_textChanged( const QString & theQString )
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


void PluginGuiBase::leGPXFile_textChanged( const QString & theQString )
{

}


void PluginGuiBase::pbnGPXSelectFile_clicked()
{

}


void PluginGuiBase::cbGPXWaypoints_toggled( bool )
{

}


void PluginGuiBase::cbGPXRoutes_toggled( bool )
{

}


void PluginGuiBase::cbGPXTracks_toggled( bool )
{

}
