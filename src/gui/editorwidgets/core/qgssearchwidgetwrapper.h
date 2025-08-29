/***************************************************************************
    qgssearchwidgetwrapper.h
     --------------------------------------
    Date                 : 31.5.2015
    Copyright            : (C) 2015 Karolina Alexiou (carolinux)
    Email                : carolinegr at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSEARCHWIDGETWRAPPER_H
#define QGSSEARCHWIDGETWRAPPER_H

#include <QObject>
#include "qgis_sip.h"
#include <QMap>
#include <QVariant>

class QgsVectorLayer;
class QgsField;

#include "qgsattributeeditorcontext.h"
#include "qgswidgetwrapper.h"
#include "qgis_gui.h"

#ifdef SIP_RUN
//%MappedType QList<QgsSearchWidgetWrapper::FilterFlag> /TypeHintIn = "Iterable[QgsSearchWidgetWrapper.FilterFlag]",TypeHintOut = "List[QgsSearchWidgetWrapper.FilterFlag]", TypeHintValue = "[]"/
{
  //%TypeHeaderCode
#include <QList>
  //%End

  //%ConvertFromTypeCode
  PyObject *l = PyList_New( sipCpp->size() );

  if ( !l )
    return 0;

  for ( int i = 0; i < sipCpp->size(); ++i )
  {
    PyObject *eobj = sipConvertFromEnum( static_cast<int>( sipCpp->at( i ) ), sipType_QgsSearchWidgetWrapper_FilterFlag );

    if ( !eobj )
    {
      Py_DECREF( l );

      return 0;
    }

    PyList_SetItem( l, i, eobj );
  }

  return l;
  //%End

  //%ConvertToTypeCode
  PyObject *iter = PyObject_GetIter( sipPy );

  if ( !sipIsErr )
  {
    PyErr_Clear();
    Py_XDECREF( iter );

    return ( iter && !PyBytes_Check( sipPy ) && !PyUnicode_Check( sipPy ) );
  }

  if ( !iter )
  {
    *sipIsErr = 1;

    return 0;
  }

  QList<QgsSearchWidgetWrapper::FilterFlag> *ql = new QList<QgsSearchWidgetWrapper::FilterFlag>;

  for ( Py_ssize_t i = 0;; ++i )
  {
    PyErr_Clear();
    PyObject *itm = PyIter_Next( iter );

    if ( !itm )
    {
      if ( PyErr_Occurred() )
      {
        delete ql;
        Py_DECREF( iter );
        *sipIsErr = 1;

        return 0;
      }

      break;
    }

    int v = sipConvertToEnum( itm, sipType_QgsSearchWidgetWrapper_FilterFlag );

    if ( PyErr_Occurred() )
    {
      PyErr_Format( PyExc_TypeError, "index %zd has type '%s' but 'QgsSearchWidgetWrapper.FilterFlag' is expected", i, sipPyTypeName( Py_TYPE( itm ) ) );

      Py_DECREF( itm );
      delete ql;
      Py_DECREF( iter );
      *sipIsErr = 1;

      return 0;
    }

    ql->append( static_cast<QgsSearchWidgetWrapper::FilterFlag>( v ) );

    Py_DECREF( itm );
  }

  Py_DECREF( iter );

  *sipCppPtr = ql;

  return sipGetState( sipTransferObj );
  //%End
};
#endif
/**
 * \ingroup gui
 *
 * \brief Shows a search widget on a filter form.
 */
class GUI_EXPORT QgsSearchWidgetWrapper : public QgsWidgetWrapper
{
    Q_OBJECT
  public:
    /**
     * Flags which indicate what types of filtering and searching is possible using the widget
     */
    enum FilterFlag SIP_ENUM_BASETYPE( IntFlag )
    {
      EqualTo = 1 << 1,              //!< Supports equal to
      NotEqualTo = 1 << 2,           //!< Supports not equal to
      GreaterThan = 1 << 3,          //!< Supports greater than
      LessThan = 1 << 4,             //!< Supports less than
      GreaterThanOrEqualTo = 1 << 5, //!< Supports >=
      LessThanOrEqualTo = 1 << 6,    //!< Supports <=
      Between = 1 << 7,              //!< Supports searches between two values
      CaseInsensitive = 1 << 8,      //!< Supports case insensitive searching
      Contains = 1 << 9,             //!< Supports value "contains" searching
      DoesNotContain = 1 << 10,      //!< Supports value does not contain searching
      IsNull = 1 << 11,              //!< Supports searching for null values
      IsNotBetween = 1 << 12,        //!< Supports searching for values outside of a set range
      IsNotNull = 1 << 13,           //!< Supports searching for non-null values
      StartsWith = 1 << 14,          //!< Supports searching for strings that start with
      EndsWith = 1 << 15,            //!< Supports searching for strings that end with
    };
    Q_DECLARE_FLAGS( FilterFlags, FilterFlag )

    /**
     * Returns a list of exclusive filter flags, which cannot be combined with other flags (e.g., EqualTo/NotEqualTo)
     * \see nonExclusiveFilterFlags()
     */
    static QList<QgsSearchWidgetWrapper::FilterFlag> exclusiveFilterFlags();

    /**
     * Returns a list of non-exclusive filter flags, which can be combined with other flags (e.g., CaseInsensitive)
     * \see exclusiveFilterFlags()
     */
    static QList<QgsSearchWidgetWrapper::FilterFlag> nonExclusiveFilterFlags();

    /**
     * Returns a translated string representing a filter flag.
     * \param flag flag to convert to string
     */
    static QString toString( QgsSearchWidgetWrapper::FilterFlag flag );

    /**
     * Create a new widget wrapper
     *
     * \param vl        The layer on which the field is
     * \param fieldIdx  The field which will be controlled
     * \param parent    A parent widget for this widget wrapper and the created widget.
     */
    explicit QgsSearchWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns filter flags supported by the search widget.
     * \see defaultFlags()
     */
    virtual FilterFlags supportedFlags() const;

    /**
     * Returns the filter flags which should be set by default for the search widget.
     * \see supportedFlags()
     */
    virtual FilterFlags defaultFlags() const;

    /**
     * Will be used to access the widget's value. Read the value from the widget and
     * return it properly formatted to be saved in the attribute.
     *
     * If an invalid variant is returned this will be interpreted as no change.
     * Be sure to return a NULL QVariant if it should be set to NULL.
     *
     * \returns The current value the widget represents
     */
    virtual QString expression() const = 0;

    /**
     * If this is TRUE, then this search widget should take effect directly
     * when its expression changes
     */
    virtual bool applyDirectly() = 0;

    // TODO QGIS 4.0 - make pure virtual

    /**
     * Creates a filter expression based on the current state of the search widget
     * and the specified filter flags.
     * \param flags filter flags
     * \returns filter expression
     */
    virtual QString createExpression( FilterFlags flags ) const
    {
      Q_UNUSED( flags )
      return QStringLiteral( "TRUE" );
    }

    /**
     * Gets a field name or expression to use as field comparison.
     * If in SearchMode returns a quoted field identifier.
     * If in AggregateSearchMode returns an appropriate aggregate expression.
     *
     */
    QString createFieldIdentifier() const;

    /**
     * If in AggregateSearch mode, which aggregate should be used to construct
     * the filter expression. Is a Null String if none.
     *
     */
    QString aggregate() const;

    /**
     * If in AggregateSearch mode, which aggregate should be used to construct
     * the filter expression. Is a Null String if none.
     *
     */
    void setAggregate( const QString &aggregate );

    /**
     * Returns the field index
     * \since QGIS 3.10
     */
    int fieldIndex() const;

  public slots:

    /**
     * Clears the widget's current value and resets it back to the default state
     */
    virtual void clearWidget() {}

    /**
     * Toggles whether the search widget is enabled or disabled.
     * \param enabled set to TRUE to enable widget
     */
    void setEnabled( bool enabled ) override { Q_UNUSED( enabled ) }

  signals:

    /**
     * Emitted whenever the expression changes
     * \param exp The new search expression
     */
    void expressionChanged( const QString &exp );

    /**
     * Emitted when a user changes the value of the search widget.
     */
    void valueChanged();

    /**
     * Emitted when a user changes the value of the search widget back
     * to an empty, default state.
     */
    void valueCleared();

  protected slots:

    /**
     * Set the \a expression which is currently used as filter for this widget.
     */
    virtual void setExpression( const QString &expression ) = 0;

    void setFeature( const QgsFeature &feature ) override;

  protected:
    //! clears the expression to search for all features
    void clearExpression();

    QString mExpression;
    int mFieldIdx;

  private:
    QString mAggregate;
    QgsRelation mAggregateRelation;
};
// We'll use this class inside a QVariant in the widgets properties
Q_DECLARE_METATYPE( QgsSearchWidgetWrapper * )

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsSearchWidgetWrapper::FilterFlags )

#endif // QGSSEARCHWIDGETWRAPPER_H
