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
  /** Destructor  */
   ~MeridianSwitcher();
   /** Dis waar die kak aangejaag word.... */
  void doSwitch(QString theInputFileString, QString theOutputFileString);
private:
  //
  //   Private attributes
  //

};

#endif

