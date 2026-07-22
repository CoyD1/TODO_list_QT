#include "networkclient.h"
#include "message.h"

#include <QJsonDocument>
#include <QDataStream>
#include <QIODevice>
#include <memory>

NetworkClient::NetworkClient(QObject* parent)
    : QObject(parent),
    m_socket(std::make_unique<QTcpSocket>())
{
    connect(m_socket.get(), &QTcpSocket::connected,
            this, &NetworkClient::onConnected);
    connect(m_socket.get(), &QTcpSocket::disconnected,
            this, &NetworkClient::onDisconnected);
    connect(m_socket.get(), &QTcpSocket::errorOccurred,
            this, &NetworkClient::onErrorOccurred);
    connect(m_socket.get(), &QTcpSocket::readyRead,
            this, &NetworkClient::onReadyRead);
}

bool NetworkClient::connectToServer(const QString& host, quint16 port, int timeoutMs)
{
    if (m_socket->state() != QAbstractSocket::UnconnectedState)
    {
        m_socket->abort();
    }

    m_socket->connectToHost(host, port);
    return m_socket->waitForConnected(timeoutMs);
}

void NetworkClient::disconnectFromServer()
{
    m_socket->disconnectFromHost();
}

bool NetworkClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void NetworkClient::sendAddTask(const Task& task)
{
    AddTaskMessage msg(task);
    QJsonDocument doc(msg.toJson());
    sendMessage(doc.toJson(QJsonDocument::Compact));
}

void NetworkClient::sendRemoveTask(int taskId)
{
    RemoveTaskMessage msg(taskId);
    QJsonDocument doc(msg.toJson());
    sendMessage(doc.toJson(QJsonDocument::Compact));
}

void NetworkClient::sendToggleTask(int taskId)
{
    ToggleTaskMessage msg(taskId);
    QJsonDocument doc(msg.toJson());
    sendMessage(doc.toJson(QJsonDocument::Compact));
}

void NetworkClient::sendEditTask(const Task& task)
{
    EditTaskMessage msg(task);
    QJsonDocument doc(msg.toJson());
    sendMessage(doc.toJson(QJsonDocument::Compact));
}

void NetworkClient::onConnected()
{
    m_buffer.clear();
    emit connected();
}

void NetworkClient::onDisconnected()
{
    emit disconnected();
}

void NetworkClient::onErrorOccurred(QAbstractSocket::SocketError)
{
    emit connectionError(m_socket->errorString());
}

void NetworkClient::onReadyRead()
{
    m_buffer.append(m_socket->readAll());

    while (m_buffer.size() >= 4)
    {
        QDataStream stream(m_buffer);
        quint32 msgSize;
        stream >> msgSize;

        if (m_buffer.size() < static_cast<int>(msgSize) + 4)
        {
            break;
        }

        QByteArray msgData = m_buffer.mid(4, msgSize);
        m_buffer.remove(0, 4 + msgSize);

        QJsonDocument doc = QJsonDocument::fromJson(msgData);

        if (!doc.isObject())
        {
            continue;
        }

        std::unique_ptr<Message> msg = Message::fromJson(doc.object());

        if (!msg)
        {
            continue;
        }

        if (msg->type() == MessageType::Sync)
        {
            auto* syncMsg = static_cast<SyncMessage*>(msg.get());
            emit tasksReceived(syncMsg->tasks());
        }
    }
}

void NetworkClient::sendMessage(const QByteArray& data)
{
    if (!isConnected())
    {
        return;
    }

    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream << static_cast<quint32>(data.size());
    packet.append(data);
    m_socket->write(packet);
}
