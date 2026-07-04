#include "task.h"

Task::Task()
    : m_priority(TaskPriority::Medium),
    m_completed(false)
{
}

Task::Task(const QString& title,
           const QString& description,
           const QStringList& tags,
           TaskPriority priority)
    : m_title(title),
    m_description(description),
    m_tags(tags),
    m_priority(priority),
    m_completed(false)
{
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

bool Task::isCompleted() const
{
    return m_completed;
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

void Task::setCompleted(bool completed)
{
    m_completed = completed;
}