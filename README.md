# ClientServerApp

Многопоточное асинхронное клиент-серверное приложение на C++ с использованием Boost.Asio (на корутинах) и юнит-тестами на Boost.Test.

---

## Возможности

### Сервер (C++):
- Многопоточная асинхронная работа на Boost.Asio с корутинами (C++20).
- Паралельная работа множества клиентов.
- Команды:
  - `$get <key>` - получить значение.
  - `$set <key>=<value>` - установить значение.
- Хранение конфигурации в `config.txt` с автоматическим сохранением.
- Статистика запросов:
  - по ключам (reads/writes),
  - общая (каждые 5 секунд в консоль).
  
### Клиент (Python):
- Эмуляция параллельной работы нескольких клиентов (количество задаётся параметром запуска).
- Клиент работает в одном потоке.
- Генерирует 10 000 случайных команд (99% - чтение, 1% - запись).
- Поддерживает реконнект при потере соединения.

---

## Зависимости

### Сервер
- Компилятор C++ с поддержкой стандарта C++20 (например, GCC 10+, Clang 11+, MSVC 2019+)
- [Boost](https://www.boost.org/) (через [vcpkg] для Windows (https://github.com/microsoft/vcpkg)):
  - `boost-system`
  - `boost-asio`
  - `boost-test`
- [nlohmann/json](https://github.com/nlohmann/json)
- [zlib](https://zlib.net/)

### Клиент
- Python 3.7+
- Модули стандартной библиотеки: `socket`, `random`, `time`, `zlib`, `sys`, `concurrent.futures`

---

## Установка зависимостей

### Linux
```sh
sudo apt install libboost-system-dev nlohmann-json3-dev zlib1g-dev python3
```

### Windows (через vcpkg)
```sh
vcpkg install boost-system boost-test nlohmann-json zlib
```
Убедитесь, что путь к vcpkg прописан в переменной среды или используйте `-DCMAKE_TOOLCHAIN_FILE`.

---

## Сборка проекта

### Linux
```sh
mkdir build
cd build
cmake ..
cmake --build . --config Debug
```

### Windows (через vcpkg)
```sh
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[путь_к_vcpkg]/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Debug
```
Замените `[путь_к_vcpkg]` на путь к вашей папке vcpkg.

---

## Запуск юнит-тестов

```sh
cd build
ctest -C Debug --output-on-failure
```

Тесты покрывают:

* логику конфигурации (ConfigManager)
* подсчёт статистики (Stats)

---

## Структура проекта

```
ClientServerApp/
├── ServerApp/
│   ├── src/            # Исходники сервера
│   ├── test/           # Юнит-тесты на Boost.Test
│   └── CMakeLists.txt
├── ClientApp/          # Клиент (Python)
│   └── client.py
├── CMakeLists.txt      # Главный CMake
```

---

## Запуск сервера

### Linux
```sh
cd build/ServerApp
./ServerApp
```

### Windows
```bat
cd build\ServerApp\Debug
ServerApp.exe
```
Если config.txt не существует, он будет создан автоматически.

---

## Запуск клиента

Перейдите в папку клиента:
```sh
cd ClientApp
```

Запуск одного клиента:
```sh
python3 client.py
```
или для Windows:
```bat
py client.py
```

Запуск нескольких клиентов (например, 3):
```sh
python3 client.py 3
```
или для Windows:
```bat
py client.py 3
```

Клиент подключается к серверу на `localhost:8888` и отправляет 10 000 команд.

---
