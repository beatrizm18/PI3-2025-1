
# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS "main.c" "pid.c"  # Adicione "pid.c" aqui para ser compilado junto com o main.c
    INCLUDE_DIRS "."       # Inclui o diretório atual, onde está o pid.h
)

# Criação da partição SPIFFS
spiffs_create_partition_image(storage ../partition FLASH_IN_PROJECT)

