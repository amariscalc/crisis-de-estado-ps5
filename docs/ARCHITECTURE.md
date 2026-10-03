# Arquitectura

`src/game.hpp` contiene el estado y la simulación independiente de plataforma. Un paso corresponde a 1/30 s nominal. La campaña usa cinco capítulos con tres oleadas y jefe en la tercera. El escenario es la misma plaza del Congreso; los capítulos son fases de la cumbre, no mapas de países distintos.

`src/main.cpp` dibuja las escenas y gestiona entrada DualSense. El jugador, las oleadas y el render usan arrays de tamaño fijo. Los sprites se ordenan por posición vertical. Las animaciones son cambios de posición, tamaño, orientación y efectos; el atlas contiene una pose por líder.

`src/art.hpp` lee texturas RGBA con cabecera CDEA, límite de dimensiones y comprobación de lectura completa. `src/demo_renderer.cpp` implementa VideoOut, dibujo acotado, composición alfa y un adaptador lineal para escritorio. El dibujo nativo usa el layout de teselas del renderer base.

`tools/prepare-art.py` convierte PNGs a texturas y genera un atlas tipográfico recortando cada glifo a su celda. `tools/preview.cpp` y `tools/desktop.cpp` usan el mismo código de escenas y lógica.

Los enemigos buscan latas cercanas, las consumen con 24 pasos de anticipación y reciben los mismos multiplicadores que el jugador. Monster dura 240 pasos y multiplica el daño por 1,65; Red Bull dura 180 y multiplica la velocidad por 1,55. Los buffs se acumulan por tipo y se refrescan al consumir otra lata del mismo tipo.

El runtime libre y los escritores ELF/FSELF proceden del toolchain original. El montaje y el entorno de explotación son externos al proyecto.
