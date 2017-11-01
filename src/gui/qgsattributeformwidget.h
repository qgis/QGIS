#ifndef QGSATTRIBUTEFORMWIDGET_H
#define QGSATTRIBUTEFORMWIDGET_H

#include "qgis.h"
#include "qgis_gui.h"
#include "qgsattributeeditorcontext.h"
#include "qgssearchwidgetwrapper.h"

#include <QWidget>
#include <QVariant>

class QgsAttributeForm;
class QStackedWidget;
class QgsSearchWidgetToolButton;

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
      AggregateSearchMode,
    };

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

    QgsVectorLayer *layer();

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

  protected:
    QWidget *editPage() const SIP_SKIP;

    QStackedWidget *stack() const SIP_SKIP;

    QWidget *searchPage() const SIP_SKIP;

    /**
     * Returns a pointer to the search widget tool button in the widget.
     * \note this method is in place for unit testing only, and is not considered
     * stable API
     */
    QgsSearchWidgetToolButton *searchWidgetToolButton();

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
