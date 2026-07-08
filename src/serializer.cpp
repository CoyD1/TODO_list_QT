#include "serializer.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>

void JsonSerializer::save(const QVector<Task>& tasks, const QString& filePath)
{
    QJsonArray arr;
    for (const Task& task : tasks)
    {
        arr.append(task.toJson());
    }

    QJsonDocument doc(arr);
    QFile file(filePath);

    if (file.open(QIODevice::WriteOnly))
    {
        file.write(doc.toJson(QJsonDocument::Indented));
    }
}

QVector<Task> JsonSerializer::load(const QString& filePath)
{
    QVector<Task> tasks;
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly))
    {
        return tasks;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());

    if (!doc.isArray())
    {
        return tasks;
    }

    const QJsonArray arr = doc.array();
    for (const QJsonValue& val : arr)
    {
        tasks.append(Task::fromJson(val.toObject()));
    }

    return tasks;
}
