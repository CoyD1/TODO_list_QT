#include "mainwindow.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QFont>
#include <algorithm>

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

    m_searchInput = new QLineEdit(this);
    m_searchInput->setPlaceholderText("Поиск по названию");

    QHBoxLayout* filterLayout = new QHBoxLayout();
    m_filterInput = new QLineEdit(this);
    m_filterInput->setPlaceholderText("Фильтр по тегу");
    m_filterButton = new QPushButton("Фильтр", this);
    m_resetFilterButton = new QPushButton("Сбросить", this);
    filterLayout->addWidget(m_filterInput);
    filterLayout->addWidget(m_filterButton);
    filterLayout->addWidget(m_resetFilterButton);

    QHBoxLayout* sortLayout = new QHBoxLayout();
    m_sortBox = new QComboBox(this);
    m_sortBox->addItem("Порядок добавления");
    m_sortBox->addItem("Сначала важные");
    m_sortBox->addItem("Сначала простые");
    m_hideCompletedBox = new QCheckBox("Скрыть выполненные", this);
    m_clearCompletedButton = new QPushButton("Очистить выполненные", this);
    sortLayout->addWidget(m_sortBox);
    sortLayout->addWidget(m_hideCompletedBox);
    sortLayout->addWidget(m_clearCompletedButton);

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
    mainLayout->addWidget(m_searchInput);
    mainLayout->addLayout(filterLayout);
    mainLayout->addLayout(sortLayout);
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
    connect(m_clearCompletedButton, &QPushButton::clicked,
            this, &MainWindow::clearCompletedTasks);
    connect(m_taskList, &QListWidget::itemDoubleClicked,
            this, &MainWindow::toggleSelectedTask);
    connect(m_searchInput, &QLineEdit::textChanged,
            this, &MainWindow::updateTaskList);
    connect(m_sortBox, &QComboBox::currentIndexChanged,
            this, &MainWindow::updateTaskList);
    connect(m_hideCompletedBox, &QCheckBox::toggled,
            this, &MainWindow::updateTaskList);

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

void MainWindow::clearCompletedTasks()
{
    m_taskManager->clearCompleted();
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

QString MainWindow::priorityText(TaskPriority priority)
{
    switch (priority)
    {
    case TaskPriority::Low:
        return "Низкий";
    case TaskPriority::Medium:
        return "Средний";
    case TaskPriority::High:
        return "Высокий";
    }

    return "Средний";
}

int MainWindow::priorityWeight(TaskPriority priority)
{
    switch (priority)
    {
    case TaskPriority::Low:
        return 0;
    case TaskPriority::Medium:
        return 1;
    case TaskPriority::High:
        return 2;
    }

    return 1;
}

QVector<QPair<int, Task>> MainWindow::visibleTasks() const
{
    QVector<QPair<int, Task>> tasks;
    const QVector<Task> allTasks = m_taskManager->tasks();
    const QString searchText = m_searchInput->text().trimmed();

    for (int i = 0; i < allTasks.size(); ++i)
    {
        const Task& task = allTasks[i];

        if (!searchText.isEmpty()
            && !task.title().contains(searchText, Qt::CaseInsensitive))
        {
            continue;
        }

        if (!m_activeFilter.isEmpty()
            && !task.tags().contains(m_activeFilter, Qt::CaseInsensitive))
        {
            continue;
        }

        if (m_hideCompletedBox->isChecked() && task.isCompleted())
        {
            continue;
        }

        tasks.append(qMakePair(i, task));
    }

    if (m_sortBox->currentIndex() == 1)
    {
        std::sort(tasks.begin(), tasks.end(),
                  [](const QPair<int, Task>& left, const QPair<int, Task>& right)
                  {
                      return priorityWeight(left.second.priority()) > priorityWeight(right.second.priority());
                  });
    }
    else if (m_sortBox->currentIndex() == 2)
    {
        std::sort(tasks.begin(), tasks.end(),
                  [](const QPair<int, Task>& left, const QPair<int, Task>& right)
                  {
                      return priorityWeight(left.second.priority()) < priorityWeight(right.second.priority());
                  });
    }

    return tasks;
}

void MainWindow::updateTaskList()
{
    m_taskList->clear();

    QVector<QPair<int, Task>> tasks = visibleTasks();

    for (const QPair<int, Task>& entry : tasks)
    {
        const Task& task = entry.second;
        const int sourceIndex = entry.first;

        QString statusText = task.isCompleted() ? "[✓]" : "[ ]";
        QString itemText = statusText + " "
                           + task.title()
                           + " | Приоритет: " + priorityText(task.priority())
                           + " | Теги: " + task.tags().join(", ");

        if (!task.description().isEmpty())
        {
            itemText += " | " + task.description();
        }

        QListWidgetItem* item = new QListWidgetItem(itemText, m_taskList);
        item->setData(Qt::UserRole, sourceIndex);

        if (task.isCompleted())
        {
            QFont font = item->font();
            font.setStrikeOut(true);
            item->setFont(font);
        }
    }

    int total = m_taskManager->tasks().size();
    int completed = m_taskManager->completedCount();
    QString statusText = QString("Всего задач: %1 | Выполнено: %2 | Показано: %3")
                             .arg(total)
                             .arg(completed)
                             .arg(tasks.size());

    if (!m_activeFilter.isEmpty())
    {
        statusText += QString(" | Фильтр: %1").arg(m_activeFilter);
    }

    if (!m_searchInput->text().trimmed().isEmpty())
    {
        statusText += QString(" | Поиск: %1").arg(m_searchInput->text().trimmed());
    }

    m_statusLabel->setText(statusText);
}
