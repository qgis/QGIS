/***************************************************************************
                          qgssimadialog.h 
                         Single marker renderer dialog
                             -------------------
    begin                : March 2004
    copyright            : (C) 2004 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSSIMADIALOG_H
#define QGSSIMADIALOG_H
#ifdef WIN32
#include "qgssimadialogbase.h"
#else
#include "qgssimadialogbase.uic.h"
#endif

#include <qiconview.h>

class QgsVectorLayer;

class QgsSiMaDialog: public QgsSiMaDialogBase
{
    Q_OBJECT
public:
    QgsSiMaDialog(QgsVectorLayer* vectorlayer);
    ~QgsSiMaDialog();
    static QString defaultDir();
    void apply();
    void setMarker(const QString& file, double scaleFactor);
    const QString& getPicture() const;
    double getScaleFactor() const;
    
protected:
    QgsVectorLayer* mVectorLayer;
    QString mCurrentDir;


public slots:
    /**Brings up the file dialog and triggers visualizeMarkers*/
    void mBrowseDirectoriesButton_clicked();

private:
    /**File name of the selected marker*/
    QString mSelectedMarker;
    void mIconView_selectionChanged(QIconViewItem *);
    /**Renders the SVG pictures of directory to mIconView*/
    void visualizeMarkers(QString directory);
    void mScaleSpin_valueChanged( int theSize);

signals:
    void settingsChanged();
};

#endif
