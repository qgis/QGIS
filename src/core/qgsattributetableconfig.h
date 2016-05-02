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

#include "qgsfield.h"

/**
 * This is a container for configuration of the attribute table.
 * The configuration is specific for one vector layer.
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
      Type mType;    //!< The type of this column.
      QString mName; //!< The name of the attribute if this column represents a field
      bool mHidden;  //!< Flag that controls if the column is hidden
    };

    /**
     * The style of the action widget in the attribute table.
     */
    enum ActionWidgetStyle
    {
      ButtonList,   //!< A list of buttons
      DropDown      //!< A tool button with a dropdown to select the current action
    };

    QgsAttributeTableConfig();

    /**
     * Get the list with all columns and their configuration.
     * The list order defines the order of appearance.
     */
    QVector<ColumnConfig> columns() const;

    /**
     * Set the list of columns visible in the attribute table.
     * The list order defines the order of appearance.
     */
    void setColumns( const QVector<ColumnConfig>& columns );

    /**
     * Update the configuration with the given fields.
     * Any field which is present in the configuration but not present in the
     * parameter fields will be removed. Any field which is in the parameter
     * fields but not in the configuration will be appended.
     */
    void update( const QgsFields& fields );

    /**
     * Returns true if the action widget is visible
     */
    bool actionWidgetVisible() const;

    /**
     * Set if the action widget is visible
     */
    void setActionWidgetVisible( bool visible );

    /**
     * Get the style of the action widget
     */
    ActionWidgetStyle actionWidgetStyle() const;

    /**
     * Set the style of the action widget
     */
    void setActionWidgetStyle( const ActionWidgetStyle& actionWidgetStyle );

    /**
     * Serialize to XML on layer save
     */
    void writeXml( QDomNode& node ) const;

    /**
     * Deserialize to XML on layer load
     */
    void readXml( const QDomNode& node );


  private:
    QVector<ColumnConfig> mColumns;
    ActionWidgetStyle mActionWidgetStyle;
};

Q_DECLARE_METATYPE( QgsAttributeTableConfig::ColumnConfig )

#endif // QGSATTRIBUTETABLECONFIG_H
