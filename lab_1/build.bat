@echo off
echo Обновление исходных кодов...
git pull origin main

echo Создание папки для сборки...
mkdir .cmake
cd .cmake

echo Генерация файлов сборки...
cmake ..

echo Компиляция проекта...
cmake --build .

echo Сборка завершена!
pause