#include <qapplication.h>
#include "qgsmapserverexport.h"
int main( int argc, char **argv )
{
  QApplication a( argc, argv );
  // Set up the QSettings environment must be done after qapp is created
  QCoreApplication::setOrganizationName("QuantumGIS");
  QCoreApplication::setOrganizationDomain("qgis.org");
  QCoreApplication::setApplicationName("qgis");

  QgsMapserverExport *mse = new QgsMapserverExport();
  mse->show();
  return a.exec();
}
