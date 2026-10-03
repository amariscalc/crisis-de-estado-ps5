# Compilación y empaquetado

## Entorno recomendado

Ubuntu 24.04 x86_64 o WSL2. Requisitos: Clang 18, LLVM 18 (incluido `llvm-config-18`), lld 18, Make, Ninja, Python 3, g++, wget, curl, unzip, tar y exfatprogs. Ccache es opcional; desactívalo con `USE_CCACHE=0`.

```bash
sudo apt update
sudo apt install clang-18 llvm-18-dev lld-18 make ninja-build ccache g++ python3 wget curl unzip tar exfatprogs
export LLVM_CONFIG=llvm-config-18
export TAR_OPTIONS=--no-same-owner
make USE_CCACHE=0 BUILD_JOBS=4
```

La primera compilación descarga el PS5 Payload SDK v0.42 y zlib 1.3.2 con hashes fijados en `tools/setup-native-dependencies.sh`. Después construye el runtime libre y el ejecutable. No necesita un SDK propietario.

`TAR_OPTIONS` evita problemas de propietarios al extraer archivos en contenedores. `LLVM_CONFIG` permite al linker del SDK localizar `ld.lld`. La compilación completa requiere acceso a las dependencias públicas.

## Artefactos

| Ruta | Uso |
| --- | --- |
| `dist/PPSA99763/eboot.bin` | Ejecutable nativo en contenedor FSELF de desarrollo |
| `dist/PPSA99763/sce_module/libc.prx` | Runtime construido desde código fuente |
| `dist/PPSA99763/sce_sys/param.json` | Identidad, nombre y versión de la aplicación |
| `dist/PPSA99763.zip` | Carpeta de aplicación completa comprimida |
| `dist/CrisisDeEstado-v0.1.exfat` | Imagen de sistema de archivos para el montador |

El FSELF de desarrollo no es un paquete retail firmado por Sony. El juego no se lanza enviando `eboot.bin` al puerto del cargador de payloads.

## Construir exFAT

```bash
python3 tools/create-exfat.py dist/PPSA99763 dist/CrisisDeEstado-v0.1.exfat
fsck.exfat -n dist/CrisisDeEstado-v0.1.exfat
python3 tools/verify-exfat.py dist/CrisisDeEstado-v0.1.exfat dist/PPSA99763
sha256sum dist/CrisisDeEstado-v0.1.exfat
```

El packer crea una imagen sin particiones, con clústeres de 64 KiB, contenido de la aplicación en la raíz, cadenas FAT y entradas exFAT. No monta el archivo ni requiere privilegios root. Su tamaño mínimo es 128 MiB; contiene espacio libre y se comprime mucho al distribuirlo como ZIP.

Está limitado a árboles de lanzamiento con nombres ASCII, un único clúster por directorio y una tabla de asignación que quepa en un clúster. Rechaza enlaces simbólicos y sobrescrituras. No es una herramienta general para convertir dumps de juegos grandes. Antes de reconstruir, elimina únicamente la imagen antigua de `dist` o elige otro nombre.

## Pruebas de escritorio

```bash
mkdir -p build
c++ -std=c++20 -Wall -Wextra -Werror tests/crisis/game_test.cpp -o build/game_test
./build/game_test
c++ -std=c++20 -DCRISIS_HOST_PREVIEW -Wall -Wextra -Werror tools/preview.cpp -o build/preview
./build/preview build/preview
```

El segundo programa genera `build/preview-menu.ppm` y `build/preview-arena.ppm`. Comparte el código de lógica, escenas y mapas de botones; sustituye la presentación VideoOut y las llamadas al mando por adaptadores de escritorio. No verifica el ABI del sistema operativo, el montaje ni la ejecución real en PS5.

## Actualizaciones

Mantén `titleId`, `contentId` y `conceptId` coherentes. El identificador `PPSA99763` es de desarrollo, no un ID comercial registrado. Antes de distribuir ampliamente, comprueba que no colisiona con otra aplicación del usuario. Actualiza `contentVersion`, `CHANGELOG.md` y las notas de lanzamiento al crear una versión.
