#include "taskserver.h"
#include "message.h"

#include <QJsonDocument>
#include <QDataStream>
#include <QIODevice>

TaskServer::TaskServer(TaskManager* taskManager, QObject* parent)
    : QObject(parent),
    m_server(new QTcpServer(this)),
    m_taskManager(taskManager)
{
    connect(m_server, &QTcpServer::newConnection,
            this, &TaskServer::onNewConnection);
}

bool TaskServer::start(quint16 port)
{
    if (!m_server->listen(QHostAddress::Any, port))
    {
        return false;
    }

    emit serverStarted();
    return true;
}

void TaskServer::stop()
{
    for (QTcpSocket* socket : m_clients)
    {
        socket->disconnectFromHost();
    }

    m_clients.clear();
    m_server->close();
    emit serverStopped();
}

bool TaskServer::isRunning() const
{
    return m_server->isListening();
}

void TaskServer::onNewConnection()
{
    while (m_server->hasPendingConnections())
    {
        QTcpSocket* socket = m_server->nextPendingConnection();
        m_clients.append(socket);

        connect(socket, &QTcpSocket::readyRead,
                this, &TaskServer::onReadyRead);
        connect(socket, &QTcpSocket::disconnected,
                this, &TaskServer::onClientDisconnected);

        emit clientConnected(socket->peerAddress().toString());

        SyncMessage sync(m_taskManager->tasks());
        QJsonDocument doc(sync.toJson());
        sendToClient(socket, doc.toJson(QJsonDocument::Compact));
    }
}

void TaskServer::onReadyRead()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());

    if (!socket)
    {
        return;
    }

    while (socket->bytesAvailable() > 0)
    {
        QByteArray data = socket->readAll();
        processMessage(socket, data);
    }
}

void TaskServer::onClientDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());

    if (!socket)
    {
        return;
    }

    m_clients.removeOne(socket);
    emit clientDisconnected(socket->peerAddress().toString());
    socket->deleteLater();
}

void TaskServer::sendToClient(QTcpSocket* socket, const QByteArray& data)
{
    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream << static_cast<quint32>(data.size());
    packet.append(data);
    socket->write(packet);
}

void TaskServer::broadcastMessage(const QByteArray& data)
{
    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream << static_cast<quint32>(data.size());
    packet.append(data);

    for (QTcpSocket* socket : m_clients)
    {
        socket->write(packet);
    }
}

void TaskServer::processMessage(QTcpSocket* sender, const QByteArray& data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (!doc.isObject())
    {
        return;
    }

    Message* msg = Message::fromJson(doc.object());

    if (!msg)
    {
        return;
    }

    switch (msg->type())
    {
    case MessageType::AddTask:
    {
        AddTaskMessage* addMsg = static_cast<AddTaskMessage*>(msg);
        m_taskManager->addTask(addMsg->task());
        break;
    }
    case MessageType::RemoveTask:
    {
        RemoveTaskMessage* rmMsg = static_cast<RemoveTaskMessage*>(msg);
        int idx = m_taskManager->taskIndexById(rmMsg->taskId());

        if (idx >= 0)
        {
            m_taskManager->removeTask(idx);
        }

        break;
    }
    case MessageType::ToggleTask:
    {
        ToggleTaskMessage* togMsg = static_cast<ToggleTaskMessage*>(msg);
        int idx = m_taskManager->taskIndexById(togMsg->taskId());

        if (idx >= 0)
        {
            m_taskManager->toggleCompleted(idx);
        }

        break;
    }
    case MessageType::EditTask:
    {
        EditTaskMessage* editMsg = static_cast<EditTaskMessage*>(msg);
        int idx = m_taskManager->taskIndexById(editMsg->task().id());

        if (idx >= 0)
        {
            m_taskManager->updateTask(idx, editMsg->task());
        }

        break;
    }
    case MessageType::Sync:
        break;
    }

    delete msg;

    SyncMessage sync(m_taskManager->tasks());
    QJsonDocument syncDoc(sync.toJson());
    broadcastMessage(syncDoc.toJson(QJsonDocument::Compact));
}
