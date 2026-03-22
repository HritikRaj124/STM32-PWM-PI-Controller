# STM32-PWM-PI-Controller
Bare-metal closed-loop PWM PI speed controller for STM32F103 Cortex-M3 in C

# STM32 Bare-Metal PWM PI Controller

A closed-loop PI speed controller implemented in bare-metal C 
targeting STM32F103 Cortex-M3. Developed and validated using 
Renode hardware simulation.

## What This Project Demonstrates

- Register-level peripheral configuration in bare-metal C
- PWM generation via STM32 Timer 1 (1kHz, variable duty cycle)
- UART communication driver for real-time signal logging
- PI controller with anti-windup for closed-loop speed control
- Hardware simulation and validation using Renode

## System Architecture
```
┌─────────────────────────────────────────┐
│           PI Controller                 │
│   error = TARGET - measured_speed       │
│   integral += error (anti-windup)       │
│   output = KP×error + KI×integral      │
└──────────────┬──────────────────────────┘
               │
               ▼
┌─────────────────────────────────────────┐
│         PWM Driver (Timer 1)            │
│   PSC = 72-1  → 1MHz timer clock       │
│   ARR = 999   → 1kHz PWM frequency     │
│   CCR1 = duty → 0-100% duty cycle      │
└──────────────┬──────────────────────────┘
               │
               ▼
┌─────────────────────────────────────────┐
│      STM32F103 Cortex-M3 (Renode)       │
│   72MHz, bare-metal, no RTOS            │
└─────────────────────────────────────────┘
```

## Key Technical Details

| Parameter | Value |
|---|---|
| Target MCU | STM32F103C8 (Cortex-M3) |
| Clock Speed | 72MHz |
| PWM Frequency | 1kHz |
| PWM Resolution | 0-1000 (0.1% steps) |
| UART Baud Rate | 9600 |
| Controller Type | PI with anti-windup |
| Binary Size | 1420 bytes |
| Warnings | 0 |

## Register-Level Implementation

All peripherals configured directly via memory-mapped registers:
```c
// Enable Timer 1 clock
RCC_APB2ENR |= (1 << 11);

// Configure PWM frequency: 72MHz / 72 / 1000 = 1kHz
TIM1_PSC = 72 - 1;   // Prescaler: divide by 72 → 1MHz
TIM1_ARR = 1000 - 1; // Period: 1000 ticks → 1kHz

// Set duty cycle (0-1000 = 0-100%)
TIM1_CCR1 = duty;
```

## PI Controller
```c
// Calculate error
int32_t error = TARGET_SPEED - measured_speed;

// Accumulate integral with anti-windup
integral += error;
if(integral >  500) integral =  500;
if(integral < -500) integral = -500;

// Compute output
int32_t output = (KP * error) + (KI * integral);
```

## Simulation Results

Validated in Renode STM32 hardware simulation:
- Speed converges from 0 to target across multiple operating points
- Steady state error eliminated by integral term
- Anti-windup prevents integral saturation
- Real-time UART logging confirms control loop behavior

![UART Output](docs/results.png)

## Tools Used

| Tool | Purpose |
|---|---|
| STM32CubeIDE | Development environment |
| ARM GNU Toolchain 15.2 | C compiler for Cortex-M3 |
| Renode | STM32 hardware simulation |
| Git | Version control |

## How to Run

1. Clone this repository
2. Open STM32CubeIDE → Import existing project
3. Build with Ctrl+B
4. Open Renode and run:
```
include @renode/run.resc
```
5. Observe UART output in analyzer window

## Author

Hritik Raj — Master's Student, International Automotive Engineering  
Technische Hochschule Ingolstadt
```

---

## Step 6 — Push to GitHub

In Command Prompt:
```
cd C:\projects\STM32-PWM-PI-Controller

git init
git add .
git commit -m "Initial commit: bare-metal PWM PI controller for STM32F103"
git branch -M main
git remote add origin https://github.com/YOURUSERNAME/STM32-PWM-PI-Controller.git
git push -u origin main
```

Replace `YOURUSERNAME` with your actual GitHub username.

---

## Step 7 — Add your screenshot

Take a screenshot of your Renode UART output right now:
- Press **Windows + Shift + S**
- Save as `results.png`
- Copy to `C:\projects\STM32-PWM-PI-Controller\docs\results.png`
- Then:
```
git add docs/results.png
git commit -m "Add simulation results screenshot"
git push
