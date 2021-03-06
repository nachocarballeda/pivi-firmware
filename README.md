# PIVI #

This is the firmware that runs on the XMega found on the PIVI sensor boards.

All the code in this repository is licensed Creative Commons 

![Attribution-NonCommercial-ShareAlike 4.0 International ](https://bitbucket.org/repo/xj5nj8/images/1002571225-88x31.png)

**Copyright (C) 2014, 2015 LESS Induestries S.A.**

Author: Diego Mascialino <dmascialino@gmail.com>

## ASF version dependency ##

In this project we use the Atmel Software Framework, version 3.21.0.6

```
$ wget http://www.atmel.com/images/asf-standalone-archive-3.21.0.6.zip
$ md5sum asf-standalone-archive-3.21.0.6.zip
fc1cb0440882f899c0158331292af757  asf-standalone-archive-3.21.0.6.zip
```

unzip the asf in the same directory of medidor-pivi repository, so you should
have this strcutre:

```
$ ls -l
drwxr-xr-x 10 diego users      4096 Jan  6 07:31 asf-3.21.0
-rw-r--r--  1 diego users 294699920 Jan 20 14:55 asf-standalone-archive-3.21.0.6.zip
drwxr-xr-x 15 diego users      4096 Jan 24 12:58 medidor-pivi
```

## Cómo compilar? ##

Una vez instalado ASF siguiendo los pasos anteriores:

Para compilar el firmware de calibracíon:

```
$ make PYVI_BOARD=PYVI_CALIBRATION
```

Una vez calibrado y generado el .h de calibración se recompila el firmware
para esa placa (cambiando el numero de placa):

```
$ make PYVI_BOARD=PYVI_1
```

## Cómo funciona? ##

Para cada tarea que debe realizar el firmware hay un modulo que abstrae la
utilización de los drivers. Estos son:

* Communication: Utiliza el USART e implementa el link protocol especificado.

* Analog: Utiliza el ADC y brinda la funcionalidad de tomar un muestra de la
  señale deseada y aplicar las correcciones necesarias para tener su valor en
  la unidad correspondiente (Volt o Amper).

* Time: Utiliza los Timers y permite ejecutar un callback cuando hay que tomar
  una muestra de V y otro para muestrear I.

* Measurements: Implementa la lógica para medir uno de los circuitos y envía el
  resultado a la Pi.

 * Firmware: Se ocupa de iterar por siempre sobre la lista de circuitos y tomar
   una medición de cada uno.


## Aclaración sobre el delay entre V e I ##

Las 2 señales deben ser muestreadas a la misma Frecuencia, preo debido al
filtro que se encuentra en el canal de tensión, es necesario agregar un delay.

La implementación entonces consiste en muestrear el canal de corriente a la
frecuencia especificada y se van guardando los resultados en un buffer
circular. Antes de tomar una muestra de tensión se espera el tiempo
correspondiente al circuito que se está midiendo. El resto de las muestras se
toman a la frecuencia especificada. Con cada nueva muestra de tensión se puede
utilizar la muestra de corriente correspondiente que se encuentra en el buffer
cirular.

Por lo tanto cuando se tomaron todas las muestras necesarias de tensión se
concluye la medición pues las de corriente se terminan de tomar antes.
