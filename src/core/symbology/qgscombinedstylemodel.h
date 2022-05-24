/***************************************************************************
    qgscombinedstylemodel.h
    ---------------
    begin                : May 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMBINEDSTYLEMODEL_H
#define QGSCOMBINEDSTYLEMODEL_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QtGlobal>

SIP_IF_MODULE( CONCATENATED_TABLES_MODEL )

#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
#include <QConcatenateTablesProxyModel>

class QgsStyle;
class QgsStyleModel;
class QgsSingleItemModel;


/**
 * \ingroup core
 * \class QgsCombinedStyleModel
 *
 * \brief A model which contains entities from multiple QgsStyle databases.
 *
 * \note Only available in builds based on Qt 5.13 or later
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsCombinedStyleModel: public QConcatenateTablesProxyModel
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsCombinedStyleModel with the specified \a parent object.
     */
    explicit QgsCombinedStyleModel( QObject *parent SIP_TRANSFERTHIS = nullptr );

    QVariant headerData( int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole ) const override;

    /**
     * Adds a style to the model.
     *
     * Ownership of \a style is not transferred.
     *
     * \see styles()
     * \see addDefaultStyle()
     * \see removeStyle()
     */
    void addStyle( QgsStyle *style );

    /**
     * Adds the default style (QgsStyle::defaultStyle()) to the model.
     *
     * \see addStyle()
     */
    void addDefaultStyle();

    /**
     * Removes a \a style from the model.
     *
     * \see addStyle()
     */
    void removeStyle( QgsStyle *style );

    /**
     * Returns a list of all styles shown in the model.
     *
     * \see addStyle()
     */
    QList< QgsStyle * > styles() const;

    /**
     * Adds an additional icon \a size to generate for Qt::DecorationRole data.
     *
     * This allows style icons to be generated at an icon size which
     * corresponds exactly to the view's icon size in which this model is used.
     */
    void addDesiredIconSize( QSize size );

  private:

    QList< QgsStyle * > mStyles;
    QHash< QgsStyle *, QgsStyleModel * > mOwnedStyleModels;
    QHash< QgsStyle *, QgsSingleItemModel * > mTitleModels;

    QList< QSize > mAdditionalSizes;

};
#endif

#endif //QGSCOMBINEDSTYLEMODEL_H
