/***************************************************************************
                         qgssinglebandpseudocolorrendererwidget.h
                         ----------------------------------------
    begin                : February 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSINGLEBANDCOLORRENDERERWIDGET_H
#define QGSSINGLEBANDCOLORRENDERERWIDGET_H

#include "qgsrasterrendererwidget.h"
#include "qgscolorrampshader.h"
#include "ui_qgssinglebandpseudocolorrendererwidgetbase.h"

class QgsSingleBandPseudoColorRendererWidget: public QgsRasterRendererWidget,
      private Ui::QgsSingleBandPseudoColorRendererWidgetBase
{
    Q_OBJECT
  public:
    QgsSingleBandPseudoColorRendererWidget( QgsRasterLayer* layer );
    ~QgsSingleBandPseudoColorRendererWidget();

    static QgsRasterRendererWidget* create( QgsRasterLayer* layer ) { return new QgsSingleBandPseudoColorRendererWidget( layer ); }
    QgsRasterRenderer* renderer();

  private:
    void populateColormapTreeWidget( const QList<QgsColorRampShader::ColorRampItem>& colorRampItems );

  private slots:
    void on_mAddEntryButton_clicked();
    void on_mDeleteEntryButton_clicked();
    void on_mSortButton_clicked();
    void on_mClassifyButton_clicked();
    void on_mLoadFromBandButton_clicked();
    void on_mLoadFromFileButton_clicked();
    void on_mExportToFileButton_clicked();
    void on_mColormapTreeWidget_itemDoubleClicked( QTreeWidgetItem* item, int column );
};

#endif // QGSSINGLEBANDCOLORRENDERERWIDGET_H
