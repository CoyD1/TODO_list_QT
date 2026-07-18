#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QVector>
#include "task.h"

class NetworkClient : public QObject
{
    Q_OBJECT

public:
    explicit NetworkClient(QObject* parent = nullptr);

    bool connectToServer(const QString& host, quint16 port, int timeoutMs = 1500);
    void disconnectFromServer();
    bool isConnected() const;

    void sendAddTask(const Task& task);
    void sendRemoveTask(int taskId);
    void sendToggleTask(int taskId);
    void sendEditTask(const Task& task);

signals:
    void connected();
    void disconnected();
    void connectionError(const QString& error);
    void tasksReceived(const QVector<Task>& tasks);

private slots:
    void onConnected();
    void onDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError error);
    void onReadyRead();

private:
    void sendMessage(const QByteArray& data);

    QTcpSocket* m_socket;
    QByteArray m_buffer;
};

#endif
