%MappedType QList<QgsSearchWidgetWrapper::FilterFlag>
{
%TypeHeaderCode
#include <QList>
%End

%ConvertFromTypeCode
  // Create the list.
  PyObject *l;

  if ((l = PyList_New(sipCpp->size())) == NULL)
    return NULL;

  // Set the list elements.
  QList<QgsSearchWidgetWrapper::FilterFlag>::iterator it = sipCpp->begin();
  for (int i = 0; it != sipCpp->end(); ++it, ++i)
  {
    PyObject *tobj;

    if ((tobj = sipConvertFromEnum(*it, sipType_QgsSearchWidgetWrapper_FilterFlag)) == NULL)
    {
      Py_DECREF(l);
      return NULL;
    }
    PyList_SET_ITEM(l, i, tobj);
  }

  return l;
%End

%ConvertToTypeCode
  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
    return PyList_Check(sipPy);

  QList<QgsSearchWidgetWrapper::FilterFlag> *qlist = new QList<QgsSearchWidgetWrapper::FilterFlag>;

  for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
  {
    *qlist << (QgsSearchWidgetWrapper::FilterFlag)SIPLong_AsLong(PyList_GET_ITEM(sipPy, i));
  }

  *sipCppPtr = qlist;
  return sipGetState(sipTransferObj);
%End
};

/**
 * Manages an editor widget
 * Widget and wrapper share the same parent
 *
 * A wrapper controls one attribute editor widget and is able to create a default
 * widget or use a pre-existent widget. It is able to set the widget to the value implied
 * by a field of a vector layer, or return the value it currently holds. Every time it is changed
 * it has to emit a valueChanged signal. If it fails to do so, there is no guarantee that the
 * changed status of the widget will be saved.
 */
class QgsSearchWidgetWrapper : QgsWidgetWrapper
{
%TypeHeaderCode
#include <qgssearchwidgetwrapper.h>
%End
  public:

    //! Flags which indicate what types of filtering and searching is possible using the widget
    //! @note added in QGIS 2.16
    enum FilterFlag
    {
      EqualTo, /*!< Supports equal to */
      NotEqualTo, /*!< Supports not equal to */
      GreaterThan, /*!< Supports greater than */
      LessThan, /*!< Supports less than */
      GreaterThanOrEqualTo, /*!< Supports >= */
      LessThanOrEqualTo, /*!< Supports <= */
      Between, /*!< Supports searches between two values */
      CaseInsensitive, /*!< Supports case insensitive searching */
      Contains, /*!< Supports value "contains" searching */
      DoesNotContain, /*!< Supports value does not contain searching */
      IsNull, /*!< Supports searching for null values */
      IsNotBetween, /*!< Supports searching for values outside of a set range */
      IsNotNull, /*!< Supports searching for non-null values */
    };
    typedef QFlags<QgsSearchWidgetWrapper::FilterFlag> FilterFlags;

    /** Returns a list of exclusive filter flags, which cannot be combined with other flags (eg EqualTo/NotEqualTo)
     * @note added in QGIS 2.16
     * @see nonExclusiveFilterFlags()
     */
    static QList< QgsSearchWidgetWrapper::FilterFlag > exclusiveFilterFlags();

    /** Returns a list of non-exclusive filter flags, which can be combined with other flags (eg CaseInsensitive)
     * @note added in QGIS 2.16
     * @see exclusiveFilterFlags()
     */
    static QList< QgsSearchWidgetWrapper::FilterFlag > nonExclusiveFilterFlags();

    /** Returns a translated string representing a filter flag.
     * @param flag flag to convert to string
     * @note added in QGIS 2.16
     */
    static QString toString( FilterFlag flag );

    /**
     * Create a new widget wrapper
     *
     * @param vl        The layer on which the field is
     * @param fieldIdx  The field which will be controlled
     * @param parent    A parent widget for this widget wrapper and the created widget.
     */
    explicit QgsSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* parent /TransferThis/ = nullptr );

    /** Returns filter flags supported by the search widget.
     * @note added in QGIS 2.16
     * @see defaultFlags()
     */
    virtual FilterFlags supportedFlags() const;

    /** Returns the filter flags which should be set by default for the search widget.
     * @note added in QGIS 2.16
     * @see supportedFlags()
     */
    virtual FilterFlags defaultFlags() const;

    /**
     * Will be used to access the widget's value. Read the value from the widget and
     * return it properly formatted to be saved in the attribute.
     *
     * If an invalid variant is returned this will be interpreted as no change.
     * Be sure to return a NULL QVariant if it should be set to NULL.
     *
     * @return The current value the widget represents
     */
    virtual QString expression() = 0;
    /**
     * If this is true, then this search widget should take effect directly
     * when its expression changes
     */
    virtual bool applyDirectly() = 0;

    /** Creates a filter expression based on the current state of the search widget
     * and the specified filter flags.
     * @param flags filter flags
     * @returns filter expression
     * @note added in QGIS 2.16
     */
    // TODO QGIS 3.0 - make pure virtual
    virtual QString createExpression( FilterFlags flags ) const;

  public slots:

    /** Clears the widget's current value and resets it back to the default state
     * @note added in QGIS 2.16
     */
    virtual void clearWidget();

    /** Toggles whether the search widget is enabled or disabled.
     * @param enabled set to true to enable widget
     * @note added in QGIS 2.16
     */
    virtual void setEnabled( bool enabled );

  signals:

    /**
     * Emitted whenever the expression changes
     * @param exp The new search expression
     */
    void expressionChanged( const QString& exp );

    /** Emitted when a user changes the value of the search widget.
     * @note added in QGIS 2.16
     */
    void valueChanged();

    /** Emitted when a user changes the value of the search widget back
     * to an empty, default state.
     * @note added in QGIS 2.16
     */
    void valueCleared();

  protected slots:

    virtual void setExpression( QString value ) = 0;
    void setFeature( const QgsFeature& feature );

  protected:
    //! clears the expression to search for all features
    void clearExpression();

};

QFlags<QgsSearchWidgetWrapper::FilterFlag> operator|(QgsSearchWidgetWrapper::FilterFlag f1, QFlags<QgsSearchWidgetWrapper::FilterFlag> f2);
