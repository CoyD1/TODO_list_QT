#ifndef TASKSERVER_H
#define TASKSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include "taskmanager.h"

class TaskServer : public QObject
{
    Q_OBJECT

public:
    explicit TaskServer(TaskManager* taskManager, QObject* parent = nullptr);

    bool start(quint16 port);
    void stop();
    bool isRunning() const;

signals:
    void serverStarted();
    void serverStopped();
    void clientConnected(const QString& address);
    void clientDisconnected(const QString& address);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();

private:
    void sendToClient(QTcpSocket* socket, const QByteArray& data);
    void broadcastMessage(const QByteArray& data);
    void processMessage(QTcpSocket* sender, const QByteArray& data);

    QTcpServer* m_server;
    QVector<QTcpSocket*> m_clients;
    TaskManager* m_taskManager;
};

#endif
