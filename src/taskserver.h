#ifndef TASKSERVER_H
#define TASKSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QHash>
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
    void broadcastTasks();

private:
    void sendToClient(QTcpSocket* socket, const QByteArray& data);
    void broadcastMessage(const QByteArray& data);
    void processMessage(const QByteArray& data);

    QTcpServer* m_server;
    QVector<QTcpSocket*> m_clients;
    QHash<QTcpSocket*, QByteArray> m_clientBuffers;
    TaskManager* m_taskManager;
};

#endif
