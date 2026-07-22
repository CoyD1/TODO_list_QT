# Командный TODO-лист

Графическое приложение для совместного управления задачами небольшой команды. Проект написан на C++ с использованием Qt Widgets, Qt Network и CMake.

## Возможности

- создание, редактирование, удаление и дублирование задач;
- назначение исполнителя и управление списком участников команды;
- статусы «Запланирована», «В работе» и «Выполнена»;
- низкий, средний и высокий приоритет;
- теги, описание, срок и дата создания задачи;
- режим задач без срока и подсветка близких сроков;
- поиск, сортировка и фильтрация по тегу, исполнителю и статусу;
- отдельный фильтр просроченных задач;
- сохранение и загрузка списка задач в JSON;
- автоматическое сохранение локального списка;
- синхронизация задач между несколькими экземплярами приложения;
- автоматическое обнаружение общего списка в локальной сети;
- отображение количества подключённых участников.

## Получение исходного кода

Склонируйте репозиторий и перейдите в его каталог:

```bash
git clone https://github.com/CoyD1/TODO_list_QT.git
cd TODO_list_QT
```

## Сборка в Windows через MSYS2

Установите [MSYS2](https://www.msys2.org/), откройте терминал **MSYS2 MinGW x64** и установите необходимые пакеты:

```bash
pacman -Syu
pacman -S --needed mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-make mingw-w64-x86_64-qt6-base
```

Если после `pacman -Syu` терминал попросит перезапуск, закройте его, снова откройте **MSYS2 MinGW x64** и повторите обновление.

В каталоге проекта выполните:

```bash
cmake -S . -B build-release -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j
cmake --install build-release --prefix "$PWD/dist"
```

Готовое приложение будет находиться в каталоге `dist/bin`. Команда установки также копирует необходимые библиотеки Qt и плагины.

## Сборка в Windows через Qt Creator

1. Установите Qt 6 с комплектом **MinGW 64-bit** и Qt Creator.
2. Откройте корневой файл `CMakeLists.txt` через Qt Creator.
3. Выберите комплект Desktop Qt 6 MinGW 64-bit.
4. Выберите конфигурацию Release.
5. Нажмите **Build → Build Project**.
6. Для запуска используйте **Run → Run**.

## Сборка в macOS

Установите Xcode Command Line Tools:

```bash
xcode-select --install
```

При использовании Homebrew установите Qt и CMake:

```bash
brew install qt cmake
```

В каталоге проекта выполните:

```bash
cmake -S . -B build-release \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build build-release -j
cmake --install build-release --prefix "$PWD/dist"
```

После установки приложение будет находиться в каталоге `dist/TODO_lists_QT.app`.
