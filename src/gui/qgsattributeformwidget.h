#ifndef QGSATTRIBUTEFORMWIDGET_H
#define QGSATTRIBUTEFORMWIDGET_H

#include "qgis.h"
#include "qgis_gui.h"
#include "qgsattributeeditorcontext.h"
#include "qgssearchwidgetwrapper.h"

#include <QWidget>
#include <QVariant>

class QgsAttributeForm;

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
    virtual QString currentFilterExpression() const = 0;


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

  private:
    QgsAttributeFormWidget::Mode mMode = DefaultMode;
    virtual void updateWidgets() = 0;
    QgsAttributeForm *mForm = nullptr;
};

#endif // QGSATTRIBUTEFORMWIDGET_H
