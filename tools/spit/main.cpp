#include <qapplication.h>
#include "qgsspit.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    QgsSpit w;
    w.show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    return a.exec();
}
