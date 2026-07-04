#include "mainwindow.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
    m_taskManager(new TaskManager(this))
{
    setWindowTitle("Network TODO List");

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    QLabel* titleLabel = new QLabel("Сетевой TODO-лист", this);
    mainLayout->addWidget(titleLabel);

    m_titleInput = new QLineEdit(this);
    m_titleInput->setPlaceholderText("Название задачи");

    m_descriptionInput = new QLineEdit(this);
    m_descriptionInput->setPlaceholderText("Описание задачи");

    m_tagsInput = new QLineEdit(this);
    m_tagsInput->setPlaceholderText("Теги через запятую, например: учеба, срочно");

    m_priorityBox = new QComboBox(this);
    m_priorityBox->addItem("Низкий");
    m_priorityBox->addItem("Средний");
    m_priorityBox->addItem("Высокий");

    m_addButton = new QPushButton("Добавить задачу", this);
    m_taskList = new QListWidget(this);

    mainLayout->addWidget(m_titleInput);
    mainLayout->addWidget(m_descriptionInput);
    mainLayout->addWidget(m_tagsInput);
    mainLayout->addWidget(m_priorityBox);
    mainLayout->addWidget(m_addButton);
    mainLayout->addWidget(m_taskList);

    setCentralWidget(centralWidget);

    connect(m_addButton, &QPushButton::clicked,
            this, &MainWindow::addTask);

    connect(m_taskManager, &TaskManager::tasksChanged,
            this, &MainWindow::updateTaskList);
}

MainWindow::~MainWindow()
{
}

void MainWindow::addTask()
{
    QString title = m_titleInput->text().trimmed();
    QString description = m_descriptionInput->text().trimmed();
    QStringList tags = m_tagsInput->text().split(",", Qt::SkipEmptyParts);

    for (QString& tag : tags)
    {
        tag = tag.trimmed();
    }

    if (title.isEmpty())
    {
        return;
    }

    TaskPriority priority = TaskPriority::Medium;

    if (m_priorityBox->currentIndex() == 0)
    {
        priority = TaskPriority::Low;
    }
    else if (m_priorityBox->currentIndex() == 2)
    {
        priority = TaskPriority::High;
    }

    Task task(title, description, tags, priority);
    m_taskManager->addTask(task);

    m_titleInput->clear();
    m_descriptionInput->clear();
    m_tagsInput->clear();
}

void MainWindow::updateTaskList()
{
    m_taskList->clear();

    QVector<Task> tasks = m_taskManager->tasks();

    for (const Task& task : tasks)
    {
        QString priorityText;

        switch (task.priority())
        {
        case TaskPriority::Low:
            priorityText = "Низкий";
            break;
        case TaskPriority::Medium:
            priorityText = "Средний";
            break;
        case TaskPriority::High:
            priorityText = "Высокий";
            break;
        }

        QString itemText = task.title()
                           + " | Приоритет: " + priorityText
                           + " | Теги: " + task.tags().join(", ");

        m_taskList->addItem(itemText);
    }
}