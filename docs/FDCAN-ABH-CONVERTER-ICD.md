# FDCAN-ABH Converter Interface Control Document (ICD)

**Document Number:** FDCAN-ABH-ICD-001
**Revision:** 1.0
**Date:** November 17, 2025
**Organization:** Psyonic Inc.

---

## Document Control

| Version | Date | Author | Description |
|---------|------|--------|-------------|
| 1.0 | 2025-11-17 | Auto-generated | Initial release |

---

## Table of Contents

1. [Scope](#1-scope)
2. [Referenced Documents](#2-referenced-documents)
3. [Interface Overview](#3-interface-overview)
4. [Interface Characteristics](#4-interface-characteristics)
5. [Memory Map and Register Definitions](#5-memory-map-and-register-definitions)
6. [Operational Modes](#6-operational-modes)
7. [Command Sequences](#7-command-sequences)
8. [Timing and Performance Characteristics](#8-timing-and-performance-characteristics)
9. [Non-Volatile Configuration Management](#9-non-volatile-configuration-management)
10. [Physical and Electrical Characteristics](#10-physical-and-electrical-characteristics)
11. [Appendices](#11-appendices)

---

## 1. Scope

### 1.1 Identification

This Interface Control Document (ICD) defines the electrical, functional, and protocol interfaces for the **FDCAN-ABH Converter** firmware. This firmware implements a bidirectional bridge between:

- **CAN interface** using the DARTT (Dual Address Real-Time Transport) protocol
- **UART interface** using the Psyonic Ability Hand Extended Mode API protocol

### 1.2 System Overview

The FDCAN-ABH Converter serves as a protocol translation gateway enabling CAN-based controllers to command and monitor Psyonic Ability Hand prosthetic devices. The converter operates on an STM32G4 microcontroller with FDCAN and UART peripherals.

**Primary Functions:**
- Translate DARTT CAN commands to Ability Hand UART frames
- Perform HDLC byte stuffing/unstuffing for UART communication
- Provide high-level structured motor control API
- Support direct UART pass-through mode for custom protocols
- Maintain non-volatile configuration storage

### 1.3 Document Overview

This document specifies:
- Register-level memory map using DARTT word addressing
- CAN and UART protocol requirements
- Operational modes and command sequences
- Timing and performance characteristics
- Configuration procedures

### 1.4 Intended Audience

This document is intended for:
- Embedded systems engineers developing CAN-based controllers
- Integration engineers implementing Ability Hand control systems
- Test and validation engineers
- Technical support personnel

---

## 2. Referenced Documents

### 2.1 Applicable Documents

| Document ID | Title | Version |
|-------------|-------|---------|
| ABH-ICD-001 | Ability Hand Interface Control Document | Rev 3.0 |
| DARTT-SPEC | DARTT Protocol Specification | Latest |

---

## 3. Interface Overview

### 3.1 Interface Summary

| Interface | Direction | Protocol | Purpose |
|-----------|-----------|----------|---------|
| CAN RX | Input | DARTT | Receive commands and configuration from controller |
| CAN TX | Output | DARTT | Send feedback and status to controller |
| UART TX | Output | Ability Hand API | Send motor commands to prosthetic |
| UART RX | Input | Ability Hand API | Receive feedback from prosthetic |
| Flash | Storage | N/A | Persist configuration across power cycles |

### 3.2 Key Features

- **Dual Operating Modes:** High-level API or low-level pass-through
- **Automatic Frame Construction:** Generate properly formatted UART frames from structured commands
- **HDLC Processing:** Automatic unstuffing on RX, manual stuffing required on TX for pass-through
- **Non-Volatile Storage:** Flash-based persistence for CAN and UART configuration
- **Real-Time Performance:** Sub-millisecond latency for typical command-response cycles

---

## 4. Interface Characteristics

### 4.1 CAN Interface

#### 4.1.1 Physical Layer

- **Bus Standard:** CAN FD (ISO 11898-1)
- **Nominal Bit Rate:** 500 kbit/s (default, configurable)
- **Data Bit Rate:** Same as nominal (FDCAN not utilizing data phase speed-up)
- **Transceiver:** Integrated 5V CAN transceiver (on-board)
- **Termination:** Integrated 120Ω termination resistor (on-board)
- **Bus Signals:** CANH and CANL differential pair

#### 4.1.2 CAN Addressing

The converter uses a configurable CAN arbitration ID scheme:

| Parameter | Default Value | Configuration Register |
|-----------|---------------|------------------------|
| Module Number | 0x50 | MODULE_NUMBER (0x000) |
| Complementary CAN ID | 0x7FF - module_number | Calculated (0x7AF default) |

**Addressing Rules:**
- **DEFAULT CAN ID**: 0x7AF
- Primary ID - not used
- Complementary ID used for all DARTT messages
- Module number must be unique on the CAN bus
- Valid range: 0x01 - 0x7FE

#### 4.1.3 DARTT Protocol

The DARTT (Dual Address Real-Time Transport) protocol provides block memory read/write access over CAN.

**Key Characteristics:**
- **Word Size:** 32 bits (4 bytes)
- **Addressing:** Word-aligned (addresses are word indexes, not byte offsets)
- **Access Modes:** Block read, block write
- **Endianness:** Little-endian (index and num_bytes fields)
- **Payload Endianness:** Application-defined - Little-endian for this device
- **Maximum Block Size:** Application-defined - 6 bytes maximum for this device
- **Message Type:** TYPE_ADDR_CRC_MESSAGE (Type 2) - CAN provides addressing and error checking

**CAN Frame Formats:**

This device uses DARTT Type 2 messages (TYPE_ADDR_CRC_MESSAGE), which rely on CAN's built-in arbitration ID and CRC, eliminating protocol overhead.

**Write Frame (Controller → Converter):**

| Bytes 0-1 | Bytes 2-N |
|-----------|-----------|
| Index (R=0) | Payload Data |

- **Index**: 16-bit little-endian word index (bit 15 = 0 for write)
- **Payload**: N bytes of data to write (between 4 and 6 on this device for standard CAN compatibility)

**Read Request Frame (Controller → Converter):**

| Bytes 0-1 | Bytes 2-3 |
|-----------|-----------|
| Index (R=1) | Num Bytes |

- **Index**: 16-bit little-endian word index (bit 15 = 1 for read)
- **Num Bytes**: 16-bit little-endian byte count to read. Requesting more than 8 bytes from this device will result in no response.

**Read Reply Frame (Converter → Controller):**

| Bytes 0-N |
|-----------|
| Requested Data Block |

- **Data Block**: N bytes of requested data (raw payload)

**Index Encoding:**
```
Word Index 0 → Byte offset 0   → DARTT Index field = 0x0000
Word Index 1 → Byte offset 4   → DARTT Index field = 0x0001
Word Index 2 → Byte offset 8   → DARTT Index field = 0x0002
...
Word Index N → Byte offset 4N  → DARTT Index field = 0x000N

For reads:  Index field = 0x8000 | word_index  (bit 15 = 1)
For writes: Index field = 0x0000 | word_index  (bit 15 = 0)
```

**Example - Write 8 bytes to word index 0x008:**
```
CAN ID: 0x7AF
Payload: [0x08, 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF]
         |Index=0x0008| |------ 6 bytes of data --------|
```

**Example - Read 6 bytes from word index 0x014:**
```
CAN ID: 0x7AF
Payload: [0x14, 0x80, 0x06, 0x00]
         |Index=0x8014| |Num=6 |
Reply:
CAN ID: 0x7AF
Payload: [12 bytes of data from word 0x014-0x016]
```

### 4.2 UART Interface

#### 4.2.1 Physical Layer

- **Baud Rate:** 460800 bps (default, configurable)
- **Data Format:** 8 data bits, no parity, 1 stop bit (8N1)
- **Flow Control:** None
- **Direction:** Full duplex

#### 4.2.2 Supported Baud Rates

| Baud Rate (bps) | UART_BAUD_RATE Value | Common Use |
|-----------------|----------------------|------------|
| 9600 | 9600 | Debugging, legacy |
| 19200 | 19200 | Low-speed reliable |
| 38400 | 38400 | Standard |
| 57600 | 57600 | Standard |
| 115200 | 115200 | Common high-speed |
| 230400 | 230400 | High-speed |
| 460800 | 460800 | Default (recommended) |
| 921600 | 921600 | Maximum reliable |
| 1000000 | 1000000 | Maximum |

**Note:** Baud rates above 460800 may require careful cable selection and short cable runs (<2m).

#### 4.2.3 HDLC Framing

The UART frame synchronization is based on HDLC (High-Level Data Link Control) byte stuffing. It deviates from this specification slightly in that a unique frame delimiter is expected at both the beginning and end of each frame. 
Refer to the [following library](https://github.com/ocanath/byte-stuffing) for compatible C and Python implementations.

**Stuffing Requirements (TX Pass-Through Mode):**
- Controller MUST perform stuffing before writing to UART_TX_MEM
- Failure to stuff will result in no communication to/from the hand

#### 4.2.4 Ability Hand Protocol Summary

The downstream UART protocol supports multiple control modes:

| Command Header | Control Mode | Data Type | Reply Variant |
|----------------|--------------|-----------|---------------|
| 0x10-0x12 | Position | int16_t (±32767 = 0-150°) | 1, 2, or 3 |
| 0x20-0x22 | Velocity | int16_t (±32767 = 0-3000°/s) | 1, 2, or 3 |
| 0x30-0x32 | Torque/Current | int16_t | 1, 2, or 3 |
| 0x40-0x42 | Voltage | int16_t | 1, 2, or 3 |
| 0xA0-0xA2 | Read-only | N/A | 1, 2, or 3 |

**Reply Variants:**
- **Variant 1 (TX1):** Position + Current feedback
- **Variant 2 (TX2):** Position + Velocity feedback
- **Variant 3 (TX3):** Position + Current + FSR data

**Frame Structure:**
```
[0x7E] [Address] [Command Header] [12-byte Payload] [Checksum] [0x7E]
```

**Checksum:** 8-bit two's complement negation of sum of all bytes (address through payload).

Refer to ABILITY-HAND-ICD.pdf for complete Ability Hand protocol details.

---

## 5. Memory Map and Register Definitions

### 5.1 Memory Map Overview

The FDCAN-ABH Converter exposes all configurable parameters and operational data through a DARTT-accessible memory map. All registers are organized as 32-bit words with the following access types:

- **R/W**: Read/Write - Can be read and modified
- **RO**: Read-Only - Can only be read
- **WO**: Write-Only - Writing triggers an action
- **R/W-NV**: Read/Write Non-Volatile - Persists across power cycles when committed

#### 5.1.1 Memory Map Structure

| Base Word Index | Block Name | Size (words) | Description |
|-----------------|------------|--------------|-------------|
| 0x000 | Non-Volatile Configuration | 6 | CAN and UART configuration parameters |
| 0x006 | Ability Hand Control API | 40 | High-level motor control interface |
| 0x02E | Register Access Interface | 3 | Direct register read/write (unimplemented) |
| 0x031 | ABH Read Timeout | 1 | Reply timeout configuration |
| 0x032 | UART RX Decoded Buffer | 19 | HDLC-decoded receive buffer (76 bytes) |
| 0x045 | UART RX Byte Count | 1 | Valid bytes in RX buffer  |
| 0x046 | UART TX Buffer | 19 | Raw transmit buffer. Cleared on interface device once tranmission begins |
| 0x059 | UART TX Byte Count | 1 | TX trigger - write byte count to initiate transmission |
| 0x05A | Git Hash | 4 | Zero-terminated short git hash (max 15 characters, null-terminated) - for version tracking |
| 0x5E | Update nonvolatile storage | 1 | Flag for updating non-volatile storage - cleared upon completion of update operation |

**Total Memory Map Size:** 95 words (380 bytes)

---

### 5.2 Block 1: Non-Volatile Configuration (0x000-0x005)

These registers define persistent configuration parameters stored in flash memory (Page 63). Changes to these registers require setting the `UPDATE_NONVOLATILE_STORAGE` flag (0x057) to persist across power cycles.

| Word Index | Register Name | Type | Access | Default | Description |
|------------|---------------|------|--------|---------|-------------|
| 0x000 | MODULE_NUMBER | uint32_t | R/W-NV | 0x50 | CAN module address. Complementary address: 0x7FF - module_number |
| 0x001 | UART_BAUD_RATE | uint32_t | R/W-NV | 460800 | UART baud rate in bits/second. See Section 4.2.2 for valid values |
| 0x002 | FDCAN_NBRP | uint32_t | R/W-NV | 2 | FDCAN Nominal Bit Rate Prescaler. Range: 1-512 |
| 0x003 | FDCAN_NTSEG1 | uint32_t | R/W-NV | 135 | FDCAN Nominal Time Segment 1. Range: 2-256 |
| 0x004 | FDCAN_NTSEG2 | uint32_t | R/W-NV | 34 | FDCAN Nominal Time Segment 2. Range: 2-128 |
| 0x005 | UNUSED_ZEROPAD | uint32_t | RO | 0 | Unused nonvolatile storage word. |

**CAN Bit Rate Calculation:**

```
Nominal_Bit_Rate = 170_MHz / (NBRP × (1 + NTSEG1 + NTSEG2))
```

**Default Example:**
```
170_MHz / (2 × (1 + 135 + 34)) = 170_MHz / 340 = 500 kbit/s
```

**Configuration Notes:**
- Changes take effect after power cycle or system reset
- Invalid CAN timing parameters may cause bus communication failure
- Peripheral clock (PCLK): 170 MHz (STM32G4 maximum)

---

### 5.3 Block 2: Ability Hand Control API (0x006-0x026)

This block provides high-level structured control of the Ability Hand. Writing to the command setpoint arrays automatically generates properly formatted UART frames with HDLC stuffing. Reply data is parsed and populated into the feedback arrays.

#### 5.3.1 Configuration and Control (0x006-0x007)

| Word Index | Register Name | Type | Access | Default | Description |
|------------|---------------|------|--------|---------|-------------|
| 0x006 | ABH_ADDRESS | uint32_t | R/W | 0x50 | Ability Hand UART device address |
| 0x007 | ABH_COMMAND_HEADER | uint32_t | R/W | 0x10 | Frame command header. See Section 5.3.2 |

#### 5.3.2 Command Header Values

| Value | Mode | Description | Reply Format |
|-------|------|-------------|--------------|
| 0xA0 | Read-only | Dummy command, reply variant 1 | Position + Current |
| 0xA1 | Read-only | Dummy command, reply variant 2 | Position + Velocity |
| 0xA2 | Read-only | Dummy command, reply variant 3 | Position + Current + FSR |
| 0x10 | Position control | Position control, reply variant 1 | Position + Current |
| 0x11 | Position control | Position control, reply variant 2 | Position + Velocity |
| 0x12 | Position control | Position control, reply variant 3 | Position + Current + FSR |
| 0x20 | Velocity control | Velocity control, reply variant 1 | Position + Current |
| 0x21 | Velocity control | Velocity control, reply variant 2 | Position + Velocity |
| 0x22 | Velocity control | Velocity control, reply variant 3 | Position + Current + FSR |
| 0x30 | Torque control | Torque control, reply variant 1 | Position + Current |
| 0x31 | Torque control | Torque control, reply variant 2 | Position + Velocity |
| 0x32 | Torque control | Torque control, reply variant 3 | Position + Current + FSR |
| 0x40 | Voltage control | Voltage control, reply variant 1 | Position + Current |
| 0x41 | Voltage control | Voltage control, reply variant 2 | Position + Velocity |
| 0x42 | Voltage control | Voltage control, reply variant 3 | Position + Current + FSR |
| 0xDE | Register write | Write to internal register | None |
| 0xDA | Register read | Read from internal register | 32-bit register value |
| 0x07 | Bluetooth | Enable Bluetooth radio | None |
| 0x08 | Bluetooth | Disable Bluetooth radio | None |
| 0x09 | System | Restart hand controller | None |
| 0x7C | API | Exit API mode | None |

#### 5.3.3 Motor Setpoints - Position (0x008-0x00A)

| Word Index | Register Name | Type | Access | Default | Description |
|------------|---------------|------|--------|---------|-------------|
| 0x008 | Q_DESIRED_0_1 | int16_t[2] | R/W | 0 | Position setpoints for motors 0-1 |
| 0x009 | Q_DESIRED_2_3 | int16_t[2] | R/W | 0 | Position setpoints for motors 2-3 |
| 0x00A | Q_DESIRED_4_5 | int16_t[2] | R/W | 0 | Position setpoints for motors 4-5 |

**Data Format:** Each 32-bit word contains two packed int16_t values:
- Bits [15:0]: Motor N
- Bits [31:16]: Motor N+1

**Scaling:**
```
position_digital = (angle_degrees / 150.0) × 32767
```

**Motor Mapping:**
- Motor 0: Thumb rotation
- Motor 1: Thumb flexion
- Motor 2: Index finger
- Motor 3: Middle finger
- Motor 4: Ring finger
- Motor 5: Pinky finger

#### 5.3.4 Motor Setpoints - Voltage (0x00B-0x00D)

| Word Index | Register Name | Type | Access | Default | Description |
|------------|---------------|------|--------|---------|-------------|
| 0x00B | VQ_DESIRED_0_1 | int16_t[2] | R/W | 0 | Voltage setpoints for motors 0-1 |
| 0x00C | VQ_DESIRED_2_3 | int16_t[2] | R/W | 0 | Voltage setpoints for motors 2-3 |
| 0x00D | VQ_DESIRED_4_5 | int16_t[2] | R/W | 0 | Voltage setpoints for motors 4-5 |

**Data Format:** Each word contains two packed int16_t values (motor pairs).

**Scaling:** ±32767 represents full-scale PWM duty cycle (direct voltage control).

**Use Case:** Open-loop voltage control for testing or specialized control algorithms.

#### 5.3.5 Motor Setpoints - Current/Torque (0x00E-0x010)

| Word Index | Register Name | Type | Access | Default | Description |
|------------|---------------|------|--------|---------|-------------|
| 0x00E | IQ_DESIRED_0_1 | int16_t[2] | R/W | 0 | Current/torque setpoints for motors 0-1 |
| 0x00F | IQ_DESIRED_2_3 | int16_t[2] | R/W | 0 | Current/torque setpoints for motors 2-3 |
| 0x010 | IQ_DESIRED_4_5 | int16_t[2] | R/W | 0 | Current/torque setpoints for motors 4-5 |

**Data Format:** Each word contains two packed int16_t values.

**Scaling:** Hand-specific, refer to Ability Hand ICD Section 3.4 for current scaling factors.

**Use Case:** Torque control mode for force-limited grasping.

#### 5.3.6 Motor Setpoints - Velocity (0x011-0x013)

| Word Index | Register Name | Type | Access | Default | Description |
|------------|---------------|------|--------|---------|-------------|
| 0x011 | VELOCITY_DESIRED_0_1 | int16_t[2] | R/W | 0 | Velocity setpoints for motors 0-1 |
| 0x012 | VELOCITY_DESIRED_2_3 | int16_t[2] | R/W | 0 | Velocity setpoints for motors 2-3 |
| 0x013 | VELOCITY_DESIRED_4_5 | int16_t[2] | R/W | 0 | Velocity setpoints for motors 4-5 |

**Data Format:** Each word contains two packed int16_t values.

**Scaling (to hand):**
```
velocity_digital = (velocity_deg_per_sec / 3000.0) × 32767
```

**Valid Range:**
- 0 °/s = 0
- 1500 °/s = 16384
- 3000 °/s = 32767

#### 5.3.7 Motor Feedback - Position (0x014-0x016)

| Word Index | Register Name | Type | Access | Default | Description |
|------------|---------------|------|--------|---------|-------------|
| 0x014 | Q_ACTUAL_0_1 | int16_t[2] | RO | 0 | Actual position feedback for motors 0-1 |
| 0x015 | Q_ACTUAL_2_3 | int16_t[2] | RO | 0 | Actual position feedback for motors 2-3 |
| 0x016 | Q_ACTUAL_4_5 | int16_t[2] | RO | 0 | Actual position feedback for motors 4-5 |

**Scaling:** Same as position setpoints (±32767 = 0-150°).

**Update:** Populated after receiving valid UART reply from hand. Available in all reply variants.

#### 5.3.8 Motor Feedback - Current (0x017-0x019)

| Word Index | Register Name | Type | Access | Default | Description |
|------------|---------------|------|--------|---------|-------------|
| 0x017 | IQ_ACTUAL_0_1 | int16_t[2] | RO | 0 | Actual current feedback for motors 0-1 |
| 0x018 | IQ_ACTUAL_2_3 | int16_t[2] | RO | 0 | Actual current feedback for motors 2-3 |
| 0x019 | IQ_ACTUAL_4_5 | int16_t[2] | RO | 0 | Actual current feedback for motors 4-5 |

**Availability:** Populated when reply variant 1 (TX1) or 3 (TX3) is selected via command header.

**Use Case:** Current monitoring for torque estimation and fault detection.

#### 5.3.9 Motor Feedback - Velocity (0x01A-0x01C)

| Word Index | Register Name | Type | Access | Default | Description |
|------------|---------------|------|--------|---------|-------------|
| 0x01A | VELOCITY_ACTUAL_0_1 | int16_t[2] | RO | 0 | Actual velocity feedback for motors 0-1 |
| 0x01B | VELOCITY_ACTUAL_2_3 | int16_t[2] | RO | 0 | Actual velocity feedback for motors 2-3 |
| 0x01C | VELOCITY_ACTUAL_4_5 | int16_t[2] | RO | 0 | Actual velocity feedback for motors 4-5 |

**Availability:** Populated when reply variant 2 (TX2) is selected via command header.

**Scaling (from hand):**
```
velocity_rad_per_sec = velocity_digital / 4
```

**Note:** Velocity feedback uses radians/sec, while velocity setpoints use degrees/sec.

#### 5.3.10 Force Sensor Feedback (0x01D-0x024)

| Word Index | Register Name | Type | Access | Default | Description |
|------------|---------------|------|--------|---------|-------------|
| 0x01D | FSR_RAW_0_1 | uint16_t[2] | RO | 0 | FSR sensors 0-1 (12-bit packed ADC values) |
| 0x01E | FSR_RAW_2_3 | uint16_t[2] | RO | 0 | FSR sensors 2-3 |
| 0x01F | FSR_RAW_4_5 | uint16_t[2] | RO | 0 | FSR sensors 4-5 |
| 0x020 | FSR_RAW_6_7 | uint16_t[2] | RO | 0 | FSR sensors 6-7 |
| 0x021 | FSR_RAW_8_9 | uint16_t[2] | RO | 0 | FSR sensors 8-9 |
| 0x022 | FSR_RAW_10_11 | uint16_t[2] | RO | 0 | FSR sensors 10-11 |
| 0x023 | FSR_RAW_12_13 | uint16_t[2] | RO | 0 | FSR sensors 12-13 |
| 0x024 | FSR_RAW_14_15 | uint16_t[2] | RO | 0 | FSR sensors 14-15 |

**FSR Layout:** 5 fingers × 6 FSRs/finger = 30 FSR sensors total

**Finger Mapping:**
- FSR 0-5: Thumb
- FSR 6-11: Index
- FSR 12-17: Middle
- FSR 18-23: Ring
- FSR 24-29: Pinky

**Data Format:** 12-bit ADC values (0-4095). Higher values indicate greater force.

**Availability:** Populated when reply variant 3 (TX3) is selected via command header.

#### 5.3.11 Status and Reply Information (0x025-0x026)

| Word Index | Register Name | Type | Access | Default | Description |
|------------|---------------|------|--------|---------|-------------|
| 0x025 | HOT_COLD_BITMASK | uint32_t | RO | 0 | Motor temperature status bitmask. Bit N set = motor N warning |
| 0x026 | FORMAT_HEADER | uint32_t | RO | 0 | Last received reply format header. Indicates reply data structure |

**HOT_COLD_BITMASK Bit Definitions:**
- Bit 0: Motor 0 temperature warning
- Bit 1: Motor 1 temperature warning
- ...
- Bit 5: Motor 5 temperature warning
- Bits [31:6]: Reserved

**FORMAT_HEADER:** Echoes the command header from the most recent valid reply. Use to verify expected reply variant was received.

---

### 5.4 Block 3: Register Access Interface (0x027-0x029) - UNIMPLEMENTED

This block provides direct register-level access to internal Ability Hand configuration registers.

**IMPORTANT: This functionality is currently unimplemented in the firmware.** Reading or writing these registers will have no effect.

| Word Index | Register Name | Type | Access | Default | Description |
|------------|---------------|------|--------|---------|-------------|
| 0x027 | REGISTER_TARGET | uint32_t | R/W | 0 | Target register address for read/write operations |
| 0x028 | REGISTER_WRITE_VAL | uint32_t | R/W | 0 | Value to write to target register |
| 0x029 | REGISTER_READ_REPLY_VAL | uint32_t | RO | 0 | Value read back from target register |

**Intended Usage Pattern (when implemented):**

**Register Write:**
1. Write target address to REGISTER_TARGET (0x027)
2. Write value to REGISTER_WRITE_VAL (0x028)
3. Set ABH_COMMAND_HEADER (0x007) to 0xDE
4. Trigger UART transmission

**Register Read:**
1. Write target address to REGISTER_TARGET (0x027)
2. Set ABH_COMMAND_HEADER (0x007) to 0xDA
3. Trigger UART transmission
4. Wait for ABH_READ_TIMEOUT (0x02A)
5. Read value from REGISTER_READ_REPLY_VAL (0x029)

---

### 5.5 Block 4: UART Direct Buffers (0x02A-0x051)

This block provides low-level direct access to UART transmission and reception buffers for pass-through mode. The controller is responsible for HDLC byte stuffing on TX data. The firmware automatically performs HDLC unstuffing on RX data.

#### 5.5.1 Timeout Configuration (0x02A)

| Word Index | Register Name | Type | Access | Default | Description |
|------------|---------------|------|--------|---------|-------------|
| 0x02A | ABH_READ_TIMEOUT | uint32_t | R/W | 100 | Reply timeout in milliseconds. Used in automatic mode |

**Usage:** Defines how long the firmware waits for a UART reply before marking the transaction as timed out.

**Recommended Values:**
- 50 ms: Fast polling applications
- 100 ms: Default (recommended)
- 200 ms: Noisy or long cable runs

#### 5.5.2 UART RX Buffer (0x02B-0x03E)

| Word Index | Register Name | Type | Access | Size | Description |
|------------|---------------|------|--------|------|-------------|
| 0x02B-0x03D | UART_RX_DECODED[0-75] | uint8_t[76] | RO | 19 words | HDLC-decoded receive buffer. Contains unstuffed UART data |
| 0x03E | NBYTES_DECODED_UART | uint16_t | RO | Lower 16 bits | Number of valid bytes in UART_RX_DECODED buffer |

**Buffer Layout:** 76-byte buffer packed into 19 32-bit words. Firmware performs HDLC unstuffing automatically.

**Access Pattern:**
1. Read NBYTES_DECODED_UART to determine valid data length
2. Read appropriate words from UART_RX_DECODED
3. Extract bytes from packed 32-bit words (little-endian)

**Example:** To read byte 5:
```
word_index = 0x02B + (5 / 4) = 0x02C
byte_offset = 5 % 4 = 1
byte_5 = (UART_RX_DECODED[word_index] >> (8 * byte_offset)) & 0xFF
```

#### 5.5.3 UART TX Buffer (0x03F-0x052)

| Word Index | Register Name | Type | Access | Size | Description |
|------------|---------------|------|--------|------|-------------|
| 0x03F-0x051 | UART_TX_MEM[0-75] | uint8_t[76] | R/W | 19 words | Raw transmit buffer. Controller must perform HDLC stuffing |
| 0x052 | NBYTES_WRITE_UART | uint32_t | WO | 1 word | TX trigger register. Write byte count to initiate transmission |

**Buffer Layout:** 76-byte buffer packed into 19 32-bit words.

**Usage Sequence (Pass-Through Mode):**
1. Construct UART frame in local buffer
2. Perform HDLC byte stuffing (escape 0x7E and 0x7D)
3. Write stuffed bytes to UART_TX_MEM (0x03F-0x051)
4. Write byte count to NBYTES_WRITE_UART (0x052)
5. Transmission begins immediately

**CRITICAL:** Controller MUST perform HDLC stuffing before transmission. Failure to stuff will result in framing errors.

**Maximum Frame Size:**
- Unstuffed: 72 bytes (typical Ability Hand frame)
- Worst-case stuffed: 144 bytes (all bytes escaped)
- Buffer limit: 76 bytes (limits maximum stuffed frame size)

---

### 5.6 Block 5: System Information (0x053-0x057)

This block provides read-only system information and control flags for firmware management.

#### 5.6.1 Firmware Version (0x053-0x056)

| Word Index | Register Name | Type | Access | Size | Description |
|------------|---------------|------|--------|------|-------------|
| 0x053-0x056 | GIT_HASH_BUFFER[0-15] | uint8_t[16] | RO | 4 words | Git commit hash identifying firmware version (ASCII hex) |

**Format:** 16-character ASCII string representing the git commit SHA (first 16 hex characters).

**Example:** "a3f2c1b8e4d9f7a2" represents git commit hash starting with a3f2c1b8...

**Usage:** Read to verify firmware version during system initialization or diagnostics.

#### 5.6.2 Control Flags (0x057)

| Word Index | Register Name | Type | Access | Default | Description |
|------------|---------------|------|--------|---------|-------------|
| 0x057 | UPDATE_NONVOLATILE_STORAGE | uint8_t | R/W | 0 | Flash update trigger. Write 1 to persist Block 1 registers. Auto-clears |

**Usage Sequence:**
1. Modify one or more registers in Block 1 (0x000-0x005)
2. Write 1 to UPDATE_NONVOLATILE_STORAGE (0x057)
3. Firmware commits changes to flash (Page 63)
4. Flag automatically clears to 0 upon completion
5. Changes persist across power cycles

**Flash Write Time:** Approximately 100-200 ms (firmware blocks during write)

**Caution:** Excessive flash writes reduce flash lifespan. Limit to configuration changes only (not runtime updates).

---

### 5.7 Register Access Notes

#### 5.7.1 Endianness

All multi-byte values use **little-endian** byte ordering (LSB at lowest address).

**Example:** 32-bit value 0x12345678 stored as:
```
Byte 0: 0x78
Byte 1: 0x56
Byte 2: 0x34
Byte 3: 0x12
```

#### 5.7.2 Packed Arrays

Registers containing arrays of int16_t or uint16_t pack two values per 32-bit word. Access pattern:
- **Bits [15:0]:** Array element N
- **Bits [31:16]:** Array element N+1

**Example:** Q_DESIRED_0_1 (0x008):
```
Bits [15:0]:  Motor 0 position setpoint (int16_t)
Bits [31:16]: Motor 1 position setpoint (int16_t)
```

#### 5.7.3 Atomic Access

The DARTT protocol ensures atomic 32-bit word access. Sub-word fields (uint8_t, uint16_t) should be read/written as complete 32-bit words with appropriate masking and shifting.

#### 5.7.4 Write Triggers

Writing to **NBYTES_WRITE_UART** (0x052) triggers immediate UART transmission. Ensure UART_TX_MEM is fully populated before triggering.

#### 5.7.5 Read-Only Enforcement

Read-only register designations are **informational only**. The firmware does not enforce write protection. Writing to RO registers may have undefined behavior.

---

## 6. Operational Modes

The FDCAN-ABH Converter supports two distinct operational modes to accommodate different levels of control abstraction.

### 6.1 Mode 1: Automatic Ability Hand Control

In this mode, the converter handles all low-level protocol details, including frame construction, checksum calculation, and HDLC byte stuffing.

#### 6.1.1 Characteristics

- **User Interaction:** Read/write registers in Block 2 (Ability Hand Control API)
- **Frame Generation:** Automatic
- **HDLC Processing:** Automatic (both TX and RX)
- **Checksum:** Calculated automatically
- **Reply Parsing:** Automatic
- **Use Case:** Standard motor control applications

#### 6.1.2 Control Flow

```
Controller                     Converter                      Ability Hand
    |                              |                              |
    |--DARTT Write Q_DESIRED------>|                              |
    |  (Position setpoints)        |                              |
    |                              |--UART TX (stuffed frame)---->|
    |                              |                              |
    |                              |<--UART RX (stuffed reply)----|
    |                              | [Automatic unstuffing]       |
    |                              | [Parse reply data]           |
    |<--DARTT Read Q_ACTUAL--------|                              |
    |  (Position feedback)         |                              |
```

#### 6.1.3 Advantages

- Simple controller implementation
- Guaranteed protocol correctness
- Automatic error handling
- Suitable for high-frequency control loops (1-10 kHz)

#### 6.1.4 Limitations

- Limited to standard Ability Hand command set
- Cannot implement custom protocols
- Fixed frame structure

### 6.2 Mode 2: Direct UART Pass-Through

In this mode, the controller has direct access to UART buffers and is responsible for frame construction and HDLC stuffing.

#### 6.2.1 Characteristics

- **User Interaction:** Read/write registers in Block 4 (UART Direct Buffers)
- **Frame Generation:** Manual (controller responsibility)
- **HDLC TX Stuffing:** Manual (controller responsibility)
- **HDLC RX Unstuffing:** Automatic
- **Checksum:** Manual (controller responsibility)
- **Reply Parsing:** Manual (controller responsibility)
- **Use Case:** Custom protocols, debugging, advanced features

#### 6.2.2 Control Flow

```
Controller                     Converter                      Ability Hand
    |                              |                              |
    |  [Construct frame locally]   |                              |
    |  [Perform HDLC stuffing]     |                              |
    |--Write UART_TX_MEM---------->|                              |
    |--Write NBYTES_WRITE_UART---->|                              |
    |                              |--UART TX (stuffed frame)---->|
    |                              |                              |
    |                              |<--UART RX (stuffed reply)----|
    |                              | [Automatic unstuffing]       |
    |<--Read UART_RX_DECODED-------|                              |
    |  [Parse reply locally]       |                              |
```

#### 6.2.3 Advantages

- Full protocol flexibility
- Access to custom Ability Hand features
- Debugging and diagnostic capabilities
- Can implement non-standard command sequences

#### 6.2.4 Limitations

- Controller must implement HDLC stuffing
- Controller must implement checksum calculation
- Higher controller complexity
- Potential for protocol errors

### 6.3 Mode Selection

The converter does **not** have an explicit mode selection register. The operational mode is implicitly determined by which registers the controller accesses:

- **Automatic Mode:** Access Block 2 registers (0x006-0x026)
- **Pass-Through Mode:** Access Block 4 registers (0x02A-0x052)

**Note:** The two modes can be mixed, but this is not recommended as it may lead to race conditions where automatic frame generation conflicts with manual buffer writes.

### 6.4 Recommended Mode Selection

| Application | Recommended Mode | Rationale |
|-------------|------------------|-----------|
| Position control loop | Automatic | Simplicity, high update rate |
| Velocity control loop | Automatic | Simplicity, high update rate |
| Torque control loop | Automatic | Simplicity, high update rate |
| Register configuration | Pass-Through | Requires custom commands |
| Firmware updates | Pass-Through | Custom protocol |
| Diagnostics | Pass-Through | Direct access needed |
| Production systems | Automatic | Reliability, simplicity |

---

## 7. Command Sequences

This section provides detailed step-by-step command sequences for common operations.

### 7.1 System Initialization

#### 7.1.1 Sequence: Power-On Configuration Verification

**Purpose:** Verify configuration after power-on or reset.

| Step | Action | Register | Value | Notes |
|------|--------|----------|-------|-------|
| 1 | Read module number | 0x000 | Expected: 0x50 | Verify CAN address |
| 2 | Read UART baud rate | 0x001 | Expected: 460800 | Verify UART config |
| 3 | Read firmware version | 0x053-0x056 | N/A | Log for diagnostics |
| 4 | Verify CAN timing | 0x002-0x004 | NBRP=2, NTSEG1=135, NTSEG2=34 | Confirm 500 kbit/s |

**Expected Completion Time:** <10 ms

#### 7.1.2 Sequence: First-Time Configuration

**Purpose:** Configure a new converter module for specific CAN ID and baud rate.

| Step | Action | Register | Value | Notes |
|------|--------|----------|-------|-------|
| 1 | Write new module number | 0x000 | User-defined (e.g., 0x51) | Must be unique on bus |
| 2 | Write UART baud rate | 0x001 | User-defined (e.g., 460800) | See Section 4.2.2 |
| 3 | Optionally adjust CAN timing | 0x002-0x004 | User-defined | For non-standard bus rates |
| 4 | Trigger flash write | 0x057 | 1 | Commit to non-volatile |
| 5 | Wait for completion | 0x057 | Read until 0 | Auto-clears when done |
| 6 | Power cycle | N/A | N/A | Required for changes to take effect |

**Expected Completion Time:** 100-200 ms (flash write time)

**CAUTION:** Do not interrupt power during flash write (Step 4-5).

### 7.2 Position Control (Automatic Mode)

#### 7.2.1 Sequence: Single Position Command with Feedback

**Purpose:** Command finger positions and read back actual positions.

| Step | Action | Register | Value | Notes |
|------|--------|----------|-------|-------|
| 1 | Set command header | 0x007 | 0x10 | Position control, reply variant 1 |
| 2 | Set device address | 0x006 | 0x50 | Default Ability Hand address |
| 3 | Write position setpoints | 0x008-0x00A | Motor commands | Scaled: angle/150 × 32767 |
| 4 | Wait for reply | N/A | Delay 10-50 ms | Depends on UART latency |
| 5 | Read position feedback | 0x014-0x016 | N/A | Actual motor positions |
| 6 | Read current feedback | 0x017-0x019 | N/A | Actual motor currents |
| 7 | Check temperature status | 0x025 | N/A | Monitor for warnings |

**Expected Completion Time:** 10-50 ms (round-trip)

**Example - Command all fingers to 90°:**

```
angle_desired = 90° (degrees)
position_digital = (90 / 150) × 32767 = 19660 (0x4CCC)

Write to registers:
0x008 = 0x4CCC4CCC  (Motors 0-1: both 90°)
0x009 = 0x4CCC4CCC  (Motors 2-3: both 90°)
0x00A = 0x4CCC4CCC  (Motors 4-5: both 90°)
```

#### 7.2.2 Sequence: High-Frequency Position Control Loop

**Purpose:** Continuous position control at 1 kHz update rate.

| Step | Action | Register | Frequency | Notes |
|------|--------|----------|-----------|-------|
| 1 | Initialize command header | 0x007 | Once | Set to 0x10 |
| 2 | Loop: Write new setpoints | 0x008-0x00A | 1 kHz | Update every 1 ms |
| 3 | Loop: Read feedback | 0x014-0x016 | 1 kHz | Verify tracking |
| 4 | Loop: Monitor errors | 0x025 | 1 kHz | Temperature warnings |

**Performance Considerations:**
- Use DARTT block write for efficient multi-register updates
- Read feedback in separate CAN transaction if bus bandwidth allows
- Monitor HOT_COLD_BITMASK for thermal shutdown

### 7.3 Velocity Control (Automatic Mode)

#### 7.3.1 Sequence: Velocity Command with Velocity Feedback

**Purpose:** Command finger velocities and read back actual velocities.

| Step | Action | Register | Value | Notes |
|------|--------|----------|-------|-------|
| 1 | Set command header | 0x007 | 0x21 | Velocity control, reply variant 2 |
| 2 | Write velocity setpoints | 0x011-0x013 | Motor commands | Scaled: velocity/3000 × 32767 |
| 3 | Wait for reply | N/A | Delay 10-50 ms | Depends on UART latency |
| 4 | Read position feedback | 0x014-0x016 | N/A | Actual motor positions |
| 5 | Read velocity feedback | 0x01A-0x01C | N/A | Actual motor velocities |

**Example - Command 1500°/s on all motors:**

```
velocity_desired = 1500 (degrees/second)
velocity_digital = (1500 / 3000) × 32767 = 16384 (0x4000)

Write to registers:
0x011 = 0x40004000  (Motors 0-1: both 1500°/s)
0x012 = 0x40004000  (Motors 2-3: both 1500°/s)
0x013 = 0x40004000  (Motors 4-5: both 1500°/s)
```

**Note:** Velocity feedback uses radians/sec scaling (divide by 4), not degrees/sec.

### 7.4 Direct UART Pass-Through (Manual Mode)

#### 7.4.1 Sequence: Send Custom UART Frame

**Purpose:** Send arbitrary UART command using pass-through buffers.

| Step | Action | Register/Local | Value | Notes |
|------|--------|----------------|-------|-------|
| 1 | Construct frame locally | Local buffer | Frame data | Build complete frame |
| 2 | Calculate checksum locally | Local | 2's complement sum | See Appendix B |
| 3 | Perform HDLC stuffing locally | Local | Escape 0x7E, 0x7D | See Appendix B |
| 4 | Write stuffed bytes | 0x03F-0x051 | Stuffed frame | Pack into 32-bit words |
| 5 | Trigger transmission | 0x052 | Byte count | Write stuffed frame length |
| 6 | Wait for TX completion | N/A | ~1-10 ms | Depends on baud and length |

**Example - Send position control frame:**

**Unstuffed frame:**
```
[0x7E] [0x50] [0x10] [12 bytes payload] [checksum] [0x7E]
Total: 16 bytes unstuffed
```

**After stuffing (worst case):**
```
May expand to 18-32 bytes depending on data content
```

#### 7.4.2 Sequence: Receive Custom UART Frame

**Purpose:** Read arbitrary UART reply using pass-through buffers.

| Step | Action | Register | Value | Notes |
|------|--------|----------|-------|-------|
| 1 | Wait for RX data | 0x03E | Poll until > 0 | NBYTES_DECODED_UART indicates valid data |
| 2 | Read byte count | 0x03E | N | Number of unstuffed bytes |
| 3 | Read decoded bytes | 0x02B-0x03D | Frame data | Already unstuffed by firmware |
| 4 | Verify checksum locally | Local | N/A | Calculate and compare |
| 5 | Parse frame locally | Local | N/A | Extract data fields |

**Note:** Firmware performs unstuffing automatically. Received data in UART_RX_DECODED is already unstuffed.

### 7.5 Register Read/Write (Unimplemented)

#### 7.5.1 Sequence: Write Ability Hand Internal Register (Future)

**Purpose:** Configure internal hand registers (e.g., PID gains, limits).

**Status:** Unimplemented in current firmware.

| Step | Action | Register | Value | Notes |
|------|--------|----------|-------|-------|
| 1 | Set command header | 0x007 | 0xDE | Write register command |
| 2 | Set target address | 0x027 | Target register | Hand-specific address |
| 3 | Set write value | 0x028 | Value | 32-bit value |
| 4 | Trigger write | (mechanism TBD) | N/A | Implementation pending |

#### 7.5.2 Sequence: Read Ability Hand Internal Register (Future)

**Purpose:** Read internal hand configuration or status.

**Status:** Unimplemented in current firmware.

| Step | Action | Register | Value | Notes |
|------|--------|----------|-------|-------|
| 1 | Set command header | 0x007 | 0xDA | Read register command |
| 2 | Set target address | 0x027 | Target register | Hand-specific address |
| 3 | Trigger read | (mechanism TBD) | N/A | Implementation pending |
| 4 | Wait for reply | 0x02A | Timeout | Default 100 ms |
| 5 | Read reply value | 0x029 | N/A | 32-bit register value |

---

## 8. Timing and Performance Characteristics

### 8.1 Latency Analysis

#### 8.1.1 Automatic Mode Round-Trip Latency

**Components:**

| Stage | Typical Time | Notes |
|-------|--------------|-------|
| CAN TX (controller → converter) | 0.1-0.5 ms | Depends on CAN bus load |
| DARTT processing | <0.01 ms | Minimal overhead |
| Frame construction | <0.05 ms | Firmware builds UART frame |
| UART TX (converter → hand) | 0.3-3 ms | Depends on baud rate and frame size |
| Hand processing | 0.5-5 ms | Hand controller latency |
| UART RX (hand → converter) | 1.5-15 ms | Depends on baud rate and reply size |
| Reply parsing | <0.05 ms | Firmware parses reply |
| CAN TX (converter → controller) | 0.1-0.5 ms | Feedback data |

**Total Round-Trip Time:**
- **Best case:** 2.5 ms (high baud rate, low bus load, simple command)
- **Typical case:** 10-20 ms (default configuration)
- **Worst case:** 50 ms (low baud rate, high bus load, complex reply)

#### 8.1.2 Pass-Through Mode Latency

**Advantages:**
- Eliminates frame construction overhead (~0.05 ms)
- Eliminates reply parsing overhead (~0.05 ms)

**Total savings:** ~0.1 ms (marginal benefit)

**Recommendation:** Use automatic mode unless protocol flexibility is required.

### 8.2 Throughput Analysis

#### 8.2.1 CAN Bus Utilization

**DARTT Block Write (6 motors, 3 words):**
- Data payload: 12 bytes (3 words × 4 bytes)
- CAN overhead: ~8 bytes (arbitration, CRC, etc.)
- Total: ~20 bytes per CAN frame
- At 800 kbit/s: ~0.2 ms/frame

**Maximum update rate (CAN-limited):** ~5000 Hz (theoretical)

**Practical limit:** 1000-2000 Hz (allows time for feedback reads)

#### 8.2.2 UART Bus Utilization

**Position Control Frame (unstuffed):**
- Frame structure: Address (1) + Header (1) + Payload (12) + Checksum (1) = 15 bytes
- Frame delimiters: 2 bytes (start/end 0x7E)
- Total unstuffed: 17 bytes
- Worst-case stuffed: 34 bytes (all payload bytes escaped)

**At 460800 baud:**
- Unstuffed: 17 bytes × 10 bits/byte ÷ 460800 = 0.37 ms
- Stuffed (typical ~20% expansion): ~0.44 ms

**Position Control Reply (variant 1, unstuffed):**
- Address (1) + Header (1) + Position (12) + Current (12) + Status (2) + Checksum (1) = 29 bytes
- With delimiters: 31 bytes unstuffed
- Typical stuffed: ~37 bytes

**At 460800 baud:**
- ~0.80 ms per reply

**Round-trip UART time:** 0.44 ms + 0.80 ms = 1.24 ms

**Maximum UART update rate:** ~800 Hz (theoretical)

**Practical limit:** 100-500 Hz (allows hand processing time)

### 8.3 Maximum Control Loop Rates

| Control Mode | Limiting Factor | Max Theoretical | Recommended Practical |
|--------------|-----------------|-----------------|------------------------|
| Position (automatic) | UART round-trip | 800 Hz | 100-200 Hz |
| Velocity (automatic) | UART round-trip | 800 Hz | 100-200 Hz |
| Torque (automatic) | UART round-trip | 800 Hz | 100-200 Hz |
| Pass-through | UART round-trip | 800 Hz | 100-500 Hz |

**Note:** Ability Hand internal control loop runs at 1 kHz. Update rates above 200 Hz provide diminishing returns.

### 8.4 Timeout Recommendations

| Operation | Recommended Timeout | Register |
|-----------|---------------------|----------|
| UART reply (default) | 100 ms | ABH_READ_TIMEOUT (0x02A) |
| UART reply (fast) | 50 ms | ABH_READ_TIMEOUT (0x02A) |
| UART reply (noisy line) | 200 ms | ABH_READ_TIMEOUT (0x02A) |
| Flash write | 500 ms | N/A (poll 0x057) |
| CAN frame ACK | 10 ms | CAN controller timeout |

### 8.5 Power Consumption

| Mode | Typical Current | Notes |
|------|-----------------|-------|
| Active (idle) | 50 mA @ 3.3V | No UART/CAN traffic |
| Active (1 kHz control) | 60 mA @ 3.3V | Continuous operation |
| Flash write | 80 mA @ 3.3V | Peak during write |

**Total Power:** ~200-265 mW typical

---

## 9. Non-Volatile Configuration Management

### 9.1 Flash Memory Organization

The converter uses STM32G4 internal flash for non-volatile storage.

**Flash Parameters:**
- **Page Used:** Page 63 (last page of flash)
- **Page Size:** 2 KB (2048 bytes)
- **Write Granularity:** 64-bit (8 bytes)
- **Erase Granularity:** Full page (2048 bytes)

**Storage Layout (Page 63):**
```
Offset 0x000: fds_params_t structure (24 bytes, padded to 32 bytes)
Offset 0x020: Reserved for future use
...
Offset 0x7FF: End of page
```

### 9.2 Configuration Persistence Mechanism

#### 9.2.1 Write Sequence (Internal Firmware)

1. Controller modifies registers in Block 1 (0x000-0x005)
2. Controller writes 1 to UPDATE_NONVOLATILE_STORAGE (0x057)
3. Firmware unlocks flash controller
4. Firmware erases Page 63
5. Firmware writes new configuration to Page 63
6. Firmware locks flash controller
7. Firmware clears UPDATE_NONVOLATILE_STORAGE (0x057)

**Duration:** 100-200 ms (blocking operation)

#### 9.2.2 Read Sequence (Boot-Time)

1. Firmware reads Page 63 into RAM (fds_params_t structure)
2. Firmware validates configuration (basic sanity checks)
3. If valid: Apply configuration to peripherals (CAN, UART)
4. If invalid: Use defaults (MODULE_NUMBER=0x50, UART_BAUD_RATE=460800, etc.)

**Duration:** <1 ms (non-blocking during normal operation)

### 9.3 Flash Write Limitations

#### 9.3.1 Endurance

**STM32G4 Flash Endurance:** 10,000 erase/write cycles (minimum, per datasheet)

**Practical Limits:**
- Configuration changes: Unlimited (infrequent)
- Do NOT write on every control cycle
- Do NOT write more than 10,000 times over device lifetime

**Recommended Usage:**
- Write only during initial configuration
- Write only when user explicitly changes settings
- Avoid automated runtime writes

#### 9.3.2 Power Loss During Write

**Risk:** If power is lost during flash write (Step 4-5 of Section 9.2.1), flash page may be corrupted.

**Mitigation:**
1. Firmware validates configuration on boot (Section 9.2.2 Step 2)
2. If validation fails, firmware uses safe defaults
3. Controller should avoid triggering writes during critical operations

**Best Practice:**
- Perform configuration writes during safe periods (e.g., system startup)
- Avoid writes during active prosthetic use
- Implement timeout monitoring (poll 0x057 for completion)

### 9.4 Default Configuration

If flash contains invalid or corrupted data, firmware uses these defaults:

| Parameter | Default Value | Register |
|-----------|---------------|----------|
| MODULE_NUMBER | 0x50 | 0x000 |
| UART_BAUD_RATE | 460800 | 0x001 |
| FDCAN_NBRP | 2 | 0x002 |
| FDCAN_NTSEG1 | 135 | 0x003 |
| FDCAN_NTSEG2 | 34 | 0x004 |
| UNUSED_ZEROPAD | 0 | 0x005 |

**CAN Bit Rate:** 500 kbit/s (calculated from defaults)

### 9.5 Configuration Validation

The firmware performs basic validation on boot:

**Validation Checks:**
1. MODULE_NUMBER in range [0x01, 0x7FE]
2. UART_BAUD_RATE in range [1200, 1000000]
3. FDCAN_NBRP in range [1, 512]
4. FDCAN_NTSEG1 in range [2, 256]
5. FDCAN_NTSEG2 in range [2, 128]

**Failure Action:** If any check fails, use default configuration.

**Note:** Validation is basic. Invalid but in-range values (e.g., NBRP=500) will not be caught.

---

## 10. Physical and Electrical Characteristics

### 10.1 Microcontroller Platform

| Parameter | Value |
|-----------|-------|
| MCU | STM32G431xx |
| Core | ARM Cortex-M4F |
| Clock Frequency | 170 MHz (max) |
| Flash | 128 KB |
| RAM | 32 KB |
| Package | LQFP64 (typical) |

### 10.2 CAN Interface

| Parameter | Min | Typ | Max | Unit | Notes |
|-----------|-----|-----|-----|------|-------|
| Transceiver Supply Voltage | 4.5 | 5.0 | 5.5 | V | Integrated 5V CAN transceiver |
| Differential Voltage (CANH-CANL) Dominant | 1.5 | 2.0 | 3.0 | V | Bus dominant state |
| Differential Voltage (CANH-CANL) Recessive | -0.5 | 0 | 0.05 | V | Bus recessive state |
| Common Mode Voltage | 2.0 | 2.5 | 3.0 | V | Both CANH and CANL |
| Bit Rate (nominal) | 125 | 500 | 1000 | kbit/s | Default: 500 kbit/s, classic CAN compliant |
| Bus Load (max recommended) | - | 60 | 80 | % | For reliable operation |
| Termination Resistance | - | 120 | - | Ω | Integrated on-board |

**Integrated Components:**
- 5V CAN transceiver (on-board)
- 120Ω termination resistor (on-board)

**External Connection:**
- CANH and CANL differential pair to CAN bus
- Device acts as bus termination node (place at bus end or disable external terminations)

### 10.3 UART Interface

| Parameter | Min | Typ | Max | Unit | Notes |
|-----------|-----|-----|-----|------|-------|
| Logic High (TX) | 2.0 | 3.3 | 3.6 | V | MCU UART output |
| Logic Low (TX) | 0 | 0 | 0.4 | V | MCU UART output |
| Input Threshold (RX) | 0.8 | - | 2.0 | V | MCU UART input |
| Baud Rate | 9600 | 460800 | 1000000 | bps | See Section 4.2.2 |
| Cable Length (460800 bps) | - | 1 | 5 | m | Depends on cable quality |
| Cable Length (115200 bps) | - | 10 | 30 | m | Lower baud = longer cable |

**External Components Required:**
- RS-232 level shifter (e.g., MAX3232) for RS-232 signals
- RS-485 transceiver (e.g., MAX485) for differential signals
- Termination resistors (for RS-485, 120Ω typical)

**Note:** Ability Hand uses 3.3V logic UART. Direct connection possible if converter outputs 3.3V UART.

### 10.4 Power Supply

| Parameter | Min | Typ | Max | Unit | Notes |
|-----------|-----|-----|-----|------|-------|
| Supply Voltage (VDD) | 3.0 | 3.3 | 3.6 | V | MCU supply |
| Supply Current (idle) | 40 | 50 | 70 | mA | No active communication |
| Supply Current (active) | 50 | 60 | 80 | mA | 1 kHz control loop |
| Supply Current (flash write) | 70 | 80 | 100 | mA | Peak during write |
| Power Consumption | 150 | 200 | 300 | mW | Total system |

**Decoupling Requirements:**
- 100 nF ceramic capacitor on each VDD pin (close to MCU)
- 10 μF bulk capacitor on main supply rail
- Low-ESR capacitors recommended for noise immunity

### 10.5 Environmental

| Parameter | Min | Typ | Max | Unit | Notes |
|-----------|-----|-----|-----|------|-------|
| Operating Temperature | -10 | 25 | 70 | °C | Typical ambient |
| Storage Temperature | -40 | - | 85 | °C | Non-operating |
| Humidity (non-condensing) | 10 | - | 90 | % RH | Operating condition |

**Note:** Extended temperature range (-40 to 85°C) possible with industrial-grade MCU variant.

### 10.6 Mechanical

Mechanical dimensions depend on carrier board design. Typical implementations:

| Board Form Factor | Dimensions (mm) | Notes |
|-------------------|-----------------|-------|
| Custom PCB | 40 × 30 | Typical compact design |
| Development Board | 50 × 70 | Includes debugging headers |

**Connector Recommendations:**
- **CAN:** Screw terminal or DB9
- **UART:** JST-XH or screw terminal
- **Power:** JST-XH or barrel jack

---

## 11. Appendices

---

### Appendix A: Revision History

| Revision | Date | Author | Description |
|----------|------|--------|-------------|
| 1.0 | 2025-11-17 | Auto-generated | Initial release |

---

### Appendix B: Glossary

| Term | Definition |
|------|------------|
| ABH | Ability Hand - Psyonic's prosthetic hand device |
| CAN | Controller Area Network - Serial communication protocol |
| DARTT | Dual Address Real-Time Transport - Block memory protocol over CAN |
| FDCAN | Flexible Data-rate CAN - Enhanced CAN protocol (ISO 11898-1) |
| FSR | Force-Sensitive Resistor - Pressure sensor |
| HDLC | High-Level Data Link Control - Byte stuffing protocol (RFC 1662) |
| ICD | Interface Control Document - Specification document |
| LSB | Least Significant Bit/Byte |
| MSB | Most Significant Bit/Byte |
| NV | Non-Volatile - Data retained across power cycles |
| PPP | Point-to-Point Protocol - HDLC framing variant |
| RO | Read-Only - Register cannot be written |
| R/W | Read/Write - Register can be read and written |
| UART | Universal Asynchronous Receiver/Transmitter - Serial interface |
| WO | Write-Only - Writing triggers an action |

---

## Document Approval

| Role | Name | Signature | Date |
|------|------|-----------|------|
| Author | Auto-generated | | 2025-11-17 |
| Technical Reviewer | | | |
| Project Manager | | | |

---

**END OF DOCUMENT**
