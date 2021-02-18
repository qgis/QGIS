/***************************************************************************
                         qgslayoutlabelwidget.h
                         ----------------------
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

#ifndef QGSLAYOUTLABELWIDGET_H
#define QGSLAYOUTLABELWIDGET_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_gui.h"
#include "ui_qgslayoutlabelwidgetbase.h"
#include "qgslayoutitemwidget.h"
#include "qgslayoutitemlabel.h"

/**
 * \ingroup gui
 * \brief A widget for layout item settings.
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutLabelWidget: public QgsLayoutItemBaseWidget, private Ui::QgsLayoutLabelWidgetBase
{
    Q_OBJECT
  public:
    //! constructor
    explicit QgsLayoutLabelWidget( QgsLayoutItemLabel *label );
    void setMasterLayout( QgsMasterLayoutInterface *masterLayout ) override;

  protected:

    bool setNewItem( QgsLayoutItem *item ) override;

  private slots:
    void mHtmlCheckBox_stateChanged( int i );
    void mTextEdit_textChanged();
    void mInsertExpressionButton_clicked();
    void mMarginXDoubleSpinBox_valueChanged( double d );
    void mMarginYDoubleSpinBox_valueChanged( double d );
    void mFontColorButton_colorChanged( const QColor &newLabelColor );
    void mCenterRadioButton_clicked();
    void mLeftRadioButton_clicked();
    void mRightRadioButton_clicked();
    void mTopRadioButton_clicked();
    void mBottomRadioButton_clicked();
    void mMiddleRadioButton_clicked();
    void setGuiElementValues();
    void fontChanged();
    void justifyClicked();

  private:
    QPointer< QgsLayoutItemLabel > mLabel = nullptr;

    QgsLayoutItemPropertiesWidget *mItemPropertiesWidget = nullptr;

    void blockAllSignals( bool block );
};

#endif //QGSLAYOUTLABELWIDGET_H
