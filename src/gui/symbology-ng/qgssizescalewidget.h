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
#include "ui_widget_size_scale.h"
#include <QStandardItemModel>
#include <QItemDelegate>
#include "qgis_gui.h"
#include "qgsproperty.h"

class QgsVectorLayer;
class QgsSymbol;
class QgsLayerTreeLayer;
class QgsScaleExpression;
class QgsAbstractProperty;
class QgsMapCanvas;

/** \ingroup gui
 * \class QgsDataDefinedAssistant
 * An assistant (wizard) dialog, accessible from a QgsDataDefinedButton.
 * Can be used to guide users through creation of an expression for the
 * data defined button.
 * @note added in 2.10
 */
class GUI_EXPORT QgsDataDefinedAssistant: public QDialog
{
    Q_OBJECT

  public:
    QgsDataDefinedAssistant() : mMapCanvas( nullptr ) {}

    /**
     * Returns the property which was defined by this assistant.
     */
    virtual QgsProperty property() const = 0;

    /** Sets the map canvas associated with the widget. This allows the widget to retrieve the current
     * map scale and other properties from the canvas.
     * @param canvas map canvas
     * @see mapCanvas()
     * @note added in QGIS 2.12
     */
    virtual void setMapCanvas( QgsMapCanvas* canvas ) { mMapCanvas = canvas; }

    /** Returns the map canvas associated with the widget.
     * @see setMapCanvas
     * @note added in QGIS 2.12
     */
    const QgsMapCanvas* mapCanvas() const { return mMapCanvas; }

  protected:

    QgsMapCanvas* mMapCanvas;
};


/** \ingroup gui
 * \class QgsSizeScaleWidget
 */
class GUI_EXPORT QgsSizeScaleWidget : public QgsDataDefinedAssistant, private Ui_SizeScaleBase, private QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    QgsSizeScaleWidget( const QgsVectorLayer * layer, const QgsSymbol * symbol );

    QgsProperty property() const override;

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

    const QgsSymbol* mSymbol;
    QgsVectorLayer* mLayer;
    QgsLayerTreeLayer* mLayerTreeLayer;
    QgsLayerTreeGroup mRoot;
    QStandardItemModel mPreviewList;
    QgsMapCanvas* mMapCanvas;

    QgsScaleExpression* createExpression() const;
    void setFromSymbol();

    QgsExpressionContext createExpressionContext() const override;
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
