#include "message.h"

#include <QJsonDocument>

// --- Message factory ---

Message* Message::fromJson(const QJsonObject& json)
{
    const QString typeStr = json["type"].toString();

    if (typeStr == "add_task")
    {
        return new AddTaskMessage(json);
    }
    if (typeStr == "remove_task")
    {
        return new RemoveTaskMessage(json);
    }
    if (typeStr == "toggle_task")
    {
        return new ToggleTaskMessage(json);
    }
    if (typeStr == "edit_task")
    {
        return new EditTaskMessage(json);
    }
    if (typeStr == "sync")
    {
        return new SyncMessage(json);
    }

    return nullptr;
}

// --- AddTaskMessage ---

AddTaskMessage::AddTaskMessage(const Task& task)
    : m_task(task)
{
}

AddTaskMessage::AddTaskMessage(const QJsonObject& json)
    : m_task(Task::fromJson(json["task"].toObject()))
{
}

MessageType AddTaskMessage::type() const
{
    return MessageType::AddTask;
}

QJsonObject AddTaskMessage::toJson() const
{
    QJsonObject obj;
    obj["type"] = "add_task";
    obj["task"] = m_task.toJson();
    return obj;
}

Task AddTaskMessage::task() const
{
    return m_task;
}

// --- RemoveTaskMessage ---

RemoveTaskMessage::RemoveTaskMessage(int taskId)
    : m_taskId(taskId)
{
}

RemoveTaskMessage::RemoveTaskMessage(const QJsonObject& json)
    : m_taskId(json["id"].toInt())
{
}

MessageType RemoveTaskMessage::type() const
{
    return MessageType::RemoveTask;
}

QJsonObject RemoveTaskMessage::toJson() const
{
    QJsonObject obj;
    obj["type"] = "remove_task";
    obj["id"] = m_taskId;
    return obj;
}

int RemoveTaskMessage::taskId() const
{
    return m_taskId;
}

// --- ToggleTaskMessage ---

ToggleTaskMessage::ToggleTaskMessage(int taskId)
    : m_taskId(taskId)
{
}

ToggleTaskMessage::ToggleTaskMessage(const QJsonObject& json)
    : m_taskId(json["id"].toInt())
{
}

MessageType ToggleTaskMessage::type() const
{
    return MessageType::ToggleTask;
}

QJsonObject ToggleTaskMessage::toJson() const
{
    QJsonObject obj;
    obj["type"] = "toggle_task";
    obj["id"] = m_taskId;
    return obj;
}

int ToggleTaskMessage::taskId() const
{
    return m_taskId;
}

// --- EditTaskMessage ---

EditTaskMessage::EditTaskMessage(const Task& task)
    : m_task(task)
{
}

EditTaskMessage::EditTaskMessage(const QJsonObject& json)
    : m_task(Task::fromJson(json["task"].toObject()))
{
}

MessageType EditTaskMessage::type() const
{
    return MessageType::EditTask;
}

QJsonObject EditTaskMessage::toJson() const
{
    QJsonObject obj;
    obj["type"] = "edit_task";
    obj["task"] = m_task.toJson();
    return obj;
}

Task EditTaskMessage::task() const
{
    return m_task;
}

// --- SyncMessage ---

SyncMessage::SyncMessage(const QVector<Task>& tasks)
    : m_tasks(tasks)
{
}

SyncMessage::SyncMessage(const QJsonObject& json)
{
    const QJsonArray arr = json["tasks"].toArray();
    for (const QJsonValue& val : arr)
    {
        m_tasks.append(Task::fromJson(val.toObject()));
    }
}

MessageType SyncMessage::type() const
{
    return MessageType::Sync;
}

QJsonObject SyncMessage::toJson() const
{
    QJsonObject obj;
    obj["type"] = "sync";

    QJsonArray arr;
    for (const Task& task : m_tasks)
    {
        arr.append(task.toJson());
    }
    obj["tasks"] = arr;

    return obj;
}

QVector<Task> SyncMessage::tasks() const
{
    return m_tasks;
}
