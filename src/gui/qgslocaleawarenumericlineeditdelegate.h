/***************************************************************************
  qgslocaleawarenumericlineeditdelegate.h - QgsLocaleAwareNumericLineEditDelegate

 ---------------------
 begin                : 5.11.2020
 copyright            : (C) 2020 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLOCALEAWARENUMERICLINEEDITDELEGATE_H
#define QGSLOCALEAWARENUMERICLINEEDITDELEGATE_H

#include <QStyledItemDelegate>

#include "qgis.h"

#define SIP_NO_FILE

/// @cond PRIVATE

/**
 * QgsLocaleAwareNumericLineEditDelegate class provides a QLineEdit editor
 * for QgsColorRampShaderWidget and QgsPalettedRendererWidget value columns, it accepts
 * localized numeric inputs and displays the proper number of decimals according to the
 * raster band data type.
 *
 * This delegate assumes that the value stored in the DisplayRole is numeric (can be converted to double).
 *
 */
class QgsLocaleAwareNumericLineEditDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    /**
     * QgsLocaleAwareNumericLineEditDelegate
     * \param dataType raster band data type
     * \param parent
     */
    QgsLocaleAwareNumericLineEditDelegate( Qgis::DataType dataType, QWidget *parent = nullptr );

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

    // QAbstractItemDelegate interface
  public:
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
    QString displayText( const QVariant &value, const QLocale &locale ) const override;

    /**
     * Sets data type to \a dataType
     */
    void setDataType( const Qgis::DataType &dataType );

  private:
    Qgis::DataType mDataType;
};

///@endcond

#endif // QGSLOCALEAWARENUMERICLINEEDITDELEGATE_H
