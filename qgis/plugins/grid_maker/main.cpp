#ifdef HAVE_CONFIG_H
#include <qgsconfig.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include "plugingui.h"
#include <qapplication.h>
#include <qtextcodec.h>
#include <qtranslator.h>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  /* Load translationfile */
  QTranslator tor(0);
  tor.load(QString("qgis_") + QTextCodec::locale(), QString(PKGDATAPATH) + "/i18n");
  a.installTranslator(&tor);
  
  PluginGui *myPluginGui=new PluginGui();
  a.setMainWidget(myPluginGui);
  myPluginGui->show();

  return a.exec();

  return EXIT_SUCCESS;
}
