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
     * \param form parent attribute form
     */
    explicit QgsAttributeFormEditorWidget( QgsEditorWidgetWrapper *editorWidget, const QString &widgetType,
                                           QgsAttributeForm *form  SIP_TRANSFERTHIS );

    ~QgsAttributeFormEditorWidget();

    virtual void createSearchWidgetWrappers( const QgsAttributeEditorContext &context SIP_PYARGREMOVE = QgsAttributeEditorContext() ) override;

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
     * Creates an expression matching the current search filter value and
     * search properties represented in the widget.
     * \since QGIS 2.16
     */
    QString currentFilterExpression() const override;

    /**
     * Set the constraint status for this widget.
     */
    void setConstraintStatus( const QString &constraint, const QString &description, const QString &err, QgsEditorWidgetWrapper::ConstraintResult result );

    /**
     * Set the constraint result lable visible or invisible according to the layer editable status
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

    /**
     * Resets the search/filter value of the widget.
     */
    void resetSearch();

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

    //! Triggered when search button flags are changed
    void searchWidgetFlagsChanged( QgsSearchWidgetWrapper::FilterFlags flags );

  protected:

    /**
     * Returns a pointer to the search widget tool button in the widget.
     * \note this method is in place for unit testing only, and is not considered
     * stable API
     */
    QgsSearchWidgetToolButton *searchWidgetToolButton();

    /**
     * Sets the search widget wrapper for the widget used when the form is in
     * search mode.
     * \param wrapper search widget wrapper.
     * \note the search widget wrapper should be created using searchWidgetFrame()
     * as its parent
     * \note this method is in place for unit testing only, and is not considered
     * stable API
     */
    void setSearchWidgetWrapper( QgsSearchWidgetWrapper *wrapper );

    /**
     * Returns the widget which should be used as a parent during construction
     * of the search widget wrapper.
     * \note this method is in place for unit testing only, and is not considered
     * stable AP
     */
    QWidget *searchWidgetFrame();

    /**
     * Returns the search widget wrapper used in this widget. The wrapper must
     * first be created using createSearchWidgetWrapper()
     * \note this method is in place for unit testing only, and is not considered
     * stable API
     */
    QList< QgsSearchWidgetWrapper * > searchWidgetWrappers();

  private:

    QWidget *mEditPage = nullptr;
    QWidget *mSearchPage = nullptr;
    QStackedWidget *mStack = nullptr;
    QWidget *mSearchFrame = nullptr;

    QString mWidgetType;
    QgsEditorWidgetWrapper *mWidget = nullptr;
    QList< QgsSearchWidgetWrapper * > mSearchWidgets;
    QgsAttributeForm *mForm = nullptr;
    QLabel *mConstraintResultLabel = nullptr;

    QgsMultiEditToolButton *mMultiEditButton = nullptr;
    QgsSearchWidgetToolButton *mSearchWidgetToolButton = nullptr;
    QVariant mPreviousValue;
    bool mBlockValueUpdate;
    bool mIsMixed;
    bool mIsChanged;

    void updateWidgets() override;
};

#endif // QGSATTRIBUTEFORMEDITORWIDGET_H
