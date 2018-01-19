#ifndef QWINEVENTNOTIFIER_H
#define QWINEVENTNOTIFIER_H
#include "qgis_core.h"
#include <QObject>

#include <windows.h>

// Ugly: copied private Qt header file
QT_BEGIN_NAMESPACE

class Q_CORE_EXPORT QWinEventNotifier : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QObject)

public:
    explicit QWinEventNotifier(QObject *parent = nullptr);
    explicit QWinEventNotifier(HANDLE hEvent, QObject *parent = nullptr);
    ~QWinEventNotifier();

    void setHandle(HANDLE hEvent);
    HANDLE handle() const;

    bool isEnabled() const;

public Q_SLOTS:
    void setEnabled(bool enable);

Q_SIGNALS:
    void activated(HANDLE hEvent);

protected:
    bool event(QEvent *e);

private:
    Q_DISABLE_COPY(QWinEventNotifier)

    HANDLE handleToEvent;
    bool enabled;
};

QT_END_NAMESPACE

#endif // QWINEVENTNOTIFIER_H
