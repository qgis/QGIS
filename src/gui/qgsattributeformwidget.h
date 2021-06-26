/***************************************************************************
    qgsattributeformwidget.h
    ---------------------
    begin                : November 2017
    copyright            : (C) 2017 by Matthias Kuhn
    email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSATTRIBUTEFORMWIDGET_H
#define QGSATTRIBUTEFORMWIDGET_H

#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgsattributeeditorcontext.h"
#include "qgssearchwidgetwrapper.h"

#include <QWidget>
#include <QVariant>

class QgsAttributeForm;
class QStackedWidget;
class QgsSearchWidgetToolButton;

/**
 * \ingroup gui
 *
 * Base class for all widgets shown on a QgsAttributeForm.
 * Consists of the widget which is visible in edit mode as well as the widget visible in search mode.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsAttributeFormWidget : public QWidget // SIP_ABSTRACT
{
    Q_OBJECT

  public:

    //! Widget modes
    enum Mode
    {
      DefaultMode, //!< Default mode, only the editor widget is shown
      MultiEditMode, //!< Multi edit mode, both the editor widget and a QgsMultiEditToolButton is shown
      SearchMode, //!< Layer search/filter mode
      AggregateSearchMode, //!< Embedded in a search form, show additional aggregate function toolbutton
    };

    /**
     * A new form widget for the wrapper \a widget on \a form.
     */
    explicit QgsAttributeFormWidget( QgsWidgetWrapper *widget, QgsAttributeForm *form );

    /**
     * Creates the search widget wrappers for the widget used when the form is in
     * search mode.
     *
     * \param context editor context (not available in Python bindings)
     */
    virtual void createSearchWidgetWrappers( const QgsAttributeEditorContext &context SIP_PYARGREMOVE = QgsAttributeEditorContext() ) = 0;

    /**
     * Creates an expression matching the current search filter value and
     * search properties represented in the widget.
     * \since QGIS 2.16
     */
    virtual QString currentFilterExpression() const;


    /**
     * Sets the current mode for the widget. The widget will adapt its state and visible widgets to
     * reflect the updated mode. For example, showing multi edit tool buttons if the mode is set to MultiEditMode.
     * \param mode widget mode
     * \see mode()
     */
    void setMode( Mode mode );

    /**
     * Returns the current mode for the widget.
     * \see setMode()
     */
    Mode mode() const { return mMode; }

    /**
     * The layer for which this widget and its form is shown.
     */
    QgsVectorLayer *layer();

    /**
     * The form on which this widget is shown.
     */
    QgsAttributeForm *form() const;

    /**
     * Returns the widget which should be used as a parent during construction
     * of the search widget wrapper.
     * \note this method is in place for unit testing only, and is not considered
     * stable API
     */
    QWidget *searchWidgetFrame() SIP_SKIP;


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
     * Adds an additional search widget wrapper.
     * Used to register a secondary search widget as used for "between" searches.
     */
    void addAdditionalSearchWidgetWrapper( QgsSearchWidgetWrapper *wrapper );

    /**
     * Returns the search widget wrapper used in this widget. The wrapper must
     * first be created using createSearchWidgetWrapper()
     * \note this method is in place for unit testing only, and is not considered
     * stable API
     */
    QList< QgsSearchWidgetWrapper * > searchWidgetWrappers();

    /**
     * Resets the search/filter value of the widget.
     */
    void resetSearch();

    /**
     * The visibility of the search widget tool button, that allows (de)activating
     * this search widgte or defines the comparison operator to use.
     */
    bool searchWidgetToolButtonVisible() const;

    /**
     * The visibility of the search widget tool button, that allows (de)activating
     * this search widgte or defines the comparison operator to use.
     */
    void setSearchWidgetToolButtonVisible( bool searchWidgetToolButtonVisible );

  protected:

    /**
     * Returns a pointer to the EDIT page widget.
     * \note this method is in place for unit testing only, and is not considered
     * stable API
     * \note not available in Python bindings
     */
    QWidget *editPage() const SIP_SKIP;

    /**
     * Returns a pointer to the stacked widget managing edit and search page.
     * \note this method is in place for unit testing only, and is not considered
     * stable API
     * \note not available in Python bindings
     */
    QStackedWidget *stack() const SIP_SKIP;

    /**
     * Returns a pointer to the search page widget.
     * \note this method is in place for unit testing only, and is not considered
     * stable API
     * \note not available in Python bindings
     */
    QWidget *searchPage() const SIP_SKIP;

  private slots:

    //! Triggered when search button flags are changed
    void searchWidgetFlagsChanged( QgsSearchWidgetWrapper::FilterFlags flags );

  private:
    virtual void updateWidgets();
    QgsAttributeFormWidget::Mode mMode = DefaultMode;
    QgsSearchWidgetToolButton *mSearchWidgetToolButton = nullptr;
    QWidget *mEditPage = nullptr;
    QWidget *mSearchPage = nullptr;
    QStackedWidget *mStack = nullptr;
    QWidget *mSearchFrame = nullptr;
    QgsAttributeForm *mForm = nullptr;
    QList< QgsSearchWidgetWrapper * > mSearchWidgets;
    QgsWidgetWrapper *mWidget = nullptr;
};

#endif // QGSATTRIBUTEFORMWIDGET_H
