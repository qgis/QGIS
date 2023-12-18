/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfiledropedit.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsFileDropEdit: QLineEdit
{
%Docstring
 A line edit for capturing file names that can have files dropped onto
 it via drag & drop.

 Dropping can be limited to files only, files with a specific extension
or directories only. By default, dropping is limited to files only.
%End

%TypeHeaderCode
#include "qgsfiledropedit.h"
%End
  public:
    QgsFileDropEdit( QWidget *parent /TransferThis/ = 0 );

    bool isDirOnly() const;
%Docstring
 :rtype: bool
%End

    void setDirOnly( bool isDirOnly );
%Docstring
Limit drops to directories.
%End

    bool isFileOnly() const;
%Docstring
 :rtype: bool
%End

    void setFileOnly( bool isFileOnly );
%Docstring
Limit drops to files.
%End

    QString suffixFilter() const;
%Docstring
 :rtype: str
%End

    void setSuffixFilter( const QString &suffix );
%Docstring
Limit drops to files with specified extension.
%End

  protected:

    virtual void dragEnterEvent( QDragEnterEvent *event );
    virtual void dragLeaveEvent( QDragLeaveEvent *event );
    virtual void dropEvent( QDropEvent *event );
    virtual void paintEvent( QPaintEvent *e );

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfiledropedit.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
