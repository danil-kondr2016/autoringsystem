# Система автоматической подачи звонков

## Состав репозитория

 -  ./src - исходные коды панели управления (проект Qt Creator);
 -  ./AutoRingServer - проект для Arduino IDE с комплектом плат ESP8266.

## Описание принципа работы

Система автоматической подачи звонков состоит из двух частей: исполнительного блока и программного обеспечения на компьютере пользователя (т. е. «Панели управления системой автоматической подачи звонков»).

Взаимодействие компьютера с установленным программным обеспечением и исполнительного блока осуществляется посредством Wi-Fi. Главное преимущество использования этого способа взаимодействия — удобство монтажа, которое достигается за счёт излучения радиоволн в диапазоне 2,4 ГГц, а значит, и отсутствия проводов.

Для управления компьютер должен быть подключён к исполнительному блоку либо напрямую, т. е. при помощи Wi-Fi-адаптера, встроенного в компьютер или подключенного при помощи USB, либо посредством Wi-Fi-приёмника, включённого в локальную вычислительную сеть школы или кабинета информатики.

## Системные требования

### Аппаратная часть

 - ОС: Windows XP (Qt 5.4) и выше;
 - Серверная часть: 
   + ESP8266 (ESP-12E);
   + реле 5В (1 шт);
   + блок питания 5В (1 шт);

### Программная часть

Для сборки:

- Qt 5.4 (Windows XP, Vista), Qt 5.9 и выше (Windows 7 и выше), соответственно - Qt Creator;
- Arduino IDE + плагин для плат ESP8266

Установщик для системы может собираться отдельно при помощи любого инструмента, например, NSIS или Inno Setup (5 - для XP и Vista, 6 и выше - для 7 и выше). Официальная сборка содержит установщики на базе Inno Setup.

Обязательное условие в этом случае - к установщику должен быть приложен файл лицензии GNU GPL 3 в формате RTF, а в скрипте формирования прописано, что установщик должен поставить в реестре ассоциацию файлов ".shdl" с этой программой.