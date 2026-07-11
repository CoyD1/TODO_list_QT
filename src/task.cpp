#include "task.h"

#include <QJsonArray>

Task::Task()
    : m_id(-1),
    m_priority(TaskPriority::Medium),
    m_completed(false)
{
}

Task::Task(const QString& title,
           const QString& description,
           const QStringList& tags,
           TaskPriority priority)
    : m_id(-1),
    m_title(title),
    m_description(description),
    m_tags(tags),
    m_priority(priority),
    m_completed(false)
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

TaskPriority Task::priority() const
{
    return m_priority;
}

QDate Task::dueDate() const
{
    return m_dueDate;
}

bool Task::isCompleted() const
{
    return m_completed;
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

void Task::setPriority(TaskPriority priority)
{
    m_priority = priority;
}

void Task::setDueDate(const QDate& date)
{
    m_dueDate = date;
}

void Task::setCompleted(bool completed)
{
    m_completed = completed;
}

QJsonObject Task::toJson() const
{
    QJsonObject obj;
    obj["id"] = m_id;
    obj["title"] = m_title;
    obj["description"] = m_description;
    obj["priority"] = static_cast<int>(m_priority);
    obj["completed"] = m_completed;

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
    task.m_priority = static_cast<TaskPriority>(json["priority"].toInt(1));
    task.m_completed = json["completed"].toBool();

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
