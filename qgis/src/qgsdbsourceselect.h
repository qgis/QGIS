#ifndef QGSDBSOURCESELECT_H
#define QGSDBSOURCESELECT_H
#include "qgsdbsourceselectbase.h"
/*! \class QgsDbSourceSelect
 * \brief Dialog to create connections and add tables from PostgresQL.
 *
 * This dialog allows the user to define and save connection information
 * for PostGIS enabled PostgresQL databases. The user can then connect and add 
 * tables from the database to the map canvas.
 */
class QgsDbSourceSelect : public QgsDbSourceSelectBase 
{
 public:
    //! Constructor
    QgsDbSourceSelect();
    //! Destructor
    ~QgsDbSourceSelect();
    //! Opens the create connection dialog to build a new connection
    void addNewConnection();
    //! Opens a dialog to edit an existing connection
    void editConnection();
    //! Determines the tables the user selected and closes the dialog
    void addTables();
    /*! Connects to the database using the stored connection parameters. 
    * Once connected, available layers are displayed.
    */
    void dbConnect();
    //! String list containing the selected tables
    QStringList selectedTables();
    //! Connection info (database, host, user, password)
    QString connInfo();
 private:
    QString m_connInfo;
    QStringList m_selectedTables;
};


#endif // QGSDBSOURCESELECT_H
