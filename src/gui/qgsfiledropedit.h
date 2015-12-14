/***************************************************************************
    qgsfiledropedit.h - File Dropable LineEdit
     --------------------------------------
    Date                 : 31-Jan-2007
    Copyright            : (C) 2007 by Tom Elwertowski
    Email                : telwertowski at users dot sourceforge dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFILEDROPEDIT_H
#define QGSFILEDROPEDIT_H

#include <QLineEdit>

/** \ingroup gui
 * A line edit for capturing file names that can have files dropped onto
 * it via drag & drop.
 */
class GUI_EXPORT QgsFileDropEdit: public QLineEdit
{
    Q_OBJECT

  public:
    QgsFileDropEdit( QWidget *parent = nullptr );
    virtual ~QgsFileDropEdit();

    bool isDirOnly() const { return mDirOnly; }
    void setDirOnly( bool isDirOnly );

    bool isFileOnly() const { return mFileOnly; }
    void setFileOnly( bool isFileOnly );

    QString suffixFilter() const { return mSuffix; }
    void setSuffixFilter( const QString& suffix );

  protected:

    virtual void dragEnterEvent( QDragEnterEvent *event ) override;
    virtual void dragLeaveEvent( QDragLeaveEvent *event ) override;
    virtual void dropEvent( QDropEvent *event ) override;
    virtual void paintEvent( QPaintEvent *e ) override;

  private:
    QString acceptableFilePath( QDropEvent *event ) const;

    QString mSuffix;
    bool mDirOnly;
    bool mFileOnly;
    bool mDragActive;
};

#endif
