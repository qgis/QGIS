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
/* $Id$ */
#ifndef QGSFILEDROPEDIT_H
#define QGSFILEDROPEDIT_H

#include <QLineEdit>

/** \ingroup gui
 * A line edit for capturing file names that can have files dropped onto
 * it via drag & drop.
 */
class GUI_EXPORT QgsFileDropEdit: public QLineEdit
{
  public:
    QgsFileDropEdit( QWidget *parent = 0 );
    virtual ~QgsFileDropEdit();

    bool isDirOnly() const { return mDirOnly; }
    void setDirOnly( bool isDirOnly );

    bool isFileOnly() const { return mFileOnly; }
    void setFileOnly( bool isFileOnly );

    const QString& suffixFilter() const { return mSuffix; }
    void setSuffixFilter( const QString& suffix );

  protected:

    virtual void dragEnterEvent( QDragEnterEvent *event );
    virtual void dragLeaveEvent( QDragLeaveEvent *event );
    virtual void dropEvent( QDropEvent *event );
    virtual void paintEvent( QPaintEvent *e );

  private:
    QString acceptableFilePath( QDropEvent *event ) const;

    QString mSuffix;
    bool mDirOnly;
    bool mFileOnly;
    bool mDragActive;
};

#endif
