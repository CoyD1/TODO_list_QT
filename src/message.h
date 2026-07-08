#ifndef MESSAGE_H
#define MESSAGE_H

#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include "task.h"

enum class MessageType
{
    AddTask,
    RemoveTask,
    ToggleTask,
    EditTask,
    Sync
};

class Message
{
public:
    virtual ~Message() = default;

    virtual MessageType type() const = 0;
    virtual QJsonObject toJson() const = 0;

    static Message* fromJson(const QJsonObject& json);
};

class AddTaskMessage : public Message
{
public:
    explicit AddTaskMessage(const Task& task);
    explicit AddTaskMessage(const QJsonObject& json);

    MessageType type() const override;
    QJsonObject toJson() const override;

    Task task() const;

private:
    Task m_task;
};

class RemoveTaskMessage : public Message
{
public:
    explicit RemoveTaskMessage(int taskId);
    explicit RemoveTaskMessage(const QJsonObject& json);

    MessageType type() const override;
    QJsonObject toJson() const override;

    int taskId() const;

private:
    int m_taskId;
};

class ToggleTaskMessage : public Message
{
public:
    explicit ToggleTaskMessage(int taskId);
    explicit ToggleTaskMessage(const QJsonObject& json);

    MessageType type() const override;
    QJsonObject toJson() const override;

    int taskId() const;

private:
    int m_taskId;
};

class EditTaskMessage : public Message
{
public:
    explicit EditTaskMessage(const Task& task);
    explicit EditTaskMessage(const QJsonObject& json);

    MessageType type() const override;
    QJsonObject toJson() const override;

    Task task() const;

private:
    Task m_task;
};

class SyncMessage : public Message
{
public:
    explicit SyncMessage(const QVector<Task>& tasks);
    explicit SyncMessage(const QJsonObject& json);

    MessageType type() const override;
    QJsonObject toJson() const override;

    QVector<Task> tasks() const;

private:
    QVector<Task> m_tasks;
};

#endif
