/***************************************************************************

               ----------------------------------------------------
              date                 : 16.5.2019
              copyright            : (C) 2019 by Matthias Kuhn
              email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTEMPLATEPROJECTSMODEL_H
#define QGSTEMPLATEPROJECTSMODEL_H

#include <QFileSystemWatcher>
#include <QStandardItemModel>
#include <QTemporaryDir>

class QgsTemplateProjectsModel : public QStandardItemModel
{
    Q_OBJECT

  public:
    /**
     * Custom model roles.
     *
     * \since QGIS 4.0
     */
    enum class CustomRole : int
    {
      TitleRole = Qt::UserRole + 1,
      PathRole,
      NativePathRole,
      CrsRole,
      PreviewImagePathRole,
    };
    Q_ENUM( CustomRole )

    QgsTemplateProjectsModel( QObject *parent = nullptr );

    QHash<int, QByteArray> roleNames() const override;

  private slots:

  private:
    void addTemplateDirectory( const QString &path );
    void scanDirectory( const QString &path );

    QFileSystemWatcher mFileSystemWatcher;

    QTemporaryDir mTemporaryDir;
};


#endif // QGSTEMPLATEPROJECTSMODEL_H
