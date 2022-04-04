/***************************************************************************
                          qgscustomprojectionoptions.h

                             -------------------
    begin                : 2005
    copyright            : (C) 2005 by Tim Sutton
    email                : tim@linfiniti.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCUSTOMPROJECTIONOPTIONS_H
#define QGSCUSTOMPROJECTIONOPTIONS_H

#include "ui_qgscustomprojectiondialogbase.h"
#include "qgsoptionswidgetfactory.h"
#include "qgshelp.h"
#include "qgscoordinatereferencesystem.h"
#include "qgis_app.h"

class QDir;

/**
 * \ingroup app
 * \class QgsCustomProjectionOptionsWidget
 * \brief The custom projection widget is used to define the projection family, ellipsoid and parameters needed by proj to assemble a customized projection definition.
 *
 * The resulting projection will be stored in an sqlite backend.
 */
class QgsCustomProjectionOptionsWidget: public QgsOptionsPageWidget, private Ui::QgsCustomProjectionWidgetBase
{
    Q_OBJECT
  public:
    QgsCustomProjectionOptionsWidget( QWidget *parent = nullptr );

    QString helpKey() const override;
    bool isValid() override;
    void apply() override;

  private slots:
    void pbnAdd_clicked();
    void pbnRemove_clicked();
    void leNameList_currentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem *prev );
    void updateListFromCurrentItem();

  private:

    //helper functions
    void populateList();
    bool saveCrs( const QgsCoordinateReferenceSystem &crs, const QString &name, const QString &id, bool newEntry, Qgis::CrsDefinitionFormat format );
    QString multiLineWktToSingleLine( const QString &wkt );

    //These two QMap store the values as they are on the database when loading
    QMap <QString, QString> mExistingCRSproj;
    QMap <QString, QString> mExistingCRSwkt;
    QMap <QString, QString> mExistingCRSnames;

    struct Definition
    {
      QString name;
      QString id;
      QString wkt;
      QString proj;
    };

    enum Roles
    {
      FormattedWktRole = Qt::UserRole + 1,
    };

    QList< Definition > mDefinitions;

    //vector saving the CRS to be deleted
    QStringList mDeletedCRSs;

    //Columns in the tree widget
    enum Columns { QgisCrsNameColumn, QgisCrsIdColumn, QgisCrsParametersColumn };

    int mBlockUpdates = 0;

};


class QgsCustomProjectionOptionsFactory : public QgsOptionsWidgetFactory
{
    Q_OBJECT

  public:

    QgsCustomProjectionOptionsFactory();

    QIcon icon() const override;
    QgsOptionsPageWidget *createWidget( QWidget *parent = nullptr ) const override;
    QStringList path() const override;

};


#endif // QGSCUSTOMPROJECTIONOPTIONS_H
