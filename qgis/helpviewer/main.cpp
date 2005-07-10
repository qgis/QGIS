#include <qapplication.h>
#include <qstring.h>
#include "qgshelpserver.h"
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

  // Create socket for client to send context requests to.
  // This allows an existing viewer to be reused rather then creating
  // an additional viewer if one is already running.
  QgsHelpContextServer *helpServer = new QgsHelpContextServer();
  // Make port number availaable to client
  printf("%d\n", helpServer->port());
  fflush(stdout);
  // Pass context request from socket to viewer widget
  QObject::connect(helpServer, SIGNAL(setContext(const QString&)),
    &w, SLOT(setContext(const QString&)));

  return a.exec();
}
