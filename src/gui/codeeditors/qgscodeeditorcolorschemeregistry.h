/***************************************************************************
                             qgscodeeditorcolorschemeregistry.h
                             ------------------------
    begin                : October 2020
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

#ifndef QGSCODEEDITORCOLORSCHEMEREGISTRY_H
#define QGSCODEEDITORCOLORSCHEMEREGISTRY_H

#include "qgis_gui.h"
#include "qgscodeeditorcolorscheme.h"
#include <QList>

/**
 * \ingroup gui
 * \class QgsCodeEditorColorSchemeRegistry
 * \brief A registry of color schemes for use in QgsCodeEditor widgets.
 *
 * QgsCodeEditorColorSchemeRegistry is not usually directly created, but rather accessed through
 * QgsGui::codeEditorColorSchemeRegistry().
 *
 * \since QGIS 3.16
 */
class GUI_EXPORT QgsCodeEditorColorSchemeRegistry
{
  public:
    /**
     * Constructor for a color scheme registry.
     */
    QgsCodeEditorColorSchemeRegistry();

    /**
     * Adds a color \a scheme to the registry.
     *
     * Returns TRUE if the scheme was successfully added.
     */
    bool addColorScheme( const QgsCodeEditorColorScheme &scheme );

    /**
     * Removes the color scheme with matching \a id from the registry.
     *
     * Returns TRUE if scheme was found and removed.
     */
    bool removeColorScheme( const QString &id );

    /**
     * Returns a list of the QgsCodeEditorColorScheme::id() values for all registered color schemes.
     */
    QStringList schemes() const;

    /**
     * Returns the color scheme with matching \a id.
     *
     * If the specified scheme \a id does not exist then the default scheme will be returned instead.
     */
    QgsCodeEditorColorScheme scheme( const QString &id ) const;

  private:
    QMap<QString, QgsCodeEditorColorScheme> mColorSchemes;
};

#endif // QGSCODEEDITORCOLORSCHEMEREGISTRY_H
