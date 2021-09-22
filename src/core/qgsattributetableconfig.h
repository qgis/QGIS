/***************************************************************************
  qgsattributetableconfig.h - QgsAttributeTableConfig

 ---------------------
 begin                : 27.4.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSATTRIBUTETABLECONFIG_H
#define QGSATTRIBUTETABLECONFIG_H

#include <QString>
#include <QVector>
#include <QDomNode>
#include <QVariant>

#include "qgis_sip.h"
#include "qgis_core.h"

class QgsFields;

/**
 * \ingroup core
 * \brief This is a container for configuration of the attribute table.
 * The configuration is specific for one vector layer.
 * \since QGIS 2.16
 */

class CORE_EXPORT QgsAttributeTableConfig
{
  public:

    /**
     * The type of an attribute table column.
     */
    enum Type
    {
      Field,    //!< This column represents a field
      Action    //!< This column represents an action widget
    };

    /**
     * Defines the configuration of a column in the attribute table.
     */
    struct ColumnConfig
    {
      //! Constructor for ColumnConfig
      ColumnConfig() = default;

      // TODO c++20 - replace with = default
      bool operator== ( const QgsAttributeTableConfig::ColumnConfig &other ) const SIP_SKIP;

      //! The type of this column.
      QgsAttributeTableConfig::Type type = Field;

      //! The name of the attribute if this column represents a field
      QString name;

      //! Flag that controls if the column is hidden
      bool hidden = false;

      //! Width of column, or -1 for default width
      int width = -1;
    };

    /**
     * The style of the action widget in the attribute table.
     */
    enum ActionWidgetStyle
    {
      ButtonList,   //!< A list of buttons
      DropDown      //!< A tool button with a drop-down to select the current action
    };

    /**
     * Constructor for QgsAttributeTableConfig.
     */
    QgsAttributeTableConfig() = default;

    /**
     * Gets the list with all columns and their configuration.
     * The list order defines the order of appearance.
     */
    QVector<QgsAttributeTableConfig::ColumnConfig> columns() const;

    /**
     * Returns TRUE if the configuration is empty, ie it contains no columns.
     *
     * \see size()
     */
    bool isEmpty() const;

    /**
     * Returns the number of columns in the configuration.
     *
     * \since QGIS 3.22
     */
    int size() const;

#ifdef SIP_RUN
    int __len__() const;
    % MethodCode
    sipRes = sipCpp->size();
    % End
#endif

    /**
     * Maps a visible column index to its original column index.
     * \param visibleColumn index of visible column
     * \returns corresponding index when hidden columns are considered
     */
    int mapVisibleColumnToIndex( int visibleColumn ) const;

    /**
     * Set the list of columns visible in the attribute table.
     * The list order defines the order of appearance.
     */
    void setColumns( const QVector<QgsAttributeTableConfig::ColumnConfig> &columns );

    /**
     * Update the configuration with the given fields.
     * Any field which is present in the configuration but not present in the
     * parameter fields will be removed. Any field which is in the parameter
     * fields but not in the configuration will be appended.
     */
    void update( const QgsFields &fields );

    /**
     * Returns TRUE if the action widget is visible
     */
    bool actionWidgetVisible() const;

    /**
     * Set if the action widget is visible
     */
    void setActionWidgetVisible( bool visible );

    /**
     * Gets the style of the action widget
     */
    ActionWidgetStyle actionWidgetStyle() const;

    /**
     * Set the style of the action widget
     */
    void setActionWidgetStyle( ActionWidgetStyle actionWidgetStyle );

    /**
     * Serialize to XML on layer save
     */
    void writeXml( QDomNode &node ) const;

    /**
     * Deserialize to XML on layer load
     */
    void readXml( const QDomNode &node );

    /**
     * Gets the expression used for sorting.
     */
    QString sortExpression() const;

    /**
     * Set the sort expression used for sorting.
     */
    void setSortExpression( const QString &sortExpression );

#ifndef SIP_RUN

    /**
     * Returns the width of a column, or -1 if column should use default width.
     * \param column column index
     * \see setColumnWidth()
     */
    int columnWidth( int column ) const;
#else

    /**
     * Returns the width of a column, or -1 if column should use default width.
     * \param column column index
     * \throws IndexError if the column is not found
     * \see setColumnWidth()
     */
    int columnWidth( int column ) const;
    % MethodCode
    {
      if ( a0 < 0 || a0 >= sipCpp->size() )
      {
        PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
        sipIsErr = 1;
      }
      else
      {
        return PyLong_FromLong( sipCpp->columnWidth( a0 ) );
      }
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Sets the width of a column.
     * \param column column index
     * \param width column width in pixels, or -1 if column should use default width
     * \see columnWidth()
     */
    void setColumnWidth( int column, int width );
#else

    /**
     * Sets the width of a column.
     * \param column column index
     * \param width column width in pixels, or -1 if column should use default width
     * \throws IndexError if the column is not found
     * \see columnWidth()
     */
    void setColumnWidth( int column, int width );
    % MethodCode
    if ( a0 < 0 || a0 >= sipCpp->size() )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      sipCpp->setColumnWidth( a0, a1 );
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Returns TRUE if the specified column is hidden.
     * \param column column index
     * \see setColumnHidden()
     */
    bool columnHidden( int column ) const;
#else

    /**
     * Returns TRUE if the specified column is hidden.
     * \param column column index
     * \throws IndexError if the column is not found
     * \see setColumnHidden()
     */
    bool columnHidden( int column ) const;
    % MethodCode
    {
      if ( a0 < 0 || a0 >= sipCpp->size() )
      {
        PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
        sipIsErr = 1;
      }
      else
      {
        return PyBool_FromLong( sipCpp->columnHidden( a0 ) );
      }
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Sets whether the specified column should be hidden.
     * \param column column index
     * \param hidden set to TRUE to hide column
     * \see columnHidden()
     */
    void setColumnHidden( int column, bool hidden );
#else

    /**
     * Sets whether the specified column should be hidden.
     * \param column column index
     * \param hidden set to TRUE to hide column
     * \throws IndexError if the column is not found
     * \see columnHidden()
     */
    void setColumnHidden( int column, bool hidden );
    % MethodCode
    if ( a0 < 0 || a0 >= sipCpp->size() )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      sipCpp->setColumnHidden( a0, a1 );
    }
    % End
#endif

    /**
     * Gets the sort order
     * \since QGIS 2.16
     */
    Qt::SortOrder sortOrder() const;

    /**
     * Set the sort order
     * \since QGIS 2.16
     */
    void setSortOrder( Qt::SortOrder sortOrder );

    /**
     * Compare this configuration's columns name, type, and order to \a other.
     * The column's width is not considered.
     */
    bool hasSameColumns( const QgsAttributeTableConfig &other ) const;

    /**
     * Compare this configuration to other.
     */
    bool operator!= ( const QgsAttributeTableConfig &other ) const;

  private:
    QVector<ColumnConfig> mColumns;
    ActionWidgetStyle mActionWidgetStyle = DropDown;
    QString mSortExpression;
    Qt::SortOrder mSortOrder = Qt::AscendingOrder;
};

Q_DECLARE_METATYPE( QgsAttributeTableConfig::ColumnConfig )

#endif // QGSATTRIBUTETABLECONFIG_H
