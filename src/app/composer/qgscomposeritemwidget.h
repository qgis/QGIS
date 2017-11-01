/***************************************************************************
                         qgscomposeritemwidget.h
                         -------------------------
    begin                : August 2008
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

#ifndef QGSCOMPOSERITEMWIDGET_H
#define QGSCOMPOSERITEMWIDGET_H

#include "ui_qgscomposeritemwidgetbase.h"
#include "qgscomposeritem.h"
#include "qgspanelwidget.h"

class QgsComposerItem;
class QgsAtlasComposition;


// NOTE - the inheritance here is tricky, as we need to avoid the multiple inheritance
// diamond problem and the ideal base object (QgsComposerConfigObject) MUST be a QObject
// because of its slots.

// So here we go:
// QgsComposerItemWidget is just a QWidget which is embedded inside specific item property
// widgets and contains common settings like position and rotation of the items. While the
// actual individual item type widgets MUST be QgsPanelWidgets unfortunately QgsComposerItemWidget
// CANNOT be a QgsPanelWidget and must instead be a generic QWidget (otherwise a QgsPanelWidget
// contains a child QgsPanelWidget, which breaks lots of assumptions made in QgsPanelWidget
// and related classes).
// So QgsComposerItemWidget HAS a QgsComposerConfigObject to handle these common tasks.
// Specific item property widgets (e.g., QgsComposerMapWidget) should inherit from QgsComposerItemBaseWidget
// (which is a QgsPanelWidget) and also HAS a QgsComposerConfigObject, with protected methods
// which are just proxied through to the QgsComposerConfigObject.
// phew!
// long story short - don't change this without good reason. If you add a new item type, inherit
// from QgsComposerItemWidget and trust that everything else has been done for you.

/**
 * An object for property widgets for composer items. All composer config type widgets should contain
 * this object.
*/
class QgsComposerConfigObject: public QObject
{
    Q_OBJECT
  public:
    QgsComposerConfigObject( QWidget *parent, QgsComposerObject *composerObject );

    /**
     * Registers a data defined button, setting up its initial value, connections and description.
     * \param button button to register
     * \param key corresponding data defined property key
     */
    void initializeDataDefinedButton( QgsPropertyOverrideButton *button, QgsComposerObject::DataDefinedProperty key );

    /**
     * Updates a data defined button to reflect the item's current properties.
     */
    void updateDataDefinedButton( QgsPropertyOverrideButton *button );

    //! Returns the current atlas coverage layer (if set)
    QgsVectorLayer *atlasCoverageLayer() const;

    //! Returns the atlas for the composition
    QgsAtlasComposition *atlasComposition() const;

  private slots:
    //! Must be called when a data defined button changes
    void updateDataDefinedProperty();

    //! Updates data defined buttons to reflect current state of atlas (e.g., coverage layer)
    void updateDataDefinedButtons();

  private:

    QgsComposerObject *mComposerObject = nullptr;
};

/**
 * A base class for property widgets for composer items. All composer item widgets should inherit from
 * this base class.
 */
class QgsComposerItemBaseWidget: public QgsPanelWidget
{
    Q_OBJECT

  public:
    QgsComposerItemBaseWidget( QWidget *parent, QgsComposerObject *composerObject );

  protected:

    /**
     * Registers a data defined button, setting up its initial value, connections and description.
     * \param button button to register
     * \param property corresponding data defined property key
     */
    void registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsComposerObject::DataDefinedProperty property );

    /**
     * Updates a previously registered data defined button to reflect the item's current properties.
     */
    void updateDataDefinedButton( QgsPropertyOverrideButton *button );

    //! Returns the current atlas coverage layer (if set)
    QgsVectorLayer *atlasCoverageLayer() const;

    //! Returns the atlas for the composition
    QgsAtlasComposition *atlasComposition() const;

  private:

    QgsComposerConfigObject *mConfigObject = nullptr;
};

/**
 * A class to enter generic properties for composer items (e.g. background, stroke, frame).
 This widget can be embedded into other item widgets*/
class QgsComposerItemWidget: public QWidget, private Ui::QgsComposerItemWidgetBase
{
    Q_OBJECT
  public:
    QgsComposerItemWidget( QWidget *parent, QgsComposerItem *item );

    //! A combination of upper/middle/lower and left/middle/right
    QgsComposerItem::ItemPositionMode positionMode() const;

    //! Toggles display of the background group
    void showBackgroundGroup( bool showGroup );
    //! Toggles display of the frame group
    void showFrameGroup( bool showGroup );

  public slots:

    /**
     * Set the frame color
     */
    void mFrameColorButton_colorChanged( const QColor &newFrameColor );
    void mBackgroundColorButton_clicked();

    /**
     * Set the background color
     */
    void mBackgroundColorButton_colorChanged( const QColor &newBackgroundColor );
//    void on_mTransparencySlider_valueChanged( int value );
//    void on_mTransparencySpinBox_valueChanged( int value );
    void mStrokeWidthSpinBox_valueChanged( double d );
    void mFrameGroupBox_toggled( bool state );
    void mFrameJoinStyleCombo_currentIndexChanged( int index );
    void mBackgroundGroupBox_toggled( bool state );
    void mItemIdLineEdit_editingFinished();

    //adjust coordinates in line edits
    void mPageSpinBox_valueChanged( int );
    void mXPosSpin_valueChanged( double );
    void mYPosSpin_valueChanged( double );
    void mWidthSpin_valueChanged( double );
    void mHeightSpin_valueChanged( double );

    void mUpperLeftCheckBox_stateChanged( int state );
    void mUpperMiddleCheckBox_stateChanged( int state );
    void mUpperRightCheckBox_stateChanged( int state );
    void mMiddleLeftCheckBox_stateChanged( int state );
    void mMiddleCheckBox_stateChanged( int state );
    void mMiddleRightCheckBox_stateChanged( int state );
    void mLowerLeftCheckBox_stateChanged( int state );
    void mLowerMiddleCheckBox_stateChanged( int state );
    void mLowerRightCheckBox_stateChanged( int state );

    void mBlendModeCombo_currentIndexChanged( int index );
    void opacityChanged( double value );

    void mItemRotationSpinBox_valueChanged( double val );
    void mExcludeFromPrintsCheckBox_toggled( bool checked );

    void setValuesForGuiElements();
    //sets the values for all position related (x, y, width, height) elements
    void setValuesForGuiPositionElements();
    //sets the values for all non-position related elements
    void setValuesForGuiNonPositionElements();

  protected slots:
    //! Initializes data defined buttons to current atlas coverage layer
    void initializeDataDefinedButtons();
    //! Sets data defined button state to match item
    void populateDataDefinedButtons();

  private:

    QgsComposerItem *mItem = nullptr;
    QgsComposerConfigObject *mConfigObject = nullptr;

    bool mFreezeXPosSpin;
    bool mFreezeYPosSpin;
    bool mFreezeWidthSpin;
    bool mFreezeHeightSpin;
    bool mFreezePageSpin;

//    void changeItemTransparency( int value );
    void changeItemPosition();

  private slots:

    void variablesChanged();
    void updateVariables();
};

#endif //QGSCOMPOSERITEMWIDGET_H
