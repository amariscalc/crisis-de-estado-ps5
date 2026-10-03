# Validación y límites

## Validado en entorno de desarrollo

- Compilación C++20 con Clang 18 y PS5 Payload SDK v0.42.
- Linkado y conversión a ejecutable PS5 nativo.
- Integridad de `eboot.bin` y `sce_module/libc.prx` mediante las herramientas del toolchain.
- Lógica de selección, daño y dirección del golpe, especial, salto, bloqueo, pausa, límites de movimiento, recogibles, corrupción, progresión, victoria, derrota y reinicio.
- Entrada DualSense simulada por el adaptador de escritorio.
- Revisión visual del menú y de la arena renderizados por el mismo código de escenas en escritorio.
- Imagen `.exfat` limpia según exfatprogs 1.2.2, clústeres de 64 KiB y comparación byte por byte de sus archivos con la carpeta compilada.

## Pendiente en hardware

| Comprobación | Estado |
| --- | --- |
| Detección y registro por ShadowMount en 13.60 | Pendiente |
| Arranque con kstuff y Relapse | Pendiente |
| VideoOut, colores, estabilidad y cierre por el sistema | Pendiente |
| DualSense real y conexión tras desconectarlo | Pendiente |
| Duración de partida completa y rendimiento | Pendiente |

El uso de encabezados públicos y la compilación correcta comprueban consistencia estática; no prueban que el ABI o la disponibilidad de módulos coincidan con un firmware concreto.

## Sesión de prueba propuesta

1. Registra versiones exactas del firmware, AutoWebKit, kstuff y ShadowMount.
2. Verifica que la imagen se registra con el nombre y el icono correctos.
3. Comprueba los seis personajes y una partida completa de tres oleadas.
4. Prueba movimiento, ataque repetido, salto, especial, bloqueo y pausa.
5. Recoge bebidas y maletines; comprueba cambios de vida, energía y corrupción.
6. Desconecta y reconecta el mando. La partida debe congelarse mientras no está conectado.
7. Cierra desde el menú del sistema y vuelve a abrir.
8. Adjunta código de error, log del montador y reproducción si hay un fallo.

El loop de presentación usa doble búfer y dos esperas de VBlank por actualización como primera estrategia conservadora. Su cadencia y rendimiento reales se deben medir en consola. No hay guardado persistente ni red en el juego.
