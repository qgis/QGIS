#include <unistd.h>
#include <iostream>

#include "QgisUntwine.hpp"

int main()
{
  untwine::QgisUntwine::StringList files;
  untwine::QgisUntwine::Options options;
  untwine::QgisUntwine api( "/Users/acbell/untwine/build/untwine" );

  files.push_back( "/Users/acbell/nyc2" );
  api.start( files, "./out", options );

  bool stopped = false;
  while ( true )
  {
    sleep( 1 );
    int percent = api.progressPercent();
    std::string s = api.progressMessage();
    std::cerr << "Percent/Msg = " << percent << " / " << s << "!\n";

    /**
    if (!stopped && percent >= 50)
    {
        stopped = true;
        api.stop();
    }
    **/
    if ( !api.running() )
      break;
  }
}
