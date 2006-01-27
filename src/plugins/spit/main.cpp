#include <qapplication.h>
#include "qgsspit.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    QgsSpit w(0, Qt::Window);
    w.show();
    
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    
    return a.exec();
}
