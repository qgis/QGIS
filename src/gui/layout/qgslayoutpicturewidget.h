/***************************************************************************
                         qgslayoutpicturewidget.h
                         --------------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTPICTUREWIDGET_H
#define QGSLAYOUTPICTUREWIDGET_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_gui.h"
#include "ui_qgslayoutpicturewidgetbase.h"
#include "qgslayoutitemwidget.h"

class QgsLayoutItemPicture;

/**
 * \ingroup gui
 * \brief A widget for configuring layout picture items.
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutPictureWidget : public QgsLayoutItemBaseWidget, private Ui::QgsLayoutPictureWidgetBase
{
    Q_OBJECT

  public:
    //! constructor
    explicit QgsLayoutPictureWidget( QgsLayoutItemPicture *picture );
    void setMasterLayout( QgsMasterLayoutInterface *masterLayout ) override;

  private slots:
    void mPictureRotationSpinBox_valueChanged( double d );
    void mRotationFromComposerMapCheckBox_stateChanged( int state );
    void mapChanged( QgsLayoutItem *item );
    void mResizeModeComboBox_currentIndexChanged( int index );
    void mAnchorPointComboBox_currentIndexChanged( int index );

  protected:
    bool setNewItem( QgsLayoutItem *item ) override;

  protected slots:
    //! Initializes data defined buttons to current atlas coverage layer
    void populateDataDefinedButtons();

  private slots:
    //! Sets the GUI elements to the values of mPicture
    void setGuiElementValues();

    //! Sets the picture rotation GUI control value
    void setPicRotationSpinValue( double r );

    void mFillColorButton_colorChanged( const QColor &color );
    void mStrokeColorButton_colorChanged( const QColor &color );
    void mStrokeWidthSpinBox_valueChanged( double d );
    void mPictureRotationOffsetSpinBox_valueChanged( double d );
    void mNorthTypeComboBox_currentIndexChanged( int index );

    void sourceChanged( const QString &source );
    void setSvgDynamicParameters( const QMap<QString, QgsProperty> &parameters );
    void modeChanged( bool checked );

  private:
    QPointer<QgsLayoutItemPicture> mPicture;
    QgsLayoutItemPropertiesWidget *mItemPropertiesWidget = nullptr;

    void updateSvgParamGui( bool resetValues = true );
};

#endif // QGSLAYOUTPICTUREWIDGET_H
