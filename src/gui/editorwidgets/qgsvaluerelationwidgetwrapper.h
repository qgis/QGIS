/***************************************************************************
    qgsvaluerelationwidgetwrapper.h
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVALUERELATIONWIDGETWRAPPER_H
#define QGSVALUERELATIONWIDGETWRAPPER_H

#include <QTableWidget>

#include "qgseditorwidgetwrapper.h"
#include "qgsvaluerelationfieldformatter.h"
#include "qgis_gui.h"

class QComboBox;
class QLineEdit;
class QgsValueRelationWidgetFactory;
class QgsFilterLineEdit;

SIP_NO_FILE

///@cond PRIVATE

/**
 * \brief The QgsFilteredTableWidget class
 *
 * This is a helper widget for QgsValueRelationWidgetWrapper
 * This widget is a QTableWidget showing checkable items, with an optional QgsFilterLineEdit on top that allows filtering the table's items
 */
class QgsFilteredTableWidget : public QWidget
{
    Q_OBJECT

  public:

    /**
     * \brief QgsFilteredTableWidget constructor
     * \param parent
     * \param showSearch Whether the search QgsFilterLineEdit should be visible or not
     */
    QgsFilteredTableWidget( QWidget *parent, bool showSearch );

    bool eventFilter( QObject *watched, QEvent *event ) override;

    /**
     * Returns the list of selected (checked) items
     */
    QStringList selection() const;

    /**
     * Updates the check state of the table's items. Items whose DisplayRole is contained in \a checked are checked, the rest are unchecked
     */
    void checkItems( const QStringList &checked );

    /**
     * Populate the table using \a cache
     */
    void populate( QgsValueRelationFieldFormatter::ValueRelationCache cache );

    /**
     * Sets all items to be partially checked
     */
    void setIndeterminateState();

    /**
     * Set all table items to \a enabled.
     */
    void setEnabledTable( const bool enabled );

    /**
     * Sets the number of columns of the table
     */
    void setColumnCount( const int count );

    /**
     * Returns the number of rows of the table
     */
    int rowCount() const { return mTableWidget->rowCount(); }

  signals:

    /**
     * Emitted when an \a item is changed by the user
     */
    void itemChanged( QTableWidgetItem *item );

  private:
    void filterStringChanged( const QString &filterString );
    void itemChanged_p( QTableWidgetItem *item );
    QTableWidgetItem *item( const int row, const int column ) const { return mTableWidget->item( row, column ); }

    int mColumnCount = 1;
    QgsFilterLineEdit *mSearchWidget = nullptr;
    QTableWidget *mTableWidget = nullptr;
    bool mEnabledTable = true;
    QVector<QPair<QgsValueRelationFieldFormatter::ValueRelationItem, Qt::CheckState>> mCache;
    friend class TestQgsValueRelationWidgetWrapper;
};

///@endcond

/**
 * \ingroup gui
 * \brief Wraps a value relation widget. This widget will offer a combobox with values from another layer
 * referenced by a foreign key (a constraint may be set but is not required on data level).
 * This is useful for having value lists on a separate layer containing codes and their
 * translation to human readable names.
 *
 * Options:
 *
 * <ul>
 * <li><b>Layer</b> <i>The id of the referenced layer.</i></li>
 * <li><b>Key</b> <i>The key field on the referenced layer (code).</i></li>
 * <li><b>Value</b> <i>The value field on the referenced layer (human readable name).</i></li>
 * <li><b>AllowMulti</b> <i>If set to TRUE, will allow multiple selections. This requires the data type to be a string. This does NOT work with normalized database structures.</i></li>
 * <li><b>AllowNull</b> <i>Will offer NULL as a possible value.</i></li>
 * <li><b>FilterExpression</b> <i>If not empty, will be used as expression. Only if this evaluates to TRUE, the value will be shown.</i></li>
 * <li><b>OrderByValue</b> <i>Will order by value instead of key.</i></li>
 * </ul>
 * \note not available in Python bindings
 */

class GUI_EXPORT QgsValueRelationWidgetWrapper : public QgsEditorWidgetWrapper
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsValueRelationWidgetWrapper.
     *
     * The \a layer and \a fieldIdx arguments specify the vector layer field associated with the wrapper.
     *
     * The \a editor argument indicates the editor widget to use with the wrapper. This can be NULLPTR if a
     * new widget should be autogenerated.
     *
     * A \a parent widget for this widget wrapper and the created widget can also be specified.
     */
    explicit QgsValueRelationWidgetWrapper( QgsVectorLayer *layer, int fieldIdx, QWidget *editor = nullptr, QWidget *parent = nullptr );

    QVariant value() const override;

    void showIndeterminateState() override;

    void setEnabled( bool enabled ) override;

  public slots:

    void parentFormValueChanged( const QString &attribute, const QVariant &value ) override;

  protected:
    QWidget *createWidget( QWidget *parent ) override;
    void initWidget( QWidget *editor ) override;
    bool valid() const override;

    /**
     * Will be called when a value in the current edited form or table row
     * changes
     *
     * Update widget cache if the value is used in the filter expression and
     * stores current field values to be used in expression form scope context
     *
     * \param attribute The name of the attribute that changed.
     * \param newValue     The new value of the attribute.
     * \param attributeChanged If TRUE, it corresponds to an actual change of the feature attribute
     * \since QGIS 3.2.0
     */
    void widgetValueChanged( const QString &attribute, const QVariant &newValue, bool attributeChanged );

    /**
     * Will be called when the feature changes
     *
     * Is forwarded to the slot setValue() and updates the widget cache if
     * the filter expression context contains values from the current feature
     *
     * \param feature The new feature
     */
    void setFeature( const QgsFeature &feature ) override;

  private slots:
    void emitValueChangedInternal( const QString &value );

  private:
    void updateValues( const QVariant &value, const QVariantList & = QVariantList() ) override;


    /**
     * Returns the value configured in `NofColumns` or 1 if not
     * a positive integer.
     */
    int columnCount() const;

    //! Returns the variant type of the fk
    QVariant::Type fkType() const;

    //! Sets the values for the widgets, re-creates the cache when required
    void populate( );

    QComboBox *mComboBox = nullptr;
    QgsFilteredTableWidget *mTableWidget = nullptr;
    QLineEdit *mLineEdit = nullptr;

    QgsValueRelationFieldFormatter::ValueRelationCache mCache;
    QgsVectorLayer *mLayer = nullptr;

    bool mEnabled = true;
    QString mExpression;

    friend class QgsValueRelationWidgetFactory;
    friend class TestQgsValueRelationWidgetWrapper;

};

#endif // QGSVALUERELATIONWIDGETWRAPPER_H
