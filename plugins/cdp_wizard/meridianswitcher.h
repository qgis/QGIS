#ifndef _MERIDIANSWITCHER_H_
#define _MERIDIANSWITCHER_H_

#include <qfile.h>
#include <iostream>
#include <qstring.h>
#include <qvaluevector.h>
#include <qmap.h>

/** This class will shift the meridian (where greenwich meridian is found in the leftmost column)
 * of aarcinfo grid file.....later maybe any gdal file that supports creation.
 **/
class MeridianSwitcher
{

public:

  /** Default constructor */
  MeridianSwitcher();
  /** Constructor taking the name of the file to open. */
  MeridianSwitcher(QString theFileNameString);
  /** Destructor  */
   ~MeridianSwitcher();
private:
  //
  //   Private attributes
  //

};

#endif

