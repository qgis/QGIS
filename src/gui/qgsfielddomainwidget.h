/***************************************************************************
    qgsfielddomainwidget.h
    ------------------
    Date                 : February 2022
    Copyright            : (C) 2022 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFIELDDOMAINWIDGET_H
#define QGSFIELDDOMAINWIDGET_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "ui_qgsfielddomainwidgetbase.h"
#include "ui_qgsrangedomainwidgetbase.h"
#include "ui_qgsglobdomainwidgetbase.h"
#include "ui_qgscodedvaluedomainwidgetbase.h"
#include "qgis.h"
#include "qgsfielddomain.h"
#include <QAbstractTableModel>
#include <QDialog>

class QDialogButtonBox;

#ifndef SIP_RUN

/**
 * \ingroup gui
 * \brief Abstract base class for widgets which configure the extended properties of a QgsFieldDomain subclass.
 *
 * \see QgsFieldDomainWidget
 *
 * \note Not available in Python bindings
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsAbstractFieldDomainWidget : public QWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsAbstractFieldDomainWidget, with the specified \a parent widget.
     */
    QgsAbstractFieldDomainWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    virtual ~QgsAbstractFieldDomainWidget();

    /**
     * Sets the current field domain to show properties for in the widget.
     *
     * \see createFieldDomain()
     */
    virtual void setFieldDomain( const QgsFieldDomain *domain ) = 0;

    /**
     * Creates a new field domain using the properties from the widget.
     *
     * Caller takes ownership of the returned object.
     *
     * \see setFieldDomain()
     */
    virtual QgsFieldDomain *createFieldDomain( const QString &name, const QString &description, QVariant::Type fieldType ) const = 0 SIP_FACTORY;

    /**
     * Returns TRUE if the widget currently represents a valid field domain configuration.
     */
    virtual bool isValid() const = 0;

  signals:

    /**
     * Emitted whenever the field domain configuration in the widget changes.
     */
    void changed();

};

/**
 * \ingroup gui
 * \brief A widget for configuration of the extended properties of a QgsRangeFieldDomain.
 *
 * \see QgsFieldDomainWidget
 *
 * \note Not available in Python bindings
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsRangeDomainWidget : public QgsAbstractFieldDomainWidget, private Ui_QgsRangeDomainWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsRangeDomainWidget, with the specified \a parent widget.
     */
    QgsRangeDomainWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    void setFieldDomain( const QgsFieldDomain *domain ) override;
    QgsFieldDomain *createFieldDomain( const QString &name, const QString &description, QVariant::Type fieldType ) const override SIP_FACTORY;
    bool isValid() const override;

};

/**
 * \ingroup gui
 * \brief A widget for configuration of the extended properties of a QgsGlobFieldDomain.
 *
 * \see QgsFieldDomainWidget
 *
 * \note Not available in Python bindings
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsGlobDomainWidget : public QgsAbstractFieldDomainWidget, private Ui_QgsGlobDomainWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsGlobDomainWidget, with the specified \a parent widget.
     */
    QgsGlobDomainWidget( QWidget *parent SIP_TRANSFERTHIS  = nullptr );

    void setFieldDomain( const QgsFieldDomain *domain ) override;
    QgsFieldDomain *createFieldDomain( const QString &name, const QString &description, QVariant::Type fieldType ) const override SIP_FACTORY;
    bool isValid() const override;
};

/**
 * \ingroup gui
 * \brief A table model for representing the values in a QgsCodedValue list.
 *
 * \note Not available in Python bindings
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsCodedValueTableModel : public QAbstractTableModel
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsCodedValueTableModel, with the specified \a parent object.
     */
    QgsCodedValueTableModel( QObject *parent );

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    bool insertRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;
    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;

    /**
     * Sets the \a values to show in the model.
     *
     * \see values()
     */
    void setValues( const QList< QgsCodedValue > &values );

    /**
     * Returns the values from the model.
     *
     * \see setValues()
     */
    QList< QgsCodedValue > values() const { return mValues; }

  private:

    QList<QgsCodedValue> mValues;
};


/**
 * \ingroup gui
 * \brief A widget for configuration of the extended properties of a QgsCodedFieldDomain.
 *
 * \see QgsCodedFieldDomainWidget
 *
 * \note Not available in Python bindings
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsCodedFieldDomainWidget : public QgsAbstractFieldDomainWidget, private Ui_QgsCodedValueDomainWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsCodedFieldDomainWidget, with the specified \a parent widget.
     */
    QgsCodedFieldDomainWidget( QWidget *parent SIP_TRANSFERTHIS  = nullptr );

    void setFieldDomain( const QgsFieldDomain *domain ) override;
    QgsFieldDomain *createFieldDomain( const QString &name, const QString &description, QVariant::Type fieldType ) const override SIP_FACTORY;
    bool isValid() const override;

  private:

    QgsCodedValueTableModel *mModel = nullptr;

};
#endif

/**
 * \ingroup gui
 * \brief A widget for configuration of the properties of a QgsFieldDomain.
 *
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsFieldDomainWidget : public QWidget, private Ui_QgsFieldDomainWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsFieldDomainWidget for the given domain \a type, with the specified \a parent widget.
     */
    QgsFieldDomainWidget( Qgis::FieldDomainType type, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the current field domain to show properties for in the widget.
     *
     * \see createFieldDomain()
     */
    void setFieldDomain( const QgsFieldDomain *domain );

    /**
     * Creates a new field domain using the properties from the widget.
     *
     * Caller takes ownership of the returned object.
     *
     * \see setFieldDomain()
     */
    QgsFieldDomain *createFieldDomain() const SIP_FACTORY;

    /**
     * Returns TRUE if the widget currently represents a valid field domain configuration.
     *
     * \see validityChanged()
     */
    bool isValid() const;

  signals:

    /**
     * Emitted whenever the validity of the field domain configuration in the widget changes.
     *
     * \see isValid()
     */
    void validityChanged( bool isValid );

  private:

    QgsAbstractFieldDomainWidget *mDomainWidget = nullptr;
};



/**
 * \ingroup gui
 * \brief A dialog for configuration of the properties of a QgsFieldDomain.
 *
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsFieldDomainDialog: public QDialog
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsFieldDomainDialog for the given domain \a type, with the specified \a parent widget and window \a flags.
     */
    explicit QgsFieldDomainDialog( Qgis::FieldDomainType type, QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags flags = Qt::WindowFlags() );

    /**
     * Sets the current field domain to show properties for in the dialog.
     *
     * \see createFieldDomain()
     */
    void setFieldDomain( const QgsFieldDomain *domain );

    /**
     * Creates a new field domain using the properties from the dialog.
     *
     * Caller takes ownership of the returned object.
     *
     * \see setFieldDomain()
     */
    QgsFieldDomain *createFieldDomain() const SIP_FACTORY;

  public slots:

    void accept() override;

  private slots:

    void validityChanged( bool isValid );

  private:
    QgsFieldDomainWidget *mWidget = nullptr;
    QDialogButtonBox *mButtonBox = nullptr;
};


#endif // QGSFIELDDOMAINWIDGET_H
