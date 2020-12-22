/***************************************************************************
                         qgslayoutmanualtablewidget.h
                         ---------------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#ifndef QGSLAYOUTMANUALTABLEWIDGET_H
#define QGSLAYOUTMANUALTABLEWIDGET_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "ui_qgslayoutmanualtablewidgetbase.h"
#include "qgslayoutitemwidget.h"
#include "qgstableeditordialog.h"
#include <QPointer>

class QgsLayoutItemManualTable;
class QgsLayoutFrame;

/**
 * \ingroup gui
 * A widget for configuring layout manual table items.
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutManualTableWidget: public QgsLayoutItemBaseWidget, private Ui::QgsLayoutManualTableWidgetBase
{
    Q_OBJECT
  public:
    //! constructor
    QgsLayoutManualTableWidget( QgsLayoutFrame *frame );

    void setMasterLayout( QgsMasterLayoutInterface *masterLayout ) override;

  protected:

    bool setNewItem( QgsLayoutItem *item ) override;

  private:
    QPointer< QgsLayoutItemManualTable > mTable;
    QPointer< QgsLayoutFrame > mFrame;
    QgsLayoutItemPropertiesWidget *mItemPropertiesWidget = nullptr;

    QPointer< QgsTableEditorDialog > mEditorDialog;

    //! Blocks / unblocks the signals of all GUI elements
    void blockAllSignals( bool b );

  private slots:

    void setTableContents();
    void mMarginSpinBox_valueChanged( double d );
    void mGridStrokeWidthSpinBox_valueChanged( double d );
    void mGridColorButton_colorChanged( const QColor &newColor );
    void mBackgroundColorButton_colorChanged( const QColor &newColor );
    void headerFontChanged();
    void contentFontChanged();
    void mDrawHorizontalGrid_toggled( bool state );
    void mDrawVerticalGrid_toggled( bool state );
    void mShowGridGroupCheckBox_toggled( bool state );
    void mHeaderHAlignmentComboBox_currentIndexChanged( int index );
    void mHeaderModeComboBox_currentIndexChanged( int index );
    void mAddFramePushButton_clicked();
    void mResizeModeComboBox_currentIndexChanged( int index );
    void mDrawEmptyCheckBox_toggled( bool checked );
    void mEmptyFrameCheckBox_toggled( bool checked );
    void mHideEmptyBgCheckBox_toggled( bool checked );
    void mWrapBehaviorComboBox_currentIndexChanged( int index );
    void mAdvancedCustomizationButton_clicked();
    void updateGuiElements();

};

#endif // QGSLAYOUTMANUALTABLEWIDGET_H
