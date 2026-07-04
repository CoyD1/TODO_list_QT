#include "mainwindow.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QFont>

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

    QHBoxLayout* filterLayout = new QHBoxLayout();
    m_filterInput = new QLineEdit(this);
    m_filterInput->setPlaceholderText("Фильтр по тегу");
    m_filterButton = new QPushButton("Фильтр", this);
    m_resetFilterButton = new QPushButton("Сбросить", this);
    filterLayout->addWidget(m_filterInput);
    filterLayout->addWidget(m_filterButton);
    filterLayout->addWidget(m_resetFilterButton);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    m_toggleButton = new QPushButton("Отметить выполненной", this);
    m_removeButton = new QPushButton("Удалить задачу", this);
    actionLayout->addWidget(m_toggleButton);
    actionLayout->addWidget(m_removeButton);

    m_taskList = new QListWidget(this);

    m_statusLabel = new QLabel(this);

    mainLayout->addWidget(m_titleInput);
    mainLayout->addWidget(m_descriptionInput);
    mainLayout->addWidget(m_tagsInput);
    mainLayout->addWidget(m_priorityBox);
    mainLayout->addWidget(m_addButton);
    mainLayout->addLayout(filterLayout);
    mainLayout->addLayout(actionLayout);
    mainLayout->addWidget(m_taskList);
    mainLayout->addWidget(m_statusLabel);

    setCentralWidget(centralWidget);

    connect(m_addButton, &QPushButton::clicked,
            this, &MainWindow::addTask);
    connect(m_removeButton, &QPushButton::clicked,
            this, &MainWindow::removeSelectedTask);
    connect(m_toggleButton, &QPushButton::clicked,
            this, &MainWindow::toggleSelectedTask);
    connect(m_filterButton, &QPushButton::clicked,
            this, &MainWindow::applyTagFilter);
    connect(m_resetFilterButton, &QPushButton::clicked,
            this, &MainWindow::resetTagFilter);
    connect(m_taskList, &QListWidget::itemDoubleClicked,
            this, &MainWindow::toggleSelectedTask);

    connect(m_taskManager, &TaskManager::tasksChanged,
            this, &MainWindow::updateTaskList);

    updateTaskList();
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

void MainWindow::removeSelectedTask()
{
    int index = selectedTaskIndex();

    if (index < 0)
    {
        return;
    }

    m_taskManager->removeTask(index);
}

void MainWindow::toggleSelectedTask()
{
    int index = selectedTaskIndex();

    if (index < 0)
    {
        return;
    }

    m_taskManager->toggleCompleted(index);
}

void MainWindow::applyTagFilter()
{
    m_activeFilter = m_filterInput->text().trimmed();
    updateTaskList();
}

void MainWindow::resetTagFilter()
{
    m_activeFilter.clear();
    m_filterInput->clear();
    updateTaskList();
}

int MainWindow::selectedTaskIndex() const
{
    QListWidgetItem* item = m_taskList->currentItem();

    if (!item)
    {
        return -1;
    }

    return item->data(Qt::UserRole).toInt();
}

void MainWindow::updateTaskList()
{
    m_taskList->clear();

    QVector<Task> tasks = m_taskManager->tasks();

    for (int i = 0; i < tasks.size(); ++i)
    {
        const Task& task = tasks[i];

        if (!m_activeFilter.isEmpty()
            && !task.tags().contains(m_activeFilter, Qt::CaseInsensitive))
        {
            continue;
        }

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

        QString statusText = task.isCompleted() ? "[✓]" : "[ ]";
        QString itemText = statusText + " "
                           + task.title()
                           + " | Приоритет: " + priorityText
                           + " | Теги: " + task.tags().join(", ");

        if (!task.description().isEmpty())
        {
            itemText += " | " + task.description();
        }

        QListWidgetItem* item = new QListWidgetItem(itemText, m_taskList);
        item->setData(Qt::UserRole, i);

        if (task.isCompleted())
        {
            QFont font = item->font();
            font.setStrikeOut(true);
            item->setFont(font);
        }
    }

    int total = m_taskManager->tasks().size();
    int completed = m_taskManager->completedCount();
    QString statusText = QString("Всего задач: %1 | Выполнено: %2").arg(total).arg(completed);

    if (!m_activeFilter.isEmpty())
    {
        statusText += QString(" | Фильтр: %1").arg(m_activeFilter);
    }

    m_statusLabel->setText(statusText);
}
