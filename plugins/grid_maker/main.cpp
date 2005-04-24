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

#if defined(Q_OS_MACX) || defined(WIN32)
  QString PKGDATAPATH = qApp->applicationDirPath() + "/share/qgis";
#endif

  /* Load translationfile */
  QTranslator tor(0);
  tor.load(QString("qgis_") + QTextCodec::locale(), QString(PKGDATAPATH) + "/i18n");
  a.installTranslator(&tor);
  
  QgsGridMakerPluginGui *myPluginGui=new QgsGridMakerPluginGui();
  a.setMainWidget(myPluginGui);
  myPluginGui->show();

  return a.exec();

  return EXIT_SUCCESS;
}
