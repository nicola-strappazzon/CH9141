# Client with native bluetooth driver

Este ejemplo no usa la librería WCHBLElib proporcionada por el fabricante, sino usa las de Linux que es mucho mejor.

```bash
sudo ./main --mac D1:5E:28:7A:4E:E0 --message "Hi from Linux" --wait
```

Ejemplo de comunicación desde una Raspberry PI que se conecta por BLE al CH9141K y el MAC se conecta usando un CH340.

![](https://github.com/nstrappazzonc/CH9141/blob/main/img/demo01.png?raw=true)
