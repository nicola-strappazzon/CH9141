# Configuración

El IC debe ser configurado mediante comandos AT a través del protocolo UART, puede usar el IC [CH340](https://github.com/nstrappazzonc/CH340) para conectarse e interactuar. Recuerde que debe iniciar el IC en `AT mode`, para eso debe conectar el puerto AT a GND.

![](https://github.com/nstrappazzonc/CH9141/blob/main/img/connection.png?raw=true)

El diagrama anterior solo muestra la conexión entre un CH340G y CH9141K de forma orientativa para establecer la comunicación necesaria para la configuración y pruebas básicas.

Para practicar y entender mejor todo lo que se explica, haga uso de la [siguiente guía para crear un entorno de desarrollo](https://github.com/nstrappazzonc/CH9141/blob/main/doc/development.md).

## Clientes

Listamos una serie de clientes recomendados para conectarnos, luego la configuración que deberá definir para establecer la conexión:

- Linux: [moserial](https://wiki.gnome.org/Apps/Moserial), minicom, screen.
- MAC: [CoolTerm](https://freeware.the-meiers.org).
- Windows: [PuTTY](https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html).

**NOTA:** Tener cuidado que algunos clientes que no permiten enviar comandos como `tio`.

Para conectarnos, el IC utiliza las siguientes propiedades de conexión por defecto, las mismas que deberá definir en el cliente:

- Port: Depende del sistema operativo.
- Baudrate: 115200
- Data bits: 8
- Stop bits: 1
- Parity: none

![](https://github.com/nstrappazzonc/CH9141/blob/main/img/config01.png?raw=true)

![](https://github.com/nstrappazzonc/CH9141/blob/main/img/config02.png?raw=true)

## Lo básico

La sintaxis de los comandos AT que serán usadas es la siguiente:

```
<AT><+><COMMAND><operator><parameter><{CR}{LF}>
```

Todo comando empieza por `AT`, el símbolo `+` es opcional, más adelante hablaremos de los `COMMAND` más importantes, el `operator` hace referencia a dos modos; obtener `?` o definir `=`, luego tenemos el `parameter` que indica el nuevo valor que se le asignara y por último `<{CR}{LF}>` que hace referencia a `\r\n`. Es importante terminar cada comando con `<{CR}{LF}>` sino este no será ejecutado.

Si la instrucción es recibida por el host de forma satisfactoria, este responderá con un `<OK><{CR}{LF}>`. De lo contrario, responderá un mensaje de error `<{CR}{LF}><ERR:><error code><{CR}{LF}>`.

Cuando se conecta al IC con la configuración por defecto de fábrica, podrá recibir un mensaje que dice `Welcome` y si escribe `AT...` recibirá un `OK`. Este último comando es utilizado para verificar que la conexión es correcta.

Si queremos preguntar el nombre del dispositivo lo hacemos de la siguiente forma:

```
AT NAME?
```

Si queremos configurar un nuevo nombre al dispositivo, hágalo de la siguiente forma:

```
AT NAME=demo
AT SAVE
AT RESET
```

Para este caso estamos usando tres comandos, el primero `AT NAME=demo` le está asignando un nombre, el segundo `AT SAVE` guarda la configuración y el tercero `AT RESTART` reinicia el IC para que se inicie con los nuevos cambios.

## Errores

| Code | Mensaje                                                                                                       |
|:----:|---------------------------------------------------------------------------------------------------------------|
|  01  | Error de cache, el chip no tiene cache para responder, debe intentar más tarde.                               |
|  02  | Error de parámetro, no está completo o bien formado la secuencia completa del comando AT.                     |
|  03  | Comando no soportado, puede que algún comando no esté soportado según el modo actual.                         |
|  04  | El comando no puede ser ejecutado, el comando no puede ser ejecutado temporalmente y deberá intentarlo luego. |

## Lista de comandos

Listamos los comandos necesarios para el CH9141K, para obtener más detalle no olvide consultar el [datasheet](https://github.com/nstrappazzonc/CH9141/blob/main/doc/CH9141DS1.PDF).

| Comando  | Descripción                                                 |
|----------|-------------------------------------------------------------|
|AT...     | Entra en el modo de configuración AT.                       |
|AT RESET  | Reinicia el chip, **no** borra la configuración.            |
|AT VER    | Obtiene el número de versión del chip.                      |
|AT HELLO  | Obtiene/Define el mensaje de bienvenida.                    |
|AT RELOAD | Reinicia todos los parámetros.                              |
|AT SAVE   | Guarda todos los parámetros.                                |
|AT EXIT   | Sale del modo de configuración AT.                          |
|AT UART   | Obtiene/Define los parámetros del UART.                     |
|AT MAC    | Obtiene la dirección local del MAC.                         |
|AT TPL    | Obtiene/Define la potencia de transmición.                  |
|AT BLESTA | Obtiene el estado del Bluetooth.                            |
|AT DISCONN| Desconecta la actual conección.                             |
|AT BLEMODE| Obtiene/Define el modo del Bluetooth.                       |
|AT CCADD  | Obtiene la dirección MAC del device conectado.              |
|AT NAME   | Obtiene/Define el nombre del chip, max 18 carácteres.       |
|AT PNAME  | Obtiene/Define el nombre del dispositivo, max 18 carácteres.|
