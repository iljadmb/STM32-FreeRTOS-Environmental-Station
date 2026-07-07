# STM32 FreeRTOS Environmental Monitoring Station

Real-time multi-sensor environmental station on an **STM32F446RE** (ARM Cortex-M4).
It reads acceleration, angular rate, temperature and air pressure from two I2C sensors,
processes the data in separate **FreeRTOS** tasks, and visualizes it live on an OLED
display — including a scrolling temperature graph. **All sensor and display drivers were
written from scratch directly from the datasheets** (no ready-made libraries), and the
firmware is cleanly separated into reusable modules.

<img width="4032" height="2268" alt="IMG_6424" src="https://github.com/user-attachments/assets/525ab418-4be0-4341-9dc5-bf8fb8c38ef0" />

---

## Features

- **Two I2C sensors:** MPU-9250 (accelerometer + gyroscope) and BMP280 (temperature + pressure)
- **Concurrent FreeRTOS architecture:** three tasks, a message queue and a mutex
- **Custom bare-metal drivers** for both sensors and the SH1106 OLED — implemented from the datasheets
- **Live OLED visualization** with a scrolling temperature graph (Bresenham line drawing)
- **Clean, modular firmware:** each peripheral in its own reusable driver module

---

## System Architecture

Instead of a single super-loop, the application is split into three independent tasks that
the scheduler runs concurrently. Two producer tasks each read one sensor and send the
readings through a queue to the display task (consumer). A mutex protects the shared I2C bus.

```
    +-------------+          +-------------+
    |  ImuTask    |          |  EnvTask    |
    |  MPU-9250   |          |  BMP280     |
    +------+------+          +------+------+
           | source = 0            | source = 1
           +----------+------------+
                      v
              [   sensorQueue   ]   FIFO, 2 producers
                      |
                      v
              +---------------+
              |  DisplayTask  | ---> SH1106 OLED (text + graph)
              +---------------+

    I2CMutex  ->  every I2C access runs exclusively over the shared bus
```

### Design decisions

- **Mutex for the I2C bus.** Three devices share one physical bus. Without mutual exclusion,
  two tasks transacting at the same time would interleave and corrupt the bus. The mutex
  (with priority inheritance) guarantees exclusive access — and is held *only* around the
  actual I2C calls, never around computation or drawing into the RAM framebuffer.
- **Queue instead of shared variables.** The queue passes readings by value and decouples
  sampling (100 ms / 1 s) from display refresh. The display task blocks for free until data
  arrives — no polling. A `source` tag cleanly separates the two producers.
- **Modular, RTOS-agnostic drivers.** Each driver receives the I2C handle as a parameter
  (dependency injection), so it is reusable and testable. Concurrency (the mutex) is handled
  only in the application layer.

---

## Hardware

| Component | Function | Interface / Address |
|---|---|---|
| STM32F446RE Nucleo | Microcontroller (Cortex-M4, 180 MHz) | — |
| MPU-9250 | IMU: accelerometer + gyroscope | I2C, `0x68` |
| BMP280 | temperature + air pressure | I2C, `0x76` |
| SH1106 1.3" OLED | display, 128×64, monochrome | I2C, `0x3C` |
| Shared bus | I2C1 @ 100 kHz (PB8 SCL / PB9 SDA) | 3 devices on one bus |

---

## Project Structure

Hand-written code is marked; everything else is ST/CubeMX-generated or third-party
(kept in the repo so the project builds out of the box).

```text
.
├── Core/
│   ├── Inc/
│   │   ├── imu.h      # MPU-9250 driver              (own)
│   │   ├── bmp.h      # BMP280 driver                (own)
│   │   ├── sh1106.h   # OLED driver                  (own)
│   │   └── fonts.h    # 5x7 font table               (own)
│   └── Src/
│       ├── imu.c      # MPU-9250 driver              (own)
│       ├── bmp.c      # BMP280 driver + compensation (own)
│       ├── sh1106.c   # OLED framebuffer driver      (own)
│       └── main.c     # app + FreeRTOS tasks         (own)
├── Drivers/           # ST HAL + CMSIS               (generated)
├── Middlewares/       # FreeRTOS kernel              (third-party)
├── EnvStationV1.ioc   # STM32CubeMX configuration
└── STM32F446RETX_FLASH.ld   # linker script
```

**The code worth reading is in `Core/Src/imu.c`, `bmp.c`, `sh1106.c` and the task
functions in `main.c`.**

---

## Building & Flashing

1. Install **STM32CubeIDE**.
2. Clone this repository and open the project via **File → Open Projects from File System…**
   (or open `EnvStationV1.ioc` in STM32CubeMX to inspect/regenerate the configuration).
3. Enable **"Use float with printf"** (Project → Properties → C/C++ Build → Settings →
   MCU Settings) — required for the `%f` formatting on the OLED.
4. Build (hammer icon) and flash to the Nucleo (Run).
5. Optional: connect a serial terminal (USART2, 115200 baud) for debug output.

**Wiring:** all three devices share I2C1 (SCL → PB8, SDA → PB9, 3V3, GND).
On the BMP280 breakout tie **CSB → VCC** (I2C mode) and **SDO → GND** (address `0x76`).

---

## Key Technical Highlights

- **I2C at register level:** master transmit/receive, repeated start, auto-increment burst reads
- **MPU-9250:** 14-byte burst from `0x3B`, 16-bit big-endian assembly, scaling and tilt via `atan2f`
- **BMP280:** per-chip calibration coefficients (little-endian, signed/unsigned), forced mode,
  and the 64-bit Bosch compensation formulas (temperature sets `t_fine` for pressure)
- **SH1106:** custom framebuffer driver, page-based flushing with a 2-pixel column offset,
  font rendering, Bresenham lines and a scrolling live graph
- **Debugging:** isolated a defective sensor with an I2C bus scan; traced calibration
  byte-order and variable-shadowing bugs with the debugger

---

## What I learned

FreeRTOS (tasks, scheduler, priorities, queues, mutexes with priority inheritance),
STM32 HAL & I2C, reading and implementing directly from datasheets, fixed-point sensor
compensation, and structuring embedded firmware into clean, reusable modules.
