# Arquitectura

| Archivo | Responsabilidad |
| --- | --- |
| `src/game.hpp` | Estado y reglas del juego sin dependencia del sistema PS5 |
| `src/main.cpp` | Lectura de entrada, selección, HUD, caricaturas y dibujo de la arena |
| `src/pad_abi.h` | ABI público de mando procedente del port PS5 de SDL |
| `src/demo_renderer.cpp` | Memoria directa, VideoOut, doble búfer y fuente bitmap |
| `src/demo_renderer.hpp` | API limitada de dibujo |
| `tooling/native/` | Runtime, conversión ELF y FSELF de desarrollo |
| `tools/create-exfat.py` | Creación de la imagen de lanzamiento sin montar un disco |
| `tools/verify-exfat.py` | Lectura independiente y comparación de archivos |
| `tools/preview.cpp` | Adaptador de escritorio para revisar las escenas |

La PS5 ejecuta una aplicación nativa con `eboot.bin`. No existe una aplicación web, streaming ni emulador en la ruta de juego. La variante de iOS en Swift/SpriteKit es un proyecto diferente; esta implementación reescribe el concepto en C++20.

La escena de selección pasa a combate, después a victoria o derrota y permite empezar de nuevo. La pausa y la desconexión congelan las actualizaciones. La simulación avanza una vez por presentación, sin depender de una promesa de 60 FPS.

Los seis personajes comparten la base de combate. Las caricaturas y los textos cambian por personaje. Los especiales usan una misma zona de impacto; a partir de 50 de corrupción arcade causan más daño. A 100 de corrupción se pierde vida gradualmente. Cada oleada repone recogibles.

Los gráficos son formas CPU dibujadas en buffers VideoOut. No requiere SDL como biblioteca de ejecución; solo utiliza sus declaraciones públicas del ABI del mando. El runtime libre se empaqueta en `sce_module/libc.prx`.

La aplicación conserva el proceso vivo y se cierra desde el menú del sistema, siguiendo el contexto de lanzamiento del toolchain base. No escribe partidas ni configuración y no abre conexiones de red.

La documentación heredada del toolchain está en `docs/toolchain/`. Sus menciones a pruebas en otros firmwares corresponden al proyecto de origen y no certifican este juego.
