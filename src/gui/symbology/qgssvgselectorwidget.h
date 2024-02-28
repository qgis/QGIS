/***************************************************************************
    qgssvgselectorwidget.h - group and preview selector for SVG files
                             built off of work in qgssymbollayerwidget

    ---------------------
    begin                : April 2, 2013
    copyright            : (C) 2013 by Larry Shaffer
    email                : larrys at dakcarto dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSVGSELECTORWIDGET_H
#define QGSSVGSELECTORWIDGET_H

#include "ui_widget_svgselector.h"
#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgsguiutils.h"
#include "qgsproperty.h"

#include <QSortFilterProxyModel>
#include <QAbstractListModel>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLayout>
#include <QStandardItemModel>
#include <QWidget>
#include <QThread>
#include <QElapsedTimer>
#include <QItemDelegate>


class QCheckBox;
class QLayout;
class QLineEdit;
class QListView;
class QPushButton;
class QTreeView;

class QgsExpressionContextGenerator;
class QgsSvgSelectorListModel;
class QgsPropertyOverrideButton;


#ifndef SIP_RUN
///@cond PRIVATE


/**
 * \ingroup gui
 * \class QgsSvgParametersModel
 * \brief A model to hold dynamic SVG parameters
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsSvgParametersModel : public QAbstractTableModel
{
    Q_OBJECT

  public:
    enum class Column : int
    {
      NameColumn = 0,
      ExpressionColumn = 1,
    };

    QgsSvgParametersModel( QObject *parent = nullptr );

    //! Sets the parameters
    void setParameters( const QMap<QString, QgsProperty> &parameters );
    //! Returns the valid parameters of the model
    QMap<QString, QgsProperty> parameters() const;

    //! Remove the parameters at the given indexes
    void removeParameters( const QModelIndexList &indexList );

    //! Sets the vector layer
    void setLayer( QgsVectorLayer *layer );
    //! Returns the vector layer
    QgsVectorLayer *layer() const {return mLayer;}

    //! Sets the expression context generator
    void setExpressionContextGenerator( const QgsExpressionContextGenerator *generator );
    //! Returns the expression context generator
    const QgsExpressionContextGenerator *expressionContextGenerator() const {return mExpressionContextGenerator;}

    int rowCount( const QModelIndex &parent ) const override;
    int columnCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;

  public slots:
    //! Adds a new parameter
    void addParameter();

  signals:
    //! Emitted when the parameters have changed
    void parametersChanged( const QMap<QString, QgsProperty> &parameters );

  private:
    struct Parameter
    {
      Parameter( const QString &name, const QgsProperty &property )
        : name( name ), property( property ) {}

      QString name;
      QgsProperty property;
    };

    QList<Parameter> mParameters;
    QgsVectorLayer *mLayer = nullptr;
    const QgsExpressionContextGenerator *mExpressionContextGenerator = nullptr;
};

/**
 * \ingroup gui
 * \class QgsSvgParameterValueDelegate
 * \brief A delegate which will show a field expression widget to set the value of the SVG parameter
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsSvgParameterValueDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    QgsSvgParameterValueDelegate( QObject *parent = nullptr )
      : QItemDelegate( parent )
    {}

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
};



/**
 * \ingroup gui
 * \class QgsSvgSelectorLoader
 * \brief Recursively loads SVG images from a path in a background thread.
 */
class GUI_EXPORT QgsSvgSelectorLoader : public QThread
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsSvgSelectorLoader
     * \param parent parent object
     */
    QgsSvgSelectorLoader( QObject *parent = nullptr );

    ~QgsSvgSelectorLoader() override;

    /**
     * Starts the loader finding and generating previews for SVG images. foundSvgs() will be
     * emitted as the loader encounters SVG images.
     * \brief run
     */
    void run() override;

    /**
     * Cancels the current loading operation. Waits until the thread has finished operation
     * before returning.
     */
    virtual void stop();

    /**
     * Sets the root path containing SVG images to load. If no path is set, the default SVG
     * search paths will be used instead.
     */
    void setPath( const QString &path )
    {
      mPath = path;
    }

  signals:

    /**
     * Emitted when the loader has found a block of SVG images. This signal is emitted with blocks
     * of SVG images to prevent spamming any connected model.
     * \param svgs list of SVGs and preview images found.
     */
    void foundSvgs( QStringList svgs );

  private:

    QString mPath;
    bool mCanceled = false;
    QStringList mQueuedSvgs;

    QElapsedTimer mTimer;
    int mTimerThreshold = 0;
    QSet< QString > mTraversedPaths;

    void loadPath( const QString &path );
    void loadImages( const QString &path );

};

/**
 * \ingroup gui
 * \class QgsSvgGroupLoader
 * \brief Recursively loads SVG paths in a background thread.
 */
class GUI_EXPORT QgsSvgGroupLoader : public QThread
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsSvgGroupLoader
     * \param parent parent object
     */
    QgsSvgGroupLoader( QObject *parent = nullptr );

    ~QgsSvgGroupLoader() override;

    /**
     * Starts the loader finding folders for SVG images.
     * \brief run
     */
    void run() override;

    /**
     * Cancels the current loading operation. Waits until the thread has finished operation
     * before returning.
     */
    virtual void stop();

    /**
     * Sets the root path containing child paths to find. If no path is set, the default SVG
     * search paths will be used instead.
     */
    void setParentPaths( const QStringList &parentPaths )
    {
      mParentPaths = parentPaths;
    }

  signals:

    /**
     * Emitted when the loader has found a block of SVG images. This signal is emitted with blocks
     * of SVG images to prevent spamming any connected model.
     * \param svgs list of SVGs and preview images found.
     */
    void foundPath( const QString &parentPath, const QString &path );

  private:

    QStringList mParentPaths;
    bool mCanceled = false;
    QSet< QString > mTraversedPaths;

    void loadGroup( const QString &parentPath );

};

///@endcond
#endif

/**
 * \ingroup gui
 * \class QgsSvgSelectorFilterModel
 * \brief A model for displaying SVG files with a preview icon which can be filtered by file name.
 * Population of the model is performed in a background thread to ensure that
 * initial creation of the model is responsive and does not block the GUI.
 * \since QGIS 3.20
 */
class GUI_EXPORT QgsSvgSelectorFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:

    /**
     * Constructor for creating a model for SVG files in a specific path.
     * \param parent parent object
     * \param path initial path, which is recursively searched
     * \param iconSize desired size of SVG icons to create
     */
    QgsSvgSelectorFilterModel( QObject *parent SIP_TRANSFERTHIS, const QString &path = QString(), int iconSize = 30 );

  private:
    QgsSvgSelectorListModel *mModel = nullptr;
};

/**
 * \ingroup gui
 * \class QgsSvgSelectorListModel
 * \brief A model for displaying SVG files with a preview icon. Population of the model is performed in
 * a background thread to ensure that initial creation of the model is responsive and does
 * not block the GUI.
 */
class GUI_EXPORT QgsSvgSelectorListModel : public QAbstractListModel
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsSvgSelectorListModel. All SVGs in folders from the application SVG
     * search paths will be shown.
     * \param parent parent object
     * \param iconSize desired size of SVG icons to create
     */
    QgsSvgSelectorListModel( QObject *parent SIP_TRANSFERTHIS, int iconSize = 30 );

    /**
     * Constructor for creating a model for SVG files in a specific path.
     * \param parent parent object
     * \param path initial path, which is recursively searched
     * \param iconSize desired size of SVG icons to create
     */
    QgsSvgSelectorListModel( QObject *parent SIP_TRANSFERTHIS, const QString &path, int iconSize = 30 );

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;

  protected:
    QStringList mSvgFiles;

  private:
    QPixmap createPreview( const QString &entry ) const;
    QgsSvgSelectorLoader *mSvgLoader = nullptr;

    int mIconSize = 30;

  private slots:

    /**
     * Called to add SVG files to the model.
     * \param svgs list of SVG files to add to model.
     */
    void addSvgs( const QStringList &svgs );

};


/**
 * \ingroup gui
 * \class QgsSvgSelectorGroupsModel
 * \brief A model for displaying SVG search paths. Population of the model is performed in
 * a background thread to ensure that initial creation of the model is responsive and does
 * not block the GUI.
 */
class GUI_EXPORT QgsSvgSelectorGroupsModel : public QStandardItemModel
{
    Q_OBJECT

  public:
    QgsSvgSelectorGroupsModel( QObject *parent SIP_TRANSFERTHIS );
    ~QgsSvgSelectorGroupsModel() override;

  private:
    QgsSvgGroupLoader *mLoader = nullptr;
    QHash< QString, QStandardItem * > mPathItemHash;

  private slots:

    void addPath( const QString &parentPath, const QString &path );
};

/**
 * \ingroup gui
 * \class QgsSvgSelectorWidget
 */
class GUI_EXPORT QgsSvgSelectorWidget : public QWidget, private Ui::WidgetSvgSelector
{
    Q_OBJECT

  public:

    //! Constructor for QgsSvgSelectorWidget
    QgsSvgSelectorWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Initialize the parameters model so the context and the layer are referenced.
     * \since QGIS 3.18
     */
    void initParametersModel( const QgsExpressionContextGenerator *generator, QgsVectorLayer *layer = nullptr );

    QString currentSvgPath() const;

    /**
     * Returns the source line edit
     * \since QGIS 3.16
     */
    QgsPictureSourceLineEditBase *sourceLineEdit() const {return mSourceLineEdit;}

    /**
     * Defines if the group box to fill parameters is visible
     * \since QGIS 3.18
     */
    void setAllowParameters( bool allow );

    /**
     * Returns if the group box to fill parameters is visible
     * \since QGIS 3.18
     * \deprecated Use allowParameters()
     */
    Q_DECL_DEPRECATED bool allowParamerters() const SIP_DEPRECATED {return mAllowParameters;} // spellok

    /**
     * Returns if the group box to fill parameters is visible
     * \since QGIS 3.38
     */
    bool allowParameters() const {return mAllowParameters;}

    /**
     * Defines if the SVG browser should be visible
     * \since QGIS 3.20
     */
    void setBrowserVisible( bool visible );

    /**
     * Returns if the SVG browser should be visible
     * \since QGIS 3.20
     */
    bool browserVisible() const {return mBrowserVisible;}

    /**
     * Returns the property override tool button of the file line edit
     * \since QGIS 3.20
     */
    QgsPropertyOverrideButton *propertyOverrideToolButton() const;

  public slots:
    //! Accepts absolute paths
    void setSvgPath( const QString &svgPath );

    /**
     * Sets the dynamic parameters
     * \since QGIS 3.18
     */
    void setSvgParameters( const QMap<QString, QgsProperty> &parameters );

  signals:
    void svgSelected( const QString &path );

    /**
     * Emitted when the parameters have changed
     * \since QGIS 3.18
     */
    void svgParametersChanged( const QMap<QString, QgsProperty> &parameters );

  protected:
    void populateList();

  private slots:
    void populateIcons( const QModelIndex &idx );
    void svgSelectionChanged( const QModelIndex &idx );
    void updateCurrentSvgPath( const QString &svgPath );
    void svgSourceChanged( const QString &text );

  private:
    int mIconSize = 30;
    QString mCurrentSvgPath; //!< Always stored as absolute path
    bool mAllowParameters = false;
    bool mBrowserVisible = true;
    QgsSvgParametersModel *mParametersModel = nullptr;
};

/**
 * \ingroup gui
 * \class QgsSvgSelectorDialog
 */
class GUI_EXPORT QgsSvgSelectorDialog : public QDialog
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsSvgSelectorDialog.
     */
    QgsSvgSelectorDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr,
                          Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags,
                          QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Close | QDialogButtonBox::Ok,
                          Qt::Orientation orientation = Qt::Horizontal );

    //! Returns pointer to the embedded SVG selector widget
    QgsSvgSelectorWidget *svgSelector() { return mSvgSelector; }

  protected:
    QVBoxLayout *mLayout = nullptr;
    QDialogButtonBox *mButtonBox = nullptr;
    QgsSvgSelectorWidget *mSvgSelector = nullptr;
};

#endif // QGSSVGSELECTORWIDGET_H
