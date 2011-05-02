
/**QgsLogger is a class to print debug/warning/error messages to the console. The advantage of this class over std::cout, std::cerr & co. is that the output can be controlled with environment variables:

QGIS_DEBUG is an int describing what debug messages are written to the console. If the debug level of a message is <= QGIS_DEBUG, the message is written to the console. It the variable QGIS_DEBUG is not defined, it defaults to 1 for debug mode and to 0 for release mode

QGIS_DEBUG_FILE may contain a file name. Only the messages from this file are printed (provided they have the right debuglevel). If QGIS_DEBUG_FILE is not set, messages from all files are printed
*/

class QgsLogger
{
%TypeHeaderCode
#include <qgslogger.h>
%End

 public:

  /**Goes to qDebug. 
  @param msg the message to be printed
  @param debuglevel 
  @param file file name where the message comes from
  @param function function where the message comes from
  @param line place in file where the message comes from*/
  static void debug(const QString& msg, int debuglevel = 1, const char* file = NULL, const char* function = NULL, int line = -1);

  /**Similar to the previous method, but prints a variable int-value pair*/
  static void debug(const QString& var, int val, int debuglevel = 1, const char* file = NULL, const char* function = NULL, int line = -1);

  /**Similar to the previous method, but prints a variable double-value pair*/
  // PyQGIS: commented out - has the same python signature as method above
  //static void debug(const QString& var, double val, int debuglevel = 1, const char* file = NULL, const char* function = NULL, int line = -1);

  /**Prints out a variable/value pair for types with overloaded operator<<*/
  // TODO: is it possible to make this working?
  //template <typename T> static void debug(const QString& var, T val, const char* file = 0, const char* function = 0,
  //                            					  int line = -1, int debuglevel = 1);

  /**Goes to qWarning*/
  static void warning(const QString& msg); 
  
  /**Goes to qCritical*/
  static void critical(const QString& msg);

  /**Goes to qFatal*/
  static void fatal(const QString& msg);

 private:
  /**Reads the environment variable QGIS_DEBUG and converts it to int. If QGIS_DEBUG is not set,\
   the function returns 1 if QGISDEBUG is defined and 0 if not*/
  static int debugLevel();

  /**Reads the environment variable QGIS_DEBUG_FILE. Returns NULL if the variable is not set*/
  static const char* debugFile();
};

