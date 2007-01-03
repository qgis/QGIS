#include <QApplication>
#include <QTranslator>
#include <QTextCodec>
#include <QStringList>

#include "qgsmapserverexport.h"
#include "src/core/qgsapplication.h"

#include <iostream>

int main( int argc, char **argv )
{
  // Using QgsApplication instead of QApplication gives access to the
  // qgis translation files.
  QgsApplication a( argc, argv , true);

  // Set up the QSettings environment must be done after a is created
  QCoreApplication::setOrganizationName("QuantumGIS");
  QCoreApplication::setOrganizationDomain("qgis.org");
  QCoreApplication::setApplicationName("qgis");

  // Install translations if available. Based on the code in the Qgis
  // main.cpp file. 

  // Let the user set the locale on the command line. If this program
  // needs to parse any more arguments, it'll be worthwhile using
  // something like getopt.
  QString translationCode;
  QStringList args = a.arguments();
  int i = args.lastIndexOf("-h");
  if (i != -1)
  {
    std::cout << "Usage: msexport [--lang language]\n"
      "\t[--lang language]\tuse language for interface text (optional)\n";
    exit(0);
  }

  i = args.lastIndexOf("--lang");
  if (i != -1)
  {
    if (args.count() > i+1)
      translationCode = args[i+1];
  }

  if (translationCode.isEmpty())
    translationCode = QTextCodec::locale();

  QString i18nPath = QgsApplication::i18nPath();

  QTranslator qttor;
  if (qttor.load("qt_" + translationCode, i18nPath))
    a.installTranslator(&qttor);

  QTranslator mstor;
  if (mstor.load("qgis_" + translationCode, i18nPath))
    a.installTranslator(&mstor);

  QgsMapserverExport *mse = new QgsMapserverExport();
  mse->show();
  return a.exec();
}
