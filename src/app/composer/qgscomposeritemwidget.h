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
class QgsDataDefinedButton;


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
// Specific item property widgets (eg QgsComposerMapWidget) should inherit from QgsComposerItemBaseWidget
// (which is a QgsPanelWidget) and also HAS a QgsComposerConfigObject, with protected methods
// which are just proxied through to the QgsComposerConfigObject.
// phew!
// long story short - don't change this without good reason. If you add a new item type, inherit
// from QgsComposerItemWidget and trust that everything else has been done for you.

/** An object for property widgets for composer items. All composer config type widgets should contain
 * this object.
*/
class QgsComposerConfigObject: public QObject
{
    Q_OBJECT
  public:
    QgsComposerConfigObject( QWidget* parent, QgsComposerObject* composerObject );
    ~QgsComposerConfigObject();

    /** Sets a data defined property for the item from its current data defined button settings*/
    void setDataDefinedProperty( const QgsDataDefinedButton *ddBtn, QgsComposerObject::DataDefinedProperty p );

    /** Registers a data defined button, setting up its initial value, connections and description.
     * @param button button to register
     * @param property correponding data defined property
     * @param type valid data types for button
     * @param description user visible description for data defined property
     */
    void registerDataDefinedButton( QgsDataDefinedButton* button, QgsComposerObject::DataDefinedProperty property,
                                    QgsDataDefinedButton::DataType type, const QString& description );

    /** Returns the current atlas coverage layer (if set)*/
    QgsVectorLayer* atlasCoverageLayer() const;

    /** Returns the atlas for the composition*/
    QgsAtlasComposition *atlasComposition() const;

  private slots:
    /** Must be called when a data defined button changes*/
    void updateDataDefinedProperty();

    //! Updates data defined buttons to reflect current state of atlas (eg coverage layer)
    void updateDataDefinedButtons();

  private:

    QgsComposerObject* mComposerObject;
};

/**
 * A base class for property widgets for composer items. All composer item widgets should inherit from
 * this base class.
 */
class QgsComposerItemBaseWidget: public QgsPanelWidget
{
    Q_OBJECT

  public:
    QgsComposerItemBaseWidget( QWidget* parent, QgsComposerObject* composerObject );

  protected:

    /** Registers a data defined button, setting up its initial value, connections and description.
     * @param button button to register
     * @param property correponding data defined property
     * @param type valid data types for button
     * @param description user visible description for data defined property
     */
    void registerDataDefinedButton( QgsDataDefinedButton* button, QgsComposerObject::DataDefinedProperty property,
                                    QgsDataDefinedButton::DataType type, const QString& description );

    /** Returns the current atlas coverage layer (if set)*/
    QgsVectorLayer* atlasCoverageLayer() const;

    /** Returns the atlas for the composition*/
    QgsAtlasComposition *atlasComposition() const;

  private:

    QgsComposerConfigObject* mConfigObject;
};

/** A class to enter generic properties for composer items (e.g. background, outline, frame).
 This widget can be embedded into other item widgets*/
class QgsComposerItemWidget: public QWidget, private Ui::QgsComposerItemWidgetBase
{
    Q_OBJECT
  public:
    QgsComposerItemWidget( QWidget* parent, QgsComposerItem* item );
    ~QgsComposerItemWidget();

    /** A combination of upper/middle/lower and left/middle/right*/
    QgsComposerItem::ItemPositionMode positionMode() const;

    /** Toggles display of the background group*/
    void showBackgroundGroup( bool showGroup );
    /** Toggles display of the frame group*/
    void showFrameGroup( bool showGroup );

  public slots:

    /** Set the frame color
     */
    void on_mFrameColorButton_colorChanged( const QColor& newFrameColor );
    void on_mBackgroundColorButton_clicked();
    /** Set the background color
     */
    void on_mBackgroundColorButton_colorChanged( const QColor& newBackgroundColor );
//    void on_mTransparencySlider_valueChanged( int value );
//    void on_mTransparencySpinBox_valueChanged( int value );
    void on_mOutlineWidthSpinBox_valueChanged( double d );
    void on_mFrameGroupBox_toggled( bool state );
    void on_mFrameJoinStyleCombo_currentIndexChanged( int index );
    void on_mBackgroundGroupBox_toggled( bool state );
    void on_mItemIdLineEdit_editingFinished();

    //adjust coordinates in line edits
    void on_mPageSpinBox_valueChanged( int );
    void on_mXPosSpin_valueChanged( double );
    void on_mYPosSpin_valueChanged( double );
    void on_mWidthSpin_valueChanged( double );
    void on_mHeightSpin_valueChanged( double );

    void on_mUpperLeftCheckBox_stateChanged( int state );
    void on_mUpperMiddleCheckBox_stateChanged( int state );
    void on_mUpperRightCheckBox_stateChanged( int state );
    void on_mMiddleLeftCheckBox_stateChanged( int state );
    void on_mMiddleCheckBox_stateChanged( int state );
    void on_mMiddleRightCheckBox_stateChanged( int state );
    void on_mLowerLeftCheckBox_stateChanged( int state );
    void on_mLowerMiddleCheckBox_stateChanged( int state );
    void on_mLowerRightCheckBox_stateChanged( int state );

    void on_mBlendModeCombo_currentIndexChanged( int index );
    void on_mTransparencySpnBx_valueChanged( int value );

    void on_mItemRotationSpinBox_valueChanged( double val );
    void on_mExcludeFromPrintsCheckBox_toggled( bool checked );

    void setValuesForGuiElements();
    //sets the values for all position related (x, y, width, height) elements
    void setValuesForGuiPositionElements();
    //sets the values for all non-position related elements
    void setValuesForGuiNonPositionElements();

  protected slots:
    /** Initializes data defined buttons to current atlas coverage layer*/
    void populateDataDefinedButtons();

  private:

    QgsComposerItem* mItem;
    QgsComposerConfigObject* mConfigObject;

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
