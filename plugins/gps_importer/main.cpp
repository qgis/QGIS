#ifdef HAVE_CONFIG_H
#include <qgsconfig.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include "plugingui.h"
#include <qapplication.h>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  PluginGui *myPluginGui=new PluginGui();
  a.setMainWidget(myPluginGui);
  myPluginGui->show();
  

  return a.exec();
  
  
  return EXIT_SUCCESS;
}
