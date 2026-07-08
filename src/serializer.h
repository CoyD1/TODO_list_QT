#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <QString>
#include <QVector>
#include "task.h"

class TaskSerializer
{
public:
    virtual ~TaskSerializer() = default;

    virtual void save(const QVector<Task>& tasks, const QString& filePath) = 0;
    virtual QVector<Task> load(const QString& filePath) = 0;
};

class JsonSerializer : public TaskSerializer
{
public:
    void save(const QVector<Task>& tasks, const QString& filePath) override;
    QVector<Task> load(const QString& filePath) override;
};

#endif
