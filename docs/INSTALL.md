# Instalación en PS5

Entorno objetivo: PS5 13.60 con AutoWebKit 0.5.2 / Relapse, kstuff y ShadowMount ya configurados. La versión concreta de kstuff y del montador debe ser compatible con el firmware y con aplicaciones nativas. AutoWebKit por sí solo no registra ni monta este título.

## Desde una imagen .exfat

1. Extrae `CrisisDeEstado-v0.1.exfat` del paquete de la versión.
2. Cierra cualquier instancia anterior de Crisis de Estado.
3. Copia la imagen a un directorio que tu ShadowMount escanee. Ejemplo para un USB: `homebrew/CrisisDeEstado-v0.1.exfat` en su raíz.
4. Activa tu cadena de homebrew habitual y espera a que el montador detecte el título. Si ya está activa, espera al siguiente escaneo.
5. Abre **Crisis de Estado** desde el menú de juegos de la consola.
6. Usa cruceta izquierda/derecha para elegir personaje y X para comenzar.

No es necesario modificar el firmware ni reinstalar AutoWebKit. Este paquete no cambia la configuración de tus payloads. No mantengas la carpeta de aplicación y la imagen `.exfat` del mismo Title ID en rutas de escaneo simultáneamente.

## Alternativa: carpeta completa

Si tu montador admite aplicaciones nativas por carpeta, copia **todo** `PPSA99763/` al directorio de homebrew que tengas configurado. Una ubicación habitual es `/data/homebrew/PPSA99763/`. Dentro deben quedar `eboot.bin`, `sce_sys/`, `sce_module/` y `assets/`. No copies el ZIP directamente ni solo el ejecutable.

## Si no aparece

- Confirma que el archivo tiene extensión `.exfat` y que no sigue dentro de otro ZIP.
- Comprueba la ruta de escaneo de tu instalación y que no existe otra copia con Title ID `PPSA99763`.
- Revisa el registro del montador. En ShadowMountPlus la ruta documentada habitual es `/data/shadowmount/debug.log`.
- Verifica el SHA-256 contra `SHA256SUMS.txt` del lanzamiento.

## Si aparece pero no arranca

Anota el código de error y las versiones exactas de firmware, kstuff y ShadowMount. 

Si aparece una pantalla negra, registra si recibiste la notificación de inicio y si el sistema permite cerrar la aplicación. Si la imagen aparece pero el mando no responde, prueba un DualSense conectado al usuario que ha iniciado sesión y registra si aparece `CONECTA EL DUALSENSE`.

## Actualizar

Cierra el juego. Sustituye la imagen cuando el montador haya liberado la anterior; si la conserva montada, usa el procedimiento de desmontaje/recarga de tu versión de ShadowMount. No reemplaces un archivo mientras el título está ejecutándose.

## Fuentes técnicas

- [AutoWebKit](https://github.com/itsPLK/ps5-webkit-autoloader)
- [ShadowMountPlus: formatos y rutas](https://github.com/drakmor/ShadowMountPlus)
- [Base de aplicación nativa](https://github.com/blackbearreloaded/ps5-native-app-boilerplate)
