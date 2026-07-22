#include "task.h"

#include <QJsonArray>

Task::Task()
    : m_id(-1),
    m_priority(TaskPriority::Medium),
    m_status(TaskStatus::Planned),
    m_createdAt(QDateTime::currentDateTime())
{
}

Task::Task(const QString& title,
           const QString& description,
           const QStringList& tags,
           const QString& assignee,
           TaskPriority priority,
           TaskStatus status)
    : m_id(-1),
    m_title(title),
    m_description(description),
    m_tags(tags),
    m_assignee(assignee),
    m_priority(priority),
    m_status(status),
    m_createdAt(QDateTime::currentDateTime())
{
}

int Task::id() const
{
    return m_id;
}

QString Task::title() const
{
    return m_title;
}

QString Task::description() const
{
    return m_description;
}

QStringList Task::tags() const
{
    return m_tags;
}

QString Task::assignee() const
{
    return m_assignee;
}

TaskPriority Task::priority() const
{
    return m_priority;
}

TaskStatus Task::status() const
{
    return m_status;
}

QDate Task::dueDate() const
{
    return m_dueDate;
}

QDateTime Task::createdAt() const
{
    return m_createdAt;
}

bool Task::isCompleted() const
{
    return m_status == TaskStatus::Completed;
}

bool Task::isOverdue() const
{
    return m_dueDate.isValid()
           && !isCompleted()
           && m_dueDate < QDate::currentDate();
}

bool Task::isDueSoon(int days) const
{
    const QDate today = QDate::currentDate();
    return days >= 0
           && m_dueDate.isValid()
           && !isCompleted()
           && m_dueDate >= today
           && m_dueDate <= today.addDays(days);
}

void Task::setId(int id)
{
    m_id = id;
}

void Task::setTitle(const QString& title)
{
    m_title = title;
}

void Task::setDescription(const QString& description)
{
    m_description = description;
}

void Task::setTags(const QStringList& tags)
{
    m_tags = tags;
}

void Task::setAssignee(const QString& assignee)
{
    m_assignee = assignee;
}

void Task::setPriority(TaskPriority priority)
{
    m_priority = priority;
}

void Task::setStatus(TaskStatus status)
{
    m_status = status;
}

void Task::setDueDate(const QDate& date)
{
    m_dueDate = date;
}

void Task::setCreatedAt(const QDateTime& dateTime)
{
    m_createdAt = dateTime;
}

void Task::setCompleted(bool completed)
{
    m_status = completed ? TaskStatus::Completed : TaskStatus::Planned;
}

QJsonObject Task::toJson() const
{
    QJsonObject obj;
    obj["id"] = m_id;
    obj["title"] = m_title;
    obj["description"] = m_description;
    obj["assignee"] = m_assignee;
    obj["priority"] = static_cast<int>(m_priority);
    obj["status"] = static_cast<int>(m_status);
    obj["completed"] = isCompleted();
    obj["createdAt"] = m_createdAt.toString(Qt::ISODate);

    if (m_dueDate.isValid())
    {
        obj["dueDate"] = m_dueDate.toString(Qt::ISODate);
    }

    QJsonArray tagsArray;
    for (const QString& tag : m_tags)
    {
        tagsArray.append(tag);
    }
    obj["tags"] = tagsArray;

    return obj;
}

Task Task::fromJson(const QJsonObject& json)
{
    Task task;
    task.m_id = json["id"].toInt(-1);
    task.m_title = json["title"].toString();
    task.m_description = json["description"].toString();
    task.m_assignee = json["assignee"].toString();
    task.m_priority = static_cast<TaskPriority>(json["priority"].toInt(1));

    if (json.contains("status"))
    {
        task.m_status = static_cast<TaskStatus>(json["status"].toInt(0));
    }
    else if (json["completed"].toBool())
    {
        task.m_status = TaskStatus::Completed;
    }

    if (json.contains("createdAt"))
    {
        const QDateTime createdAt = QDateTime::fromString(
            json["createdAt"].toString(), Qt::ISODate);

        if (createdAt.isValid())
        {
            task.m_createdAt = createdAt;
        }
    }

    if (json.contains("dueDate"))
    {
        task.m_dueDate = QDate::fromString(json["dueDate"].toString(), Qt::ISODate);
    }

    const QJsonArray tagsArray = json["tags"].toArray();
    for (const QJsonValue& val : tagsArray)
    {
        task.m_tags.append(val.toString());
    }

    return task;
}
