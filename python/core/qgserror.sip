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
     *  @param theMessage error message string
     *  @param theTag error label, for example GDAL, GDAL Provider, Raster layer
     *  @param theFile the file where error was created
     *  @param theFunction the function where error was created
     *  @param theLine the line where error was created
     */
    QgsErrorMessage( const QString & theMessage, const QString & theTag = QString::null, const QString & theFile = QString::null, const QString & theFunction = QString::null, int theLine = 0 );

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
     *  @param theMessage error message
     *  @param theTag short description, e.g. GDAL, Provider, Layer
     */
    QgsError( const QString & theMessage, const QString & theTag );

    /** Append new error message.
     *  @param theMessage error message string
     *  @param theTag error label, for example GDAL, GDAL Provider, Raster layer
     */
    void append( const QString & theMessage, const QString & theTag );

    /** Append new error message.
     *  @param theMessage error message
     */
    void append( const QgsErrorMessage & theMessage );

    /** Test if any error is set.
     *  @return true if contains error
     */
    bool isEmpty() const;

    /** Full error messages description
     *  @param theFormat output format
     *  @return error report
     */
    QString message( QgsErrorMessage::Format theFormat = QgsErrorMessage::Html ) const;

    /** Short error description, usually the first error in chain, the real error.
     *  @return error description
     */
    QString summary() const;

    /** Clear error messages */
    void clear();
};

