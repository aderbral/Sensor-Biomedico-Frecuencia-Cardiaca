/*******************************************************************************************
 * TESIS: Propuesta de sensor biomédico y monitoreo de la frecuencia cardíaca 
 *        para pacientes de los centros de rehabilitación
 *
 * AUTOR: Adrián Bruno
 * CARRERA: Ingeniería Mecatrónica
 * INSTITUCIÓN: Universidad Privada del Norte
 * AÑO: 2025
 *
 * PROYECTO: Sensor biomédico para el monitoreo de la frecuencia cardíaca en tiempo real
 * MICROCONTROLADOR: ESP32 (DevKit V1)
 * SENSOR: MAX30105 (PPG infrarrojo)
 * DISPLAY: OLED 128x64 (SSD1306, I2C)
 *
 * OBJETIVO:
 * - Medir y visualizar la frecuencia cardíaca (BPM) de pacientes en tiempo real.
 * - Aplicar un filtro digital Biquad IIR pasa banda (0.5–4 Hz) para reducir ruido y artefactos.
 *
 * PRINCIPIO DE FUNCIONAMIENTO:
 * Basado en la fotopletismografía (PPG), el sensor detecta variaciones en el volumen sanguíneo
 * mediante luz infrarroja reflejada en la piel. El filtrado Biquad IIR elimina el ruido de baja
 * frecuencia (movimiento) y las interferencias de alta frecuencia, conservando el rango cardíaco
 * (0.5–4 Hz ≈ 30–240 BPM).
 *
 * LIBRERÍAS NECESARIAS:
 * - SparkFun MAX3010x Pulse Oximeter Library
 * - Adafruit GFX Library
 * - Adafruit SSD1306 Library
 *
 * CONEXIONES:
 * MAX30102  -> ESP32
 *   VIN     -> 3.3V
 *   GND     -> GND
 *   SCL     -> GPIO 22
 *   SDA     -> GPIO 21
 *
 * OLED 128x64 -> ESP32
 *   VDD     -> 3.3V
 *   GND     -> GND
 *   SCL     -> GPIO 22
 *   SDA     -> GPIO 21
 *
 * NOTA:
 * Este código forma parte del desarrollo experimental descrito en la tesis aplicada con enfoque 
 * cuantitativo y diseño no experimental, cuyo propósito es proponer un sensor biomédico para el 
 * monitoreo preciso de la frecuencia cardíaca en centros de rehabilitación.
 *******************************************************************************************/

#include <Wire.h>
#include "MAX30105.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

// CONFIGURACIÓN OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// CONFIGURACIÓN SENSOR
MAX30105 sensor;
float avgBpm = 0;
bool peakDetected = false;

// BITMAP DEL CORAZÓN (16x16)
static const unsigned char PROGMEM heart_bmp[] = {
  0b00001100, 0b00110000,
  0b00011110, 0b01111000,
  0b00111111, 0b11111100,
  0b01111111, 0b11111110,
  0b01111111, 0b11111110,
  0b01111111, 0b11111110,
  0b00111111, 0b11111100,
  0b00011111, 0b11111000,
  0b00001111, 0b11110000,
  0b00000111, 0b11100000,
  0b00000011, 0b11000000,
  0b00000001, 0b10000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
};

// Variables para el corazón latiendo
bool heartVisible = true;
unsigned long lastHeartToggle = 0;

// CLASE FILTRO PASA BANDA BIQUAD (0.5–4 Hz)
class BiquadBP {
public:
  float a0, a1, a2, b0, b1, b2;
  float x1 = 0, x2 = 0, y1 = 0, y2 = 0;

  void setup(float fs, float f1, float f2) {
    float fc = (f1 + f2) / 2.0f;
    float bw = (f2 - f1);
    float Q = fc / bw;
    float w0 = 2.0f * PI * fc / fs;
    float cosw0 = cos(w0);
    float sinw0 = sin(w0);
    float alpha = sinw0 / (2.0f * Q);

    b0 = alpha;
    b1 = 0.0f;
    b2 = -alpha;
    a0 = 1.0f + alpha;
    a1 = -2.0f * cosw0;
    a2 = 1.0f - alpha;

    b0 /= a0; b1 /= a0; b2 /= a0;
    a1 /= a0; a2 /= a0;
  }

  float apply(float x) {
    float y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
    x2 = x1; x1 = x;
    y2 = y1; y1 = y;
    return y;
  }
};

BiquadBP filtro;

// VARIABLES DE PROCESAMIENTO
#define FS 100.0
#define ECG_POINTS 64
uint8_t ecgData[ECG_POINTS];
int ecgIndex = 0;

#define WINDOW 15
float irBuffer[WINDOW];
int bufIndex = 0;

#define SMOOTH_POINTS 3
float smoothBuffer[SMOOTH_POINTS];
int smoothIndex = 0;

float thresholdOn = 0, thresholdOff = 0;
float minV = 1e9, maxV = -1e9;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Error: OLED no detectado");
    while (1);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(10, 28);
  display.println("Iniciando sensor...");
  display.display();

  // SENSOR MAX30105
  if (!sensor.begin(Wire, I2C_SPEED_STANDARD)) {
    display.clearDisplay();
    display.setCursor(20, 28);
    display.println("Sensor no detectado");
    display.display();
    while (1);
  }

  // CONFIGURACIÓN DEL SENSOR
  sensor.setup(0x3F, 4, 2, 100, 411, 16384);
  sensor.setPulseAmplitudeIR(0x3F);
  sensor.setPulseAmplitudeRed(0x3F);

  // FILTRO PASA BANDA CONFIGURADO
  filtro.setup(FS, 0.5, 4.0);

  display.clearDisplay();
  display.setCursor(25, 28);
  display.println("Sensor listo");
  display.display();
  delay(800);
}

void loop() {
  // ADQUISICIÓN DE SEÑAL IR
  long irValue = sensor.getIR();
  float filtered = filtro.apply((float)irValue);
  filtered *= 10.0; // Escalado

  // SUAVIZADO PROMEDIO MÓVIL (3 muestras)
  smoothBuffer[smoothIndex] = filtered;
  smoothIndex = (smoothIndex + 1) % SMOOTH_POINTS;
  float smoothSum = 0;
  for (int i = 0; i < SMOOTH_POINTS; i++) smoothSum += smoothBuffer[i];
  filtered = smoothSum / SMOOTH_POINTS;

  // MEDIA Y DESVIACIÓN ESTÁNDAR (VENTANA DESLIZANTE)
  irBuffer[bufIndex] = filtered;
  bufIndex = (bufIndex + 1) % WINDOW;
  float mean = 0, stddev = 0;
  for (int i = 0; i < WINDOW; i++) mean += irBuffer[i];
  mean /= WINDOW;
  for (int i = 0; i < WINDOW; i++) stddev += pow(irBuffer[i] - mean, 2);
  stddev = sqrt(stddev / WINDOW);

  // UMBRAL ADAPTATIVO MEJORADO
  thresholdOn  = mean + 1.5 * stddev;
  thresholdOff = mean + 0.8 * stddev;

  // DETECCIÓN DE LATIDOS
  bool sinDedo = (irValue < 40000);
  static unsigned long lastPeakTime = millis();

  if (!sinDedo) {
    if (!peakDetected && filtered > thresholdOn && (millis() - lastPeakTime) > 600) {
      peakDetected = true;
      unsigned long now = millis();
      unsigned long delta = now - lastPeakTime;
      lastPeakTime = now;
      float instBpm = 60.0 / (delta / 1000.0);
      // Validar rango fisiológico
      if (instBpm > 50 && instBpm < 120)
        avgBpm = (avgBpm * 0.8f) + (instBpm * 0.2f);
    }
    if (peakDetected && filtered < thresholdOff) peakDetected = false;
  } else {
    avgBpm = 0;
  }

  // ACTUALIZACIÓN DE ONDA ECG MINI
  ecgData[ecgIndex] = map(constrain(filtered, mean - 1500, mean + 1500),
                          mean - 1500, mean + 1500, 15, 25);
  ecgIndex = (ecgIndex + 1) % ECG_POINTS;

  // EVALUACIÓN DE ESTADO
  String estado = "Esperando";
  if (!sinDedo) {
    if (stddev < 200) estado = "Estable";
    else if (stddev < 800) estado = "Leyendo";
    else estado = "Inestable";
  }

  // ACTUALIZACIÓN OLED
  display.clearDisplay();

  // CORAZÓN EN ESQUINA SUPERIOR IZQUIERDA
  int heartX = 0, heartY = 0;
  if (avgBpm > 0) {
    if (millis() - lastHeartToggle >= 500) {
      heartVisible = !heartVisible;
      lastHeartToggle = millis();
    }
    if (heartVisible)
      display.drawBitmap(heartX, heartY, heart_bmp, 16, 16, SSD1306_WHITE);
  } else {
    display.drawBitmap(heartX, heartY, heart_bmp, 16, 16, SSD1306_WHITE);
  }

  // ICONO BATERÍA
  display.drawRect(118, 1, 8, 5, SSD1306_WHITE);
  display.drawRect(126, 2, 2, 3, SSD1306_WHITE);
  display.fillRect(119, 2, 6, 3, SSD1306_WHITE);

  // TEXTO BPM CENTRADO
  display.setTextSize(3);
  String numText = String((int)avgBpm);
  String bpmLabel = "BPM";
  int16_t x1, y1; uint16_t w1, h1;
  int16_t x2, y2; uint16_t w2, h2;
  display.getTextBounds(numText, 0, 0, &x1, &y1, &w1, &h1);
  display.getTextBounds(bpmLabel, 0, 0, &x2, &y2, &w2, &h2);
  int spacing = 5;
  int totalWidth = w1 + spacing + w2;
  int bpmX = (SCREEN_WIDTH - totalWidth) / 2;
  int bpmY = 18;
  display.setCursor(bpmX, bpmY);
  display.print(numText);
  display.setCursor(bpmX + w1 + spacing, bpmY);
  display.print(bpmLabel);

  // ONDA ECG MINI
  for (int i = 0; i < ECG_POINTS - 1; i++) {
    int x1 = (i * 1) + 5;
    int y1 = 63 - ecgData[(ecgIndex + i) % ECG_POINTS];
    int x2 = ((i + 1) * 1) + 5;
    int y2 = 63 - ecgData[(ecgIndex + i + 1) % ECG_POINTS];
    if (x2 < SCREEN_WIDTH / 2)
      display.drawLine(x1, y1 + 10, x2, y2 + 10, SSD1306_WHITE);
  }

  // INDICADOR DE ESTADO
  display.setTextSize(1);
  int16_t x, y; uint16_t w, h;
  display.getTextBounds(estado, 0, 0, &x, &y, &w, &h);
  display.setCursor((SCREEN_WIDTH - w - 6), 55);
  display.print(estado);

  display.display();

  // SALIDA SERIAL
  Serial.print("IR="); Serial.print(irValue);
  Serial.print("\tFilt="); Serial.print(filtered, 1);
  Serial.print("\tMean="); Serial.print(mean, 1);
  Serial.print("\tThrOn="); Serial.print(thresholdOn, 1);
  Serial.print("\tBPM="); Serial.println(avgBpm);

  delay(10); // Frecuencia de muestreo de 100 Hz
}
