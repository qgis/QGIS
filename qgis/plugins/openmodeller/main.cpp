#include <stdio.h>
#include <stdlib.h>
#include "openmodellergui.h"
#include <qapplication.h>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  OpenModellerGui *myOpenModellerGui=new OpenModellerGui();
  a.setMainWidget(myOpenModellerGui);
  myOpenModellerGui->show();
  

  return a.exec();
  
  
  return EXIT_SUCCESS;
}
