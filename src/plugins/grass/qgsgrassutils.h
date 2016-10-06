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

/** \class QgsGrassUtils
 *  \brief Various utilities.
 */
class QgsGrassUtils
{

  public:
    //! Constructor
    QgsGrassUtils();

    //! Destructor
    ~QgsGrassUtils();

  public:
    // Create vector name from map name, layer name and number of layers
    static QString vectorLayerName( QString map, QString layer, int nLayers );

    // Add all vector layers to QGIS view
    static void addVectorLayers( QgisInterface *iface,
                                 QString gisbase, QString location, QString mapset, QString map );

    // Check if element exists in current mapset
    static bool itemExists( QString element, QString item );

    //! Get path to custom HTML browser starter executable
    static QString htmlBrowserPath();
};

/** \class QgsGrassElementDialog
 *  \brief Get name for new element
 */
class QgsGrassElementDialog: public QObject
{
    Q_OBJECT

  public:
    //! Constructor
    explicit QgsGrassElementDialog( QWidget *parent );

    //! Destructor
    ~QgsGrassElementDialog();

  public:
    //! Get a name for new GRASS element (map)
    // \param source local source
    QString getItem( QString element,
                     QString title, QString label,
                     QString text, QString source = 0,
                     bool * ok = 0 );

  public slots:
    void textChanged();

  private:
    QString mElement;
    QString mSource;
    QDialog *mDialog;
    QLineEdit *mLineEdit;
    QLabel *mLabel;
    QLabel *mErrorLabel;
    QPushButton *mOkButton;
    QPushButton *mCancelButton;
    QWidget *mParent;
};

#endif // QGSGRASSUTILS_H
