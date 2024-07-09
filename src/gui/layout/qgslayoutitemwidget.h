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

class QgsLayoutDesignerInterface;
class QgsPropertyOverrideButton;
class QgsLayoutAtlas;
class QgsMasterLayoutInterface;

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
// which are just proxied through to the QgsLayoutConfigObject.
// phew!
// long story short - don't change this without good reason. If you add a new item type, inherit
// from QgsLayoutItemBaseWidget and trust that everything else has been done for you.

/**
 * \class QgsLayoutConfigObject
 * \ingroup gui
 *
 * \brief An object for property widgets for layout items. All layout config type widgets should contain
 * this object.
 *
 * If you are creating a new QgsLayoutItem configuration widget, you should instead
 * inherit from QgsLayoutItemBaseWidget (rather then directly working with QgsLayoutConfigObject).
 *
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

    //! Returns the atlas for the layout, if available
    QgsLayoutAtlas *layoutAtlas() const;

    /**
     * Links a new layout \a object to this QgsLayoutConfigObject. The object must be the same type as the existing
     * object.
     *
     * \note Not available in Python bindings
     * \since QGIS 3.4
     */
    void setObject( QgsLayoutObject *object ) SIP_SKIP;

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
 * \brief A base class for property widgets for layout items. All layout item widgets should inherit from
 * this base class.
 *
 *
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
     * Sets the current \a item to show in the widget. If TRUE is returned, \a item
     * was an acceptable type for display in this widget and the widget has been
     * updated to match \a item's properties.
     *
     * If FALSE is returned, then the widget could not be successfully updated
     * to show the properties of \a item.
     */
    bool setItem( QgsLayoutItem *item );

    /**
     * Sets the \a string to use to describe the current report type (e.g.
     * "atlas" or "report").
     * Subclasses which display this text to users should override this
     * and update their widget labels accordingly.
     */
    virtual void setReportTypeString( const QString &string );

    /**
     * Sets the the layout designer interface in which the widget is
     * being shown.
     *
     * \since QGIS 3.6
     */
    virtual void setDesignerInterface( QgsLayoutDesignerInterface *iface );

    /**
     * Sets the master layout associated with the item.
     *
     * \since QGIS 3.10
     */
    virtual void setMasterLayout( QgsMasterLayoutInterface *masterLayout );

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
     * Implementations must return TRUE if the item was accepted and
     * the widget was updated.
     */
    virtual bool setNewItem( QgsLayoutItem *item );

    //! Returns the atlas for the layout (if available)
    QgsLayoutAtlas *layoutAtlas() const;

  private:
    QgsLayoutConfigObject *mConfigObject = nullptr;

    QgsLayoutObject *mObject = nullptr;
};


/**
 * \class QgsLayoutItemPropertiesWidget
 * \ingroup gui
 * \brief A widget for controlling the common properties of layout items (e.g. position and size, background, stroke, frame).
 * This widget can be embedded into other layout item widgets.
*/
class GUI_EXPORT QgsLayoutItemPropertiesWidget: public QWidget, private Ui::QgsLayoutItemWidgetBase
{
    Q_OBJECT
  public:

    /**
     * Constructs a QgsLayoutItemPropertiesWidget with a \a parent and for the given layout \a item.
     */
    QgsLayoutItemPropertiesWidget( QWidget *parent, QgsLayoutItem *item );

    //! Returns the position mode
    QgsLayoutItem::ReferencePoint positionMode() const;

    //! Determines if the background of the group box shall be shown
    void showBackgroundGroup( bool showGroup );

    //! Determines if the frame of the group box shall be shown
    void showFrameGroup( bool showGroup );

    //! Sets the layout item
    void setItem( QgsLayoutItem *item );

    /**
     * Sets the master layout associated with the item.
     *
     * \since QGIS 3.10
     */
    void setMasterLayout( QgsMasterLayoutInterface *masterLayout );

    /**
     * Updates the variables widget, refreshing the values of variables shown.
     *
     * \since QGIS 3.10
     */
    void updateVariables();

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
    void strokeUnitChanged( Qgis::LayoutUnit unit );
    void mFrameGroupBox_toggled( bool state );
    void mFrameJoinStyleCombo_currentIndexChanged( int index );
    void mBackgroundGroupBox_toggled( bool state );
    void mItemIdLineEdit_editingFinished();
    void exportGroupNameEditingFinished();

    //adjust coordinates in line edits
    void mPageSpinBox_valueChanged( int );
    void mXPosSpin_valueChanged( double );
    void mYPosSpin_valueChanged( double );
    void positionUnitsChanged( Qgis::LayoutUnit unit );
    void mWidthSpin_valueChanged( double );
    void mHeightSpin_valueChanged( double );
    void sizeUnitsChanged( Qgis::LayoutUnit unit );

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

  private:

    QPointer< QgsLayoutItem > mItem;
    QgsLayoutConfigObject *mConfigObject = nullptr;

    bool mFreezeXPosSpin = false;
    bool mFreezeYPosSpin = false;
    bool mFreezeWidthSpin = false;
    bool mFreezeHeightSpin = false;
    bool mFreezePageSpin = false;
    bool mBlockVariableUpdates = false;
//    void changeItemTransparency( int value );
    void changeItemPosition();
    void changeItemReference( QgsLayoutItem::ReferencePoint point );
    void changeItemSize();

};


#endif // QGSLAYOUTITEMWIDGET_H
