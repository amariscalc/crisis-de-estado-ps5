# Registro de build v0.1.0

Fecha: 2026-10-03. Compilador: Ubuntu Clang 18.1.3. Target: x86_64-sie-ps5. PS5 Payload SDK v0.42. Release Title ID: PPSA99763.

Resultado observado:

- Compilación, linkado, conversión de ELF y generación FSELF completados sin warnings en la build final.
- Integridad de eboot.bin y libc.prx: válida según la inspección del toolchain.
- Pruebas de lógica: PASS.
- Mapeo de botones mediante PadData simulado: PASS.
- Renders de menú y arena revisados en escritorio.
- fsck.exfat 1.2.2: clean; 4 directorios y 5 archivos.
- Verificación independiente: los 5 archivos de la imagen coinciden byte por byte con el árbol compilado.
- Clústeres: 65536 bytes. Imagen: 134217728 bytes (128 MiB).

Pendiente: arranque, módulos del sistema, VideoOut y DualSense real en PS5 13.60; ejecución del workflow en GitHub.
