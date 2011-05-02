#ifndef QGSMSUTILS_H
#define QGSMSUTILS_H

#include <QString>

/**Some utility functions that may be included from everywhere in the code*/
namespace QgsMSUtils
{
  /**Creates a ramdom filename for a temporary file. This function also creates the directory to store
     the temporary files if it does not already exist. The directory is /tmp/qgis_map_serv/ under linux or
     the current working directory on windows*/
  QString createTempFilePath();
  /**Stores the specified text in a temporary file. Returns 0 in case of success*/
  int createTextFile( QString filePath, const QString& text );
}

#endif
