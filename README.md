# Sistema de Ahorro de Energía - Documentación de Instalación

## Descripción del Sistema

Sistema embebido basado en Arduino para automatización inteligente de habitaciones que permite ahorrar energía eléctrica mediante el control automático del aire acondicionado y dispositivos eléctricos basado en la detección de ocupación.

### Funcionalidades Principales

- **Detección de ocupación**: Utiliza sensores PIR y magnéticos para determinar el estado de la habitación
- **Control de temperatura del AC**: Cambia automáticamente entre la temperatura preferida del cliente y temperatura de ahorro (27°C)
- **Control de dispositivos**: Enciende/apaga TV y energía de la habitación según el estado de ocupación
- **Comunicación RS485**: Comunicación con servidor maestro para configuración y monitoreo
- **Captura de comandos IR**: Aprende y reproduce comandos IR del control remoto del AC

---

## Tabla de Pines - Entradas (Sensores)

| Pin | Componente | Tipo | Descripción | Conexión |
|-----|------------|------|-------------|----------|
| **2** | Receptor IR | Digital (INPUT) | Recibe señales IR del control remoto del AC | Conectar al pin de datos del módulo receptor IR (ej: TSOP1738, VS1838B) |
| **9** | Sensor PIR Habitación | Digital (INPUT_PULLUP) | Detecta movimiento dentro de la habitación | Conectar señal del sensor PIR (normalmente HIGH cuando detecta movimiento) |
| **10** | Sensor PIR Entrada | Digital (INPUT_PULLUP) | Detecta movimiento en la entrada de la habitación | Conectar señal del sensor PIR de entrada |
| **11** | Sensor Magnético Puerta | Digital (INPUT_PULLUP) | Detecta si la puerta está abierta o cerrada | Conectar a sensor de contacto magnético (reed switch) |

### Notas sobre Entradas

- **Pull-up interno**: Los pines 9, 10 y 11 utilizan pull-up interno, por lo que los sensores deben estar conectados a GND cuando están activos
- **Lógica invertida**: Los sensores PIR y magnético funcionan con lógica invertida (LOW = activo, HIGH = inactivo) debido al pull-up
- **Receptor IR**: El pin 2 debe conectarse directamente al pin de salida del módulo receptor IR, sin resistencia pull-up adicional

---

## Tabla de Pines - Salidas (Actuadores)

| Pin | Componente | Tipo | Descripción | Conexión |
|-----|------------|------|-------------|----------|
| **3** | Transmisor IR | Digital (OUTPUT) | Envía señales IR para controlar el AC | Conectar al pin de datos del módulo transmisor IR (ej: LED IR con transistor) |
| **6** | Relé Energía Habitación | Digital (OUTPUT, Invertido) | Controla el suministro de energía general de la habitación | Conectar a módulo relé (lógica invertida: LOW = activo) |
| **7** | Control RS485 | Digital (OUTPUT) | Controla la dirección de transmisión del módulo RS485 | Conectar al pin DE/RE del módulo RS485 (HIGH = transmitir, LOW = recibir) |
| **8** | Relé TV | Digital (OUTPUT, Invertido) | Controla el encendido/apagado de la TV | Conectar a módulo relé (lógica invertida: LOW = activo) |
| **13** | LED Depuración | Digital (OUTPUT) | LED integrado del Arduino para depuración | LED integrado (no requiere conexión externa) |
| **A0** | LED Heartbeat | Analógico (OUTPUT) | LED que indica que el sistema está funcionando (parpadea cada 500ms) | Conectar LED con resistencia 220Ω a GND |

### Notas sobre Salidas

- **Lógica invertida**: Los pines 6 y 8 tienen lógica invertida, por lo que un valor LOW activa el relé
- **RS485**: El pin 7 controla la dirección del módulo RS485. Debe estar en LOW para recibir y HIGH para transmitir
- **Transmisor IR**: El pin 3 requiere un circuito adicional con LED IR y transistor para amplificar la señal

---

## Diagrama de Conexión Simplificado

```
                    ┌─────────────────┐
                    │   ARDUINO PRO   │
                    │   (ATMega328)   │
                    └─────────────────┘
                            │
        ┌───────────────────┼───────────────────┐
        │                   │                   │
    ┌───▼───┐          ┌───▼───┐          ┌───▼───┐
    │  Pin 2│          │  Pin 3│          │  Pin 6│
    │ IR RX │          │ IR TX │          │ Relay │
    └───┬───┘          └───┬───┘          └───┬───┘
        │                  │                  │
    ┌───▼──────────┐  ┌───▼──────────┐  ┌───▼──────────┐
    │ Receptor IR   │  │ Transmisor   │  │ Relé Energía │
    │ (TSOP1738)   │  │ IR (LED+Tr)  │  │ Habitación   │
    └──────────────┘  └──────────────┘  └──────────────┘

    ┌───▼───┐          ┌───▼───┐          ┌───▼───┐
    │  Pin 7│          │  Pin 8│          │  Pin 9│
    │ RS485 │          │ Relay │          │ PIR   │
    └───┬───┘          └───┬───┘          └───┬───┘
        │                  │                  │
    ┌───▼──────────┐  ┌───▼──────────┐  ┌───▼──────────┐
    │ Módulo RS485 │  │ Relé TV      │  │ PIR Habitación│
    │ (MAX485)     │  │              │  │              │
    └──────────────┘  └──────────────┘  └──────────────┘

    ┌───▼───┐          ┌───▼───┐          ┌───▼───┐
    │ Pin 10│          │ Pin 11│          │  A0   │
    │ PIR   │          │ Mag.  │          │ LED   │
    └───┬───┘          └───┬───┘          └───┬───┘
        │                  │                  │
    ┌───▼──────────┐  ┌───▼──────────┐  ┌───▼──────────┐
    │ PIR Entrada  │  │ Sensor       │  │ LED Heartbeat│
    │              │  │ Magnético    │  │ (220Ω)       │
    └──────────────┘  └──────────────┘  └──────────────┘
```

---

## Especificaciones Técnicas

### Comunicación

- **Protocolo**: RS485 (half-duplex)
- **Baudrate**: 9600 bps (configurable)
- **Control de dirección**: Pin 7 (DE/RE)
- **Delays RS485**: 
  - Pre-transmisión: 750μs (mínimo)
  - Post-transmisión: 1750μs (mínimo)

### Infrarrojo (IR)

- **Frecuencia portadora**: 38 kHz (configurable)
- **Receptor**: Pin 2 (interrupción hardware)
- **Transmisor**: Pin 3
- **Compresión**: Señales IR comprimidas usando codificación de nibbles para ahorrar memoria

### Alimentación

- **Voltaje**: 5V (Arduino Pro)
- **Consumo**: Depende de los módulos conectados
- **Recomendación**: Usar fuente de alimentación externa si se conectan múltiples relés

---

## Instrucciones de Instalación

### 1. Conexión de Sensores

#### Sensor PIR (Pines 9 y 10)
```
Sensor PIR          Arduino
──────────          ────────
VCC      ────────── 5V
GND      ────────── GND
OUT      ────────── Pin 9/10
```

#### Sensor Magnético de Puerta (Pin 11)
```
Sensor Magnético    Arduino
───────────────     ────────
Terminal 1    ───── Pin 11
Terminal 2    ───── GND
```

#### Receptor IR (Pin 2)
```
Receptor IR         Arduino
────────────        ────────
VCC      ────────── 5V
GND      ────────── GND
OUT      ────────── Pin 2
```

### 2. Conexión de Actuadores

#### Transmisor IR (Pin 3)
```
Arduino             Circuito IR
───────             ──────────
Pin 3    ────────── Base del Transistor (NPN)
                    │
                    ├── Colector ──── LED IR (cátodo)
                    │
                    └── Emisor ────── GND
                    
5V       ────────── Ánodo LED IR (con resistencia 100Ω)
```

#### Módulos Relé (Pines 6 y 8)
```
Arduino             Módulo Relé
───────             ───────────
Pin 6/8  ────────── IN (con lógica invertida)
GND      ────────── GND
5V       ────────── VCC (si el módulo requiere 5V)
                    │
                    └─── COM, NO, NC ──── Carga (TV/Energía)
```

#### Módulo RS485 (Pin 7)
```
Arduino             MAX485
───────             ──────
Pin 7    ────────── DE/RE
Pin 0 (RX) ──────── RO
Pin 1 (TX) ──────── DI
5V       ────────── VCC
GND      ────────── GND
                    │
                    └─── A, B ──── Bus RS485
```

### 3. LED de Heartbeat (Pin A0)
```
Arduino             LED
───────             ───
A0       ────────── Ánodo LED (con resistencia 220Ω)
GND      ────────── Cátodo LED
```

---

## Configuración de Pines

Los pines pueden ser modificados editando el archivo `external_configuration.h`:

```cpp
#define EXTERNAL_RS485_PIN 7
#define EXTERNAL_ROOM_POWER_PIN 6
#define EXTERNAL_TV_POWER_PIN 8
#define EXTERNAL_ROOM_PIR_PIN 9
#define EXTERNAL_ENTRANCE_PIR_PIN 10
#define EXTERNAL_DOOR_MAGNETIC_PIN 11
#define EXTERNAL_IR_RECV_PIN 2
#define EXTERNAL_IR_SEND_PIN 3
```

**Nota**: Después de modificar los pines, es necesario recompilar y cargar el firmware.

---

## Pines Adicionales (Opcionales/Depuración)

| Pin | Uso | Estado |
|-----|-----|--------|
| **13** | LED integrado de depuración | Activo |
| **A0** | LED heartbeat del sistema | Activo |
| **A1, A2, A3** | LEDs de estado de la máquina de estados | Comentado (no utilizado) |

---

## Consideraciones de Seguridad

1. **Aislamiento**: Los módulos relé proporcionan aislamiento óptico entre el Arduino y las cargas de alto voltaje
2. **Fusibles**: Instalar fusibles apropiados en las líneas de alimentación de los relés
3. **Puesta a tierra**: Asegurar una correcta puesta a tierra del sistema
4. **RS485**: Usar cables trenzados y terminadores de 120Ω en los extremos del bus RS485
5. **IR**: Verificar que el LED IR tenga la potencia adecuada y esté orientado correctamente hacia el AC

---

## Solución de Problemas

### El receptor IR no detecta comandos
- Verificar que el pin 2 esté correctamente conectado
- Comprobar la alimentación del módulo receptor IR (5V)
- Verificar que el módulo receptor sea compatible con 38 kHz

### Los relés no se activan
- Recordar que los pines 6 y 8 tienen lógica invertida (LOW = activo)
- Verificar la alimentación del módulo relé
- Comprobar las conexiones de los relés

### Problemas de comunicación RS485
- Verificar que el pin 7 esté correctamente conectado al módulo MAX485
- Comprobar la configuración de baudrate (9600 por defecto)
- Verificar la polaridad del bus RS485 (A y B)
- Asegurar que haya terminadores de 120Ω en los extremos del bus

### Los sensores PIR no funcionan
- Recordar que usan pull-up interno (LOW = activo)
- Verificar la alimentación de los sensores (5V)
- Ajustar la sensibilidad y tiempo de retardo del sensor PIR si es posible

---

## Referencias

- **Arduino Pro**: Basado en ATmega328P
- **Protocolo**: RS485 half-duplex con CRC-16
- **IR**: Protocolo personalizado con compresión de señales
- **Máquina de Estados**: 3 estados (Empty, MaybeOccupied, Occupied)

---

## Autor

Sistema desarrollado para automatización y ahorro de energía en habitaciones inteligentes.

**Versión**: 1.0  
**Última actualización**: 2024
