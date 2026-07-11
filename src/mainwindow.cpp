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
#include <algorithm>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
    m_taskManager(new TaskManager(this)),
    m_server(nullptr),
    m_client(new NetworkClient(this)),
    m_clientMode(false)
{
    setWindowTitle("Network TODO List");
    resize(800, 700);

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
    m_searchInput->setPlaceholderText("Поиск по названию...");
    filterGroupLayout->addWidget(m_searchInput);

    QHBoxLayout* tagFilterLayout = new QHBoxLayout();
    m_filterInput = new QLineEdit(this);
    m_filterInput->setPlaceholderText("Фильтр по тегу");
    tagFilterLayout->addWidget(m_filterInput, 1);
    m_filterButton = new QPushButton("Фильтр", this);
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
    controlsLayout->addWidget(new QLabel("Сортировка:"));
    controlsLayout->addWidget(m_sortBox);
    controlsLayout->addSpacing(12);
    m_hideCompletedBox = new QCheckBox("Скрыть выполненные", this);
    controlsLayout->addWidget(m_hideCompletedBox);
    controlsLayout->addStretch();
    controlsLayout->addWidget(m_saveButton = new QPushButton("Сохранить", this));
    controlsLayout->addWidget(m_loadButton = new QPushButton("Загрузить", this));
    filterGroupLayout->addLayout(controlsLayout);

    QHBoxLayout* actionRow = new QHBoxLayout();
    m_toggleButton = new QPushButton("Отметить", this);
    actionRow->addWidget(m_toggleButton);
    m_editButton = new QPushButton("Редактировать", this);
    actionRow->addWidget(m_editButton);
    m_removeButton = new QPushButton("Удалить", this);
    actionRow->addWidget(m_removeButton);
    actionRow->addSpacing(12);
    m_clearCompletedButton = new QPushButton("Очистить выполненные", this);
    actionRow->addWidget(m_clearCompletedButton);
    actionRow->addStretch();
    filterGroupLayout->addLayout(actionRow);

    mainLayout->addWidget(filterGroup);

    // ── Сеть ──
    QGroupBox* netGroup = new QGroupBox("Сеть", this);
    QVBoxLayout* netLayout = new QVBoxLayout(netGroup);

    QHBoxLayout* serverRow = new QHBoxLayout();
    serverRow->addWidget(new QLabel("Сервер: порт"));
    m_serverPortInput = new QSpinBox(this);
    m_serverPortInput->setRange(1024, 65535);
    m_serverPortInput->setValue(9999);
    serverRow->addWidget(m_serverPortInput);
    m_startServerButton = new QPushButton("Запустить", this);
    serverRow->addWidget(m_startServerButton);
    m_stopServerButton = new QPushButton("Остановить", this);
    m_stopServerButton->setEnabled(false);
    serverRow->addWidget(m_stopServerButton);
    m_serverStatusLabel = new QLabel("Сервер не запущен", this);
    serverRow->addWidget(m_serverStatusLabel);
    serverRow->addStretch();
    netLayout->addLayout(serverRow);

    QHBoxLayout* clientRow = new QHBoxLayout();
    clientRow->addWidget(new QLabel("Клиент:"));
    m_serverHostInput = new QLineEdit(this);
    m_serverHostInput->setPlaceholderText("127.0.0.1");
    m_serverHostInput->setText("127.0.0.1");
    m_serverHostInput->setMaximumWidth(130);
    clientRow->addWidget(m_serverHostInput);
    clientRow->addWidget(new QLabel(":"));
    m_clientPortInput = new QSpinBox(this);
    m_clientPortInput->setRange(1024, 65535);
    m_clientPortInput->setValue(9999);
    m_clientPortInput->setMaximumWidth(80);
    clientRow->addWidget(m_clientPortInput);
    m_connectButton = new QPushButton("Подключиться", this);
    clientRow->addWidget(m_connectButton);
    m_disconnectButton = new QPushButton("Отключиться", this);
    m_disconnectButton->setEnabled(false);
    clientRow->addWidget(m_disconnectButton);
    m_connectionStatusLabel = new QLabel("Не подключён", this);
    clientRow->addWidget(m_connectionStatusLabel);
    clientRow->addStretch();
    netLayout->addLayout(clientRow);

    mainLayout->addWidget(netGroup);

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
    connect(m_taskManager, &TaskManager::tasksChanged,
            this, &MainWindow::updateTaskList);
    connect(m_taskManager, &TaskManager::tasksChanged,
            this, &MainWindow::autoSaveTasks);

    m_tasksFilePath = defaultTasksFilePath();
    loadTasksFromPath(m_tasksFilePath, false);

    // Server buttons
    connect(m_startServerButton, &QPushButton::clicked,
            this, &MainWindow::startServer);
    connect(m_stopServerButton, &QPushButton::clicked,
            this, &MainWindow::stopServer);

    // Client buttons
    connect(m_connectButton, &QPushButton::clicked,
            this, &MainWindow::connectToServer);
    connect(m_disconnectButton, &QPushButton::clicked,
            this, &MainWindow::disconnectFromServer);
    connect(m_client, &NetworkClient::connected,
            this, &MainWindow::onConnected);
    connect(m_client, &NetworkClient::disconnected,
            this, &MainWindow::onDisconnected);
    connect(m_client, &NetworkClient::connectionError,
            this, &MainWindow::onConnectionError);
    connect(m_client, &NetworkClient::tasksReceived,
            this, &MainWindow::onTasksReceived);

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
    m_tagsInput->clear();
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
    QLineEdit* tagsEdit = new QLineEdit(task.tags().join(", "), &dialog);
    QComboBox* priorityEdit = new QComboBox(&dialog);
    priorityEdit->addItem("Низкий");
    priorityEdit->addItem("Средний");
    priorityEdit->addItem("Высокий");
    priorityEdit->setCurrentIndex(static_cast<int>(task.priority()));

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
    form->addRow("Теги:", tagsEdit);
    form->addRow("Приоритет:", priorityEdit);
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
    Task updatedTask(title, descEdit->text().trimmed(), tags, priority);
    updatedTask.setCompleted(task.isCompleted());

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
                           + " | Приоритет: " + priorityText(task.priority());

        if (task.dueDate().isValid())
        {
            itemText += " | Срок: " + task.dueDate().toString("dd.MM.yyyy");
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

            if (task.dueDate().isValid() && task.dueDate() < QDate::currentDate())
            {
                item->setForeground(Qt::red);
            }
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

    if (m_clientMode)
    {
        statusText += " | Режим: клиент";
    }

    m_statusLabel->setText(statusText);
}

// --- Server ---

void MainWindow::startServer()
{
    if (m_server)
    {
        return;
    }

    m_server = new TaskServer(m_taskManager, this);

    if (!m_server->start(static_cast<quint16>(m_serverPortInput->value())))
    {
        QMessageBox::warning(this, "Ошибка", "Не удалось запустить сервер");
        delete m_server;
        m_server = nullptr;
        return;
    }

    m_serverStatusLabel->setText("Сервер запущен");
    m_startServerButton->setEnabled(false);
    m_stopServerButton->setEnabled(true);
    m_serverPortInput->setEnabled(false);
}

void MainWindow::stopServer()
{
    if (!m_server)
    {
        return;
    }

    m_server->stop();
    delete m_server;
    m_server = nullptr;

    m_serverStatusLabel->setText("Сервер не запущен");
    m_startServerButton->setEnabled(true);
    m_stopServerButton->setEnabled(false);
    m_serverPortInput->setEnabled(true);
}

// --- Client ---

void MainWindow::connectToServer()
{
    QString host = m_serverHostInput->text().trimmed();

    if (host.isEmpty())
    {
        host = "127.0.0.1";
    }

    if (!m_client->connectToServer(host, static_cast<quint16>(m_clientPortInput->value())))
    {
        QMessageBox::warning(this, "Ошибка", "Не удалось подключиться к серверу");
    }
}

void MainWindow::disconnectFromServer()
{
    m_client->disconnectFromServer();
}

void MainWindow::onConnected()
{
    m_connectionStatusLabel->setText("Подключён");
    m_connectButton->setEnabled(false);
    m_disconnectButton->setEnabled(true);
    m_serverHostInput->setEnabled(false);
    m_clientPortInput->setEnabled(false);
    setClientModeEnabled(true);
}

void MainWindow::onDisconnected()
{
    m_connectionStatusLabel->setText("Не подключён");
    m_connectButton->setEnabled(true);
    m_disconnectButton->setEnabled(false);
    m_serverHostInput->setEnabled(true);
    m_clientPortInput->setEnabled(true);
    setClientModeEnabled(false);
}

void MainWindow::onConnectionError(const QString& error)
{
    QMessageBox::warning(this, "Ошибка подключения", error);
}

void MainWindow::onTasksReceived(const QVector<Task>& tasks)
{
    m_taskManager->setTasks(tasks);
}

void MainWindow::setClientModeEnabled(bool enabled)
{
    m_clientMode = enabled;
}
