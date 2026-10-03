# Compilar v0.2.0

## PS5 nativa

En Ubuntu 24.04 o WSL2:

```bash
sudo apt update
sudo apt install clang-18 llvm-18-dev lld-18 make ninja-build g++ python3 wget curl unzip tar exfatprogs
export LLVM_CONFIG=/usr/bin/llvm-config-18
export TAR_OPTIONS=--no-same-owner
make USE_CCACHE=0 BUILD_JOBS=4
```

El primer build descarga SDK público v0.42 y zlib 1.3.2 con hashes fijados; construye el runtime libre y el ejecutable FSELF. El repositorio incluye los manifiestos de `tooling/native/runtime/` necesarios para reproducir `libc.prx`. No necesita archivos del SDK propietario de Sony.

`LLVM_CONFIG` debe ser la ruta absoluta del ejecutable. Si está en otra ubicación, usa `export LLVM_CONFIG="$(command -v llvm-config-18)"`.

Salida: `dist/PPSA99764/`, con `eboot.bin`, `sce_module/libc.prx`, `sce_sys/` y `assets/`. Debe distribuirse la carpeta completa.

## Imagen .exfat

```bash
make exfat USE_CCACHE=0 BUILD_JOBS=4
```

O después de compilar:

```bash
python3 tools/create-exfat.py dist/PPSA99764 dist/CrisisDeEstado-v0.2.exfat
fsck.exfat -n dist/CrisisDeEstado-v0.2.exfat
python3 tools/verify-exfat.py dist/CrisisDeEstado-v0.2.exfat dist/PPSA99764
```

El packer crea una imagen sin particiones de al menos 128 MiB, con clústeres de 64 KiB y archivos de aplicación en la raíz. Rechaza sobrescribir un archivo existente: para reconstruir, elimina solo tu imagen de salida anterior o elige otro nombre. No está pensado para empaquetar dumps comerciales grandes.

## Gráficos

Los PNG editables están en `art/`. Los datos RGBA sin dependencia de decodificación PNG se incluyen en `assets/`; para regenerarlos:

```bash
python3 -m pip install Pillow
python3 tools/prepare-art.py
```

`leaders.png` es un atlas 1536×1024, seis celdas de 512×512. Orden: Sánchez, Trump, Macron; Milei, Xi, Putin. El fondo se escala a 1920×1080 al dibujar. La tipografía Bangers tiene licencia SIL OFL en `art/font/OFL.txt`.

## Escritorio y pruebas

```bash
make gameplay-test preview
sudo apt install libsdl2-dev
make desktop
./build/crisis-desktop
```

Ejecuta desde la raíz del repositorio para que encuentre `assets/`. El modo de escritorio comparte lógica y dibujo con PS5; utiliza SDL2 únicamente para ventana y entrada.

Teclado: WASD para movimiento; J golpe; K salto; L especial; I bloqueo; Q Monster; E Red Bull; espacio dash; Intro iniciar; Escape pausa; retroceso volver al menú. Mando de escritorio: distribución equivalente; R3 o gatillo derecho para dash.

La simulación avanza a 30 pasos por segundo. El bucle nativo conserva dos esperas Vblank del renderer base; no se garantiza un framerate hasta medirlo en hardware.
