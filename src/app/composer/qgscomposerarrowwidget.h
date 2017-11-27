/***************************************************************************
                         qgscomposerarrowwidget.h
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

#ifndef QGSCOMPOSERARROWWIDGET_H
#define QGSCOMPOSERARROWWIDGET_H

#include "ui_qgscomposerarrowwidgetbase.h"
#include "qgscomposeritemwidget.h"

class QgsComposerArrow;

class QgsComposerArrowWidget: public QgsComposerItemBaseWidget, private Ui::QgsComposerArrowWidgetBase
{
    Q_OBJECT
  public:
    explicit QgsComposerArrowWidget( QgsComposerArrow *arrow );

  private:
    QgsComposerArrow *mArrow = nullptr;

    void blockAllSignals( bool block );

    QButtonGroup *mRadioButtonGroup = nullptr;

    //! Enables / disables the SVG line inputs
    void enableSvgInputElements( bool enable );

    void updateLineSymbolMarker();

  private slots:
    void mStrokeWidthSpinBox_valueChanged( double d );
    void mArrowHeadWidthSpinBox_valueChanged( double d );
    void mArrowHeadFillColorButton_colorChanged( const QColor &newColor );
    void mArrowHeadStrokeColorButton_colorChanged( const QColor &newColor );
    void mDefaultMarkerRadioButton_toggled( bool toggled );
    void mNoMarkerRadioButton_toggled( bool toggled );
    void mSvgMarkerRadioButton_toggled( bool toggled );
    void mStartMarkerLineEdit_textChanged( const QString &text );
    void mEndMarkerLineEdit_textChanged( const QString &text );
    void mStartMarkerToolButton_clicked();
    void mEndMarkerToolButton_clicked();
    void mLineStyleButton_clicked();

    void setGuiElementValues();

    void updateLineStyleFromWidget();
    void cleanUpLineStyleSelector( QgsPanelWidget *container );
};

#endif // QGSCOMPOSERARROWWIDGET_H
