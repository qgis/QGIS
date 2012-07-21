#ifndef QGSRASTERFORMATOPTIONSWIDGET_H
#define QGSRASTERFORMATOPTIONSWIDGET_H

#include "ui_qgsrasterformatoptionswidgetbase.h"

//class QgsRasterDataProvider;

class GUI_EXPORT QgsRasterFormatOptionsWidget: public QWidget, private Ui::QgsRasterFormatOptionsWidgetBase
{
    Q_OBJECT
  public:
  QgsRasterFormatOptionsWidget( QWidget* parent = 0, QString format = "GTiff", QString provider = "gdal" );
    ~QgsRasterFormatOptionsWidget();

    void setFormat( QString format ) { mFormat = format; }
    void setProvider( QString provider ) { mProvider = provider; }

  public slots:
    QStringList createOptions() const;
    void showProfileButtons( bool show = true );
    void update();
    void apply();

  private slots:
    /* void on_mProfileComboBox_currentIndexChanged( const QString & text ); */
    void profileChanged();
    void on_mProfileNewButton_clicked();
    void on_mProfileDeleteButton_clicked();

  private:

    QString mFormat;
    QString mProvider;

    QString settingsKey( QString profile ) const;
    QString createOptions( QString profile ) const;
    void deleteCreateOptions( QString profile );
    void setCreateOptions( QString profile, QString value );
    QStringList profiles() const;
    void setProfiles() const;

};

#endif // QGSRASTERLAYERSAVEASDIALOG_H
