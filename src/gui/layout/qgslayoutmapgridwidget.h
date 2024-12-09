/***************************************************************************
                         qgslayoutmapgridwidget.h
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

#ifndef QGSLAYOUTMAPGRIDWIDGET_H
#define QGSLAYOUTMAPGRIDWIDGET_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_gui.h"
#include "ui_qgslayoutmapgridwidgetbase.h"
#include "qgslayoutitemwidget.h"
#include "qgslayoutitemmapgrid.h"

/**
 * \ingroup gui
 * \brief A widget for configuring layout map grid items.
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutMapGridWidget : public QgsLayoutItemBaseWidget, private Ui::QgsLayoutMapGridWidgetBase
{
    Q_OBJECT

  public:
    //! constructor
    explicit QgsLayoutMapGridWidget( QgsLayoutItemMapGrid *mapGrid, QgsLayoutItemMap *map );

  private slots:

    void setGridItems();
    void mIntervalXSpinBox_editingFinished();
    void mIntervalYSpinBox_editingFinished();
    void mOffsetXSpinBox_valueChanged( double value );
    void mOffsetYSpinBox_valueChanged( double value );
    void mCrossWidthSpinBox_valueChanged( double val );
    void mFrameWidthSpinBox_valueChanged( double val );
    void mRotatedTicksGroupBox_toggled( bool checked );
    void mRotatedTicksLengthModeComboBox_currentIndexChanged( int );
    void mRotatedTicksThresholdSpinBox_valueChanged( double val );
    void mRotatedTicksMarginToCornerSpinBox_valueChanged( double val );
    void mRotatedAnnotationsGroupBox_toggled( bool checked );
    void mRotatedAnnotationsLengthModeComboBox_currentIndexChanged( int );
    void mRotatedAnnotationsThresholdSpinBox_valueChanged( double val );
    void mRotatedAnnotationsMarginToCornerSpinBox_valueChanged( double val );
    void mGridFrameMarginSpinBox_valueChanged( double val );
    void mFrameStyleComboBox_currentIndexChanged( int );
    void mGridFramePenSizeSpinBox_valueChanged( double d );
    void mGridFramePenColorButton_colorChanged( const QColor &newColor );
    void mGridFrameFill1ColorButton_colorChanged( const QColor &newColor );
    void mGridFrameFill2ColorButton_colorChanged( const QColor &newColor );
    void mGridTypeComboBox_currentIndexChanged( int );
    void mapGridCrsChanged( const QgsCoordinateReferenceSystem &crs );
    void mGridBlendComboBox_currentIndexChanged( int index );
    void mCheckGridLeftSide_toggled( bool checked );
    void mCheckGridRightSide_toggled( bool checked );
    void mCheckGridTopSide_toggled( bool checked );
    void mCheckGridBottomSide_toggled( bool checked );

    //frame divisions display
    void mFrameDivisionsLeftComboBox_currentIndexChanged( int index );
    void mFrameDivisionsRightComboBox_currentIndexChanged( int index );
    void mFrameDivisionsTopComboBox_currentIndexChanged( int index );
    void mFrameDivisionsBottomComboBox_currentIndexChanged( int index );

    void mDrawAnnotationGroupBox_toggled( bool state );
    void mAnnotationFormatButton_clicked();

    //annotation display
    void mAnnotationDisplayLeftComboBox_currentIndexChanged( int );
    void mAnnotationDisplayRightComboBox_currentIndexChanged( int );
    void mAnnotationDisplayTopComboBox_currentIndexChanged( int );
    void mAnnotationDisplayBottomComboBox_currentIndexChanged( int );

    //annotation position
    void mAnnotationPositionLeftComboBox_currentIndexChanged( int );
    void mAnnotationPositionRightComboBox_currentIndexChanged( int );
    void mAnnotationPositionTopComboBox_currentIndexChanged( int );
    void mAnnotationPositionBottomComboBox_currentIndexChanged( int );

    //annotation direction
    void mAnnotationDirectionComboBoxLeft_currentIndexChanged( int index );
    void mAnnotationDirectionComboBoxRight_currentIndexChanged( int index );
    void mAnnotationDirectionComboBoxTop_currentIndexChanged( int index );
    void mAnnotationDirectionComboBoxBottom_currentIndexChanged( int index );

    void mAnnotationFormatComboBox_currentIndexChanged( int index );
    void mCoordinatePrecisionSpinBox_valueChanged( int value );
    void mDistanceToMapFrameSpinBox_valueChanged( double d );

  protected:
    //! Sets the current composer map values to the GUI elements
    virtual void updateGuiElements();

  protected slots:
    //! Initializes data defined buttons to current atlas coverage layer
    void populateDataDefinedButtons();

  private slots:

    //! Sets the GUI elements to the values of mPicture
    void setGuiElementValues();
    void lineSymbolChanged();
    void markerSymbolChanged();
    void gridEnabledToggled( bool active );
    void intervalUnitChanged( int index );
    void minIntervalChanged( double interval );
    void maxIntervalChanged( double interval );
    void annotationTextFormatChanged();
    void onCrsChanged();

  private:
    QPointer<QgsLayoutItemMap> mMap;
    QPointer<QgsLayoutItemMapGrid> mMapGrid;
    int mBlockAnnotationFormatUpdates = 0;

    //! Blocks / unblocks the signals of all GUI elements
    void blockAllSignals( bool b );

    void handleChangedFrameDisplay( QgsLayoutItemMapGrid::BorderSide border, QgsLayoutItemMapGrid::DisplayMode mode );
    void handleChangedAnnotationDisplay( QgsLayoutItemMapGrid::BorderSide border, QgsLayoutItemMapGrid::DisplayMode mode );
    void handleChangedAnnotationPosition( QgsLayoutItemMapGrid::BorderSide border, QgsLayoutItemMapGrid::AnnotationPosition position );
    void handleChangedAnnotationDirection( QgsLayoutItemMapGrid::BorderSide border, QgsLayoutItemMapGrid::AnnotationDirection direction );

    void insertFrameDisplayEntries( QComboBox *c );
    void insertAnnotationDisplayEntries( QComboBox *c );
    void insertAnnotationPositionEntries( QComboBox *c );
    void insertAnnotationDirectionEntries( QComboBox *c );

    void initFrameDisplayBox( QComboBox *c, QgsLayoutItemMapGrid::DisplayMode display );
    void initAnnotationPositionBox( QComboBox *c, QgsLayoutItemMapGrid::AnnotationPosition pos );
    void initAnnotationDirectionBox( QComboBox *c, QgsLayoutItemMapGrid::AnnotationDirection dir );

    //! Enables/disables grid frame related controls
    void toggleFrameControls( bool frameEnabled, bool frameFillEnabled, bool frameSizeEnabled, bool ticksRotationEnabled );

    //! Is there some predefined scales, globally or as project's options ?
    bool hasPredefinedScales() const;
};

#endif //QGSLAYOUTMAPGRIDWIDGET_H
