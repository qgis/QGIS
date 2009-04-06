/***************************************************************************
                         qgssvgdiagramfactorywidget.h  -  description
                         --------------------------
    begin                : December 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSVGDIAGRAMFACTORYWIDGET_H
#define QGSSVGDIAGRAMFACTORYWIDGET_H

#include "qgsdiagramfactorywidget.h"
#include "ui_qgssvgdiagramfactorywidgetbase.h"


/**A widget that queries the SVG filename for proportional SVG symbols*/
class QgsSVGDiagramFactoryWidget: public QgsDiagramFactoryWidget, private Ui::QgsSVGDiagramFactoryWidgetBase
{
    Q_OBJECT

  public:
    QgsSVGDiagramFactoryWidget();
    ~QgsSVGDiagramFactoryWidget();

    QgsDiagramFactory* createFactory();
    void setExistingFactory( const QgsDiagramFactory* f );

  public slots:
    void on_mPictureBrowseButton_clicked();
    void on_mPreviewListWidget_currentItemChanged( QListWidgetItem* current, QListWidgetItem* previous );
    void on_mAddDirectoryButton_clicked();
    void on_mRemoveDirectoryButton_clicked();

  private:

    /**Add the icons of a directory to the preview. Returns 0 in case of success*/
    int addDirectoryToPreview( const QString& path );
    /**Add the icons of the standard directories to the preview*/
    void addStandardDirectoriesToPreview();
    /**Tests if a file is valid svg*/
    bool testSvgFile( const QString& filename ) const;
};

#endif
