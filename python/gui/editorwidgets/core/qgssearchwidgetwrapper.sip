/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/editorwidgets/core/qgssearchwidgetwrapper.h                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





%MappedType QList<QgsSearchWidgetWrapper::FilterFlag>
{
%TypeHeaderCode
#include <QList>
%End

%ConvertFromTypeCode
  // Create the list.
  PyObject *l;

  if ( ( l = PyList_New( sipCpp->size() ) ) == NULL )
    return NULL;

  // Set the list elements.
  QList<QgsSearchWidgetWrapper::FilterFlag>::iterator it = sipCpp->begin();
  for ( int i = 0; it != sipCpp->end(); ++it, ++i )
  {
    PyObject *tobj;

    if ( ( tobj = sipConvertFromEnum( *it, sipType_QgsSearchWidgetWrapper_FilterFlag ) ) == NULL )
    {
      Py_DECREF( l );
      return NULL;
    }
    PyList_SET_ITEM( l, i, tobj );
  }

  return l;
%End

%ConvertToTypeCode
  // Check the type if that is all that is required.
  if ( sipIsErr == NULL )
    return PyList_Check( sipPy );

  QList<QgsSearchWidgetWrapper::FilterFlag> *qlist = new QList<QgsSearchWidgetWrapper::FilterFlag>;

  for ( int i = 0; i < PyList_GET_SIZE( sipPy ); ++i )
  {
    *qlist << ( QgsSearchWidgetWrapper::FilterFlag )SIPLong_AsLong( PyList_GET_ITEM( sipPy, i ) );
  }

  *sipCppPtr = qlist;
  return sipGetState( sipTransferObj );
%End
};

class QgsSearchWidgetWrapper : QgsWidgetWrapper
{
%Docstring

Shows a search widget on a filter form.
%End

%TypeHeaderCode
#include "qgssearchwidgetwrapper.h"
%End
  public:

    enum FilterFlag
    {
      EqualTo,
      NotEqualTo,
      GreaterThan,
      LessThan,
      GreaterThanOrEqualTo,
      LessThanOrEqualTo,
      Between,
      CaseInsensitive,
      Contains,
      DoesNotContain,
      IsNull,
      IsNotBetween,
      IsNotNull,
      StartsWith,
      EndsWith,
    };
    typedef QFlags<QgsSearchWidgetWrapper::FilterFlag> FilterFlags;


    static QList< QgsSearchWidgetWrapper::FilterFlag > exclusiveFilterFlags();
%Docstring
Returns a list of exclusive filter flags, which cannot be combined with other flags (e.g., EqualTo/NotEqualTo)

.. versionadded:: 2.16

.. seealso:: :py:func:`nonExclusiveFilterFlags`
%End

    static QList< QgsSearchWidgetWrapper::FilterFlag > nonExclusiveFilterFlags();
%Docstring
Returns a list of non-exclusive filter flags, which can be combined with other flags (e.g., CaseInsensitive)

.. versionadded:: 2.16

.. seealso:: :py:func:`exclusiveFilterFlags`
%End

    static QString toString( QgsSearchWidgetWrapper::FilterFlag flag );
%Docstring
Returns a translated string representing a filter flag.

:param flag: flag to convert to string

.. versionadded:: 2.16
%End

    explicit QgsSearchWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *parent /TransferThis/ = 0 );
%Docstring
Create a new widget wrapper

:param vl:        The layer on which the field is
:param fieldIdx:  The field which will be controlled
:param parent:    A parent widget for this widget wrapper and the created widget.
%End

    virtual FilterFlags supportedFlags() const;
%Docstring
Returns filter flags supported by the search widget.

.. versionadded:: 2.16

.. seealso:: :py:func:`defaultFlags`
%End

    virtual FilterFlags defaultFlags() const;
%Docstring
Returns the filter flags which should be set by default for the search widget.

.. versionadded:: 2.16

.. seealso:: :py:func:`supportedFlags`
%End

    virtual QString expression() const = 0;
%Docstring
Will be used to access the widget's value. Read the value from the widget and
return it properly formatted to be saved in the attribute.

If an invalid variant is returned this will be interpreted as no change.
Be sure to return a NULL QVariant if it should be set to NULL.

:return: The current value the widget represents
%End

    virtual bool applyDirectly() = 0;
%Docstring
If this is true, then this search widget should take effect directly
when its expression changes
%End

    virtual QString createExpression( FilterFlags flags ) const;

    QString createFieldIdentifier() const;
%Docstring
Get a field name or expression to use as field comparison.
If in SearchMode returns a quoted field identifier.
If in AggregateSearchMode returns an appropriate aggregate expression.

.. versionadded:: 3.0
%End

    QString aggregate() const;
%Docstring
If in AggregateSearch mode, which aggregate should be used to construct
the filter expression. Is a Null String if none.

.. versionadded:: 3.0
%End

    void setAggregate( const QString &aggregate );
%Docstring
If in AggregateSearch mode, which aggregate should be used to construct
the filter expression. Is a Null String if none.

.. versionadded:: 3.0
%End

  public slots:

    virtual void clearWidget();
%Docstring
Clears the widget's current value and resets it back to the default state

.. versionadded:: 2.16
%End

    virtual void setEnabled( bool enabled );
%Docstring
Toggles whether the search widget is enabled or disabled.

:param enabled: set to true to enable widget
%End

  signals:

    void expressionChanged( const QString &exp );
%Docstring
Emitted whenever the expression changes

:param exp: The new search expression
%End

    void valueChanged();
%Docstring
Emitted when a user changes the value of the search widget.

.. versionadded:: 2.16
%End

    void valueCleared();
%Docstring
Emitted when a user changes the value of the search widget back
to an empty, default state.

.. versionadded:: 2.16
%End

  protected slots:

    virtual void setExpression( const QString &expression ) = 0;
%Docstring
Set the ``expression`` which is currently used as filter for this widget.
%End

    virtual void setFeature( const QgsFeature &feature );


  protected:
    void clearExpression();
%Docstring
clears the expression to search for all features
%End


};

QFlags<QgsSearchWidgetWrapper::FilterFlag> operator|(QgsSearchWidgetWrapper::FilterFlag f1, QFlags<QgsSearchWidgetWrapper::FilterFlag> f2);


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/editorwidgets/core/qgssearchwidgetwrapper.h                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
