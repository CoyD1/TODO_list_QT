#include "taskmanager.h"

#include <QDateTime>

TaskManager::TaskManager(QObject* parent)
    : QObject(parent),
    m_nextId(1)
{
}

void TaskManager::addTask(const Task& task)
{
    Task t = task;
    t.setId(assignId());
    m_tasks.append(t);
    emit tasksChanged();
}

void TaskManager::removeTask(int index)
{
    if (index < 0 || index >= m_tasks.size())
    {
        return;
    }

    m_tasks.removeAt(index);
    emit tasksChanged();
}

void TaskManager::toggleCompleted(int index)
{
    if (index < 0 || index >= m_tasks.size())
    {
        return;
    }

    m_tasks[index].setCompleted(!m_tasks[index].isCompleted());
    emit tasksChanged();
}

void TaskManager::duplicateTask(int index)
{
    if (index < 0 || index >= m_tasks.size())
    {
        return;
    }

    Task copy = m_tasks[index];
    copy.setId(assignId());
    copy.setTitle(copy.title() + " (копия)");
    copy.setStatus(TaskStatus::Planned);
    copy.setCreatedAt(QDateTime::currentDateTime());
    m_tasks.append(copy);
    emit tasksChanged();
}

void TaskManager::clearCompleted()
{
    QVector<Task> remaining;

    for (const Task& task : m_tasks)
    {
        if (!task.isCompleted())
        {
            remaining.append(task);
        }
    }

    if (remaining.size() == m_tasks.size())
    {
        return;
    }

    m_tasks = remaining;
    emit tasksChanged();
}

void TaskManager::updateTask(int index, const Task& task)
{
    if (index < 0 || index >= m_tasks.size())
    {
        return;
    }

    Task t = task;
    t.setId(m_tasks[index].id());
    m_tasks[index] = t;
    emit tasksChanged();
}

void TaskManager::setTasks(const QVector<Task>& tasks)
{
    m_tasks = tasks;
    m_nextId = 1;

    for (const Task& task : m_tasks)
    {
        if (task.id() >= m_nextId)
        {
            m_nextId = task.id() + 1;
        }
    }

    emit tasksChanged();
}

QVector<Task> TaskManager::tasks() const
{
    return m_tasks;
}

QVector<Task> TaskManager::filterByTag(const QString& tag) const
{
    QVector<Task> result;

    for (const Task& task : m_tasks)
    {
        if (task.tags().contains(tag, Qt::CaseInsensitive))
        {
            result.append(task);
        }
    }

    return result;
}

int TaskManager::completedCount() const
{
    int count = 0;

    for (const Task& task : m_tasks)
    {
        if (task.isCompleted())
        {
            ++count;
        }
    }

    return count;
}

int TaskManager::overdueCount() const
{
    int count = 0;

    for (const Task& task : m_tasks)
    {
        if (task.isOverdue())
        {
            ++count;
        }
    }

    return count;
}

int TaskManager::taskIndexById(int id) const
{
    for (int i = 0; i < m_tasks.size(); ++i)
    {
        if (m_tasks[i].id() == id)
        {
            return i;
        }
    }

    return -1;
}

int TaskManager::assignId()
{
    return m_nextId++;
}
