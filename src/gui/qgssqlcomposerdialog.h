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
#include "qgis_sip.h"
#include "qgsguiutils.h"

#include <QPair>
#include <QStringList>
#include <QSet>
#include "qgis_gui.h"
#include "qgssubsetstringeditorinterface.h"

SIP_NO_FILE

class QgsVectorLayer;

/**
 * \ingroup gui
 * \brief SQL composer dialog
 *  \note not available in Python bindings
 */
class GUI_EXPORT QgsSQLComposerDialog : public QgsSubsetStringEditorInterface, private Ui::QgsSQLComposerDialogBase
{
    Q_OBJECT

  public:

    //! pair (name, title)
    typedef QPair<QString, QString> PairNameTitle;

    //! pair (name, type)
    typedef QPair<QString, QString> PairNameType;

    /**
     * \ingroup gui
     * \brief Callback to do actions on table selection
     * \note not available in Python bindings
     */
    class GUI_EXPORT TableSelectedCallback
    {
      public:
        virtual ~TableSelectedCallback() = default;
        //! method called when a table is selected
        virtual void tableSelected( const QString &name ) = 0;
    };

    /**
     * \ingroup gui
     * \brief Callback to do validation check on dialog validation.
     * \note not available in Python bindings
     */
    class GUI_EXPORT SQLValidatorCallback
    {
      public:
        virtual ~SQLValidatorCallback() = default;
        //! method should return TRUE if the SQL is valid. Otherwise return FALSE and set the errorReason
        virtual bool isValid( const QString &sql, QString &errorReason, QString &warningMsg ) = 0;
    };

    //! argument of a function
    struct Argument
    {
      //! name
      QString name;
      //! type, or empty if unknown
      QString type;

      //! constructor
      Argument( const QString &nameIn = QString(), const QString &typeIn = QString() ) : name( nameIn ), type( typeIn ) {}
    };

    //! description of server functions
    struct Function
    {
      //! name
      QString name;
      //! Returns type, or empty if unknown
      QString returnType;
      //! minimum number of argument (or -1 if unknown)
      int minArgs = -1;
      //! maximum number of argument (or -1 if unknown)
      int maxArgs = -1;
      //! list of arguments. May be empty despite minArgs > 0
      QList<Argument> argumentList;

      //! constructor with name and fixed number of arguments
      Function( const QString &nameIn, int args ) : name( nameIn ), minArgs( args ), maxArgs( args ) {}
      //! constructor with name and min,max number of arguments
      Function( const QString &nameIn, int minArgs, int maxArgsIn ) : name( nameIn ), minArgs( minArgs ), maxArgs( maxArgsIn ) {}

      Function() = default;
    };

    //! constructor
    explicit QgsSQLComposerDialog( QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    /**
     * This constructor is used on an existing layer. On successful accept, it will update the layer subset string.
     * \param layer existing vector layer
     * \param parent Parent widget
     * \param fl dialog flags
     */
    QgsSQLComposerDialog( QgsVectorLayer *layer, QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    ~QgsSQLComposerDialog() override;

    //! initialize the SQL statement
    void setSql( const QString &sql );

    //! Gets the SQL statement
    QString sql() const;

    QString subsetString() const override { return sql(); }
    void setSubsetString( const QString &subsetString ) override { setSql( subsetString ); }

    //! add a list of table names
    void addTableNames( const QStringList &list );
    //! add a list of table names
    void addTableNames( const QList<PairNameTitle> &listNameTitle );
    //! add a list of column names
    void addColumnNames( const QStringList &list, const QString &tableName );
    //! add a list of column names
    void addColumnNames( const QList<PairNameType> &list, const QString &tableName );
    //! add a list of operators
    void addOperators( const QStringList &list );
    //! add a list of spatial predicates
    void addSpatialPredicates( const QStringList &list );
    //! add a list of spatial predicates
    void addSpatialPredicates( const QList<Function> &list );
    //! add a list of functions
    void addFunctions( const QStringList &list );
    //! add a list of functions
    void addFunctions( const QList<Function> &list );
    //! add a list of API for autocompletion
    void addApis( const QStringList &list );

    //! Sets if multiple tables/joins are supported. Default is FALSE
    void setSupportMultipleTables( bool bMultipleTables, const QString &mainTypename = QString() );

    /**
     * Set a callback that will be called when a new table is selected, so
     * that new column names can be added typically.
     * Ownership of the callback remains to the caller.
    */
    void setTableSelectedCallback( TableSelectedCallback *tableSelectedCallback );

    /**
     * Set a callback that will be called when the OK button is pushed.
     * Ownership of the callback remains to the caller.
    */
    void setSQLValidatorCallback( SQLValidatorCallback *sqlValidatorCallback );

  protected:
    bool eventFilter( QObject *obj, QEvent *event ) override;

  private slots:
    void accept() override;

    void mTablesCombo_currentIndexChanged( int );
    void mColumnsCombo_currentIndexChanged( int );
    void mSpatialPredicatesCombo_currentIndexChanged( int );
    void mFunctionsCombo_currentIndexChanged( int );
    void mOperatorsCombo_currentIndexChanged( int );
    void mAddJoinButton_clicked();
    void mRemoveJoinButton_clicked();
    void mTableJoins_itemSelectionChanged();
    void showHelp();
    void reset();
    void buildSQLFromFields();
    void splitSQLIntoFields();

  private:
    QgsVectorLayer *mLayer = nullptr;
    QStringList mApiList;
    QSet<QString> mAlreadySelectedTables;
    TableSelectedCallback *mTableSelectedCallback = nullptr;
    SQLValidatorCallback *mSQLValidatorCallback = nullptr;
    QObject *mFocusedObject = nullptr;
    bool mAlreadyModifyingFields = false;
    bool mDistinct = false;
    QString mResetSql;
    QMap<QString, QString> mapTableEntryTextToName;
    QMap<QString, QString> mapColumnEntryTextToName;
    QMap<QString, QString> mapSpatialPredicateEntryTextToName;
    QMap<QString, QString> mapFunctionEntryTextToName;
    QString lastSearchedText;


    void loadTableColumns( const QString &table );
    void functionCurrentIndexChanged( QComboBox *combo,
                                      const QMap<QString, QString> &mapEntryTextToName );
    void getFunctionList( const QList<Function> &list,
                          QStringList &listApi,
                          QStringList &listCombo,
                          QMap<QString, QString> &mapEntryTextToName );
};

#endif
