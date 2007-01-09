#ifdef HAVE_CONFIG_H
#include <qgsconfig.h>
#endif

#include <cstdio>
#include <cstdlib>
#include "plugingui.h"

#include <QApplication>
#include <QTranslator>
#include <QString>
#include "qgsapplication.h"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  /* Load translationfile */
  QTranslator tor(0);
  tor.load(QString("qgis_") + QTextCodec::locale(), QgsApplication::pkgDataPath() + "/i18n");
  a.installTranslator(&tor);
  
  QgsGridMakerPluginGui *myPluginGui=new QgsGridMakerPluginGui();
  a.setMainWidget(myPluginGui);
  myPluginGui->show();

  return a.exec();

  return EXIT_SUCCESS;
}
