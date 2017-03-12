class QgsErrorMessage
{
%TypeHeaderCode
#include <qgserror.h>
%End
  public:
    /** Format */
    enum Format
    {
      Text, // Plain text
      Html
    };

    QgsErrorMessage();

    /** Constructor.
     *  @param message error message string
     *  @param tag error label, for example GDAL, GDAL Provider, Raster layer
     *  @param file the file where error was created
     *  @param function the function where error was created
     *  @param line the line where error was created
     */
    QgsErrorMessage( const QString & message, const QString & tag = QString::null, const QString & file = QString::null, const QString & function = QString::null, int line = 0 );

    QString message() const;
    QString tag() const;
    QString file() const;
    QString function() const;
    int line() const;
};

/** \ingroup core
 * QgsError is container for error messages (report). It may contain chain
 * (sort of traceback) of error messages (e.g. GDAL - provider - layer).
 * Higher level messages are appended at the end.
*/
class QgsError
{
%TypeHeaderCode
#include <qgserror.h>
%End
  public:

    QgsError();

    /** Constructor with single message.
     *  @param message error message
     *  @param tag short description, e.g. GDAL, Provider, Layer
     */
    QgsError( const QString & message, const QString & tag );

    /** Append new error message.
     *  @param message error message string
     *  @param tag error label, for example GDAL, GDAL Provider, Raster layer
     */
    void append( const QString & message, const QString & tag );

    /** Append new error message.
     *  @param message error message
     */
    void append( const QgsErrorMessage & message );

    /** Test if any error is set.
     *  @return true if contains error
     */
    bool isEmpty() const;

    /** Full error messages description
     *  @param format output format
     *  @return error report
     */
    QString message( QgsErrorMessage::Format format = QgsErrorMessage::Html ) const;

    /** Short error description, usually the first error in chain, the real error.
     *  @return error description
     */
    QString summary() const;

    /** Clear error messages */
    void clear();
};

