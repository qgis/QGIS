/***************************************************************************
                         qgsnumericformatguiregistry.h
                         -----------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNUMERICFORMATGUIREGISTRY_H
#define QGSNUMERICFORMATGUIREGISTRY_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include <QList>
#include <QMap>

class QgsNumericFormatWidget;
class QgsNumericFormat;

/**
 * Interface base class for factories for numeric format configuration widgets.
 *
 * \ingroup gui
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsNumericFormatConfigurationWidgetFactory
{
  public:
    virtual ~QgsNumericFormatConfigurationWidgetFactory() = default;

    /**
     * Create a new configuration widget for a format.
     */
    virtual QgsNumericFormatWidget *create( const QgsNumericFormat *format ) const = 0 SIP_TRANSFERBACK;
};

/**
 * The QgsNumericFormatGuiRegistry is a home for widgets for configuring QgsNumericFormat objects.
 *
 * QgsNumericFormatGuiRegistry is not usually directly created, but rather accessed through
 * QgsGui::numericFormatGuiRegistry().
 *
 * \ingroup gui
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsNumericFormatGuiRegistry
{
  public:
    /**
     * Constructor. Should never be called manually, is already
     * created by QgsGui.
     */
    QgsNumericFormatGuiRegistry();
    ~QgsNumericFormatGuiRegistry();

    /**
     * Add a new configuration widget factory for customizing a numeric format with the specified \a id.
     *
     * Ownership is taken by the reigstry.
     */
    void addFormatConfigurationWidgetFactory( const QString &id, QgsNumericFormatConfigurationWidgetFactory *factory SIP_TRANSFER );

    /**
     * Removes the configuration widget factory for customizing numeric formats with the specified \a id.
     */
    void removeFormatConfigurationWidgetFactory( const QString &id );

    /**
     * Returns a new configuration widget for an \a format.
     *
     * Returns NULLPTR if no configuration widgets are available for the specified \a format.
     */
    QgsNumericFormatWidget *formatConfigurationWidget( const QgsNumericFormat *format ) const SIP_TRANSFERBACK;

  private:
    QMap<QString, QgsNumericFormatConfigurationWidgetFactory *> mFormatConfigurationWidgetFactories;
};

#endif // QGSNUMERICFORMATGUIREGISTRY_H
