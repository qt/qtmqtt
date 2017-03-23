#ifndef SUBSCRIPTIONWINDOW_H
#define SUBSCRIPTIONWINDOW_H

#include <QWidget>
#include <QtMqtt/QMqttSubscription>

namespace Ui {
class SubscriptionWindow;
}

class SubscriptionWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SubscriptionWindow(QSharedPointer<QMqttSubscription> &sub, QWidget *parent = 0);
    ~SubscriptionWindow();

public slots:
    void updateMessage(const QString &text);
    void updateStatus(QMqttSubscription::SubscriptionState state);
private:
    QSharedPointer<QMqttSubscription> m_sub;
    Ui::SubscriptionWindow *ui;
};

#endif // SUBSCRIPTIONWINDOW_H
