# esp_state_magic

# ESP8266 - Wifi Connection Configuration based on States

![esp8266](https://tuts.codingo.me/wp-content/uploads/2016/05/ESP-12.jpg)

This repository serves the code used to make every device based on [ESP8266](https://en.wikipedia.org/wiki/ESP8266) abble to work standalone and be configured in order to connect a selected network based on States:
  - State 1: AP Configuration Mode: The device goes AP mode and serves a web form so the user can configure the SSID, PASS & URL for the further ESP8266 connection, send data and command retrieval.
  - State 2: Client Mode: The device now configured and connected to specified network enters Client Mode and starts to send to the configured URL the states of its [GPIO](https://en.wikipedia.org/wiki/General-purpose_input/output) (General Purpose Input/Output) every X seconds, also configurable.
  

You can also:
  - Return to State 1 by sending specific commands by POST form Server.
  - Reconfigure from State 1 or 2 some behaviour like the time between data is sent to Server.
  - Reset the EEPROM and leave it blank but without stopping actual functionality.

Still in development...
You can contribute to freesoftware and freehardware initiatives to make free IOT be accesible to everyone.
More details coming soon.


### Todos

 - Send GPIO state or data to configured URL (Server).
 - Refactoring of code and Comments.
 - Explain in this sections every function and its purposse.
 - Make examples with specific Arduino Like Sensors (DHT11 f.e.)
 - Search for power supply alternatives than USB.
 - Stress Testing & time durability Testing
 - Make a Server than makes Configuration Easier, like a GUI with Buttons or automatic.

If you can take todo, all help are welcome :)

License
----

MIT

Based on & All Thanks to 
----

https://gist.github.com/dogrocker/f998dde4dbac923c47c1

Modified by
----

```Agustin Parmisano @AgustinParmisano UNLP Argentina```

**Free Software, Free Knowledge**
