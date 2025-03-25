Установка и запуск
1. Настройка виртуальных портов (для тестирования)
Linux:
sudo apt install socat  # Установка socat
socat -d -d pty,raw,echo=0 pty,raw,echo=0
Запомните выведенные пути (например /dev/pts/4 и /dev/pts/5)

Windows:
Используйте эмулятор портов (например com0com) или реальный COM-порт.


2. Запуск системы
bash
Copy
./temperature_server
Программа начнет работать на порту 8080.

3. Отправка тестовых данных (если используете виртуальные порты)
В другом терминале:

# Linux
echo "25.7" > /dev/pts/X  # где X - номер порта из вывода socat

# Windows 
5. Доступ к веб-интерфейсу
Откройте в браузере:

Copy
http://localhost:8080/current
или для статистики:

Copy
http://localhost:8080/stats
Структура проекта
Copy
temperature_monitor/
├── server/
│   ├── server.cpp         # Основной код сервера
│   ├── CMakeLists.txt     # Файл сборки
│   ├── httplib.h          # Версия 0.12.1
│   └── json.hpp           # nlohmann/json
├── client/
│   ├── index.html         # Веб-интерфейс
│   ├── style.css          # Стили
│   └── script.js          # JavaScript код
├── database/
│   └── temperature.db     # Файл базы данных
└── README.md              # Инструкции
Архитектура системы
SerialReader - чтение данных с последовательного порта

DatabaseHandler - работа с базой данных

Создает таблицу temperatures при инициализации

Сохраняет показания с временными метками

Предоставляет методы для запроса данных

HttpServer - веб-интерфейс

GET /current - текущее значение температуры

GET /stats - статистика за период

Основной цикл:

Чтение данных с порта

Сохранение в базу данных

Генерация тестовых данных при отсутствии устройства

Настройка
Измените параметры в начале main():

const string serial_port = "/dev/2";  // или "COM3" для Windows
const string db_file = "temperature.db";
const int http_port = 8080;