/***************************************************************************
                             qgslayoutitemwidget.h
                             ------------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYOUTITEMWIDGET_H
#define QGSLAYOUTITEMWIDGET_H

#include "qgis_gui.h"
#include "qgslayoutobject.h"
#include "qgspanelwidget.h"
#include "qgslayoutitem.h"
#include "ui_qgslayoutitemwidgetbase.h"
#include <QObject>
#include <QPointer>


class QgsPropertyOverrideButton;

// NOTE - the inheritance here is tricky, as we need to avoid the multiple inheritance
// diamond problem and the ideal base object (QgsLayoutConfigObject) MUST be a QObject
// because of its slots.

// So here we go:
// QgsLayoutItemWidget is just a QWidget which is embedded inside specific item property
// widgets and contains common settings like position and rotation of the items. While the
// actual individual item type widgets MUST be QgsPanelWidgets unfortunately QgsLayoutItemWidget
// CANNOT be a QgsPanelWidget and must instead be a generic QWidget (otherwise a QgsPanelWidget
// contains a child QgsPanelWidget, which breaks lots of assumptions made in QgsPanelWidget
// and related classes).
// So QgsLayoutItemWidget HAS a QgsLayoutConfigObject to handle these common tasks.
// Specific item property widgets (e.g., QgsLayoutMapWidget) should inherit from QgsLayoutItemBaseWidget
// (which is a QgsPanelWidget) and also HAS a QgsLayoutConfigObject, with protected methods
// which are just proxied through to the QgsComposerConfigObject.
// phew!
// long story short - don't change this without good reason. If you add a new item type, inherit
// from QgsLayoutItemBaseWidget and trust that everything else has been done for you.

/**
 * \class QgsLayoutConfigObject
 * \ingroup gui
 *
 * An object for property widgets for layout items. All layout config type widgets should contain
 * this object.
 *
 * If you are creating a new QgsLayoutItem configuration widget, you should instead
 * inherit from QgsLayoutItemBaseWidget (rather then directly working with QgsLayoutConfigObject).
 *
 * \since QGIS 3.0
*/
class GUI_EXPORT QgsLayoutConfigObject: public QObject
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsLayoutConfigObject, linked with the specified \a layoutObject.
     */
    QgsLayoutConfigObject( QWidget *parent SIP_TRANSFERTHIS, QgsLayoutObject *layoutObject );

    /**
     * Registers a data defined \a button, setting up its initial value, connections and description.
     * The corresponding property \a key must be specified.
     */
    void initializeDataDefinedButton( QgsPropertyOverrideButton *button, QgsLayoutObject::DataDefinedProperty key );

    /**
     * Updates a data defined button to reflect the item's current properties.
     */
    void updateDataDefinedButton( QgsPropertyOverrideButton *button );

    /**
     * Returns the current layout context coverage layer (if set).
     */
    QgsVectorLayer *coverageLayer() const;

#if 0 //TODO
    //! Returns the atlas for the composition
    QgsAtlasComposition *atlasComposition() const;
#endif

  private slots:
    //! Must be called when a data defined button changes
    void updateDataDefinedProperty();

    //! Updates data defined buttons to reflect current state of layout (e.g., coverage layer)
    void updateDataDefinedButtons();

  private:

    QPointer< QgsLayoutObject > mLayoutObject;
};

/**
 * \class QgsLayoutItemBaseWidget
 * \ingroup gui
 *
 * A base class for property widgets for layout items. All layout item widgets should inherit from
 * this base class.
 *
 *
 * \since QGIS 3.0
*/
class GUI_EXPORT QgsLayoutItemBaseWidget: public QgsPanelWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutItemBaseWidget, linked with the specified \a layoutObject.
     */
    QgsLayoutItemBaseWidget( QWidget *parent SIP_TRANSFERTHIS, QgsLayoutObject *layoutObject );

    /**
     * Returns the layout object associated with this widget.
     */
    QgsLayoutObject *layoutObject();

    /**
     * Sets the current \a item to show in the widget. If true is returned, \a item
     * was an acceptable type for display in this widget and the widget has been
     * updated to match \a item's properties.
     *
     * If false is returned, then the widget could not be successfully updated
     * to show the properties of \a item.
     */
    bool setItem( QgsLayoutItem *item );

  protected:

    /**
     * Registers a data defined \a button, setting up its initial value, connections and description.
     * The corresponding property \a key must be specified.
     */
    void registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsLayoutObject::DataDefinedProperty property );

    /**
     * Updates a previously registered data defined button to reflect the item's current properties.
     */
    void updateDataDefinedButton( QgsPropertyOverrideButton *button );

    /**
     * Returns the current layout context coverage layer (if set).
     */
    QgsVectorLayer *coverageLayer() const;

    /**
     * Attempts to update the widget to show the properties
     * for the specified \a item.
     *
     * Subclasses can override this if they support changing items in place.
     *
     * Implementations must return true if the item was accepted and
     * the widget was updated.
     */
    virtual bool setNewItem( QgsLayoutItem *item );


#if 0 //TODO
    //! Returns the atlas for the composition
    QgsAtlasComposition *atlasComposition() const;
#endif

  private:

    QgsLayoutConfigObject *mConfigObject = nullptr;

    QgsLayoutObject *mObject = nullptr;
};


/**
 * \class QgsLayoutItemPropertiesWidget
 * \ingroup gui
 * A widget for controlling the common properties of layout items (e.g. position and size, background, stroke, frame).
 * This widget can be embedded into other layout item widgets.
 * \since QGIS 3.0
*/
class GUI_EXPORT QgsLayoutItemPropertiesWidget: public QWidget, private Ui::QgsLayoutItemWidgetBase
{
    Q_OBJECT
  public:
    QgsLayoutItemPropertiesWidget( QWidget *parent, QgsLayoutItem *item );

    QgsLayoutItem::ReferencePoint positionMode() const;

    void showBackgroundGroup( bool showGroup );

    void showFrameGroup( bool showGroup );

    void setItem( QgsLayoutItem *item );

  protected slots:
    //! Initializes data defined buttons to current atlas coverage layer
    void initializeDataDefinedButtons();
    //! Sets data defined button state to match item
    void populateDataDefinedButtons();

  private slots:

    /**
     * Set the frame color
     */
    void mFrameColorButton_colorChanged( const QColor &newFrameColor );

    /**
     * Set the background color
     */
    void mBackgroundColorButton_colorChanged( const QColor &newBackgroundColor );
//    void on_mTransparencySlider_valueChanged( int value );
//    void on_mTransparencySpinBox_valueChanged( int value );
    void mStrokeWidthSpinBox_valueChanged( double d );
    void strokeUnitChanged( QgsUnitTypes::LayoutUnit unit );
    void mFrameGroupBox_toggled( bool state );
    void mFrameJoinStyleCombo_currentIndexChanged( int index );
    void mBackgroundGroupBox_toggled( bool state );
    void mItemIdLineEdit_editingFinished();

    //adjust coordinates in line edits
    void mPageSpinBox_valueChanged( int );
    void mXPosSpin_valueChanged( double );
    void mYPosSpin_valueChanged( double );
    void positionUnitsChanged( QgsUnitTypes::LayoutUnit unit );
    void mWidthSpin_valueChanged( double );
    void mHeightSpin_valueChanged( double );
    void sizeUnitsChanged( QgsUnitTypes::LayoutUnit unit );

    void mUpperLeftCheckBox_stateChanged( bool state );
    void mUpperMiddleCheckBox_stateChanged( bool state );
    void mUpperRightCheckBox_stateChanged( bool state );
    void mMiddleLeftCheckBox_stateChanged( bool state );
    void mMiddleCheckBox_stateChanged( bool state );
    void mMiddleRightCheckBox_stateChanged( bool state );
    void mLowerLeftCheckBox_stateChanged( bool state );
    void mLowerMiddleCheckBox_stateChanged( bool state );
    void mLowerRightCheckBox_stateChanged( bool state );

    void mBlendModeCombo_currentIndexChanged( int index );
    void opacityChanged( double value );

    void mItemRotationSpinBox_valueChanged( double val );
    void mExcludeFromPrintsCheckBox_toggled( bool checked );

    void setValuesForGuiElements();
    //sets the values for all position related (x, y, width, height) elements
    void setValuesForGuiPositionElements();
    //sets the values for all non-position related elements
    void setValuesForGuiNonPositionElements();

    void variablesChanged();
    void updateVariables();

  private:

    QgsLayoutItem *mItem = nullptr;
    QgsLayoutConfigObject *mConfigObject = nullptr;

    bool mFreezeXPosSpin = false;
    bool mFreezeYPosSpin = false;
    bool mFreezeWidthSpin = false;
    bool mFreezeHeightSpin = false;
    bool mFreezePageSpin = false;

//    void changeItemTransparency( int value );
    void changeItemPosition();
    void changeItemReference( QgsLayoutItem::ReferencePoint point );
    void changeItemSize();

};


#endif // QGSLAYOUTITEMWIDGET_H
