/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfilewidget.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsFileWidget : QWidget
{
%Docstring
 The QgsFileWidget class creates a widget for selecting a file or a folder.
%End

%TypeHeaderCode
#include "qgsfilewidget.h"
%End
%ConvertToSubClassCode
    if ( qobject_cast<QgsFileWidget *>( sipCpp ) )
      sipType = sipType_QgsFileWidget;
    else
      sipType = NULL;
%End
  public:

    enum StorageMode
    {
      GetFile,
      GetDirectory,
      GetMultipleFiles,
    };

    enum RelativeStorage
    {
      Absolute,
      RelativeProject,
      RelativeDefaultPath
    };

    explicit QgsFileWidget( QWidget *parent /TransferThis/ = 0 );
%Docstring
 QgsFileWidget creates a widget for selecting a file or a folder.
%End

    QString filePath();
%Docstring
 Returns the current file path(s)
 when multiple files are selected, they are quoted and separated
 by a single space (for example: '"/path/foo" "path/bar"')
.. seealso:: filePaths
 :rtype: str
%End

    static QStringList splitFilePaths( const QString &path );
%Docstring
 Split the the quoted and space separated ``path`` and returns a QString list
.. seealso:: filePath
 :rtype: list of str
%End

    void setFilePath( QString path );
%Docstring
Sets the file path
%End

    void setReadOnly( bool readOnly );
%Docstring
defines if the widget is readonly
%End

    QString dialogTitle() const;
%Docstring
returns the open file dialog title
 :rtype: str
%End

    void setDialogTitle( const QString &title );
%Docstring
 setDialogTitle defines the open file dialog title
.. note::

   if not defined, the title is "Select a file" or "Select a directory" or "Select one or more files" depending on the configuration.
%End

    QString filter() const;
%Docstring
returns the filters used for QDialog.getOpenFileName
 :rtype: str
%End

    void setFilter( const QString &filter );
%Docstring
 setFilter sets the filter used by the model to filters. The filter is used to specify the kind of files that should be shown.
 \param filter Only files that match the given filter are shown, it may be an empty string. If you want multiple filters, separate them with ';;',
%End

    bool fileWidgetButtonVisible() const;
%Docstring
determines if the tool button is shown
 :rtype: bool
%End
    void setFileWidgetButtonVisible( bool visible );
%Docstring
determines if the tool button is shown
%End

    bool useLink() const;
%Docstring
determines if the file path will be shown as a link
 :rtype: bool
%End
    void setUseLink( bool useLink );
%Docstring
determines if the file path will be shown as a link
%End

    bool fullUrl() const;
%Docstring
returns if the links shows the full path or not
 :rtype: bool
%End
    void setFullUrl( bool fullUrl );
%Docstring
determines if the links shows the full path or not
%End

    QString defaultRoot() const;
%Docstring
returns the default root path
 :rtype: str
%End
    void setDefaultRoot( const QString &defaultRoot );
%Docstring
determines the default root path used as the first shown location when picking a file and used if the RelativeStorage is RelativeDefaultPath
%End

    QgsFileWidget::StorageMode storageMode() const;
%Docstring
returns the storage mode (i.e. file or directory)
 :rtype: QgsFileWidget.StorageMode
%End
    void setStorageMode( QgsFileWidget::StorageMode storageMode );
%Docstring
determines the storage mode (i.e. file or directory)
%End

    QgsFileWidget::RelativeStorage relativeStorage() const;
%Docstring
returns if the relative path is with respect to the project path or the default path
 :rtype: QgsFileWidget.RelativeStorage
%End
    void setRelativeStorage( QgsFileWidget::RelativeStorage relativeStorage );
%Docstring
determines if the relative path is with respect to the project path or the default path
%End

    QLineEdit *lineEdit();
%Docstring
 Returns a pointer to the widget's line edit, which can be used to customize
 the appearance and behavior of the line edit portion of the widget.
.. versionadded:: 3.0
 :rtype: QLineEdit
%End

  signals:
    void fileChanged( const QString & );
%Docstring
emitted as soon as the current file or directory is changed
%End

};





/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfilewidget.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
