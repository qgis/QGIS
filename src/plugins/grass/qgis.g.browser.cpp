#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <QUrl>
#include <QDesktopServices>
#include <QString>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

// Open a URL by default browser
int main( int argc, char **argv )
{
  if ( argc < 2 )
  {
    fprintf( stderr, "URL argument missing\n" );
    exit( 1 );
  }
  QUrl url ( argv[1] );
#ifdef Q_OS_WIN
  // openUrl on windows fails to open 'file://c:...' it must be 'file:///c:...' (3 slashes)
  if ( url.scheme() == "file" ) {
    url.setPath ( "/" + url.path() );
    std::cout << "path reset to: " << qPrintable(url.path()) << std::endl;
  }
#endif
  QDesktopServices::openUrl( url );
#ifdef Q_OS_WIN
  Sleep( 1000 );
#else
  sleep( 1 ); // not nice but if it exits immediately the page sometimes does not open
#endif
  exit( 0 );
}
