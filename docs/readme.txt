1. jak dodawac komponenty

dodajemy katalog components
a potem git
git submodule add https://github.com/DavidAntliff/esp32-ds18b20.git components/esp32-ds18b20


w podkatalogu main cmakelists.txt
set(COMPONENT_SRCDIRS ".")
set(COMPONENT_ADD_INCLUDEDIRS ".")

to jest potrzebne zeby standardowo includowal h z podkatalogow z komponentami

