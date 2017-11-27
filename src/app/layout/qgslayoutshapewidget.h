/***************************************************************************
                         qgslayoutshapewidget.h
                         ------------------------
    begin                : November 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco@hugis.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTSHAPEWIDGET_H
#define QGSLAYOUTSHAPEWIDGET_H

#include "ui_qgslayoutshapewidgetbase.h"
#include "qgslayoutitemwidget.h"
#include "qgslayoutitemshape.h"

/**
 * \ingroup app
 * Input widget for the configuration of QgsLayoutItemShape
*/
class QgsLayoutShapeWidget: public QgsLayoutItemBaseWidget, private Ui::QgsLayoutShapeWidgetBase
{
    Q_OBJECT
  public:
    explicit QgsLayoutShapeWidget( QgsLayoutItemShape *shape );

  protected:

    bool setNewItem( QgsLayoutItem *item ) override;


  private:
    QgsLayoutItemShape *mShape = nullptr;
    QgsLayoutItemPropertiesWidget *mItemPropertiesWidget = nullptr;

    //! Blocks / unblocks the signal of all GUI elements
    void blockAllSignals( bool block );

  private slots:
    void mShapeComboBox_currentIndexChanged( const QString &text );
    void mCornerRadiusSpinBox_valueChanged( double val );
    void radiusUnitsChanged();
    void symbolChanged();

    //! Sets the GUI elements to the currentValues of mComposerShape
    void setGuiElementValues();

    //! Enables or disables the rounded radius spin box based on shape type
    void toggleRadiusSpin( QgsLayoutItemShape::Shape shape );
};

#endif // QGSLAYOUTSHAPEWIDGET_H
