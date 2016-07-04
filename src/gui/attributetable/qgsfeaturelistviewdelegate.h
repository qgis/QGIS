/***************************************************************************
    qgsfeaturelistviewdelegate.h
    ---------------------
    begin                : February 2013
    copyright            : (C) 2013 by Matthias Kuhn
    email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSATTRIBUTELISTVIEWDELEGATE_H
#define QGSATTRIBUTELISTVIEWDELEGATE_H

#include <QItemDelegate>
#include <QItemSelectionModel>

#include "qgsfeature.h"

class QgsVectorLayer;
class QgsFeatureListModel;
class QgsFeatureSelectionModel;
class QPosition;

/** \ingroup gui
 * \class QgsFeatureListViewDelegate
 */
class GUI_EXPORT QgsFeatureListViewDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    static int const sIconSize = 24;

    enum Element
    {
      EditElement,
      SelectionElement
    };

    explicit QgsFeatureListViewDelegate( QgsFeatureListModel* listModel, QObject *parent = nullptr );

    void setEditSelectionModel( QItemSelectionModel* editSelectionModel );

    Element positionToElement( QPoint pos );

    void setFeatureSelectionModel( QgsFeatureSelectionModel* featureSelectionModel );

    void setCurrentFeatureEdited( bool state );

  signals:
    void editButtonClicked( QModelIndex& index );

  protected:
    virtual QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const override;
    virtual void paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override;

  private:
    QgsFeatureSelectionModel* mFeatureSelectionModel;
    QItemSelectionModel* mEditSelectionModel;
    QgsFeatureListModel* mListModel;
    //! Set to true if the current edit selection has been edited
    bool mCurrentFeatureEdited;
};

#endif // QGSATTRIBUTELISTVIEWDELEGATE_H
