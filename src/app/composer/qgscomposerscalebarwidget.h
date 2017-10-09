/***************************************************************************
                            qgscomposerscalebarwidget.h
                            ---------------------------
    begin                : 11 June 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERSCALEBARWIDGET_H
#define QGSCOMPOSERSCALEBARWIDGET_H

#include "ui_qgscomposerscalebarwidgetbase.h"
#include "qgscomposeritemwidget.h"

class QgsComposerScaleBar;

/**
 * \ingroup app
 * A widget to define the properties of a QgsComposerScaleBarItem.
 */
class QgsComposerScaleBarWidget: public QgsComposerItemBaseWidget, private Ui::QgsComposerScaleBarWidgetBase
{
    Q_OBJECT

  public:
    explicit QgsComposerScaleBarWidget( QgsComposerScaleBar *scaleBar );

  public slots:

    void mHeightSpinBox_valueChanged( double d );
    void mLineWidthSpinBox_valueChanged( double d );
    void mSegmentSizeSpinBox_valueChanged( double d );
    void mSegmentsLeftSpinBox_valueChanged( int i );
    void mNumberOfSegmentsSpinBox_valueChanged( int i );
    void mUnitLabelLineEdit_textChanged( const QString &text );
    void mMapUnitsPerBarUnitSpinBox_valueChanged( double d );
    void mFontColorButton_colorChanged( const QColor &newColor );
    void mFillColorButton_colorChanged( const QColor &newColor );
    void mFillColor2Button_colorChanged( const QColor &newColor );
    void mStrokeColorButton_colorChanged( const QColor &newColor );
    void mStyleComboBox_currentIndexChanged( const QString &text );
    void mLabelBarSpaceSpinBox_valueChanged( double d );
    void mBoxSizeSpinBox_valueChanged( double d );
    void mAlignmentComboBox_currentIndexChanged( int index );
    void mUnitsComboBox_currentIndexChanged( int index );
    void mLineJoinStyleCombo_currentIndexChanged( int index );
    void mLineCapStyleCombo_currentIndexChanged( int index );
    void mMinWidthSpinBox_valueChanged( double d );
    void mMaxWidthSpinBox_valueChanged( double d );

  private slots:
    void setGuiElements();
    void segmentSizeRadioChanged( QAbstractButton *radio );
    void composerMapChanged( QgsComposerItem *item );
    void fontChanged();

  private:
    QgsComposerScaleBar *mComposerScaleBar = nullptr;
    QButtonGroup mSegmentSizeRadioGroup;

    //! Enables/disables the signals of the input gui elements
    void blockMemberSignals( bool enable );

    //! Enables/disables controls based on scale bar style
    void toggleStyleSpecificControls( const QString &style );

    void connectUpdateSignal();
    void disconnectUpdateSignal();
};

#endif //QGSCOMPOSERSCALEBARWIDGET_H
