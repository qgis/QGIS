/***************************************************************************
                         qgsmarkerdialog.h  -  description
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
/* $Id */


#ifndef QGSMARKERDIALOG_H
#define QGSMARKERDIALOG_H

#include "qgsmarkerdialogbase.uic.h"
#include <qdir.h>

/**A class for choosing marker images by clicking icons*/
class QgsMarkerDialog: public QgsMarkerDialogBase
{
    Q_OBJECT
 public:
    /**Constructor. If no directory is specified, the home directory is used*/
    QgsMarkerDialog(QString startdir=QDir::homeDirPath());
    /**Destructor*/
    virtual ~QgsMarkerDialog();
    /**Returns the path of the selected SVG marker*/
    QString selectedMarker();
    
    public slots:
    /**Brings up the file dialog and triggers visualizeMarkers*/
    void changeDirectory();
    /**Queries mIconView for selected marker symbols*/
    void updateSelectedMarker();

 protected:
	/**Current Directory*/
	QString mCurrentDir;
	/**File name of the selected marker*/
	QString mSelectedMarker;
	/**Renders the SVG pictures of directory to mIconView*/
	void visualizeMarkers(QString directory);
};

#endif
