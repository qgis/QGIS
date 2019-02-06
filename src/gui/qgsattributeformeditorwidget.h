/***************************************************************************
    qgsattributeformeditorwidget.h
     -----------------------------
    Date                 : March 2016
    Copyright            : (C) 2016 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTEFORMEDITORWIDGET_H
#define QGSATTRIBUTEFORMEDITORWIDGET_H

#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgseditorwidgetwrapper.h"
#include "qgsattributeformwidget.h"

class QgsEditorWidgetWrapper;
class QgsMultiEditToolButton;
class QgsSearchWidgetToolButton;
class QgsVectorLayer;
class QStackedWidget;
class QgsAttributeEditorContext;
class QLabel;
class QgsAggregateToolButton;

/**
 * \ingroup gui
 * \class QgsAttributeFormEditorWidget
 * A widget consisting of both an editor widget and additional widgets for controlling the behavior
 * of the editor widget depending on a number of possible modes. For instance, if the parent attribute
 * form is in the multi edit mode, this widget will show both the editor widget and a tool button for
 * controlling the multi edit results.
 * \since QGIS 2.16
 */
class GUI_EXPORT QgsAttributeFormEditorWidget : public QgsAttributeFormWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsAttributeFormEditorWidget.
     * \param editorWidget associated editor widget wrapper (for default/edit modes)
     * \param widgetType the type identifier of the widget passed in the
     *        wrapper
     * \param form parent attribute form
     */
    explicit QgsAttributeFormEditorWidget( QgsEditorWidgetWrapper *editorWidget, const QString &widgetType,
                                           QgsAttributeForm *form  SIP_TRANSFERTHIS );

    ~QgsAttributeFormEditorWidget() override;

    void createSearchWidgetWrappers( const QgsAttributeEditorContext &context = QgsAttributeEditorContext() ) override;

    /**
     * Resets the widget to an initial value.
     * \param initialValue initial value to show in widget
     * \param mixedValues set to true to initially show the mixed values state
     */
    void initialize( const QVariant &initialValue, bool mixedValues = false );

    /**
     * Returns true if the widget's value has been changed since it was initialized.
     * \see initialize()
     */
    bool hasChanged() const { return mIsChanged; }

    /**
     * Returns the current value of the attached editor widget.
     */
    QVariant currentValue() const;

    /**
     * Set the constraint status for this widget.
     */
    void setConstraintStatus( const QString &constraint, const QString &description, const QString &err, QgsEditorWidgetWrapper::ConstraintResult result );

    /**
     * Set the constraint result label visible or invisible according to the layer editable status
     */
    void setConstraintResultVisible( bool editable );

  public slots:

    /**
     * Sets whether the widget should be displayed in a "mixed values" mode.
     * \param mixed set to true to show in a mixed values state
     */
    void setIsMixed( bool mixed );

    /**
     * Called when field values have been committed;
     */
    void changesCommitted();

  signals:

    /**
     * Emitted when the widget's value changes
     * \param value new widget value
     */
    void valueChanged( const QVariant &value );

  private slots:

    //! Triggered when editor widget's value changes
    void editorWidgetChanged( const QVariant &value );

    //! Triggered when multi edit tool button requests value reset
    void resetValue();

    //! Triggered when the multi edit tool button "set field value" action is selected
    void setFieldTriggered();

    void onAggregateChanged();

  private:
    QString mWidgetType;
    QgsEditorWidgetWrapper *mWidget = nullptr;
    QgsAttributeForm *mForm = nullptr;
    QLabel *mConstraintResultLabel = nullptr;

    QgsMultiEditToolButton *mMultiEditButton = nullptr;
    QgsAggregateToolButton *mAggregateButton = nullptr;
    QVariant mPreviousValue;
    bool mBlockValueUpdate;
    bool mIsMixed;
    bool mIsChanged;

    void updateWidgets() override;
};

#endif // QGSATTRIBUTEFORMEDITORWIDGET_H
