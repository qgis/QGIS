#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <QUrl>
#include <QDesktopServices>

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
  QDesktopServices::openUrl( QUrl( argv[1] ) );
#ifdef Q_OS_WIN
  Sleep( 1000 );
#else
  sleep( 1 ); // not nice but if it exits immediately the page sometimes does not open
#endif
  exit( 0 );
}
