/***************************************************************************
                             qgsmetadataurlitemdelegate.h
                             ------------------
  begin                : June 21, 2021
  copyright            : (C) 2021 by Etienne Trimaille
  email                : etrimaille at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMETADATAURLITEMDELEGATE_H
#define QGSMETADATAURLITEMDELEGATE_H

#include <QStyledItemDelegate>

#define SIP_NO_FILE

///@cond PRIVATE

/**
 * \ingroup gui
 * \class MetadataUrlItemDelegate
 * \brief Special delegate for the metadata url view.
 * \since QGIS 3.22
 */
class MetadataUrlItemDelegate : public QStyledItemDelegate
{

    Q_OBJECT

  public:

    /**
     * \brief MetadataUrlItemDelegate constructor
     * \param parent
     */
    explicit MetadataUrlItemDelegate( QObject *parent = nullptr );

    /**
     * Create a special editor with a QCombobox in the link view.
     */
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
};


///@endcond

#endif // QGSMETADATAURLITEMDELEGATE_H
