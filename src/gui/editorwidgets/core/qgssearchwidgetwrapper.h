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
#include <QMap>
#include <QVariant>

class QgsVectorLayer;
class QgsField;

#include "qgseditorwidgetconfig.h"
#include "qgsattributeeditorcontext.h"
#include "qgswidgetwrapper.h"

/** \ingroup gui
 * Manages an editor widget
 * Widget and wrapper share the same parent
 *
 * A wrapper controls one attribute editor widget and is able to create a default
 * widget or use a pre-existent widget. It is able to set the widget to the value implied
 * by a field of a vector layer, or return the value it currently holds. Every time it is changed
 * it has to emit a valueChanged signal. If it fails to do so, there is no guarantee that the
 * changed status of the widget will be saved.
 */
class GUI_EXPORT QgsSearchWidgetWrapper : public QgsWidgetWrapper
{
    Q_OBJECT
  public:

    //! Flags which indicate what types of filtering and searching is possible using the widget
    //! @note added in QGIS 2.16
    enum FilterFlag
    {
      EqualTo = 1 << 1, /*!< Supports equal to */
      NotEqualTo = 1 << 2, /*!< Supports not equal to */
      GreaterThan = 1 << 3, /*!< Supports greater than */
      LessThan = 1 << 4, /*!< Supports less than */
      GreaterThanOrEqualTo = 1 << 5, /*!< Supports >= */
      LessThanOrEqualTo = 1 << 6, /*!< Supports <= */
      Between = 1 << 7, /*!< Supports searches between two values */
      CaseInsensitive = 1 << 8, /*!< Supports case insensitive searching */
      Contains = 1 << 9, /*!< Supports value "contains" searching */
      DoesNotContain = 1 << 10, /*!< Supports value does not contain searching */
      IsNull = 1 << 11, /*!< Supports searching for null values */
      IsNotBetween = 1 << 12, /*!< Supports searching for values outside of a set range */
      IsNotNull = 1 << 13, /*!< Supports searching for non-null values */
    };
    Q_DECLARE_FLAGS( FilterFlags, FilterFlag )

    /** Returns a list of exclusive filter flags, which cannot be combined with other flags (eg EqualTo/NotEqualTo)
     * @note added in QGIS 2.16
     * @see nonExclusiveFilterFlags()
     */
    static QList< FilterFlag > exclusiveFilterFlags();

    /** Returns a list of non-exclusive filter flags, which can be combined with other flags (eg CaseInsensitive)
     * @note added in QGIS 2.16
     * @see exclusiveFilterFlags()
     */
    static QList< FilterFlag > nonExclusiveFilterFlags();

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
    explicit QgsSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* parent = nullptr );

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
    virtual QString createExpression( FilterFlags flags ) const { Q_UNUSED( flags ); return "TRUE"; }

  public slots:

    /** Clears the widget's current value and resets it back to the default state
     * @note added in QGIS 2.16
     */
    virtual void clearWidget() {}

    /** Toggles whether the search widget is enabled or disabled.
     * @param enabled set to true to enable widget
     */
    virtual void setEnabled( bool enabled ) override { Q_UNUSED( enabled ); }

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
    void setFeature( const QgsFeature& feature ) override;

  protected:
    //! clears the expression to search for all features
    void clearExpression();

    QString mExpression;
    int mFieldIdx;

};
// We'll use this class inside a QVariant in the widgets properties
Q_DECLARE_METATYPE( QgsSearchWidgetWrapper* )

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsSearchWidgetWrapper::FilterFlags )

#endif // QGSSEARCHWIDGETWRAPPER_H
