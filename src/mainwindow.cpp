#include "mainwindow.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QFont>
#include <QGroupBox>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDateEdit>
#include <QDateTime>
#include <QTimer>
#include <algorithm>

namespace
{
constexpr quint16 DiscoveryPort = 45454;
const QByteArray DiscoveryRequest = "TODO_LIST_DISCOVER_V1";
const QByteArray DiscoveryResponse = "TODO_LIST_SERVER_V1:";
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
    m_taskManager(new TaskManager(this)),
    m_server(nullptr),
    m_client(new NetworkClient(this)),
    m_discoveryClient(nullptr),
    m_discoveryServer(nullptr),
    m_syncHost("127.0.0.1"),
    m_syncPort(9999),
    m_autoSyncInProgress(false),
    m_activeStatusFilter(-1),
    m_clientMode(false),
    m_connectedClients(0)
{
    setWindowTitle("Командный TODO-лист");
    resize(960, 760);

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(8);

    // ── Новая задача ──
    QGroupBox* inputGroup = new QGroupBox("Новая задача", this);
    QFormLayout* inputLayout = new QFormLayout(inputGroup);

    m_titleInput = new QLineEdit(this);
    m_titleInput->setPlaceholderText("Название задачи");
    inputLayout->addRow("Название:", m_titleInput);

    m_descriptionInput = new QLineEdit(this);
    m_descriptionInput->setPlaceholderText("Описание задачи");
    inputLayout->addRow("Описание:", m_descriptionInput);

    m_assigneeInput = new QLineEdit(this);
    m_assigneeInput->setPlaceholderText("Имя участника команды");
    inputLayout->addRow("Исполнитель:", m_assigneeInput);

    m_tagsInput = new QLineEdit(this);
    m_tagsInput->setPlaceholderText("учеба, срочно, работа");
    inputLayout->addRow("Теги:", m_tagsInput);

    QHBoxLayout* rowLayout = new QHBoxLayout();
    m_priorityBox = new QComboBox(this);
    m_priorityBox->addItem("Низкий");
    m_priorityBox->addItem("Средний");
    m_priorityBox->addItem("Высокий");
    rowLayout->addWidget(new QLabel("Приоритет:"));
    rowLayout->addWidget(m_priorityBox);
    rowLayout->addSpacing(16);
    rowLayout->addWidget(new QLabel("Статус:"));
    m_statusBox = new QComboBox(this);
    m_statusBox->addItem("Запланирована");
    m_statusBox->addItem("В работе");
    m_statusBox->addItem("Выполнена");
    rowLayout->addWidget(m_statusBox);
    rowLayout->addSpacing(16);
    rowLayout->addWidget(new QLabel("Срок:"));
    m_dueDateInput = new QDateEdit(this);
    m_dueDateInput->setCalendarPopup(true);
    m_dueDateInput->setDate(QDate::currentDate());
    rowLayout->addWidget(m_dueDateInput);
    rowLayout->addStretch();
    inputLayout->addRow("", rowLayout);

    m_addButton = new QPushButton("Добавить задачу", this);
    inputLayout->addRow("", m_addButton);

    mainLayout->addWidget(inputGroup);

    // ── Поиск и фильтры ──
    QGroupBox* filterGroup = new QGroupBox("Поиск и фильтры", this);
    QVBoxLayout* filterGroupLayout = new QVBoxLayout(filterGroup);

    m_searchInput = new QLineEdit(this);
    m_searchInput->setPlaceholderText("Поиск по названию или исполнителю...");
    filterGroupLayout->addWidget(m_searchInput);

    QHBoxLayout* tagFilterLayout = new QHBoxLayout();
    m_filterInput = new QLineEdit(this);
    m_filterInput->setPlaceholderText("Фильтр по тегу");
    tagFilterLayout->addWidget(m_filterInput, 1);
    m_assigneeFilterInput = new QLineEdit(this);
    m_assigneeFilterInput->setPlaceholderText("Фильтр по исполнителю");
    tagFilterLayout->addWidget(m_assigneeFilterInput, 1);
    m_statusFilterBox = new QComboBox(this);
    m_statusFilterBox->addItem("Все статусы");
    m_statusFilterBox->addItem("Запланированные");
    m_statusFilterBox->addItem("В работе");
    m_statusFilterBox->addItem("Выполненные");
    tagFilterLayout->addWidget(m_statusFilterBox);
    m_filterButton = new QPushButton("Применить", this);
    tagFilterLayout->addWidget(m_filterButton);
    m_resetFilterButton = new QPushButton("Сбросить", this);
    tagFilterLayout->addWidget(m_resetFilterButton);
    filterGroupLayout->addLayout(tagFilterLayout);

    QHBoxLayout* controlsLayout = new QHBoxLayout();
    m_sortBox = new QComboBox(this);
    m_sortBox->addItem("Порядок добавления");
    m_sortBox->addItem("Сначала важные");
    m_sortBox->addItem("Сначала простые");
    m_sortBox->addItem("По сроку");
    m_sortBox->addItem("Сначала новые");
    m_sortBox->addItem("Сначала старые");
    controlsLayout->addWidget(new QLabel("Сортировка:"));
    controlsLayout->addWidget(m_sortBox);
    controlsLayout->addSpacing(12);
    m_hideCompletedBox = new QCheckBox("Скрыть выполненные", this);
    m_showOverdueOnlyBox = new QCheckBox("Только просроченные", this);
    controlsLayout->addWidget(m_hideCompletedBox);
    controlsLayout->addWidget(m_showOverdueOnlyBox);
    controlsLayout->addStretch();
    controlsLayout->addWidget(m_saveButton = new QPushButton("Сохранить", this));
    controlsLayout->addWidget(m_loadButton = new QPushButton("Загрузить", this));
    filterGroupLayout->addLayout(controlsLayout);

    QHBoxLayout* actionRow = new QHBoxLayout();
    m_toggleButton = new QPushButton("Отметить", this);
    actionRow->addWidget(m_toggleButton);
    m_editButton = new QPushButton("Редактировать", this);
    actionRow->addWidget(m_editButton);
    m_duplicateButton = new QPushButton("Дублировать", this);
    actionRow->addWidget(m_duplicateButton);
    m_removeButton = new QPushButton("Удалить", this);
    actionRow->addWidget(m_removeButton);
    actionRow->addSpacing(12);
    m_clearCompletedButton = new QPushButton("Очистить выполненные", this);
    actionRow->addWidget(m_clearCompletedButton);
    actionRow->addStretch();
    filterGroupLayout->addLayout(actionRow);

    mainLayout->addWidget(filterGroup);

    // ── Совместная работа ──
    QGroupBox* syncGroup = new QGroupBox("Совместная работа", this);
    QHBoxLayout* syncLayout = new QHBoxLayout(syncGroup);
    m_syncStatusLabel = new QLabel("Настройка синхронизации...", this);
    syncLayout->addWidget(m_syncStatusLabel);
    syncLayout->addStretch();
    mainLayout->addWidget(syncGroup);

    // ── Список задач ──
    QGroupBox* listGroup = new QGroupBox("Задачи", this);
    QVBoxLayout* listLayout = new QVBoxLayout(listGroup);
    m_taskList = new QListWidget(this);
    listLayout->addWidget(m_taskList);
    mainLayout->addWidget(listGroup, 1);

    // ── Статус строка ──
    m_statusLabel = new QLabel(this);
    mainLayout->addWidget(m_statusLabel);

    setCentralWidget(centralWidget);

    connect(m_addButton, &QPushButton::clicked,
            this, &MainWindow::addTask);
    connect(m_editButton, &QPushButton::clicked,
            this, &MainWindow::editSelectedTask);
    connect(m_duplicateButton, &QPushButton::clicked,
            this, &MainWindow::duplicateSelectedTask);
    connect(m_removeButton, &QPushButton::clicked,
            this, &MainWindow::removeSelectedTask);
    connect(m_toggleButton, &QPushButton::clicked,
            this, &MainWindow::toggleSelectedTask);
    connect(m_filterButton, &QPushButton::clicked,
            this, &MainWindow::applyFilters);
    connect(m_resetFilterButton, &QPushButton::clicked,
            this, &MainWindow::resetFilters);
    connect(m_clearCompletedButton, &QPushButton::clicked,
            this, &MainWindow::clearCompletedTasks);
    connect(m_saveButton, &QPushButton::clicked,
            this, &MainWindow::saveTasksToFile);
    connect(m_loadButton, &QPushButton::clicked,
            this, &MainWindow::loadTasksFromFile);
    connect(m_taskList, &QListWidget::itemDoubleClicked,
            this, &MainWindow::editSelectedTask);
    connect(m_searchInput, &QLineEdit::textChanged,
            this, &MainWindow::updateTaskList);
    connect(m_sortBox, &QComboBox::currentIndexChanged,
            this, &MainWindow::updateTaskList);
    connect(m_hideCompletedBox, &QCheckBox::toggled,
            this, &MainWindow::updateTaskList);
    connect(m_showOverdueOnlyBox, &QCheckBox::toggled,
            this, &MainWindow::updateTaskList);
    connect(m_taskManager, &TaskManager::tasksChanged,
            this, &MainWindow::updateTaskList);
    connect(m_taskManager, &TaskManager::tasksChanged,
            this, &MainWindow::autoSaveTasks);

    m_tasksFilePath = defaultTasksFilePath();
    loadTasksFromPath(m_tasksFilePath, false);

    connect(m_client, &NetworkClient::connected,
            this, &MainWindow::onConnected);
    connect(m_client, &NetworkClient::disconnected,
            this, &MainWindow::onDisconnected);
    connect(m_client, &NetworkClient::connectionError,
            this, &MainWindow::onConnectionError);
    connect(m_client, &NetworkClient::tasksReceived,
            this, &MainWindow::onTasksReceived);

    updateTaskList();
    QTimer::singleShot(0, this, &MainWindow::startAutomaticSync);
}

MainWindow::~MainWindow()
{
}

void MainWindow::addTask()
{
    QString title = m_titleInput->text().trimmed();
    QString description = m_descriptionInput->text().trimmed();
    QString assignee = m_assigneeInput->text().trimmed();
    QStringList tags = m_tagsInput->text().split(",", Qt::SkipEmptyParts);

    for (QString& tag : tags)
    {
        tag = tag.trimmed();
    }

    if (title.isEmpty())
    {
        QMessageBox::warning(this, "Новая задача", "Введите название задачи");
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

    TaskStatus status = static_cast<TaskStatus>(m_statusBox->currentIndex());
    Task task(title, description, tags, assignee, priority, status);

    if (m_dueDateInput->date().isValid())
    {
        task.setDueDate(m_dueDateInput->date());
    }

    if (m_clientMode)
    {
        m_client->sendAddTask(task);
    }
    else
    {
        m_taskManager->addTask(task);
    }

    m_titleInput->clear();
    m_descriptionInput->clear();
    m_assigneeInput->clear();
    m_tagsInput->clear();
    m_statusBox->setCurrentIndex(0);
    m_dueDateInput->setDate(QDate::currentDate());
}

void MainWindow::editSelectedTask()
{
    int index = selectedTaskIndex();

    if (index < 0)
    {
        return;
    }

    const QVector<Task> allTasks = m_taskManager->tasks();
    if (index >= allTasks.size())
    {
        return;
    }
    const Task& task = allTasks[index];

    QDialog dialog(this);
    dialog.setWindowTitle("Редактирование задачи");

    QFormLayout* form = new QFormLayout(&dialog);

    QLineEdit* titleEdit = new QLineEdit(task.title(), &dialog);
    QLineEdit* descEdit = new QLineEdit(task.description(), &dialog);
    QLineEdit* assigneeEdit = new QLineEdit(task.assignee(), &dialog);
    QLineEdit* tagsEdit = new QLineEdit(task.tags().join(", "), &dialog);
    QComboBox* priorityEdit = new QComboBox(&dialog);
    priorityEdit->addItem("Низкий");
    priorityEdit->addItem("Средний");
    priorityEdit->addItem("Высокий");
    priorityEdit->setCurrentIndex(static_cast<int>(task.priority()));

    QComboBox* statusEdit = new QComboBox(&dialog);
    statusEdit->addItem("Запланирована");
    statusEdit->addItem("В работе");
    statusEdit->addItem("Выполнена");
    statusEdit->setCurrentIndex(static_cast<int>(task.status()));

    QDateEdit* dueDateEdit = new QDateEdit(&dialog);
    dueDateEdit->setCalendarPopup(true);
    dueDateEdit->setSpecialValueText("Без срока");

    if (task.dueDate().isValid())
    {
        dueDateEdit->setDate(task.dueDate());
    }
    else
    {
        dueDateEdit->setDate(QDate::currentDate());
    }

    form->addRow("Название:", titleEdit);
    form->addRow("Описание:", descEdit);
    form->addRow("Исполнитель:", assigneeEdit);
    form->addRow("Теги:", tagsEdit);
    form->addRow("Приоритет:", priorityEdit);
    form->addRow("Статус:", statusEdit);
    form->addRow("Срок:", dueDateEdit);

    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    QString title = titleEdit->text().trimmed();
    if (title.isEmpty())
    {
        return;
    }

    QStringList tags;
    for (const QString& tag : tagsEdit->text().split(",", Qt::SkipEmptyParts))
    {
        tags.append(tag.trimmed());
    }

    TaskPriority priority = static_cast<TaskPriority>(priorityEdit->currentIndex());
    Task updatedTask(title,
                     descEdit->text().trimmed(),
                     tags,
                     assigneeEdit->text().trimmed(),
                     priority,
                     static_cast<TaskStatus>(statusEdit->currentIndex()));
    updatedTask.setCreatedAt(task.createdAt());

    if (dueDateEdit->date().isValid())
    {
        updatedTask.setDueDate(dueDateEdit->date());
    }

    if (m_clientMode)
    {
        updatedTask.setId(task.id());
        m_client->sendEditTask(updatedTask);
    }
    else
    {
        m_taskManager->updateTask(index, updatedTask);
    }
}

void MainWindow::duplicateSelectedTask()
{
    int index = selectedTaskIndex();

    if (index < 0)
    {
        QMessageBox::information(this, "Дублирование", "Выберите задачу для копирования");
        return;
    }

    const Task& sourceTask = m_taskManager->tasks()[index];
    Task copy = sourceTask;
    copy.setTitle(sourceTask.title() + " (копия)");
    copy.setStatus(TaskStatus::Planned);
    copy.setCreatedAt(QDateTime::currentDateTime());

    if (m_clientMode)
    {
        m_client->sendAddTask(copy);
    }
    else
    {
        m_taskManager->duplicateTask(index);
    }
}

void MainWindow::removeSelectedTask()
{
    int index = selectedTaskIndex();

    if (index < 0)
    {
        return;
    }

    QString taskTitle = m_taskManager->tasks().value(index).title();
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Подтверждение",
        QString("Удалить задачу «%1»?").arg(taskTitle),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes)
    {
        return;
    }

    if (m_clientMode)
    {
        int taskId = m_taskManager->tasks()[index].id();
        m_client->sendRemoveTask(taskId);
    }
    else
    {
        m_taskManager->removeTask(index);
    }
}

void MainWindow::toggleSelectedTask()
{
    int index = selectedTaskIndex();

    if (index < 0)
    {
        return;
    }

    if (m_clientMode)
    {
        int taskId = m_taskManager->tasks()[index].id();
        m_client->sendToggleTask(taskId);
    }
    else
    {
        m_taskManager->toggleCompleted(index);
    }
}

void MainWindow::applyFilters()
{
    m_activeFilter = m_filterInput->text().trimmed();
    m_activeAssigneeFilter = m_assigneeFilterInput->text().trimmed();
    m_activeStatusFilter = m_statusFilterBox->currentIndex() - 1;
    updateTaskList();
}

void MainWindow::resetFilters()
{
    m_activeFilter.clear();
    m_activeAssigneeFilter.clear();
    m_activeStatusFilter = -1;
    m_filterInput->clear();
    m_assigneeFilterInput->clear();
    m_statusFilterBox->setCurrentIndex(0);
    updateTaskList();
}

void MainWindow::clearCompletedTasks()
{
    if (m_clientMode)
    {
        const QVector<Task> tasks = m_taskManager->tasks();
        for (const Task& task : tasks)
        {
            if (task.isCompleted())
            {
                m_client->sendRemoveTask(task.id());
            }
        }
    }
    else
    {
        m_taskManager->clearCompleted();
    }
}

QString MainWindow::defaultTasksFilePath() const
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/tasks.json";
}

bool MainWindow::loadTasksFromPath(const QString& filePath, bool showMessage)
{
    if (!QFile::exists(filePath))
    {
        return false;
    }

    JsonSerializer serializer;
    QVector<Task> tasks = serializer.load(filePath);

    if (tasks.isEmpty() && showMessage)
    {
        QMessageBox::warning(this, "Загрузка", "Файл пуст или имеет неверный формат");
        return false;
    }

    m_taskManager->setTasks(tasks);
    m_tasksFilePath = filePath;

    if (showMessage)
    {
        QMessageBox::information(this, "Загрузка",
                                 QString("Загружено задач: %1").arg(tasks.size()));
    }

    return true;
}

void MainWindow::saveTasksToFile()
{
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Сохранить задачи",
        m_tasksFilePath,
        "JSON (*.json)");

    if (filePath.isEmpty())
    {
        return;
    }

    JsonSerializer serializer;
    serializer.save(m_taskManager->tasks(), filePath);
    m_tasksFilePath = filePath;

    QMessageBox::information(this, "Сохранение", "Задачи сохранены");
}

void MainWindow::loadTasksFromFile()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Загрузить задачи",
        m_tasksFilePath,
        "JSON (*.json)");

    if (filePath.isEmpty())
    {
        return;
    }

    loadTasksFromPath(filePath, true);
}

void MainWindow::autoSaveTasks()
{
    if (m_clientMode)
    {
        return;
    }

    JsonSerializer serializer;
    serializer.save(m_taskManager->tasks(), m_tasksFilePath);
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

QString MainWindow::taskStatusText(TaskStatus status)
{
    switch (status)
    {
    case TaskStatus::Planned:
        return "Запланирована";
    case TaskStatus::InProgress:
        return "В работе";
    case TaskStatus::Completed:
        return "Выполнена";
    }

    return "Запланирована";
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
            && !task.title().contains(searchText, Qt::CaseInsensitive)
            && !task.assignee().contains(searchText, Qt::CaseInsensitive))
        {
            continue;
        }

        if (!m_activeFilter.isEmpty()
            && !task.tags().contains(m_activeFilter, Qt::CaseInsensitive))
        {
            continue;
        }

        if (!m_activeAssigneeFilter.isEmpty()
            && !task.assignee().contains(m_activeAssigneeFilter, Qt::CaseInsensitive))
        {
            continue;
        }

        if (m_activeStatusFilter >= 0
            && static_cast<int>(task.status()) != m_activeStatusFilter)
        {
            continue;
        }

        if (m_hideCompletedBox->isChecked() && task.isCompleted())
        {
            continue;
        }

        if (m_showOverdueOnlyBox->isChecked() && !task.isOverdue())
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
    else if (m_sortBox->currentIndex() == 3)
    {
        std::sort(tasks.begin(), tasks.end(),
                  [](const QPair<int, Task>& left, const QPair<int, Task>& right)
                  {
                      bool leftValid = left.second.dueDate().isValid();
                      bool rightValid = right.second.dueDate().isValid();

                      if (!leftValid && !rightValid) return false;
                      if (!leftValid) return false;
                      if (!rightValid) return true;

                      return left.second.dueDate() < right.second.dueDate();
                  });
    }
    else if (m_sortBox->currentIndex() == 4)
    {
        std::sort(tasks.begin(), tasks.end(),
                  [](const QPair<int, Task>& left, const QPair<int, Task>& right)
                  {
                      return left.second.createdAt() > right.second.createdAt();
                  });
    }
    else if (m_sortBox->currentIndex() == 5)
    {
        std::sort(tasks.begin(), tasks.end(),
                  [](const QPair<int, Task>& left, const QPair<int, Task>& right)
                  {
                      return left.second.createdAt() < right.second.createdAt();
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

        QString statusText;
        if (task.status() == TaskStatus::Completed)
        {
            statusText = "[✓]";
        }
        else if (task.status() == TaskStatus::InProgress)
        {
            statusText = "[~]";
        }
        else
        {
            statusText = "[ ]";
        }

        QString itemText = statusText + " "
                           + task.title()
                           + " | Статус: " + taskStatusText(task.status())
                           + " | Приоритет: " + priorityText(task.priority());

        if (task.createdAt().isValid())
        {
            itemText += " | Создана: " + task.createdAt().toString("dd.MM.yyyy HH:mm");
        }

        if (task.dueDate().isValid())
        {
            itemText += " | Срок: " + task.dueDate().toString("dd.MM.yyyy");

            if (task.isOverdue())
            {
                itemText += " (просрочена)";
            }
        }

        if (!task.assignee().isEmpty())
        {
            itemText += " | Исполнитель: " + task.assignee();
        }

        itemText += " | Теги: " + task.tags().join(", ");

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
        else
        {
            if (task.isOverdue())
            {
                item->setForeground(Qt::red);
            }
            else
            {
                switch (task.priority())
                {
                case TaskPriority::High:
                    item->setForeground(QColor("#C0392B"));
                    break;
                case TaskPriority::Low:
                    item->setForeground(QColor("#27AE60"));
                    break;
                default:
                    break;
                }
            }
        }
    }

    int total = m_taskManager->tasks().size();
    int completed = m_taskManager->completedCount();
    int overdue = m_taskManager->overdueCount();
    QString statusText = QString("Всего: %1 | Выполнено: %2 | Просрочено: %3 | Показано: %4")
                             .arg(total)
                             .arg(completed)
                             .arg(overdue)
                             .arg(tasks.size());

    if (!m_activeFilter.isEmpty())
    {
        statusText += QString(" | Тег: %1").arg(m_activeFilter);
    }

    if (!m_activeAssigneeFilter.isEmpty())
    {
        statusText += QString(" | Исполнитель: %1").arg(m_activeAssigneeFilter);
    }

    if (m_activeStatusFilter >= 0)
    {
        statusText += " | Статус: "
                      + taskStatusText(static_cast<TaskStatus>(m_activeStatusFilter));
    }

    if (!m_searchInput->text().trimmed().isEmpty())
    {
        statusText += QString(" | Поиск: %1").arg(m_searchInput->text().trimmed());
    }

    m_statusLabel->setText(statusText);
}

void MainWindow::startAutomaticSync()
{
    if (m_autoSyncInProgress)
    {
        return;
    }

    if (m_client->isConnected() || m_server)
    {
        updateSyncStatus();
        return;
    }

    m_autoSyncInProgress = true;
    m_syncStatusLabel->setStyleSheet("color: #666666;");
    m_syncStatusLabel->setText("Поиск общего списка в локальной сети...");

    stopDiscoverySearch();
    m_discoveryClient = new QUdpSocket(this);
    connect(m_discoveryClient, &QUdpSocket::readyRead,
            this, &MainWindow::onDiscoveryResponse);

    m_discoveryClient->writeDatagram(
        DiscoveryRequest, QHostAddress::LocalHost, DiscoveryPort);
    m_discoveryClient->writeDatagram(
        DiscoveryRequest, QHostAddress::Broadcast, DiscoveryPort);

    QTimer::singleShot(900, this, &MainWindow::finishAutomaticSyncSearch);
}

void MainWindow::finishAutomaticSyncSearch()
{
    if (!m_autoSyncInProgress || m_client->isConnected())
    {
        return;
    }

    stopDiscoverySearch();

    if (startLocalServer(m_syncPort))
    {
        m_autoSyncInProgress = false;
        updateSyncStatus();
        return;
    }

    m_autoSyncInProgress = false;
    m_syncStatusLabel->setStyleSheet("color: #B9770E;");
    m_syncStatusLabel->setText("Общий список найден · подключение...");
    QTimer::singleShot(350, this, &MainWindow::startAutomaticSync);
}

void MainWindow::onDiscoveryResponse()
{
    while (m_discoveryClient && m_discoveryClient->hasPendingDatagrams())
    {
        QHostAddress senderAddress;
        quint16 senderPort = 0;
        QByteArray data;
        data.resize(static_cast<int>(m_discoveryClient->pendingDatagramSize()));
        m_discoveryClient->readDatagram(
            data.data(), data.size(), &senderAddress, &senderPort);

        if (!data.startsWith(DiscoveryResponse))
        {
            continue;
        }

        bool portOk = false;
        const quint16 port = data.mid(DiscoveryResponse.size()).toUShort(&portOk);
        if (!portOk)
        {
            continue;
        }

        m_syncHost = senderAddress.isLoopback()
                         ? QString("127.0.0.1")
                         : senderAddress.toString();
        m_syncPort = port;
        stopDiscoverySearch();

        if (m_client->connectToServer(m_syncHost, m_syncPort, 1500))
        {
            m_autoSyncInProgress = false;
            updateSyncStatus();
            return;
        }

        m_autoSyncInProgress = false;
        QTimer::singleShot(350, this, &MainWindow::startAutomaticSync);
        return;
    }
}

bool MainWindow::startLocalServer(quint16 port)
{
    if (m_server)
    {
        return true;
    }

    m_server = new TaskServer(m_taskManager, this);
    connect(m_server, &TaskServer::clientConnected,
            this, &MainWindow::onServerClientConnected);
    connect(m_server, &TaskServer::clientDisconnected,
            this, &MainWindow::onServerClientDisconnected);

    if (!m_server->start(port))
    {
        delete m_server;
        m_server = nullptr;
        return false;
    }

    m_connectedClients = 0;
    setClientModeEnabled(false);
    startDiscoveryResponder();
    updateSyncStatus();
    return true;
}

void MainWindow::stopLocalServer()
{
    if (!m_server)
    {
        return;
    }

    m_server->stop();
    delete m_server;
    m_server = nullptr;
    m_connectedClients = 0;

    if (m_discoveryServer)
    {
        m_discoveryServer->close();
        m_discoveryServer->deleteLater();
        m_discoveryServer = nullptr;
    }
}

void MainWindow::startDiscoveryResponder()
{
    if (m_discoveryServer)
    {
        return;
    }

    m_discoveryServer = new QUdpSocket(this);
    const bool bound = m_discoveryServer->bind(
        QHostAddress::AnyIPv4,
        DiscoveryPort,
        QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    if (!bound)
    {
        m_discoveryServer->deleteLater();
        m_discoveryServer = nullptr;
        return;
    }

    connect(m_discoveryServer, &QUdpSocket::readyRead,
            this, &MainWindow::onDiscoveryRequest);
}

void MainWindow::onDiscoveryRequest()
{
    while (m_discoveryServer && m_discoveryServer->hasPendingDatagrams())
    {
        QHostAddress senderAddress;
        quint16 senderPort = 0;
        QByteArray data;
        data.resize(static_cast<int>(m_discoveryServer->pendingDatagramSize()));
        m_discoveryServer->readDatagram(
            data.data(), data.size(), &senderAddress, &senderPort);

        if (data == DiscoveryRequest && m_server)
        {
            const QByteArray response = DiscoveryResponse
                                        + QByteArray::number(m_syncPort);
            m_discoveryServer->writeDatagram(
                response, senderAddress, senderPort);
        }
    }
}

void MainWindow::stopDiscoverySearch()
{
    if (!m_discoveryClient)
    {
        return;
    }

    m_discoveryClient->close();
    m_discoveryClient->deleteLater();
    m_discoveryClient = nullptr;
}

void MainWindow::onConnected()
{
    setClientModeEnabled(true);
    updateSyncStatus();
}

void MainWindow::onDisconnected()
{
    setClientModeEnabled(false);

    if (!m_autoSyncInProgress && !m_server)
    {
        m_syncStatusLabel->setStyleSheet("color: #B9770E;");
        m_syncStatusLabel->setText("Соединение потеряно · переподключение...");
        QTimer::singleShot(1200, this, &MainWindow::startAutomaticSync);
    }
}

void MainWindow::onConnectionError(const QString& error)
{
    if (!m_autoSyncInProgress)
    {
        m_syncStatusLabel->setStyleSheet("color: #B03A2E;");
        m_syncStatusLabel->setText("Ошибка синхронизации: " + error);
    }
}

void MainWindow::onTasksReceived(const QVector<Task>& tasks)
{
    m_taskManager->setTasks(tasks);
}

void MainWindow::setClientModeEnabled(bool enabled)
{
    m_clientMode = enabled;
}

void MainWindow::onServerClientConnected()
{
    ++m_connectedClients;
    updateSyncStatus();
}

void MainWindow::onServerClientDisconnected()
{
    if (m_connectedClients > 0)
    {
        --m_connectedClients;
    }

    updateSyncStatus();
}

void MainWindow::updateSyncStatus()
{
    if (m_client->isConnected())
    {
        m_syncStatusLabel->setStyleSheet("color: #1E8449;");
        m_syncStatusLabel->setText("● Общий список подключён автоматически");
        return;
    }

    if (m_server)
    {
        m_syncStatusLabel->setStyleSheet("color: #2874A6;");
        m_syncStatusLabel->setText(
            QString("● Общий список создан · участников: %1")
                .arg(m_connectedClients + 1));
    }
}
