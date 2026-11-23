/***************************************************************************
    qgsdatetimewithtimezonewidget.h
     --------------------------------------
    Date                 : October 2026
    Copyright            : (C) 2026 Nyall Dawson
    Email                : nyall.dawson@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDATETIMEWITHTIMEZONEWIDGET_H
#define QGSDATETIMEWITHTIMEZONEWIDGET_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsguiutils.h"
#include "qgspanelwidget.h"

#include <QAbstractItemModel>
#include <QConcatenateTablesProxyModel>
#include <QDialog>
#include <QPointer>
#include <QSortFilterProxyModel>
#include <QWidget>

class QgsFilterLineEdit;
class QListView;
class QComboBox;
class QToolButton;
class StandardTimeZoneModel;
class StandardTimeZoneProxyModel;
class QDialogButtonBox;
class QgsDateTimeEdit;

/**
 * \ingroup gui
 * \brief A custom model for display of available time zones.
 *
 * \since QGIS 4.4
 */
class GUI_EXPORT QgsTimeZoneItemModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    //! Model roles
    enum class CustomRole : int
    {
      TimeZoneId = Qt::UserRole, //!< Time zone ID
      TimeZone,                  //!< QTimeZone object
      UtcOffset,                 //!< Offset from UTC
    };
    Q_ENUM( CustomRole )

    /**
     * Constructor for a QgsTimeZoneItemModel with the specified \a parent object.
     */
    QgsTimeZoneItemModel( QObject *parent SIP_TRANSFER = nullptr );

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;

    /**
     * Returns the model index corresponding to a time zone.
     */
    QModelIndex indexFromTimeZone( const QTimeZone &timeZone ) const;

    /**
     * Returns a friendly display string for a time zone.
     */
    static QString displayStringForTimeZone( const QTimeZone &timeZone, QTimeZone::NameType nameType = QTimeZone::NameType::DefaultName );

  private:
    struct TimeZoneItem
    {
        QString id;
        QString displayString;
        QTimeZone timeZone;
        int utcOffset = 0;
    };
    static std::vector< TimeZoneItem > sItems;

    static QString offsetToString( int seconds );
};

/**
 * \ingroup gui
 * \brief A sort filter model for display of available time zones.
 *
 * \since QGIS 4.4
 */
class GUI_EXPORT QgsTimeZoneSortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:
    /**
     * Constructor for a QgsTimeZoneSortFilterModel with the specified \a parent object and \a source model.
     *
     * The source model must exist for the lifetime of the proxy.
     */
    QgsTimeZoneSortFilterModel( QgsTimeZoneItemModel *source, QObject *parent SIP_TRANSFER = nullptr );

    /**
     * Returns the current filter string, if set.
     *
     * \see setFilterString()
     */
    QString filterString() const { return mFilterString; }

    /**
     * Returns the proxy model index corresponding to a time zone.
     */
    QModelIndex indexFromTimeZone( const QTimeZone &timeZone ) const;

  public slots:

    /**
     * Sets a \a filter string, such that only time zones with names containing the
     * specified string will be shown.
     *
     * \see filterString()
    */
    void setFilterString( const QString &filter );

  protected:
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const override;

  private:
    QgsTimeZoneItemModel *mSourceModel = nullptr;
    QString mFilterString;
};

/**
 * \ingroup gui
 * \brief A widget for selection of available time zones.
 *
 * The widget consists of a list of available time zones, along with a filter widget for searching through the available zones.
 *
 * \since QGIS 4.4
 */
class GUI_EXPORT QgsTimeZoneWidget : public QgsPanelWidget
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsTimeZoneWidget, with the specified \a parent widget.
     */
    explicit QgsTimeZoneWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the selected time zone.
     *
     * An invalid time zone will be returned if no item is selected.
     *
     * \see setTimeZone()
     */
    QTimeZone timeZone() const;

  public slots:

    /**
     * Sets the current time zone to select in the widget.
     *
     * \see timeZone()
     */
    void setTimeZone( const QTimeZone &zone );

  signals:

    /**
     * Emitted when the selected time zone is changed.
     *
     * \see setTimeZone()
     * \see timeZone()
     */
    void timeZoneChanged( const QTimeZone &zone );

    /**
     * Emitted when a time zone is double-clicked in the widget.
     */
    void timeZoneDoubleClicked( const QTimeZone &zone );

  private slots:

    void itemDoubleClicked( const QModelIndex &index );

  private:
    QgsTimeZoneItemModel *mModel = nullptr;
    QgsTimeZoneSortFilterModel *mProxyModel = nullptr;

    QgsFilterLineEdit *mFilterEdit = nullptr;
    QListView *mListView = nullptr;
};


/**
 * \ingroup gui
 * \brief A widget for selection of available time zones.
 *
 * A dialog for selection of a time zone, along with a filter widget for searching through the available zones.
 *
 * \since QGIS 4.4
 */
class GUI_EXPORT QgsTimeZoneDialog : public QDialog
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsTimeZoneDialog.
     */
    QgsTimeZoneDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    /**
     * Returns the selected time zone.
     *
     * An invalid time zone will be returned if no item is selected.
     *
     * \see setTimeZone()
     */
    QTimeZone timeZone() const;

  public slots:

    /**
     * Sets the current time zone to select in the dialog.
     *
     * \see timeZone()
     */
    void setTimeZone( const QTimeZone &zone );


  private:
    QgsTimeZoneWidget *mWidget = nullptr;
    QDialogButtonBox *mButtonBox = nullptr;
};

/**
 * \ingroup gui
 * \brief A combo box widget for selection of available time zones.
 *
 * The widget consists of a list of available time zones, along with a filter widget for searching through the available zones.
 *
 * \since QGIS 4.4
 */
class GUI_EXPORT QgsTimeZoneComboBox : public QWidget
{
    Q_OBJECT

  public:
    //! Standard time zone options
    enum class StandardItem SIP_ENUM_BASETYPE( IntFlag )
    {
      Invalid = 1 << 0, //!< Invalid/not set option
      Current = 1 << 1, //!< Current time zone value
      System = 1 << 2,  //!< System time zone
    };

    /**
     * Standard time zone options.
     */
    Q_DECLARE_FLAGS( StandardItems, StandardItem )

    /**
     * Constructor for QgsTimeZoneComboBox, with the specified \a parent widget.
     */
    explicit QgsTimeZoneComboBox( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the selected time zone.
     *
     * An invalid time zone will be returned if no item is selected.
     *
     * \see setTimeZone()
     */
    QTimeZone timeZone() const;

    /**
     * Sets the \a title for the time zone selector dialog window.
     * \see dialogTitle()
     */
    void setDialogTitle( const QString &title );

    /**
     * Returns the title for the time zone selector dialog window.
     * \see setDialogTitle()
     */
    QString dialogTitle() const;

  public slots:

    /**
     * Sets the current time zone to select in the widget.
     *
     * \see timeZone()
     */
    void setTimeZone( const QTimeZone &zone );

  signals:

    /**
     * Emitted when the selected time zone is changed.
     *
     * \see setTimeZone()
     * \see timeZone()
     */
    void timeZoneChanged( const QTimeZone &zone );

  private slots:

    void selectTimeZone();

  private:
    QComboBox *mComboBox = nullptr;
    QToolButton *mButton = nullptr;
    StandardTimeZoneProxyModel *mTimeZoneModel = nullptr;
    int mBlockSignals = 0;
    QPointer< QgsTimeZoneWidget> mActivePanel;
    int mIgnorePanelSignals = 0;
    QString mDialogTitle;
};

SIP_SKIP Q_DECLARE_OPERATORS_FOR_FLAGS( QgsTimeZoneComboBox::StandardItems );

///@cond PRIVATE

class StandardTimeZoneModel : public QAbstractItemModel SIP_SKIP
{
    Q_OBJECT

  public:
    enum class CustomRole : int
    {
      TimeZone = Qt::UserRole, //!< Time zone ID
    };
    StandardTimeZoneModel( QObject *parent SIP_TRANSFERTHIS = nullptr );

    QVariant data( const QModelIndex &index, int role ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex & = QModelIndex() ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;

    QgsTimeZoneComboBox::StandardItem itemForIndex( const QModelIndex &index ) const;
    QModelIndex indexForItem( QgsTimeZoneComboBox::StandardItem item ) const;
    void setCurrentTimeZone( const QTimeZone &timeZone );

  private:
    QString mNotSetText;
    QTimeZone mCurrentTimeZone;
};

class StandardTimeZoneProxyModel : public QSortFilterProxyModel SIP_SKIP
{
    Q_OBJECT

  public:
    StandardTimeZoneProxyModel( QObject *parent SIP_TRANSFERTHIS = nullptr );
    void setDisplayedItems( QgsTimeZoneComboBox::StandardItems items );
    void setCurrentTimeZone( const QTimeZone &timeZone );
    QModelIndex indexForItem( QgsTimeZoneComboBox::StandardItem item ) const;

  protected:
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;

  private:
    StandardTimeZoneModel *mModel = nullptr;
    QgsTimeZoneComboBox::StandardItems mItems;
};


///@endcond PRIVATE

/**
 * \ingroup gui
 * \brief A date time widget with time zone control.
 *
 * \since QGIS 4.4
 */
class GUI_EXPORT QgsDateTimeWithTimeZoneWidget : public QWidget
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsDateTimeWithTimeZoneWidget, with the specified \a parent widget.
     */
    explicit QgsDateTimeWithTimeZoneWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the date time edit portion of the widget.
     *
     * \warning Do not directly set or retrieve the date time from this widget, as it will not
     * respect the time zone portion of the widget.
     */
    QgsDateTimeEdit *dateTimeEdit();

    /**
     * Returns the time zone combo box portion of the widget.
     */
    QgsTimeZoneComboBox *timeZoneComboBox();

    /**
     * Sets the \a title for the time zone selector dialog window.
     * \see dialogTitle()
     */
    void setDialogTitle( const QString &title );

    /**
     * Returns the title for the time zone selector dialog window.
     * \see setDialogTitle()
     */
    QString dialogTitle() const;

    /**
     * Returns the date time shown in the widget, with corresponding time zone set.
     *
     * \see setDateTime()
     */
    QDateTime dateTime() const;

  public slots:

    /**
     * Sets the date time to show in the widget.
     *
     * The time zone will be automatically set to match the time zone of this date time value.
     *
     * \see dateTime()
     */
    void setDateTime( const QDateTime &dateTime );

  signals:

    /**
     * Emitted whenever the date time shown in the widget is changed.
     */
    void dateTimeChanged( const QDateTime &dateTime );

  private:
    QgsDateTimeEdit *mDateTimeEdit = nullptr;
    QgsTimeZoneComboBox *mTimeZoneCombo = nullptr;
    int mBlockSignals = false;
};

#endif // QGSDATETIMEWITHTIMEZONEWIDGET_H
