/***************************************************************************
 qgssizescalewidget.h - continuous size scale assistant

 ---------------------
 begin                : March 2015
 copyright            : (C) 2015 by Vincent Mora
 email                : vincent dot mora at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSIZESCALEWIDGET_H
#define QGSSIZESCALEWIDGET_H

#include "qgslayertreegroup.h"
#include "qgslayertreemodel.h"
#include "qgsdatadefinedbutton.h"
#include "ui_widget_size_scale.h"
#include <QStandardItemModel>

class QgsVectorLayer;
class QgsMarkerSymbolV2;
class QgsLayerTreeLayer;
class QgsScaleExpression;
class QgsDataDefined;

class GUI_EXPORT QgsSizeScaleWidget : public QgsDataDefinedAssistant, private Ui_SizeScaleBase
{
    Q_OBJECT

  public:
    QgsSizeScaleWidget( const QgsVectorLayer * layer, const QgsMarkerSymbolV2 * symbol );

    QgsDataDefined dataDefined() const override;

  protected:

    virtual void showEvent( QShowEvent * ) override;

  private slots:
    void computeFromLayerTriggered();
    void updatePreview();

  private:

    const QgsMarkerSymbolV2* mSymbol;
    QgsVectorLayer* mLayer;
    QgsLayerTreeLayer* mLayerTreeLayer;
    QgsLayerTreeGroup mRoot;
    QStandardItemModel mPreviewList;

    QgsScaleExpression* createExpression() const;
    void setFromSymbol();

};

#endif //QGSSIZESCALEWIDGET_H
