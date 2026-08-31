# esp32-freertos-adc-averaging

An ESP32 FreeRTOS project demonstrating thread-safe ADC data processing using hardware timers, queues, and mutexes.

## Architecture Overview

* **Hardware Timer / ISR:** Triggers ADC reads and pushes data to `que1` safely using `xQueueSendFromISR()`.
* **Task A (Processing):** Consumes queue data, calculates the average of 10 readings, and safely updates the `Avg` variable using a Mutex.
* **Task B (Interface):** Listens for Serial commands. Prints the current Mutex-protected average when `"avg"` is received over the Serial monitor.
