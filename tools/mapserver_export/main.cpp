#include <qapplication.h>
#include "qgsmapserverexport.h"
int main( int argc, char **argv )
{
  QApplication a( argc, argv );

  QgsMapserverExport *mse = new QgsMapserverExport();
  a.setMainWidget( mse );
  mse->show();
  return a.exec();
}
