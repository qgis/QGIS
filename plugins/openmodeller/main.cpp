#include <stdio.h>
#include <stdlib.h>

#include "omguimain.h"
#include <qapplication.h>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  OmGuiMain * myOmGui = new OmGuiMain();
  a.setMainWidget(myOmGui);
  
  //only show the main gui once the model has run!
  //myOmGui->show();
  

  return a.exec();
  
  
  return EXIT_SUCCESS;
}
