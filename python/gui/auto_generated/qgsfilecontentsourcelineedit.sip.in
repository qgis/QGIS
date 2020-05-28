/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfilecontentsourcelineedit.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsAbstractFileContentSourceLineEdit : QWidget /Abstract/
{
%Docstring
Abstract base class for a widgets which allows users to select content from a file, embedding a file, etc.

This class is designed to be used by content which is managed by a QgsAbstractContentCache,
i.e. it can handle either direct file paths, base64 encoded contents, or remote HTTP
urls.

.. versionadded:: 3.6
%End

%TypeHeaderCode
#include "qgsfilecontentsourcelineedit.h"
%End
  public:

    QgsAbstractFileContentSourceLineEdit( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsAbstractFileContentSourceLineEdit, with the specified ``parent`` widget.
%End

    void setMessageBar( QgsMessageBar *bar );
%Docstring
Sets the message ``bar`` associated with the widget. This allows the widget to push feedback messages
to the appropriate message bar.

.. seealso:: :py:func:`messageBar`
%End

    QgsMessageBar *messageBar() const;
%Docstring
Returns the message bar associated with the widget.

.. seealso:: :py:func:`setMessageBar`
%End

    QString source() const;
%Docstring
Returns the current file source.

.. seealso:: :py:func:`setSource`

.. seealso:: :py:func:`sourceChanged`
%End

    void setLastPathSettingsKey( const QString &key );
%Docstring
Sets a specific settings ``key`` to use when storing the last
used path for the file source.
%End

  public slots:

    void setSource( const QString &source );
%Docstring
Sets a new ``source`` to show in the widget.

.. seealso:: :py:func:`source`

.. seealso:: :py:func:`sourceChanged`
%End

  signals:

    void sourceChanged( const QString &source );
%Docstring
Emitted whenever the file source is changed in the widget.
%End

};

class QgsSvgSourceLineEdit : QgsAbstractFileContentSourceLineEdit
{
%Docstring
A line edit widget with toolbutton for setting an SVG image path.

Designed for use with :py:class:`QgsSvgCache`.

.. seealso:: :py:class:`QgsImageSourceLineEdit`

.. versionadded:: 3.4
%End

%TypeHeaderCode
#include "qgsfilecontentsourcelineedit.h"
%End
  public:

    QgsSvgSourceLineEdit( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsSvgSourceLineEdit, with the specified ``parent`` widget.
%End

};


class QgsImageSourceLineEdit : QgsAbstractFileContentSourceLineEdit
{
%Docstring
A line edit widget with toolbutton for setting a raster image path.

Designed for use with :py:class:`QgsImageCache`.

.. seealso:: :py:class:`QgsSvgSourceLineEdit`

.. versionadded:: 3.6
%End

%TypeHeaderCode
#include "qgsfilecontentsourcelineedit.h"
%End
  public:

    QgsImageSourceLineEdit( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsImageSourceLineEdit, with the specified ``parent`` widget.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfilecontentsourcelineedit.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
