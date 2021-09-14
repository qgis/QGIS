#ifndef QGSHTTPHEADERWIDGET_H
#define QGSHTTPHEADERWIDGET_H

#include <QWidget>
#include "ui_qgshttpheaderwidget.h"
#include "qgshttpheaders.h"


class GUI_EXPORT QgsHttpHeaderWidget : public QWidget, private Ui::QgsHttpHeaderWidget
{
    Q_OBJECT

  public:
    explicit QgsHttpHeaderWidget( QWidget *parent = nullptr );
    ~QgsHttpHeaderWidget();

    QgsHttpHeaders httpHeaders() const;

    /* mRefererLineEdit->setText( settings.value( key + "/referer" ).toString() ); */
    void setFromSettings( const QgsSettings &settings, const QString &key );

    /* settings.setValue( key + "/referer", mRefererLineEdit->text() ); */
    void updateSettings( QgsSettings &settings, const QString &key ) const;

  private slots:
    void setupConnections();
    void addQueryPair();
    void removeQueryPair();

  private:
    void addQueryPairRow( const QString &key, const QString &val );

};

#endif // QGSHTTPHEADERWIDGET_H
