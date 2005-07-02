#include <qapplication.h>
#include <qstring.h>
#include "qgshelpviewer.h"

int main( int argc, char ** argv )
{
  QApplication a( argc, argv );
  QString context = QString::null;
  if(argc == 2)
  {
    context = argv[1];
  }
  QgsHelpViewer w(context);
  w.show();

  a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));

  return a.exec();
}
