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
    connect(m_taskManager, &TaskManager::tasksChanged,
            this, &TaskServer::broadcastTasks);
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
    m_clientBuffers.clear();
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
        m_clientBuffers.insert(socket, QByteArray());

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

    QByteArray& buffer = m_clientBuffers[socket];
    buffer.append(socket->readAll());

    while (buffer.size() >= 4)
    {
        QDataStream stream(buffer);
        quint32 messageSize = 0;
        stream >> messageSize;

        if (buffer.size() < static_cast<int>(messageSize) + 4)
        {
            break;
        }

        const QByteArray data = buffer.mid(4, messageSize);
        buffer.remove(0, 4 + messageSize);
        processMessage(data);
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
    m_clientBuffers.remove(socket);
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

void TaskServer::broadcastTasks()
{
    SyncMessage sync(m_taskManager->tasks());
    QJsonDocument doc(sync.toJson());
    broadcastMessage(doc.toJson(QJsonDocument::Compact));
}

void TaskServer::processMessage(const QByteArray& data)
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
}
