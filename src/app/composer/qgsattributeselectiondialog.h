/***************************************************************************
                         qgsattributeselectiondialog.h
                         -----------------------------
    begin                : January 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTESELECTIONDIALOG_H
#define QGSATTRIBUTESELECTIONDIALOG_H

#include <QDialog>
#include <QMap>
#include <QSet>

class QGridLayout;
class QgsVectorLayer;
class QPushButton;

/**A dialog to select what attributes to display (in the table item) and with the possibility to set different aliases*/
class QgsAttributeSelectionDialog: public QDialog
{
    Q_OBJECT
  public:
    QgsAttributeSelectionDialog( const QgsVectorLayer* vLayer, const QSet<int>& enabledAttributes, const QMap<int, QString>& aliasMap, QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsAttributeSelectionDialog();

    /**Returns indices of selected attributes*/
    QSet<int> enabledAttributes() const;
    /**Returns alias map (alias might be different than for vector layer)*/
    QMap<int, QString> aliasMap() const;

  private slots:
    void selectAllAttributes();
    void clearAttributes();

  private:
    const QgsVectorLayer* mVectorLayer;
    QGridLayout* mAttributeGridLayout;
    QPushButton* mSelectAllButton;
    QPushButton* mClearButton;

    /**Enables / disables all check boxes in one go*/
    void setAllEnabled( bool enabled );
};

#endif // QGSATTRIBUTESELECTIONDIALOG_H
