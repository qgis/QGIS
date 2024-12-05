/***************************************************************************
                              qgsgrassutils.h
                             -------------------
    begin                : March, 2006
    copyright            : (C) 2006 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGRASSUTILS_H
#define QGSGRASSUTILS_H

#include <QObject>
class QDialog;
class QLineEdit;
class QLabel;
class QPushButton;
class QgisInterface;

/**
 * \class QgsGrassUtils
 *  \brief Various utilities.
 */
class QgsGrassUtils
{
  public:
    //! Constructor
    QgsGrassUtils() = default;

  public:
    // Create vector name from map name, layer name and number of layers
    static QString vectorLayerName( QString map, QString layer, int nLayers );

    // Add all vector layers to QGIS view
    static void addVectorLayers( QgisInterface *iface, QString gisbase, QString location, QString mapset, QString map );

    // Check if element exists in current mapset
    static bool itemExists( QString element, QString item );

    //! Gets path to custom HTML browser starter executable
    static QString htmlBrowserPath();
};

/**
 * \class QgsGrassElementDialog
 * \brief Dialog for entering a name for a new GRASS element.
 */
class QgsGrassElementDialog : public QObject
{
    Q_OBJECT

  public:
    //! Constructor
    explicit QgsGrassElementDialog( QWidget *parent );

  public:
    //! Gets a name for new GRASS element (map)
    // \param source local source
    QString getItem( QString element, QString title, QString label, QString text, QString source = QString(), bool *ok = nullptr );

  public slots:
    void textChanged();

  private:
    QString mElement;
    QString mSource;
    QDialog *mDialog = nullptr;
    QLineEdit *mLineEdit = nullptr;
    QLabel *mLabel = nullptr;
    QLabel *mErrorLabel = nullptr;
    QPushButton *mOkButton = nullptr;
    QPushButton *mCancelButton = nullptr;
    QWidget *mParent = nullptr;
};

#endif // QGSGRASSUTILS_H
