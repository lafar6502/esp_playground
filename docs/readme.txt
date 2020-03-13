1. jak dodawac komponenty

dodajemy katalog components
a potem git
git submodule add https://github.com/DavidAntliff/esp32-ds18b20.git components/esp32-ds18b20


w podkatalogu main cmakelists.txt
set(COMPONENT_SRCDIRS ".")
set(COMPONENT_ADD_INCLUDEDIRS ".")

to jest potrzebne zeby standardowo includowal h z podkatalogow z komponentami


2. konfiguracja menuconfig
plik Kconfig.projbuild
w katalogu main

nastepnie w pliku h
#define GPIO_DS18B20_0       (CONFIG_ONE_WIRE_GPIO)
gdzie CONFIG_ONE_WIRE_GPIO zdefiniowalismy w Kconfig.projbuild
czyli dorzuca nam definicje preprocesora


