#include <qapplication.h>
#include "qgsspit.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    // Set up the QSettings environment must be done after qapp is created
    QCoreApplication::setOrganizationName("QuantumGIS");
    QCoreApplication::setOrganizationDomain("qgis.org");
    QCoreApplication::setApplicationName("qgis");

    QgsSpit w(0, Qt::Window);
    w.show();
    
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    
    return a.exec();
}
