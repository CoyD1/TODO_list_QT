#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QStringList>

enum class TaskPriority
{
    Low,
    Medium,
    High
};

class Task
{
public:
    Task();
    Task(const QString& title,
         const QString& description,
         const QStringList& tags,
         TaskPriority priority);

    QString title() const;
    QString description() const;
    QStringList tags() const;
    TaskPriority priority() const;
    bool isCompleted() const;

    void setTitle(const QString& title);
    void setDescription(const QString& description);
    void setTags(const QStringList& tags);
    void setPriority(TaskPriority priority);
    void setCompleted(bool completed);

private:
    QString m_title;
    QString m_description;
    QStringList m_tags;
    TaskPriority m_priority;
    bool m_completed;
};

#endif