//
// C++ Implementation: %{MODULE}
//
// Description: 
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "omguimain.h"
#include <qpixmap.h>
#include <qlabel.h>
#include <qstring.h>
#include "openmodellergui.h"

OmGuiMain::OmGuiMain()
  : OmGuiMainBase()
{
  runWizard();
}


OmGuiMain::~OmGuiMain()
{
}

void OmGuiMain::fileExit()
{
  close();
}
void OmGuiMain::runWizard()
{
  OpenModellerGui * myOpenModellerGui = new OpenModellerGui(this,"openModeller Wizard",true,0);
  connect(myOpenModellerGui, SIGNAL(drawModelImage(QString)), this, SLOT(drawModelImage(QString)));

  myOpenModellerGui->show();
}

void OmGuiMain::drawModelImage(QString theFileName)
{
   //set the image label on the calculating variables screen to show the last
  //variable calculated
  std::cout << "drawModelImage Called" << std::endl;
  QPixmap myPixmap(theFileName);
  pixModelOutputImage->setScaledContents(true);
  pixModelOutputImage->setPixmap(myPixmap); 
  pixModelOutputImage->show();
}
