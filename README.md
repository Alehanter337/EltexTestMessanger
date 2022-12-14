# Messanger

![C](https://img.shields.io/badge/C-Solution-blue.svg?style=flat&logo=C%2B%2B)
[![Generic badge](https://img.shields.io/badge/Develop-unstable-yellowgreen)](https://shields.io/)

`Tested on MacOS Big Sur 11.7 / Linux Ubuntu 20.04 / gcc version 9.4.0`


# Функции программы

Основная задача программы общение клиентов через сервер (брокер сообщений). Сообщения можно отправлять с или без гарантии доставки.

Клиенты могут заходить в одну из трёх групп (Alpha, Beta, Omega) и обмениваться сообщениями через неё.

Так же есть функция отправки сообщения с задержкой в секундах.

# Интерфейс клиента

User: USERNAME

Group: USER_GROUP

Choose action:

  1 - Check inbox

  2 - Send message

  3 - Join the group

  4 - Leave the group

  5 - Exit

У сервера нет интерфейса, выводится только логирование действий клиентов.

# Как храняться данные

* Со стороны сервера для каждого клиента в директории **clients_inbox/** формируется файл с названием **username**, в файле его входящие сообщения.
При запросе от клиента входящих сообщений - сервер отправляет содержимое данного файла.

* Со стороны клиента, при запуске программы в директории **group/** формируется файл с названием **username**, в файле его **группа**. При смене или выходе из группы файл обновляется. Так же смена группы отправляется серверу, который формирует файл **"List of clients"** вида:

`user1 - Alpha`

`user2 - Beta`

# Сборка программы

Для сборки в корневой папке используйте команду:

`make`

Для удаления бинарных файлов в корневой папке используйте команду:

`make clean`

# Запуск программ

Запуск программы клиента:

`./client -flags:`

`-s <server.ip.address>` / указание IP адреса сервера

` -u <your_username>` / выбор имени пользователя (макс. 16 символов)

Запуск сервера:

`./server -flags:`

`-i <server.ip.address>` / выбор адреса, который будет слушать сервер

`-c <path/to/config>` / указание пути до конфиг файла (дефолтный в src/config.conf)


# Конфиг файл

Настройка логирования через файл, стандартный файл config.conf находиться в папке src/

`LOG_LEVEL= 0 или 1`

> где 0 - нет логирующей информации / 1 - вывод логирующей информации в терминал


[![Anurag's GitHub stats](https://github-readme-stats.vercel.app/api?username=alehanter337)](https://github.com/alehanter337/github-readme-stats)
