# Publicar el proyecto en GitHub

## Nombre y descripción

Nombre sugerido: `crisis-de-estado-ps5`

Descripción sugerida:

> Native PS5 political satire beat’em up. C++20, DualSense controls and exFAT homebrew packaging.

Temas sugeridos: `ps5`, `homebrew`, `cpp`, `game`, `beat-em-up`, `dualsense`, `exfat`.

## Qué subir al repositorio

Sube el contenido de la carpeta `source/` del paquete: código, herramientas, metadatos, icono, documentación, pruebas, workflow y licencia. La raíz del repositorio debe contener `README.md` y `Makefile`; no añadas un nivel extra `source/` al repositorio.

`.gitignore` excluye `build/`, `dist/`, `.deps/`, `.env`, runtime compilado y ejecutables. Las imágenes `.exfat` y ZIP compilados se publican como artefactos de una release, no como archivos Git. Mantén los avisos de licencia de terceros.

## Primer commit

Crea un repositorio vacío en tu cuenta. Dentro de `source/`:

```bash
git init -b main
git add .
git status --short
git commit -m "Initial native PS5 prototype"
```

Añade tu URL real y publica:

```bash
git remote add origin https://github.com/TU_USUARIO/crisis-de-estado-ps5.git
git push -u origin main
```

`TU_USUARIO` es un marcador que debes reemplazar. Estos pasos requieren tu propia autenticación con GitHub. El paquete no contiene credenciales ni crea un repositorio automáticamente.

## GitHub Actions

`.github/workflows/tooling.yml` ejecuta las pruebas de lógica y de entrada simulada, compila la aplicación, crea una imagen exFAT, ejecuta `fsck.exfat` y verifica sus archivos. Los artefactos quedan disponibles en la ejecución de Actions.

El workflow está preparado, pero aún no se ha ejecutado en GitHub para este repositorio. Las dependencias fijadas se descargan desde fuentes públicas.

## Release v0.1.0

Después de comprobar la build en tu consola, publica una release desde GitHub → Releases → Draft a new release. Usa el tag `v0.1.0` y el título `Crisis de Estado v0.1.0 — PS5 native prototype`.

Puedes crear el tag con:

```bash
git tag -a v0.1.0 -m "Crisis de Estado v0.1.0"
git push origin v0.1.0
```

El workflow crea artefactos pero no publica releases automáticamente. Adjunta:

- `CrisisDeEstado-v0.1.exfat.zip`, con la imagen dentro.
- `PPSA99763.zip`, como alternativa por carpeta completa.
- `SHA256SUMS.txt`, con hashes de los archivos distribuidos y de la imagen descomprimida.

Copia las notas de `docs/RELEASE_NOTES-v0.1.md`. Si no has probado en consola, conserva la etiqueta **experimental / pendiente de validación en hardware**. Actualiza la matriz de `docs/TESTING.md` solo con resultados observados.

## Antes de hacer público

Revisa `git status`, licencia, créditos, Title ID y las limitaciones técnicas. No subas logs personales, direcciones de tu consola, credenciales, dumps, firmware o herramientas propietarias. El código no necesita ninguno de esos archivos para distribuirse.
