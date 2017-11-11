/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgserror.h                                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsErrorMessage
{
%Docstring
 QgsErrorMessage represents single error message.
%End

%TypeHeaderCode
#include "qgserror.h"
%End
  public:
    enum Format
    {
      Text,
      Html
    };

    QgsErrorMessage();
%Docstring
Constructor for QgsErrorMessage
%End

    QgsErrorMessage( const QString &message, const QString &tag = QString(), const QString &file = QString(), const QString &function = QString(), int line = 0 );
%Docstring
 Constructor.
  \param message error message string
  \param tag error label, for example GDAL, GDAL Provider, Raster layer
  \param file the file where error was created
  \param function the function where error was created
  \param line the line where error was created
%End

    QString message() const;
%Docstring
 :rtype: str
%End
    QString tag() const;
%Docstring
 :rtype: str
%End
    QString file() const;
%Docstring
 :rtype: str
%End
    QString function() const;
%Docstring
 :rtype: str
%End
    int line() const;
%Docstring
 :rtype: int
%End

};

class QgsError
{
%Docstring
 QgsError is container for error messages (report). It may contain chain
 (sort of traceback) of error messages (e.g. GDAL - provider - layer).
 Higher level messages are appended at the end.
%End

%TypeHeaderCode
#include "qgserror.h"
%End
  public:

    QgsError();
%Docstring
Constructor for QgsError
%End

    QgsError( const QString &message, const QString &tag );
%Docstring
 Constructor with single message.
  \param message error message
  \param tag short description, e.g. GDAL, Provider, Layer
%End

    void append( const QString &message, const QString &tag );
%Docstring
 Append new error message.
  \param message error message string
  \param tag error label, for example GDAL, GDAL Provider, Raster layer
%End

    void append( const QgsErrorMessage &message );
%Docstring
 Append new error message.
  \param message error message
%End

    bool isEmpty() const;
%Docstring
 Test if any error is set.
  :return: true if contains error
 :rtype: bool
%End

    QString message( QgsErrorMessage::Format format = QgsErrorMessage::Html ) const;
%Docstring
 Full error messages description
  \param format output format
  :return: error report
 :rtype: str
%End

    QString summary() const;
%Docstring
 Short error description, usually the first error in chain, the real error.
  :return: error description
 :rtype: str
%End

    void clear();
%Docstring
Clear error messages
%End

    QList<QgsErrorMessage> messageList() const;
%Docstring
 messageList return the list of current error messages
 :return: current list of error messages
 :rtype: list of QgsErrorMessage
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgserror.h                                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
