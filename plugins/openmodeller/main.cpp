#include <stdio.h>
#include <stdlib.h>

#include "omguimain.h"
#include <qapplication.h>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  OmGuiMain * myOmGui = new OmGuiMain();
  a.setMainWidget(myOmGui);
  myOmGui->show();
  

  return a.exec();
  
  
  return EXIT_SUCCESS;
}
