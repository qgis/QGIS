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
#include <QItemDelegate>

class QgsVectorLayer;
class QgsSymbolV2;
class QgsLayerTreeLayer;
class QgsScaleExpression;
class QgsDataDefined;
class QgsMapCanvas;

/** \ingroup gui
 * \class QgsSizeScaleWidget
 */
class GUI_EXPORT QgsSizeScaleWidget : public QgsDataDefinedAssistant, private Ui_SizeScaleBase
{
    Q_OBJECT

  public:
    QgsSizeScaleWidget( const QgsVectorLayer * layer, const QgsSymbolV2 * symbol );

    QgsDataDefined dataDefined() const override;

    /** Returns the vector layer associated with the widget.
     * @note added in QGIS 2.12
     */
    const QgsVectorLayer* layer() const { return mLayer; }

  protected:

    virtual void showEvent( QShowEvent * ) override;

  private slots:
    void computeFromLayerTriggered();
    void updatePreview();

  private:

    const QgsSymbolV2* mSymbol;
    QgsVectorLayer* mLayer;
    QgsLayerTreeLayer* mLayerTreeLayer;
    QgsLayerTreeGroup mRoot;
    QStandardItemModel mPreviewList;
    QgsMapCanvas* mMapCanvas;

    QgsScaleExpression* createExpression() const;
    void setFromSymbol();

};

/// @cond PRIVATE
class ItemDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    explicit ItemDelegate( QStandardItemModel* model ) : mModel( model ) {}

    QSize sizeHint( const QStyleOptionViewItem& /*option*/, const QModelIndex & index ) const override
    {
      return mModel->item( index.row() )->icon().actualSize( QSize( 512, 512 ) );
    }

  private:
    QStandardItemModel* mModel;

};

///@endcond

#endif //QGSSIZESCALEWIDGET_H
