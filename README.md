# Proyecto: Campaña de Voces Digitales para Juegos y Aplicaciones

Este proyecto tiene como objetivo crear una **herramienta interactiva** que permita a los usuarios seleccionar y reproducir **voces digitales** y **sonidos** en tiempo real para ser utilizados en **juegos**, **transmisiones en vivo**, **grabaciones** o cualquier otro tipo de **entorno interactivo**. A través de una **rueda de selección**, los usuarios pueden elegir rápidamente el sonido o voz que desean, y este será reproducido a través del micrófono o el sistema de salida de audio.

---

## Descripción del Proyecto

El proyecto consiste en:

1. **Selección dinámica de sonidos y voces** mediante una **rueda de selección**.
2. **Reproducción de los sonidos** o voces seleccionadas a través de un **micrófono** o **salida de audio**.
3. **Interactividad en tiempo real** para juegos y otros contextos, proporcionando una experiencia personalizada para el usuario.

---

## Herramientas y Tecnologías Utilizadas

El proyecto fue desarrollado utilizando las siguientes tecnologías:

### 1. **Qt5**
Qt5 es un framework de desarrollo de aplicaciones multiplataforma que proporciona herramientas para la creación de interfaces gráficas de usuario (GUI). Fue utilizado para crear la **interfaz gráfica de la rueda de selección de voces** y para gestionar la interacción entre el usuario y el sistema de sonido.

- **Uso:** 
    - **Creación de la interfaz** de la rueda de voces interactiva.
    - **Manejo de eventos** y la **interacción del usuario** con los controles de la rueda de selección.
    - **Organización de la vista y los elementos gráficos**.

### 2. **C++**
C++ es un lenguaje de programación poderoso y eficiente utilizado en este proyecto para manejar la lógica del sistema, la reproducción de sonidos y el manejo de la interfaz gráfica a través de Qt5.

- **Uso:**
    - **Lógica de la rueda de selección**, permitiendo la elección de sonidos.
    - **Conexión de la interfaz gráfica con la reproducción de audio**.
    - **Control del flujo de la aplicación** y manejo de errores.

### 3. **Texto a Voz (TTS)**
El proyecto incorpora tecnología de **Texto a Voz** (TTS) para generar voces digitales a partir de texto. Esto permite la creación de voces personalizadas y la reproducción de mensajes de texto.

- **Uso:** 
    - **Generación de voces personalizadas** para el usuario.
    - **Reproducción de sonidos de forma dinámica** a partir de entradas de texto.
    - **Adaptabilidad a diferentes contextos**, como juegos o grabaciones.

### 4. **Audio de Sonido**
La parte de audio se gestiona para reproducir sonidos que pueden ser seleccionados por el usuario a través de la interfaz. Los sonidos incluyen efectos, voces, y otros tipos de audios diseñados para aplicaciones interactivas.

- **Uso:** 
    - **Reproducción de sonidos seleccionados** a través de la interfaz gráfica.
    - **Integración con el sistema de audio del sistema operativo** para emitir los sonidos a través del micrófono o salida de audio.

---

## Funcionalidades

### 1. **Rueda de Selección de Voces**
La **rueda de selección** permite a los usuarios elegir entre una serie de voces o sonidos predefinidos. Al girar la rueda, el usuario puede seleccionar la opción deseada, que será reproducida automáticamente.

### 2. **Reproducción de Sonidos en Tiempo Real**
Una vez que el usuario selecciona un sonido o voz de la rueda, el sistema lo **reproduce inmediatamente** a través de un dispositivo de salida de audio o micrófono, lo que proporciona una experiencia de interacción en tiempo real.

### 3. **Personalización de Voces**
El sistema incluye un componente de **Texto a Voz (TTS)** que permite convertir texto escrito en voz digital. Esto permite crear voces personalizadas y mensajes de texto que pueden ser escuchados o integrados en juegos y otros contextos.

---

## Cómo Ejecutar el Proyecto

### Requisitos Previos

- **Qt5** instalado.
- **C++** compilador.
- Biblioteca de **Texto a Voz (TTS)** compatible.

### Pasos para Ejecutar:

1. **Clonar el repositorio**:

   ```bash
   git clone https://github.com/TU-USUARIO/TU-REPO.git
