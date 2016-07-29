/***************************************************************************
                qgssqlcomposerdialog.cpp
       Dialog to compose SQL queries

begin                : Apr 2016
copyright            : (C) 2016 Even Rouault
email                : even.rouault at spatialys.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSQLCOMPOSERDIALOG_H
#define QGSSQLCOMPOSERDIALOG_H

#include "ui_qgssqlcomposerdialogbase.h"
#include <qgis.h>
#include <qgisgui.h>
#include "qgscontexthelp.h"

#include <QPair>
#include <QStringList>
#include <QSet>

/** \ingroup gui
 * SQL composer dialog
 *  @note not available in Python bindings
 */
class GUI_EXPORT QgsSQLComposerDialog : public QDialog, private Ui::QgsSQLComposerDialogBase
{
    Q_OBJECT

  public:

    //! pair (name, title)
    typedef QPair<QString, QString> PairNameTitle;

    //! pair (name, type)
    typedef QPair<QString, QString> PairNameType;

    /** \ingroup gui
     * Callback to do actions on table selection
     * @note not available in Python bindings
     */
    class GUI_EXPORT TableSelectedCallback
    {
      public:
        virtual ~TableSelectedCallback();
        //! method called when a table is selected
        virtual void tableSelected( const QString& name ) = 0;
    };

    /** \ingroup gui
     * Callback to do validation check on dialog validation.
     * @note not available in Python bindings
     */
    class GUI_EXPORT SQLValidatorCallback
    {
      public:
        virtual ~SQLValidatorCallback();
        //! method should return true if the SQL is valid. Otherwise return false and set the errorReason
        virtual bool isValid( const QString& sql, QString& errorReason, QString& warningMsg ) = 0;
    };

    //! argument of a function
    struct Argument
    {
      //! name
      QString name;
      //! type, or empty if unknown
      QString type;

      //! constructor
      Argument( const QString& nameIn = QString(), const QString& typeIn = QString() ) : name( nameIn ), type( typeIn ) {}
    };

    //! description of server functions
    struct Function
    {
      //! name
      QString name;
      //! return type, or empty if unknown
      QString returnType;
      //! minimum number of argument (or -1 if unknown)
      int minArgs;
      //! maximum number of argument (or -1 if unknown)
      int maxArgs;
      //! list of arguments. May be empty despite minArgs > 0
      QList<Argument> argumentList;

      //! constructor with name and fixed number of arguments
      Function( const QString& nameIn, int args ) : name( nameIn ), minArgs( args ), maxArgs( args ) {}
      //! constructor with name and min,max number of arguments
      Function( const QString& nameIn, int minArgs, int maxArgsIn ) : name( nameIn ), minArgs( minArgs ), maxArgs( maxArgsIn ) {}
      //! default constructor
      Function() : minArgs( -1 ), maxArgs( -1 ) {}
    };

    //! constructor
    explicit QgsSQLComposerDialog( QWidget * parent = nullptr, Qt::WindowFlags fl = QgisGui::ModalDialogFlags );
    virtual ~QgsSQLComposerDialog();

    //! initialize the SQL statement
    void setSql( const QString& sql );

    //! get the SQL statement
    QString sql() const;

    //! add a list of table names
    void addTableNames( const QStringList& list );
    //! add a list of table names
    void addTableNames( const QList<PairNameTitle> & listNameTitle );
    //! add a list of column names
    void addColumnNames( const QStringList& list, const QString& tableName );
    //! add a list of column names
    void addColumnNames( const QList<PairNameType>& list, const QString& tableName );
    //! add a list of operators
    void addOperators( const QStringList& list );
    //! add a list of spatial predicates
    void addSpatialPredicates( const QStringList& list );
    //! add a list of spatial predicates
    void addSpatialPredicates( const QList<Function>& list );
    //! add a list of functions
    void addFunctions( const QStringList& list );
    //! add a list of functions
    void addFunctions( const QList<Function>& list );
    //! add a list of API for autocompletion
    void addApis( const QStringList& list );

    //! set if multiple tables/joins are supported. Default is false
    void setSupportMultipleTables( bool bMultipleTables, QString mainTypename = QString() );

    /** Set a callback that will be called when a new table is selected, so
        that new column names can be added typically.
        Ownership of the callback remains to the caller */
    void setTableSelectedCallback( TableSelectedCallback* tableSelectedCallback );

    /** Set a callback that will be called when the OK button is pushed.
        Ownership of the callback remains to the caller */
    void setSQLValidatorCallback( SQLValidatorCallback* sqlValidatorCallback );

  protected:
    bool eventFilter( QObject *obj, QEvent *event ) override;

  private slots:
    void accept() override;

    void on_mTablesCombo_currentIndexChanged( int );
    void on_mColumnsCombo_currentIndexChanged( int );
    void on_mSpatialPredicatesCombo_currentIndexChanged( int );
    void on_mFunctionsCombo_currentIndexChanged( int );
    void on_mOperatorsCombo_currentIndexChanged( int );
    void on_mAddJoinButton_clicked();
    void on_mRemoveJoinButton_clicked();
    void on_mTableJoins_itemSelectionChanged();

    void on_mButtonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

    void reset();
    void buildSQLFromFields();
    void splitSQLIntoFields();

  private:
    QStringList mApiList;
    QSet<QString> mAlreadySelectedTables;
    TableSelectedCallback* mTableSelectedCallback;
    SQLValidatorCallback* mSQLValidatorCallback;
    QObject* mFocusedObject;
    bool mAlreadyModifyingFields;
    bool mDistinct;
    QString mResetSql;
    QMap<QString, QString> mapTableEntryTextToName;
    QMap<QString, QString> mapColumnEntryTextToName;
    QMap<QString, QString> mapSpatialPredicateEntryTextToName;
    QMap<QString, QString> mapFunctionEntryTextToName;
    QString lastSearchedText;


    void loadTableColumns( const QString& table );
    void functionCurrentIndexChanged( QComboBox* combo,
                                      const QMap<QString, QString>& mapEntryTextToName );
    void getFunctionList( const QList<Function>& list,
                          QStringList& listApi,
                          QStringList& listCombo,
                          QMap<QString, QString>& mapEntryTextToName );
};

#endif
