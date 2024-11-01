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

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_gui.h"
#include "ui_qgslayoutshapewidgetbase.h"
#include "qgslayoutitemwidget.h"
#include "qgslayoutitemshape.h"

/**
 * \ingroup gui
 * \brief A widget for configuring layout shape items.
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutShapeWidget : public QgsLayoutItemBaseWidget, private Ui::QgsLayoutShapeWidgetBase
{
    Q_OBJECT
  public:
    //! constructor
    explicit QgsLayoutShapeWidget( QgsLayoutItemShape *shape );
    void setMasterLayout( QgsMasterLayoutInterface *masterLayout ) override;

  protected:
    bool setNewItem( QgsLayoutItem *item ) override;


  private:
    QPointer<QgsLayoutItemShape> mShape;
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
