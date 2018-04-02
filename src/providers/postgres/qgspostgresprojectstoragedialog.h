#ifndef QGSPOSTGRESPROJECTSTORAGEDIALOG_H
#define QGSPOSTGRESPROJECTSTORAGEDIALOG_H

#include <QDialog>

#include "ui_qgspostgresprojectstoragedialog.h"


class QgsPostgresProjectStorageDialog : public QDialog, private Ui::QgsPostgresProjectStorageDialog
{
    Q_OBJECT
  public:
    explicit QgsPostgresProjectStorageDialog( bool saving, QWidget *parent = nullptr );

    QString connectionName() const;
    QString schemaName() const;
    QString projectName() const;

  signals:

  private slots:
    void populateSchemas();
    void populateProjects();
    void onOK();

  private:
    bool mSaving;  //!< Whether using this dialog for loading or saving a project
};

#endif // QGSPOSTGRESPROJECTSTORAGEDIALOG_H
