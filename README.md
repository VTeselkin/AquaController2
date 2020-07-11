
    

Документация
============
![Screenshot_3.png](https://github.com/VTeselkin/AquaController2/blob/master/docs/Top.jpg?raw=false)

Описание устройства
============
Акваконтроллер предназначен для управления электрооборудованием аквариума. Содержит до восьми каналов для подключения нагрузок, которые могут управляться как в ручном режиме, так и в автоматическом режимах. А так же содержит до 10 шим каналов для управление внешними LED драйверами. Имеет гибкую систему настройки. Любой выход контроллера можно подключить к любому таймеру или температурному датчику. Также можно запрограммировать на один выход несколько таймеров.
 
* * *
*   Часы. Индикация даты и времени.
*   Управление аквариумными нагрузками от 4 до 8 каналов.
*   До 10 ежедневных программ таймера.
*   До 10 ежечасных программ таймера. С диапазоном длительности работы до 60 минут.
*   До 10 секундных разовых таймеров. С диапазоном длительности работы от 1 до 999 секунд.

*   Поддержка до 8 цифровых датчиков температуры на 3-х проводной шине.
*   Поддержка двух электродов рН и управление клапаном СО2\. Точность измерения pH +/- 0,02.
*   Измерение температуры в пределах -25…+85 градусов. Точность измерения 0,5°C. Возможность управления нагрузкой (обогревателем/охладителем) на любом из 8 каналов выхода.
*   Аккумулятор для работы часов микроконтроллера. Сохранение хода часов.
*   Автоматическое сохранение в энергонезависимой памяти всех настроек (кроме хода часов) при полном отключении питания и разряде аккумулятора.
*   Управление устройством через 2-ух осевой джойстик.
*   Вывод информации на жидкокристаллический дисплей 20*4 с минимальными сокращениями, интуитивно-понятный интерфейс.
*   Связь со смартфоном на ОС Android по Wi-Fi. Дополнительное программное обеспечение для быстрого программирования таймеров, термостата, синхронизации времени.
*   Удаленное управление устройством по средствам Wi-Fi в локальной сети (либо сети Internet при определенной настройка сетевого оборудования).
*   Самостоятельная синхронизация и корректировка даты и времени устройства при наличии доступа устройству в сеть Internet. Либо удаленная синхронизация через смартфон.
*   Функция слежения за отклонением показаний датчиков от среднего установленного значения со звуковым оповещением и индикацией аварийного датчика.
*   Отключение нагрузки (фильтра) на произвольном выходе при нажатии одной кнопки на 5-30 минут (временное отключение фильтра).
*  
Протокол передачи данных 
=========================

AquaController v3.0
17 января 2017

[](#)

Для связи с устройством используется встроенный модуль связи ESP32. Для обмена данных используется порт **8888** и сетевой протокол передачи данных **UDP**. Для обменом информации между устройством и клиентом пользователя используется внутренний протокол устройства основанный на JSON:

Устройство связи принимает команды трех типов: **GET**, **POST**, **INFO**:

Команды типа GET позволяют делать запрос на получения данных от устройства.

**Синтаксис команд:** все команды отправляются только в строчном виде. В устройстве присутствует внутренний валидатор запросов, однако проверки на валидность параметров не предусмотрено, и вся валидация параметров должна происходить на стороне клиента, поэтому при разработке необходимо учитывать этот момент. Так как использование не валидных параметров может вызвать неоднозначную работу устройства.

**Формат команд GET:**
----------------------

{"status":"get","message":"device"}

**status** - тип отправляемой команды: GET, POST, INFO

**message** - команда устройству:

*   dev - информация об устройстве.
*   c\_s - информация об состоянии каналов устройства
*   td\_s - информация о ежедневных таймерах устройства
*   th\_s - информация о ежечасных таймерах устройства
*   ts\_s - информация о секундных таймерах устройства
*   te\_s - информация о температурных настройках устройства
*   t\_sen - информация о температурных датчиках устройства

**data** - дополнительный параметр для передачи параметров запроса, в запросе типа GET не используется.

На каждый запрос GET устройство шлет ответ в виде своего внутреннего JSON. Такие запросы и примеры таких ответов представлены ниже:

### **Информация об устройстве:**

*   **Запрос:**
    
        {
        	"status": "get",
        	"message": "dev"
        }
    

*   **Ответ:**
    
        {
            "status": "success",
            "message": "dev",
            "data": {
                "ver": "AQ_CH08W",
                "m_t": 10,
                "m_t_se": 4,
                "min_t": 1600,
                "max_t": 3500
            }
        }
    

*   _"ver": "AQ\_CH08W" - версия устройства_
*   _"m\_t": 8 - максимальное количество таймеров (ежедневных, ежечасных, секундных)_
*   _"m\_t\_se": 4 - максимально возможное количество подключенных температурных датчиков_
*   _"min\_t": 1600 - порог минимальной температуры_
*   _"max\_t": 3500 - порог максимальной температуры_

### **Информация об состоянии каналов устройства:**

*   **Запрос:**
    
        {
        	"status": "get",
        	"message": "c_s"
        }
    
*   **Ответ:**
    
        {
            "status": "success",
            "message": "c_s",
            "data": {
                "cl": [2, 1, 1, 1, 1, 1, 1, 1],
                "c_t": [2, 3, 3, 3, 3, 3, 3, 3]
            }
        }
    
*   _"cl": \[2, 2, 1, 1, 1, 1, 1, 1\] - состояние канала (1 - off, 2 - on, 3-daily, 4-hour, 5-sec, 6-temp)_
*   _"c\_t": \[3, 2, 1, 1, 1, 1, 1, 1\] - настройки канала (1 - off, 2 - on, 3 - auto)_

### **Информация о ежедневных таймерах устройства:**

*   **Запрос:**
    
        {
        	"status": "get",
        	"message": "td_s"
        }
    
*   **Ответ:**
    
        {
            "status": "success",
            "message": "td_s",
            "data": {
                "dt_h_s": [0, 12, 12, 0, 0, 0, 0, 0, 0, 0],
                "dt_h_end": [0, 20, 21, 0, 0, 0, 0, 0, 0, 0],
                "dt_m_s": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                "dt_m_e": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                "dt_s": [0, 1, 1, 0, 0, 0, 0, 0, 0, 0],
                "dt_c": [0, 1, 2, 0, 0, 0, 0, 0, 0, 0]
            }
        }
    
*   _"dt\_h\_s": \[0, 0, 0, 0, 0, 0, 0, 0\] - час включения таймера (0...23)_
*   _"dt\_h\_end": \[0, 0, 0, 0, 0, 0, 0, 0\] - час выключения таймера (0...23)_
*   _"dt\_m\_s": \[0, 0, 0, 0, 0, 0, 0, 0\] - минута включения таймера (0...59)_
*   _"dt\_m\_e": \[0, 0, 0, 0, 0, 0, 0, 0\] - минута выключения таймера (0...59)_
*   _"dt\_s": \[0, 0, 0, 0, 0, 0, 0, 0\] - состояние таймера (0 - off, 1 - on)_
*   _"dt\_c": \[0, 0, 0, 0, 0, 0, 0, 0\] - канал управляемый таймером (0...max\_canal)_

### **Информация о ежечасных таймерах устройства:**

*   **Запрос:**
    
        {
        	"status": "get",
        	"message": "th_s"
        }
    
*   **Ответ:**
    
        {
            "status": "success",
            "message": "th_s",
            "data": {
                "ht_m_st": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                "ht_m_sp": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                "ht_s": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                "ht_c": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
            }
        }
    
*   _"ht\_m\_st": \[0, 0, 0, 0, 0, 0, 0, 0\] - минута включения таймера (0...59)_
*   _"ht\_m\_sp": \[0, 0, 0, 0, 0, 0, 0, 0\] - минута выключения таймера (0...59)_
*   _"ht\_s": \[0, 0, 0, 0, 0, 0, 0, 0\] - состояние таймера (0 - off, 1 - on)_
*   _"ht\_c": \[0, 0, 0, 0, 0, 0, 0, 0\] - канал управляемый таймером (0...max\_canal)_

### **Информация о секундных таймерах устройства:**

*   **Запрос:**
    
        {
        	"status": "get",
        	"message": "ts_s"
        }
    
*   **Ответ:**
    
        {
            "status": "success",
            "message": "ts_s",
            "data": {
                "st_h_s": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                "st_m_s": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                "st_d": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                "st_s": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                "st_c": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
            }
        }
    
*   _"st\_h\_s: \[0, 0, 0, 0, 0, 0, 0, 0\] - час включения таймера (0...23)_
*   _"st\_m\_s": \[0, 0, 0, 0, 0, 0, 0, 0\] - минута включения таймера (0...59)_
*   _"st\_d": \[0, 0, 0, 0, 0, 0, 0, 0\] - длительность работы таймера в секундах (0...255)_
*   _"st\_s": \[0, 0, 0, 0, 0, 0, 0, 0\] - состояние таймера (0 - off, 1 - on)_
*   _"st\_c": \[0, 0, 0, 0, 0, 0, 0, 0\] - канал управляемый таймером (0...max\_canal)_

### **Информация о температурных настройках устройства:**

*   **Запрос:**
*       {
        	"status": "get",
        	"message": "te_s"
        }
    
*   **Ответ:**
    
*       {
            "status": "success",
            "message": "te_s",
            "data": {
                "tt_s": [0, 0, 0, 0],
                "tt_m_s": [0, 0, 0, 0],
                "tt_m_e": [76, 76, 76, 76],
                "tt_c": [1, 0, 0, 0]
            }
        }
    
*   _"tt\_s": \[1, 0, 0, 0,\] - состояние температурного таймера_
*   _"tt\_m\_s": \[0, 0, 0, 0\] - температура включения канала (min\_temp...max\_temp, кратна 50)_
*   _"tt\_m\_e": \[76, 76, 0, 0\] - температура выключения канала (min\_temp...max\_temp кратна 50)_
*   _"tt\_c": \[1, 0, 0, 0\] - канала управляемый таймером (0...max\_canal)_

### **Информация о температурных датчиках устройства:**

*   **Запрос:**
*       {
        	"status": "get",
        	"message": "t_se"
        }
    
    **Ответ:**
    
        {
            "status": "success",
            "message": "t_sen",
            "data": {
                "t_se": [4, 3, 0, 0]
            }
        }
    
*   _"t\_se": \[4, 3, 0, 0\] - температура датчиков (min\_temp...max\_temp)_

**Формат команд POST**
----------------------

Команды POST отличаются от команд GET только тем что в запросе необходимо в поле **data** указывать параметры запроса и поле **status** содержит параметр **post**. Формат данных для поля **data** аналогичен данным приходящим от устройства при запросе командой GET.

**Примеры возможных запросов:**

### **Изменение состояния каналов устройства:**

*   **Запрос:**

    {
    	"status": "post",
    	"message": "c_s",
    	"data": {
    		"c_t": [1, 0, 0, 0, 0, 0, 0, 0]
    	}
    }

Обратите внимание для изменения настроек каналов, отправлять нужно только параметр **c\_t**, параметр **cl** не предназначен для изменения, он отвечает только за текущее состояние канала в зависимости от настроек таймеров или ручных настроек.

### **Изменение ежедневных таймеров устройства:**

*   **Запрос:**

    {
    	"status": "post",
    	"message": "td_s",
    	"data": {
    		"dt_h_s": [1, 0, 0, 0, 0, 0, 0, 0],
    		"dt_h_end": [2, 0, 0, 0, 0, 0, 0, 0],
    		"dt_m_s": [5, 0, 0, 0, 0, 0, 0, 0],
    		"dt_m_e": [55, 0, 0, 0, 0, 0, 0, 0],
    		"dt_s": [1, 0, 0, 0, 0, 0, 0, 0],
    		"dt_c": [3, 0, 0, 0, 0, 0, 0, 0]
    	}
    }

### **Изменение ежечасных таймеров устройства:**

*   **Запрос:**

    {
    	"status": "post",
    	"message": "th_s",
    	"data": {
    		"ht_m_st": [45, 0, 0, 0, 0, 0, 0, 0],
    		"ht_m_sp": [56, 0, 0, 0, 0, 0, 0, 0],
    		"ht_s": [1, 0, 0, 0, 0, 0, 0, 0],
    		"ht_c": [3, 0, 0, 0, 0, 0, 0, 0]
    	}
    }

### **Изменение секундных таймеров устройства:**

*   **Запрос:**

    {
    	"status": "post",
    	"message": "ts_s",
    	"data": {
    		"st_h_s": [5, 0, 0, 0, 0, 0, 0, 0],
    		"st_m_s": [26, 0, 0, 0, 0, 0, 0, 0],
    		"st_d": [125, 0, 0, 0, 0, 0, 0, 0],
    		"st_s": [1, 0, 0, 0, 0, 0, 0, 0],
    		"st_c": [2, 0, 0, 0, 0, 0, 0, 0]
    	}
    }

### **Изменение температурных настроек устройства:**

*   **Запрос:**

    {
    	"status": "post",
    	"message": "te_s",
    	"data": {
    		"tt_s": [0, 0, 0, 0, 0, 0, 0, 0],
    		"tt_m_s": [2200, 2250, 0, 0, 0, 0, 0, 0],
    		"tt_m_e": [2700, 2850, 0, 0, 0, 0, 0, 0],
    		"tt_c": [5, 0, 0, 0, 0, 0, 0, 0]
    	}
    }

При POST запросе нет необходимости указывать все параметры в поле **data**. При передаче значений в устройство можно отправлять только те данные которые изменились на клиенте. На каждый выполненный **POST** запрос, ответом будет ответ аналогичный **GET** с тем же **message** параметром, но уже с измененными данными.

### **Пример:**

*   **Запрос**
*       {
        	"status": "post",
        	"message": "c_s",
        	"data": {
        		"c_t": [1, 1, 1, 2, 3, 2, 2, 2]
        	}
        }
    
*   **Ответ**
    
*       {
        	"status": "get",
        	"message": "c_s",
        	"data": {
        		"cl": [1, 1, 1, 2, 1, 2, 2, 2],
        		"c_t": [1, 1, 1, 2, 3, 2, 2, 2]
        	}
        }
    
*   Обратите внимание что ответ на POST приходит всегда в полном виде независимо от тех параметров которые вы отправляете.
    

**Формат команд INFO:**
-----------------------

В данный момент запросы формата INFO используются только для внутренней передачи данных между Arduino и модулем связи ESP8266. В частности используется ESP8266 для передачи своего состояния и логирования процессов. Использование внешними клиентами в данной версии не предусмотрено.

### **Пример отправки логов WIFI соединения на устройство:**

    {
    	"status": "info",
    	"message": "wifi_log",
    	"log": ""
    } 

