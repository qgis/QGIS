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

#include <QWidget>
#include <QVariant>
#include "qgseditorwidgetconfig.h"
#include "qgsattributeeditorcontext.h"
#include "qgssearchwidgetwrapper.h"

class QgsAttributeForm;
class QgsEditorWidgetWrapper;
class QgsMultiEditToolButton;
class QgsSearchWidgetToolButton;
class QgsVectorLayer;
class QStackedWidget;
class QgsAttributeEditorContext;

/** \ingroup gui
 * \class QgsAttributeFormEditorWidget
 * A widget consisting of both an editor widget and additional widgets for controlling the behaviour
 * of the editor widget depending on a number of possible modes. For instance, if the parent attribute
 * form is in the multi edit mode, this widget will show both the editor widget and a tool button for
 * controlling the multi edit results.
 * \note Added in version 2.16
 */
class GUI_EXPORT QgsAttributeFormEditorWidget : public QWidget
{
    Q_OBJECT

  public:

    //! Widget modes
    enum Mode
    {
      DefaultMode, /*!< Default mode, only the editor widget is shown */
      MultiEditMode, /*!< Multi edit mode, both the editor widget and a QgsMultiEditToolButton is shown */
      SearchMode, /*!< Layer search/filter mode */
    };

    /** Constructor for QgsAttributeFormEditorWidget.
     * @param editorWidget associated editor widget wrapper (for default/edit modes)
     * @param form parent attribute form
     */
    explicit QgsAttributeFormEditorWidget( QgsEditorWidgetWrapper* editorWidget,
                                           QgsAttributeForm* form );

    ~QgsAttributeFormEditorWidget();

    /** Creates the search widget wrappers for the widget used when the form is in
     * search mode.
     * @param widgetId id of the widget type to create a search wrapper for
     * @param fieldIdx index of field associated with widget
     * @param config configuration which should be used for the widget creation
     * @param context editor context (not available in python bindings)
     */
    void createSearchWidgetWrappers( const QString& widgetId, int fieldIdx,
                                     const QgsEditorWidgetConfig& config,
                                     const QgsAttributeEditorContext &context = QgsAttributeEditorContext() );

    /** Sets the current mode for the widget. The widget will adapt its state and visible widgets to
     * reflect the updated mode. Eg, showing multi edit tool buttons if the mode is set to MultiEditMode.
     * @param mode widget mode
     * @see mode()
     */
    void setMode( Mode mode );

    /** Returns the current mode for the widget.
     * @see setMode()
     */
    Mode mode() const { return mMode; }

    /** Resets the widget to an initial value.
     * @param initialValue initial value to show in widget
     * @param mixedValues set to true to initially show the mixed values state
     */
    void initialize( const QVariant& initialValue, bool mixedValues = false );

    /** Returns true if the widget's value has been changed since it was initialized.
     * @see initialize()
     */
    bool hasChanged() const { return mIsChanged; }

    /** Returns the current value of the attached editor widget.
     */
    QVariant currentValue() const;

    /** Creates an expression matching the current search filter value and
     * search properties represented in the widget.
     * @note added in QGIS 2.16
     */
    QString currentFilterExpression() const;

  public slots:

    /** Sets whether the widget should be displayed in a "mixed values" mode.
     * @param mixed set to true to show in a mixed values state
     */
    void setIsMixed( bool mixed );

    /** Called when field values have been committed;
     */
    void changesCommitted();

    /** Resets the search/filter value of the widget.
     */
    void resetSearch();

  signals:

    //! Emitted when the widget's value changes
    //! @param value new widget value
    void valueChanged( const QVariant& value );

  private slots:

    //! Triggered when editor widget's value changes
    void editorWidgetChanged( const QVariant& value );

    //! Triggered when multi edit tool button requests value reset
    void resetValue();

    //! Triggered when the multi edit tool button "set field value" action is selected
    void setFieldTriggered();

    //! Triggered when search button flags are changed
    void searchWidgetFlagsChanged( QgsSearchWidgetWrapper::FilterFlags flags );

  protected:

    /** Returns a pointer to the search widget tool button in the widget.
     * @note this method is in place for unit testing only, and is not considered
     * stable API
     */
    QgsSearchWidgetToolButton* searchWidgetToolButton();

    /** Sets the search widget wrapper for the widget used when the form is in
     * search mode.
     * @param wrapper search widget wrapper.
     * @note the search widget wrapper should be created using searchWidgetFrame()
     * as its parent
     * @note this method is in place for unit testing only, and is not considered
     * stable AP
     */
    void setSearchWidgetWrapper( QgsSearchWidgetWrapper* wrapper );

    /** Returns the widget which should be used as a parent during construction
     * of the search widget wrapper.
     * @note this method is in place for unit testing only, and is not considered
     * stable AP
     */
    QWidget* searchWidgetFrame();

    /** Returns the search widget wrapper used in this widget. The wrapper must
     * first be created using createSearchWidgetWrapper()
     * @note this method is in place for unit testing only, and is not considered
     * stable AP
     */
    QList< QgsSearchWidgetWrapper* > searchWidgetWrappers();

  private:

    QWidget* mEditPage;
    QWidget* mSearchPage;
    QStackedWidget* mStack;
    QWidget* mSearchFrame;

    QgsEditorWidgetWrapper* mWidget;
    QList< QgsSearchWidgetWrapper* > mSearchWidgets;
    QgsAttributeForm* mForm;
    Mode mMode;

    QgsMultiEditToolButton* mMultiEditButton;
    QgsSearchWidgetToolButton* mSearchWidgetToolButton;
    QVariant mPreviousValue;
    bool mBlockValueUpdate;
    bool mIsMixed;
    bool mIsChanged;


    QgsVectorLayer* layer();
    void updateWidgets();
};

#endif // QGSATTRIBUTEFORMEDITORWIDGET_H
