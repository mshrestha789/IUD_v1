# IUD_v1: Ultrasonic Structural Health Monitoring System

This project implements an embedded Structural Health Monitoring (SHM) system on the **STM32F4 Discovery** board. It utilizes a **Pitch-Catch** ultrasonic methodology to detect structural damage by analyzing signal propagation changes. The system runs on **FreeRTOS** to manage data acquisition, signal processing, and user communication in real-time.

## Overview

The "Intelligent Ultrasonic Device" (IUD) generates a tone burst signal (Pitch) and records the structural response (Catch). It compares the current signal against a baseline "intact" signal to calculate a **Damage Index (DI)**. This index quantifies the level of structural change or damage.

## Features

* **Real-time OS**: Built on **FreeRTOS** for task scheduling and resource management.
* **Pitch-Catch Operation**: 
    * **Transmission (DAC)**: Generates a pre-defined 300kHz (approx) sine wave tone burst using DAC and DMA.
    * **Reception (ADC)**: Captures the structural response using ADC3 and DMA.
* **On-board DSP**:
    * **Digital Filtering**: Implements a 4th-order IIR Low Pass Filter (Butterworth) to clean the received signal.
    * **Damage Index Calculation**: Automatically calculates the DI based on the normalized mean squared error between the current signal and the baseline.
* **Status Indication**: Visual feedback via on-board LEDs to indicate system states (Ready, Triggered, Processing, Error).
* **Serial Communication**: UART interface to trigger tests and report results to a PC or master controller using a command-based protocol.

## Hardware Requirements

* **Microcontroller**: STM32F4 Discovery Board (STM32F407VGT6).
* **Transducers**: Piezoelectric ultrasonic transmitters and receivers.
* **Connections**:
    * **DAC Out**: PA4 or PA5 (Typical for STM32 DAC Ch1/Ch2).
    * **ADC In**: PC2 (ADC3 Channel 12).
    * **UART**: USART1 (PB6/PB7).

## Software Dependencies

* **IDE**: Keil MDK-ARM (project files included).
* **Libraries**: 
    * STM32F4 DSP and Standard Peripherals Library.
    * FreeRTOS.

## System Architecture

The application is structured into independent FreeRTOS tasks:

1.  **Task Manager (`task_manager.c`)**: The central controller.
    * Initializes the pitch-catch sequence.
    * Performs the DSP (Filtering and DI calculation).
    * Manages the baseline signal recording (first 3 cycles).
2.  **Communication Task (`user_communication.c`)**:
    * **Command Parser**: Implements a line-based parser to validate device IDs and command keywords.
    * **Reporting**: Transmits the calculated Damage Index back to the master controller.
3.  **LED Task (`led_status.c`)**:
    * Blinks specific patterns to indicate the current operation phase (e.g., waiting for trigger, data transfer).

## Usage & Protocol

1.  **Initialization**: Upon reset, the system initializes peripherals and the LED task.
2.  **Baseline Calibration**: The system automatically treats the first 3 measurement cycles as the "Intact" baseline.
3.  **UART Command Protocol**:
    The system listens on USART1 for specific command strings terminated by a newline (`\n` or `\r`).
    * **Format**: `[Device ID][Command]`
    * **Device ID**: `001` (Fixed identifier for this board).
    * **Command**: `pitch` (Triggers the measurement sequence).
    * **Example Trigger**: Sending `001pitch\n` initiates the test.
4.  **Data Output**:
    * Once processing is complete, the system transmits the calculated **Damage Index** as a floating-point string ending with `\r\n`.

## Algorithm Details

The Damage Index (DI) is calculated using the following formula:

$$DI = \frac{\sum (y(t) - x(t))^2}{\sum x(t)^2}$$

Where:
* $x(t)$ is the baseline (intact) signal.
* $y(t)$ is the current (filtered) signal.

## Author

**Manish Man Shrestha**