# ESP32 Educational Internet Radio / Radio Internet Educațional cu ESP32

![Project Status](https://img.shields.io/badge/Status-Educational_Prototype-blue) ![Platform](https://img.shields.io/badge/Platform-ESP32-green) ![License](https://img.shields.io/badge/License-MIT-yellow)

---

## 🇬🇧 English Description

### 🎓 Educational Purpose
This project is primarily an **educational tool** designed to explore and understand the fundamentals of:
- **Digital to Analog Converters (DAC)** and hardware audio decoding.
- **Audio Streaming** over WiFi using microcontrollers.
- **Hardware Integration** (SPI, I2S, I2C buses).

**Disclaimer**: This is a learning prototype and **not a commercial product**. It demonstrates the logic and synchronization required to build a functional internet radio from scratch.

### 🛠 Hardware Components
The project uses specific components chosen for their learning value and availability:

1.  **ESP32 Development Board (30-pin)**:
    - The core controller handling WiFi, logic, and component coordination.
    - Chosen for its versatility and dual-core processing power.

2.  **VS1053 MP3 Decoder Module**:
    - **Function**: Handles the heavy lifting of MP3/AAC decoding.
    - **Reason**: We used a VS1053 module we had lying around to learn how hardware decoding works compared to software decoding. It ensures reliable audio output without overtaxing the ESP32.

3.  **INMP441 I2S Microphone**:
    - **Function**: Digital microphone input.
    - **Reason**: Added to the hardware design to support future phases of the project, such as Voice Control or AI integration. Currently installed but the software logic focuses on radio streaming.

4.  **SSD1306 OLED Display (0.96")**:
    - **Function**: Provides real-time visual feedback (Station Name, IP Address, Signal Strength).
    - **Reason**: To learn I2C communication and display driver implementation.

### 💻 Software Architecture
The code is built to demonstrate several key concepts:

-   **Web & Control Interface**:
    -   A responsive **Web Interface** (hosted on the ESP32) allows users to change stations, adjust volume (Bass/Treble), and manage WiFi settings.
    -   The interface uses modern CSS/JS but is lightweight enough to run entirely from the microcontroller's memory.

-   **Streaming Logic**:
    -   Uses a **Circular Buffer** to manage the data flow from the internet to the VS1053 decoder, preventing stuttering.
    -   Implements robust reconnection logic for both WiFi and Audio Streams.

-   **Synchronization**:
    -   **Bi-directional Sync**: Commands sent from a phone (Web UI) are immediately reflected on the physical OLED display and vice-versa.

---

## 🇷🇴 Descriere în Română

### 🎓 Scop Educațional
Acest proiect este un **montaj educațional** creat cu scopul de a învăța și experimenta:
-   Funcționarea **DAC-urilor (Digital to Analog Converter)** și a decodării hardware.
-   Principiile **Streaming-ului Audio** pe microcontrollere prin WiFi.
-   Integrarea diverselor protocoale hardware (SPI, I2S, I2C).

**Notă Importantă**: Acesta este un prototip pentru învățare ("o jucărie tehnică") și **nu este un produs comercial**. Scopul principal este înțelegerea fenomenelor din spate, nu perfecțiunea comercială.

### 🛠 Componente Hardware
Am folosit componente pe care le aveam la îndemână sau care ofereau cea mai bună oportunitate de învățare:

1.  **Placă de Dezvoltare ESP32 (model 30-pini)**:
    -   "Creierul" proiectului. Gestionează conexiunea WiFi, serverul web și coordonează toate celelalte module.

2.  **Modul Decodor MP3 VS1053**:
    -   **Funcție**: Se ocupă de decodarea efectivă a fluxului audio (MP3/AAC).
    -   **De ce?**: Am folosit acest modul (recomandat pentru începători și nu numai) pentru a învăța diferența dintre decodarea hardware și cea software. Oferă un sunet clar și eliberează procesorul ESP32 de sarcini grele.

3.  **Microfon I2S INMP441**:
    -   **Funcție**: Intrare audio digitală.
    -   **De ce?**: A fost integrat în montaj pentru a asigura continuitatea proiectului pe viitor (de exemplu, pentru adăugarea comenzilor vocale sau funcții AI). Momentan este montat hardware ca bază pentru etapele următoare.

4.  **Ecran OLED SSD1306 (0.96")**:
    -   **Funcție**: Afișează vizual starea aparatului (Postul curent, IP-ul, Semnalul WiFi).
    -   **De ce?**: Pentru a exersa comunicarea I2C și afișarea datelor în timp real.

### 💻 Arhitectură Software
Codul este structurat pentru a explica câteva concepte de bază:

-   **Interfața Web**:
    -   Proiectul găzduiește propriul site web pe ESP32. De aici poți controla volumul, schimba posturile și regla sunetul (Bass/Treble).
    -   Interfața este modernă, dar optimizată să ruleze cu resurse minime.

-   **Logica de Stream**:
    -   Folosește un **Buffer Circular** pentru a stoca temporar datele de pe internet înainte de a le trimite la cipul VS1053, asigurând o audiție fără întreruperi.

-   **Sincronizare**:
    -   Orice comandă dată de pe telefon (prin interfața web) este afișată instantaneu și pe ecranul OLED al aparatului. Sistemul este gândit să fie reactiv și transparent.

---

## 🚀 How to Flash / Cum se instalează
1.  Open the project in **Arduino IDE**.
2.  Install required libraries (Adafruit GFX, Adafruit SSD1306, ESP_VS1053_Library, ArduinoJson).
3.  Select board: **DOIT ESP32 DEVKIT V1**.
4.  Upload Data Sketch (SPIFFS) to save the web interface files.
5.  Upload the code.

---
*Created by [Your Name/Handle] for educational purposes.*
