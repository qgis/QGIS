/***************************************************************************
                            qgslayoutscalebarwidget.h
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

#ifndef QGSLAYOUTSCALEBARWIDGET_H
#define QGSLAYOUTSCALEBARWIDGET_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_gui.h"
#include "ui_qgslayoutscalebarwidgetbase.h"
#include "qgslayoutitemwidget.h"

#include <QButtonGroup>

class QgsLayoutItemScaleBar;

/**
 * \ingroup gui
 * \brief A widget to define the properties of a QgsLayoutItemScaleBar.
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutScaleBarWidget: public QgsLayoutItemBaseWidget, public QgsExpressionContextGenerator, private Ui::QgsLayoutScaleBarWidgetBase
{
    Q_OBJECT

  public:
    //! constructor
    explicit QgsLayoutScaleBarWidget( QgsLayoutItemScaleBar *scaleBar );
    void setMasterLayout( QgsMasterLayoutInterface *masterLayout ) override;

    QgsExpressionContext createExpressionContext() const override;

  protected:
    bool setNewItem( QgsLayoutItem *item ) override;

  private slots:
    void lineSymbolChanged();
    void divisionSymbolChanged();
    void subdivisionSymbolChanged();
    void fillSymbol1Changed();
    void fillSymbol2Changed();
    void mHeightSpinBox_valueChanged( double d );
    void mSegmentSizeSpinBox_valueChanged( double d );
    void mSegmentsLeftSpinBox_valueChanged( int i );
    void mNumberOfSegmentsSpinBox_valueChanged( int i );
    void mNumberOfSubdivisionsSpinBox_valueChanged( int i );
    void mSubdivisionsHeightSpinBox_valueChanged( double d );
    void mUnitLabelLineEdit_textChanged( const QString &text );
    void mMapUnitsPerBarUnitSpinBox_valueChanged( double d );
    void mStyleComboBox_currentIndexChanged( const QString &text );
    void mLabelBarSpaceSpinBox_valueChanged( double d );
    void mBoxSizeSpinBox_valueChanged( double d );
    void mLabelVerticalPlacementComboBox_currentIndexChanged( int index );
    void mLabelHorizontalPlacementComboBox_currentIndexChanged( int index );
    void alignmentChanged();
    void mUnitsComboBox_currentIndexChanged( int index );
    void mMinWidthSpinBox_valueChanged( double d );
    void mMaxWidthSpinBox_valueChanged( double d );

  private slots:
    void setGuiElements();
    void segmentSizeRadioChanged( QAbstractButton *radio );
    void mapChanged( QgsLayoutItem *item );
    void textFormatChanged();
    void changeNumberFormat();

  private:
    QPointer< QgsLayoutItemScaleBar > mScalebar;
    QgsLayoutItemPropertiesWidget *mItemPropertiesWidget = nullptr;

    QButtonGroup mSegmentSizeRadioGroup;

    //! Enables/disables the signals of the input gui elements
    void blockMemberSignals( bool enable );

    //! Enables/disables controls based on scale bar style
    void toggleStyleSpecificControls( const QString &style );

    void connectUpdateSignal();
    void disconnectUpdateSignal();
    void populateDataDefinedButtons();

};

#endif //QGSLAYOUTSCALEBARWIDGET_H
