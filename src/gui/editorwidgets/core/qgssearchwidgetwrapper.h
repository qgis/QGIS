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
% MappedType QList<QgsSearchWidgetWrapper::FilterFlag>
{
  % TypeHeaderCode
#include <QList>
  % End

  % ConvertFromTypeCode
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
  % End

  % ConvertToTypeCode
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
  % End
};
#endif

/**
 * \ingroup gui
 *
 * Shows a search widget on a filter form.
 */
class GUI_EXPORT QgsSearchWidgetWrapper : public QgsWidgetWrapper
{
    Q_OBJECT
  public:

    /**
     * Flags which indicate what types of filtering and searching is possible using the widget
     * \since QGIS 2.16
     */
    enum FilterFlag
    {
      EqualTo = 1 << 1, //!< Supports equal to
      NotEqualTo = 1 << 2, //!< Supports not equal to
      GreaterThan = 1 << 3, //!< Supports greater than
      LessThan = 1 << 4, //!< Supports less than
      GreaterThanOrEqualTo = 1 << 5, //!< Supports >=
      LessThanOrEqualTo = 1 << 6, //!< Supports <=
      Between = 1 << 7, //!< Supports searches between two values
      CaseInsensitive = 1 << 8, //!< Supports case insensitive searching
      Contains = 1 << 9, //!< Supports value "contains" searching
      DoesNotContain = 1 << 10, //!< Supports value does not contain searching
      IsNull = 1 << 11, //!< Supports searching for null values
      IsNotBetween = 1 << 12, //!< Supports searching for values outside of a set range
      IsNotNull = 1 << 13, //!< Supports searching for non-null values
      StartsWith = 1 << 14, //!< Supports searching for strings that start with
      EndsWith = 1 << 15, //!< Supports searching for strings that end with
    };
    Q_DECLARE_FLAGS( FilterFlags, FilterFlag )

    /**
     * Returns a list of exclusive filter flags, which cannot be combined with other flags (e.g., EqualTo/NotEqualTo)
     * \see nonExclusiveFilterFlags()
     * \since QGIS 2.16
     */
    static QList< QgsSearchWidgetWrapper::FilterFlag > exclusiveFilterFlags();

    /**
     * Returns a list of non-exclusive filter flags, which can be combined with other flags (e.g., CaseInsensitive)
     * \see exclusiveFilterFlags()
     * \since QGIS 2.16
     */
    static QList< QgsSearchWidgetWrapper::FilterFlag > nonExclusiveFilterFlags();

    /**
     * Returns a translated string representing a filter flag.
     * \param flag flag to convert to string
     * \since QGIS 2.16
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
     * \since QGIS 2.16
     */
    virtual FilterFlags supportedFlags() const;

    /**
     * Returns the filter flags which should be set by default for the search widget.
     * \see supportedFlags()
     * \since QGIS 2.16
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
     * If this is true, then this search widget should take effect directly
     * when its expression changes
     */
    virtual bool applyDirectly() = 0;

    /**
     * Creates a filter expression based on the current state of the search widget
     * and the specified filter flags.
     * \param flags filter flags
     * \returns filter expression
     * \since QGIS 2.16
     */
    // TODO QGIS 3.0 - make pure virtual
    virtual QString createExpression( FilterFlags flags ) const { Q_UNUSED( flags ); return QStringLiteral( "TRUE" ); }

    /**
     * Gets a field name or expression to use as field comparison.
     * If in SearchMode returns a quoted field identifier.
     * If in AggregateSearchMode returns an appropriate aggregate expression.
     *
     * \since QGIS 3.0
     */
    QString createFieldIdentifier() const;

    /**
     * If in AggregateSearch mode, which aggregate should be used to construct
     * the filter expression. Is a Null String if none.
     *
     * \since QGIS 3.0
     */
    QString aggregate() const;

    /**
     * If in AggregateSearch mode, which aggregate should be used to construct
     * the filter expression. Is a Null String if none.
     *
     * \since QGIS 3.0
     */
    void setAggregate( const QString &aggregate );

  public slots:

    /**
     * Clears the widget's current value and resets it back to the default state
     * \since QGIS 2.16
     */
    virtual void clearWidget() {}

    /**
     * Toggles whether the search widget is enabled or disabled.
     * \param enabled set to true to enable widget
     */
    void setEnabled( bool enabled ) override { Q_UNUSED( enabled ); }

  signals:

    /**
     * Emitted whenever the expression changes
     * \param exp The new search expression
     */
    void expressionChanged( const QString &exp );

    /**
     * Emitted when a user changes the value of the search widget.
     * \since QGIS 2.16
     */
    void valueChanged();

    /**
     * Emitted when a user changes the value of the search widget back
     * to an empty, default state.
     * \since QGIS 2.16
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
