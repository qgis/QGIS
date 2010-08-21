#include "qgsmsutils.h"
#include "qgsmapserverlogger.h"
#include <stdlib.h>
#include <time.h>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>

QString QgsMSUtils::createTempFilePath()
{


  //save the content of the file into a temporary location
  //generate a name considering the current time
  //struct timeval currentTime;
  //gettimeofday(&currentTime, NULL);

  time_t seconds;
  time( &seconds );
  srand( seconds );

  int randomNumber = rand();
  QString tempFileName = QString::number( randomNumber );
  QString tempFilePath;
  //on windows, store the temporary file in current_path/tmp directory,
  //on unix, store it in /tmp/qgis_wms_serv
#ifndef WIN32
  QDir tempFileDir( "/tmp/qgis_map_serv" );
  if ( !tempFileDir.exists() ) //make sure the directory exists
  {
    QDir tmpDir( "/tmp" );
    tmpDir.mkdir( "qgis_map_serv" );
  }
  tempFilePath = "/tmp/qgis_map_serv/" + tempFileName;
#else
  QDir tempFileDir( QDir::currentPath() + "/tmp" );
  if ( !tempFileDir.exists() )
  {
    QDir currentDir( QDir::currentPath() );
    currentDir.mkdir( "tmp" );
  }
  tempFilePath = QDir::currentPath() + "/tmp" + "/" + tempFileName;
#endif //WIN32

  QFileInfo testFile( tempFilePath );
  while ( testFile.exists() )
  {
    //change the name
    tempFilePath += "1";
    testFile.setFile( tempFilePath );
  }
  QgsMapServerLogger::instance()->printMessage( tempFilePath );
  return tempFilePath;
}

int QgsMSUtils::createTextFile( QString filePath, const QString& text )
{
  QFile file( filePath );
  if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) )
  {
    QTextStream fileStream( &file );
    fileStream << text;
    file.close();
    return 0;
  }
  else
  {
    return 1;
  }
}
