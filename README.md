# 🫀 Sensor Biomédico – Monitoreo de Frecuencia Cardíaca

<img src="https://i.postimg.cc/bJPCmMzj/01.png" title="TESIS">

### 🎓 *“Influencia de un sensor biomédico en el monitoreo de la frecuencia cardiaca de pacientes en un centro de rehabilitación de Lima, 2025.”*  
**Autor:** Adrián Bruno  

# DESCRIPCIÓN

Este repositorio contiene el **diseño, implementación y validación del sistema biomédico** para el monitoreo de la **frecuencia cardíaca en tiempo real** mediante **fotopletismografía (PPG)**.

El sistema está basado en los módulos:

- **MAX30105** – Sensor óptico PPG (IR + Red LED)  
- **ESP32 DevKit V1** – Microcontrolador con WiFi/Bluetooth  
- **OLED SSD1306 128x64** – Pantalla para visualización  
- **Filtro digital Biquad IIR pasa banda (0.5–4 Hz)**  
- **Diseño 3D del contenedor en Fusion 360**

Este proyecto busca mejorar el **control y la precisión del monitoreo cardíaco** en entornos de rehabilitación.

# BADGES

![Arduino](https://img.shields.io/badge/Arduino-IDE-blue)
![ESP32](https://img.shields.io/badge/ESP32-DevKitV1-orange)
![PPG](https://img.shields.io/badge/PPG-Fotopletismografía-red)
![License: MIT](https://img.shields.io/badge/License-MIT-green)
![Status](https://img.shields.io/badge/Project-Active-brightgreen)

# TABLA DE CONTENIDOS
1. [Características](#-características-del-proyecto)
2. [Arquitectura](#-arquitectura-del-sistema)
3. [Requisitos](#-requisitos-de-instalación)
4. [Instalación](#-instalación)
5. [Diagrama y Conexión](#-diagrama-y-Conexión)
6. [Diseño 3D](#-diseño-3d)
7. [Citar Proyecto](#-citar-Proyecto)
8. [Licencia](#-licencia)

# CARACTERÍSTICAS

- Lectura de frecuencia cardíaca basada en **PPG infrarrojo**
- Filtrado de ruido por movimiento con **Biquad IIR**
- Visualización en tiempo real en pantalla OLED
- Diseño propio de contenedor en **Fusion 360**
- Repositorio con código, análisis y archivos técnicos
- Arquitectura basada en **sistemas embebidos**
- Compatible con **Arduino IDE 2.0+**

# ARQUITECTURA

Sensor MAX30105 → PPG cruda → Filtro Biquad IIR (0.5–4 Hz) → Procesamiento en ESP32 → Visualización OLED 128x64 → Transmisión y análisis

# REQUISITOS DE INSTALACIÓN

### Software:
- Arduino IDE 2.0+
- Librerías:
  - SparkFun MAX3010x
  - Adafruit SSD1306
  - Adafruit GFX
- MATLAB (opcional)

### Hardware:
- ESP32 DevKit V1
- MAX30105
- Pantalla OLED 128x64
- Cables Dupont
- Contenedor 3D (Fusion 360)

# INSTALACIÓN

<pre><i><n>git clone https://github.com/aderbral/Sensor-Biomedico-Frecuencia-Cardiaca.git
</pre></i></n>

# DIAGRAMA Y CONEXIÓN

<img src="https://i.postimg.cc/26zyF4Kz/Imagen1.png" title="DIAGRAMA">

| Módulo / Sensor           | Pin | ESP32   |
| ------------------------- | --- | ------- |
| **MAX30105**              | VIN | 3.3V    |
| MAX30105                  | GND | GND     |
| MAX30105                  | SCL | GPIO 22 |
| MAX30105                  | SDA | GPIO 21 |
| **OLED 128x64 (SSD1306)** | VDD | 3.3V    |
| OLED 128x64               | GND | GND     |
| OLED 128x64               | SCL | GPIO 22 |
| OLED 128x64               | SDA | GPIO 21 |

# DISEÑO 3D

<img src="https://i.postimg.cc/DZjvrRs7/01-(1).png" title="DISEÑO">

# CITAR PROYECTO

APA:

Bruno, A. (2025). Influencia de un sensor biomédico en el monitoreo de la frecuencia cardiaca de pacientes en un centro de rehabilitación de Lima, 2025. GitHub. https://github.com/aderbral/Sensor-Biomedico-Frecuencia-Cardiaca

IEEE:

A. Bruno, “Influencia de un sensor biomédico en el monitoreo de la frecuencia cardiaca…”, 2025.

# LICENCIA
Doxing is licensed. 
See [LICENSE](https://github.com/AdrianoBrunoHackingEtico/Hacking-tico/blob/main/LICENSE) for more information.

© 2025 Adrián Bruno
