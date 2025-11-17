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

- **CAN interface** using the DARTT (Direct Access Register Table Transfer) protocol
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
| RFC 1662 | PPP in HDLC-like Framing | STD 51 |

### 2.2 Reference Documents

| Standard | Title |
|----------|-------|
| ISO 11898-1 | Controller Area Network (CAN) - Part 1: Data link layer |
| MIL-STD-498 | Software Development and Documentation |
| STM32G4 Reference Manual | RM0440 Rev 7 |

---

## 3. Interface Overview

### 3.1 System Context

```
┌──────────────────┐         CAN Bus          ┌────────────────────┐         UART          ┌─────────────────┐
│  CAN Controller  │◄────────────────────────►│ FDCAN-ABH Converter│◄────────────────────►│  Ability Hand   │
│  (DARTT Client)  │   DARTT Protocol         │   (This Device)    │  Extended Mode API   │   (Prosthetic)  │
└──────────────────┘                          └────────────────────┘                       └─────────────────┘
                                                      │
                                                      │ Flash
                                                      ▼
                                              ┌───────────────┐
                                              │   Non-Volatile│
                                              │   Storage     │
                                              └───────────────┘
```

### 3.2 Interface Summary

| Interface | Direction | Protocol | Purpose |
|-----------|-----------|----------|---------|
| CAN RX | Input | DARTT | Receive commands and configuration from controller |
| CAN TX | Output | DARTT | Send feedback and status to controller |
| UART TX | Output | Ability Hand API | Send motor commands to prosthetic |
| UART RX | Input | Ability Hand API | Receive feedback from prosthetic |
| Flash | Storage | N/A | Persist configuration across power cycles |

### 3.3 Key Features

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
- **Nominal Bit Rate:** 800 kbit/s (default, configurable)
- **Data Bit Rate:** Same as nominal (FDCAN not utilizing data phase speed-up)
- **Transceiver:** External CAN transceiver required
- **Termination:** External 120Ω termination required at bus ends

#### 4.1.2 CAN Addressing

The converter uses a configurable CAN arbitration ID scheme:

| Parameter | Default Value | Configuration Register |
|-----------|---------------|------------------------|
| Module Number | 0x50 | MODULE_NUMBER (0x000) |
| Primary CAN ID | module_number | Read from MODULE_NUMBER |
| Complementary CAN ID | 0x7FF - module_number | Calculated (0x7AF default) |

**Addressing Rules:**
- Primary ID used for DARTT block transfers
- Complementary ID used for response messages
- Module number must be unique on the CAN bus
- Valid range: 0x01 - 0x7FE

#### 4.1.3 DARTT Protocol

The DARTT (Direct Access Register Table Transfer) protocol provides block memory read/write access over CAN.

**Key Characteristics:**
- **Word Size:** 32 bits (4 bytes)
- **Addressing:** Word-aligned (addresses are word indexes, not byte offsets)
- **Access Modes:** Block read, block write
- **Endianness:** Little-endian
- **Maximum Block Size:** Implementation-dependent (typically 8 words/CAN frame)

**Frame Format:**
- Standard DARTT frame structure (refer to DARTT-SPEC)
- Supports both read and write transactions
- Atomic 32-bit word access guaranteed

### 4.2 UART Interface

#### 4.2.1 Physical Layer

- **Standard:** RS-232/RS-485 compatible (electrical layer external)
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

The UART link uses HDLC (High-Level Data Link Control) byte stuffing per RFC 1662 PPP specification.

| Character | Value | Purpose |
|-----------|-------|---------|
| Frame Delimiter | 0x7E | Start and end of frame marker |
| Escape Character | 0x7D | Escape special characters in payload |
| Escape Mask | 0x20 | XOR mask applied to escaped bytes |

**Stuffing Rules:**
1. All frames begin and end with 0x7E
2. If 0x7E appears in payload, transmit as: 0x7D 0x5E (0x7E XOR 0x20)
3. If 0x7D appears in payload, transmit as: 0x7D 0x5D (0x7D XOR 0x20)

**Unstuffing Rules (RX):**
- Firmware automatically unstuffs received data
- Decoded data available in UART_RX_DECODED buffer
- Valid byte count in NBYTES_DECODED_UART

**Stuffing Requirements (TX Pass-Through Mode):**
- Controller MUST perform stuffing before writing to UART_TX_MEM
- Failure to stuff will result in protocol errors

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

Refer to Appendix A for complete Ability Hand protocol details.

---

## 5. Memory Map and Register Definitions

### 5.1 Memory Map Overview

The FDCAN-ABH Converter exposes all configurable parameters and operational data through a DARTT-accessible memory map. All registers are organized as 32-bit words with the following access types:

- **R/W**: Read/Write - Can be read and modified
- **RO**: Read-Only - Can only be read
- **WO**: Write-Only - Writing triggers an action
- **R/W-NV**: Read/Write Non-Volatile - Persists across power cycles when committed

**IMPORTANT - Address Verification Note:**

The register word indexes documented in this ICD are calculated based on the C structure definitions in the firmware source code (dartt_params.h, fds_params.h, abh_communication.h). Due to potential compiler-dependent struct padding and alignment, **actual word offsets should be verified against the compiled firmware** using one of the following methods:

1. Compile the firmware and use `sizeof()` and `offsetof()` operators to verify byte offsets
2. Use a debugger to inspect the actual memory layout of `dartt_params_t`
3. Consult with the firmware developer for the specific compiler configuration used

The functional block organization, data types, and scaling factors documented herein are accurate and compiler-independent.

#### 5.1.1 Memory Map Structure

| Base Word Index | Block Name | Size (words) | Description |
|-----------------|------------|--------------|-------------|
| 0x000 | Non-Volatile Configuration | 6 | CAN and UART configuration parameters |
| 0x006 | Ability Hand Control API | 33 | High-level motor control interface |
| 0x027 | Register Access Interface | 3 | Direct register read/write (unimplemented) |
| 0x02A | UART Direct Buffers | 40 | Raw UART buffers for pass-through mode |
| 0x052 | System Information | 5 | Firmware version and control flags |

**Total Memory Map Size:** 87 words (348 bytes)

---

### 5.2 Block 1: Non-Volatile Configuration (0x000-0x005)

These registers define persistent configuration parameters stored in flash memory (Page 63). Changes to these registers require setting the `UPDATE_NONVOLATILE_STORAGE` flag (0x057) to persist across power cycles.

| Word Index | Register Name | Type | Access | Default | Description |
|------------|---------------|------|--------|---------|-------------|
| 0x000 | MODULE_NUMBER | uint32_t | R/W-NV | 0x50 | CAN module address. Complementary address: 0x7FF - module_number |
| 0x001 | UART_BAUD_RATE | uint32_t | R/W-NV | 460800 | UART baud rate in bits/second. See Section 4.2.2 for valid values |
| 0x002 | FDCAN_NBRP | uint32_t | R/W-NV | 1 | FDCAN Nominal Bit Rate Prescaler. Range: 1-512 |
| 0x003 | FDCAN_NTSEG1 | uint32_t | R/W-NV | 63 | FDCAN Nominal Time Segment 1. Range: 2-256 |
| 0x004 | FDCAN_NTSEG2 | uint32_t | R/W-NV | 16 | FDCAN Nominal Time Segment 2. Range: 2-128 |
| 0x005 | UNUSED_ZEROPAD | uint32_t | RO | 0 | Alignment padding for 64-bit flash alignment requirement |

**CAN Bit Rate Calculation:**

```
Nominal_Bit_Rate = 64_MHz / (NBRP × (1 + NTSEG1 + NTSEG2))
```

**Default Example:**
```
64_MHz / (1 × (1 + 63 + 16)) = 64_MHz / 80 = 800 kbit/s
```

**Configuration Notes:**
- Changes take effect after power cycle or system reset
- Invalid CAN timing parameters may cause bus communication failure
- Sample point = (1 + NTSEG1) / (1 + NTSEG1 + NTSEG2) × 100%
- Default sample point: 80%

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

**Valid Range:**
- 0° = 0
- 75° = 16384
- 150° = 32767

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
| 4 | Verify CAN timing | 0x002-0x004 | NBRP=1, NTSEG1=63, NTSEG2=16 | Confirm 800 kbit/s |

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
| FDCAN_NBRP | 1 | 0x002 |
| FDCAN_NTSEG1 | 63 | 0x003 |
| FDCAN_NTSEG2 | 16 | 0x004 |
| UNUSED_ZEROPAD | 0 | 0x005 |

**CAN Bit Rate:** 800 kbit/s (calculated from defaults)

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
| Supply Voltage | 4.5 | 5.0 | 5.5 | V | External transceiver supply |
| Logic High (CANTX) | 2.0 | 3.3 | 3.6 | V | MCU output to transceiver |
| Logic Low (CANTX) | 0 | 0 | 0.4 | V | MCU output to transceiver |
| Input Threshold (CANRX) | 0.8 | - | 2.0 | V | Transceiver to MCU input |
| Bit Rate (nominal) | 10 | 800 | 1000 | kbit/s | Default: 800 kbit/s |
| Bus Load (max recommended) | - | 60 | 80 | % | For reliable operation |

**External Components Required:**
- CAN transceiver (e.g., TI SN65HVD230, NXP TJA1050)
- 120Ω termination resistors (at bus ends)
- Common mode choke (optional, for EMI)

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

### Appendix A: Ability Hand Protocol Summary

This appendix provides a condensed reference for the Ability Hand Extended Mode API protocol. For complete details, refer to **ABILITY-HAND-ICD.pdf**.

#### A.1 Frame Structure

**Standard Movement Control Frame:**
```
[Frame Start] [Address] [Command Header] [Payload (12 bytes)] [Checksum] [Frame End]
     0x7E        1 byte       1 byte        6 × int16_t       1 byte       0x7E
```

**Total unstuffed length:** 16 bytes (excluding frame delimiters)

#### A.2 Command Headers

| Header | Mode | Payload Content | Reply Content |
|--------|------|-----------------|---------------|
| 0x10 | Position control | 6 × position setpoints | Position + Current |
| 0x11 | Position control | 6 × position setpoints | Position + Velocity |
| 0x12 | Position control | 6 × position setpoints | Position + Current + FSR |
| 0x20 | Velocity control | 6 × velocity setpoints | Position + Current |
| 0x21 | Velocity control | 6 × velocity setpoints | Position + Velocity |
| 0x22 | Velocity control | 6 × velocity setpoints | Position + Current + FSR |
| 0x30 | Torque control | 6 × torque setpoints | Position + Current |
| 0x31 | Torque control | 6 × torque setpoints | Position + Velocity |
| 0x32 | Torque control | 6 × torque setpoints | Position + Current + FSR |
| 0x40 | Voltage control | 6 × voltage setpoints | Position + Current |
| 0x41 | Voltage control | 6 × voltage setpoints | Position + Velocity |
| 0x42 | Voltage control | 6 × voltage setpoints | Position + Current + FSR |

#### A.3 Data Scaling

**Position (to hand):**
```
position_digital = (angle_degrees / 150.0) × 32767
```
- Range: 0° to 150°
- Resolution: ~0.0046° per LSB

**Position (from hand):**
```
Same as setpoint scaling
```

**Velocity (to hand):**
```
velocity_digital = (velocity_deg_per_sec / 3000.0) × 32767
```
- Range: 0 to 3000°/s
- Resolution: ~0.092°/s per LSB

**Velocity (from hand):**
```
velocity_rad_per_sec = velocity_digital / 4.0
```
- Different scaling than setpoints!
- Resolution: 0.25 rad/s per LSB

**Current (from hand):**
```
Refer to Ability Hand ICD Section 3.4
```

#### A.4 Checksum Calculation

**Algorithm:** 8-bit two's complement negation

```c
uint8_t calculate_checksum(uint8_t *frame, int length) {
    uint8_t sum = 0;
    for (int i = 0; i < length; i++) {
        sum += frame[i];  // 8-bit addition (overflow wraps)
    }
    return (uint8_t)(-sum);  // Two's complement negation
}
```

**Checksum includes:** Address + Header + Payload (all bytes between frame delimiters except checksum itself)

**Verification:**
```c
bool verify_checksum(uint8_t *frame, int length, uint8_t checksum) {
    uint8_t sum = 0;
    for (int i = 0; i < length; i++) {
        sum += frame[i];
    }
    sum += checksum;
    return (sum == 0);  // Valid if sum of all bytes + checksum = 0
}
```

#### A.5 Reply Frame Structures

**Reply Variant 1 (TX1): Position + Current**
```
[0x7E] [Address] [Header] [6×Position] [6×Current] [Status] [Checksum] [0x7E]
         1 byte    1 byte    12 bytes     12 bytes    2 bytes   1 byte
Total: 29 bytes unstuffed (excluding delimiters)
```

**Reply Variant 2 (TX2): Position + Velocity**
```
[0x7E] [Address] [Header] [6×Position] [6×Velocity] [Status] [Checksum] [0x7E]
         1 byte    1 byte    12 bytes     12 bytes     2 bytes   1 byte
Total: 29 bytes unstuffed (excluding delimiters)
```

**Reply Variant 3 (TX3): Position + Current + FSR**
```
[0x7E] [Address] [Header] [6×Position] [6×Current] [30×FSR] [Status] [Checksum] [0x7E]
         1 byte    1 byte    12 bytes     12 bytes    60 bytes  2 bytes   1 byte
Total: 89 bytes unstuffed (excluding delimiters)
```

**FSR Data:** 30 sensors × 12 bits packed into 45 bytes (60 bytes with padding/alignment)

#### A.6 Temperature Protection

The Ability Hand implements multi-layer thermal protection:

| Mechanism | Threshold | Action |
|-----------|-----------|--------|
| Halt-on-collision | Configurable | Stop motor on excessive current |
| Heuristic shutdown | Configurable | Reduce power on sustained high current |
| Temperature monitoring | 60°C typical | Set bit in HOT_COLD_BITMASK |
| Emergency shutdown | 80°C typical | Disable motor |

**HOT_COLD_BITMASK (from reply):**
- Bit 0: Motor 0 warning
- Bit 1: Motor 1 warning
- ...
- Bit 5: Motor 5 warning

**Recommended Action:** If any bit is set, reduce commanded force/velocity until bit clears.

---

### Appendix B: HDLC Byte Stuffing

HDLC (High-Level Data Link Control) byte stuffing per RFC 1662 PPP specification.

#### B.1 Special Characters

| Character | Value | Purpose |
|-----------|-------|---------|
| Frame Delimiter | 0x7E | Marks start and end of frame |
| Escape Character | 0x7D | Escapes special characters in payload |
| Escape Mask | 0x20 | XOR mask applied to escaped bytes |

#### B.2 Stuffing Algorithm (TX)

**Purpose:** Prevent 0x7E or 0x7D from appearing in frame payload.

**Rules:**
1. Transmit 0x7E at frame start
2. For each payload byte:
   - If byte == 0x7E: Transmit 0x7D 0x5E (0x7E XOR 0x20 = 0x5E)
   - Else if byte == 0x7D: Transmit 0x7D 0x5D (0x7D XOR 0x20 = 0x5D)
   - Else: Transmit byte as-is
3. Transmit 0x7E at frame end

**Pseudocode:**
```c
void hdlc_stuff_and_transmit(uint8_t *frame, int length) {
    uart_transmit(0x7E);  // Frame start

    for (int i = 0; i < length; i++) {
        if (frame[i] == 0x7E) {
            uart_transmit(0x7D);
            uart_transmit(0x5E);
        } else if (frame[i] == 0x7D) {
            uart_transmit(0x7D);
            uart_transmit(0x5D);
        } else {
            uart_transmit(frame[i]);
        }
    }

    uart_transmit(0x7E);  // Frame end
}
```

#### B.3 Unstuffing Algorithm (RX)

**Purpose:** Restore original payload by removing escape sequences.

**Rules:**
1. Discard 0x7E frame delimiters
2. For each received byte:
   - If byte == 0x7D: Read next byte, XOR with 0x20, store result
   - Else: Store byte as-is

**Pseudocode:**
```c
int hdlc_unstuff(uint8_t *stuffed, int stuffed_len, uint8_t *unstuffed) {
    int j = 0;  // Unstuffed buffer index
    bool escape_next = false;

    for (int i = 0; i < stuffed_len; i++) {
        if (stuffed[i] == 0x7E) {
            continue;  // Skip frame delimiters
        } else if (stuffed[i] == 0x7D) {
            escape_next = true;
        } else {
            if (escape_next) {
                unstuffed[j++] = stuffed[i] ^ 0x20;
                escape_next = false;
            } else {
                unstuffed[j++] = stuffed[i];
            }
        }
    }

    return j;  // Length of unstuffed data
}
```

**Note:** Firmware performs RX unstuffing automatically. Controllers only need to implement stuffing for TX in pass-through mode.

#### B.4 Worst-Case Expansion

**Best case:** No special characters in payload → 0 bytes added (just frame delimiters)

**Worst case:** All bytes are 0x7E or 0x7D → Every byte doubled

**Example:**
- Unstuffed: 16 bytes
- All bytes = 0x7E
- Stuffed: 2 + (16 × 2) + 2 = 36 bytes (2.25× expansion)

**Buffer Sizing Recommendation:**
- If unstuffed frame max = N bytes
- Allocate stuffed buffer = 2×N + 4 bytes (safe margin)

---

### Appendix C: Example Code Snippets

**DISCLAIMER:** Code examples are provided for illustrative purposes only. Production implementations require proper error handling, testing, and validation.

#### C.1 CAN Bit Rate Configuration

```c
/**
 * Calculate CAN timing parameters for desired bit rate.
 *
 * @param desired_bitrate: Target bit rate in kbit/s
 * @param nbrp: Output - Bit Rate Prescaler
 * @param ntseg1: Output - Time Segment 1
 * @param ntseg2: Output - Time Segment 2
 * @return: Actual achieved bit rate (kbit/s)
 */
uint32_t calculate_can_timing(uint32_t desired_bitrate,
                                uint32_t *nbrp,
                                uint32_t *ntseg1,
                                uint32_t *ntseg2) {
    const uint32_t pclk = 64000000;  // 64 MHz peripheral clock
    const uint32_t sample_point = 80;  // 80% sample point

    uint32_t tq_total = pclk / (desired_bitrate * 1000);

    // Find prescaler that gives reasonable TQ count
    for (*nbrp = 1; *nbrp <= 512; (*nbrp)++) {
        uint32_t tq = tq_total / (*nbrp);
        if (tq >= 8 && tq <= 128) {
            *ntseg1 = (tq * sample_point / 100) - 1;
            *ntseg2 = tq - *ntseg1 - 1;
            uint32_t actual_bitrate = pclk / ((*nbrp) * tq);
            return actual_bitrate / 1000;  // Return in kbit/s
        }
    }

    return 0;  // Failed to find valid timing
}

// Example usage:
// uint32_t nbrp, ntseg1, ntseg2;
// uint32_t actual = calculate_can_timing(800, &nbrp, &ntseg1, &ntseg2);
// Write nbrp to register 0x002, ntseg1 to 0x003, ntseg2 to 0x004
```

#### C.2 Position Scaling

```c
/**
 * Convert angle in degrees to digital position value.
 *
 * @param angle_degrees: Angle in degrees (0.0 to 150.0)
 * @return: Digital position value (0 to 32767)
 */
int16_t position_degrees_to_digital(float angle_degrees) {
    if (angle_degrees < 0.0f) angle_degrees = 0.0f;
    if (angle_degrees > 150.0f) angle_degrees = 150.0f;

    return (int16_t)((angle_degrees / 150.0f) * 32767.0f);
}

/**
 * Convert digital position value to angle in degrees.
 *
 * @param position_digital: Digital position value (0 to 32767)
 * @return: Angle in degrees (0.0 to 150.0)
 */
float position_digital_to_degrees(int16_t position_digital) {
    return ((float)position_digital / 32767.0f) * 150.0f;
}

// Example usage:
// int16_t setpoint = position_degrees_to_digital(90.0);  // 90° → 19660
// float actual = position_digital_to_degrees(feedback);
```

#### C.3 Velocity Scaling

```c
/**
 * Convert velocity in deg/s to digital value (for setpoints).
 *
 * @param velocity_deg_per_sec: Velocity in degrees/second (0.0 to 3000.0)
 * @return: Digital velocity value (0 to 32767)
 */
int16_t velocity_to_digital(float velocity_deg_per_sec) {
    if (velocity_deg_per_sec < 0.0f) velocity_deg_per_sec = 0.0f;
    if (velocity_deg_per_sec > 3000.0f) velocity_deg_per_sec = 3000.0f;

    return (int16_t)((velocity_deg_per_sec / 3000.0f) * 32767.0f);
}

/**
 * Convert digital velocity feedback to rad/s.
 *
 * @param velocity_digital: Digital velocity value from hand
 * @return: Velocity in radians/second
 */
float velocity_digital_to_rad_per_sec(int16_t velocity_digital) {
    return (float)velocity_digital / 4.0f;
}

// Example usage:
// int16_t setpoint = velocity_to_digital(1500.0);  // 1500°/s → 16384
// float actual = velocity_digital_to_rad_per_sec(feedback);
```

#### C.4 Checksum Calculation

```c
/**
 * Calculate Ability Hand checksum (2's complement negation).
 *
 * @param frame: Frame data (address through payload, excluding checksum)
 * @param length: Number of bytes to checksum
 * @return: Checksum byte
 */
uint8_t calculate_abh_checksum(uint8_t *frame, int length) {
    uint8_t sum = 0;
    for (int i = 0; i < length; i++) {
        sum += frame[i];
    }
    return (uint8_t)(-sum);  // Two's complement negation
}

/**
 * Verify received frame checksum.
 *
 * @param frame: Complete frame (address through checksum)
 * @param length: Total frame length including checksum
 * @return: true if checksum valid, false otherwise
 */
bool verify_abh_checksum(uint8_t *frame, int length) {
    uint8_t sum = 0;
    for (int i = 0; i < length; i++) {
        sum += frame[i];
    }
    return (sum == 0);
}

// Example usage:
// uint8_t frame[15] = {address, header, ...payload...};
// uint8_t checksum = calculate_abh_checksum(frame, 15);
// frame[15] = checksum;
```

#### C.5 HDLC Stuffing

```c
/**
 * Perform HDLC byte stuffing on frame.
 *
 * @param unstuffed: Input unstuffed frame
 * @param unstuffed_len: Length of unstuffed frame
 * @param stuffed: Output buffer for stuffed frame
 * @return: Length of stuffed frame
 */
int hdlc_stuff(uint8_t *unstuffed, int unstuffed_len, uint8_t *stuffed) {
    int j = 0;

    stuffed[j++] = 0x7E;  // Frame start

    for (int i = 0; i < unstuffed_len; i++) {
        if (unstuffed[i] == 0x7E) {
            stuffed[j++] = 0x7D;
            stuffed[j++] = 0x5E;
        } else if (unstuffed[i] == 0x7D) {
            stuffed[j++] = 0x7D;
            stuffed[j++] = 0x5D;
        } else {
            stuffed[j++] = unstuffed[i];
        }
    }

    stuffed[j++] = 0x7E;  // Frame end

    return j;
}

// Example usage (pass-through mode):
// uint8_t unstuffed_frame[16] = {...};
// uint8_t stuffed_frame[36];  // Worst case: 2×16 + 4
// int stuffed_len = hdlc_stuff(unstuffed_frame, 16, stuffed_frame);
//
// Write stuffed_frame to UART_TX_MEM (0x03F-0x051)
// Write stuffed_len to NBYTES_WRITE_UART (0x052)
```

---

### Appendix D: Troubleshooting Guide

#### D.1 CAN Communication Issues

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| No CAN frames received | Module number mismatch | Verify MODULE_NUMBER (0x000) matches controller's target ID |
| | CAN bus not terminated | Install 120Ω resistors at both bus ends |
| | Wrong bit rate | Verify CAN timing registers (0x002-0x004) |
| Intermittent CAN errors | High bus load | Reduce update rate or optimize frame packing |
| | EMI / crosstalk | Add common-mode choke, improve shielding |
| | Long cable runs | Reduce cable length or lower bit rate |
| Transmit failures | Bus-off state | Check error counters, verify termination |
| | Arbitration loss | Lower priority CAN ID if critical |

#### D.2 UART Communication Issues

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| No UART reply | Wrong baud rate | Verify UART_BAUD_RATE (0x001) matches hand |
| | Wrong device address | Verify ABH_ADDRESS (0x006) = 0x50 (default) |
| | UART cable disconnected | Check physical connection |
| | Hand not powered | Verify hand power supply |
| Corrupted data | Framing errors | Check baud rate, ensure 8N1 format |
| | HDLC stuffing error | Verify stuffing implementation (pass-through mode) |
| | Noise on line | Reduce cable length, add shielding |
| Timeout errors | Reply too slow | Increase ABH_READ_TIMEOUT (0x02A) |
| | Hand unresponsive | Check hand status, power cycle |
| Checksum failures | Incorrect calculation | Verify checksum algorithm (Appendix A.4) |
| | Bit errors on line | Improve cable quality, reduce length |

#### D.3 Configuration Issues

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| Configuration not persisting | Flash write not triggered | Write 1 to UPDATE_NONVOLATILE_STORAGE (0x057) |
| | Power lost during write | Retry flash write, verify completion |
| | Flash corrupted | Manually rewrite all Block 1 registers |
| Wrong CAN bit rate after boot | Flash contains invalid data | Firmware using defaults, rewrite configuration |
| Flash write timeout | Normal operation | Wait 100-200 ms, poll 0x057 until 0 |

#### D.4 Performance Issues

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| High latency | Low UART baud rate | Increase to 460800 or 921600 bps |
| | High CAN bus load | Reduce update rate or use block writes |
| | Controller polling inefficiency | Use interrupt-driven or event-based reads |
| Position tracking errors | Setpoint update rate too low | Increase control loop frequency |
| | Incorrect scaling | Verify position_degrees_to_digital() |
| | Mechanical binding | Check hand hardware |
| Temperature warnings (0x025) | Excessive current | Reduce force commands |
| | High ambient temperature | Improve cooling, reduce duty cycle |
| | Sustained high velocity | Reduce velocity setpoints |

#### D.5 Pass-Through Mode Issues

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| Frame not transmitted | NBYTES_WRITE_UART not written | Write byte count to 0x052 to trigger TX |
| | Byte count = 0 | Ensure count reflects actual frame length |
| Framing errors | HDLC stuffing missing | Implement stuffing (Appendix B.2) |
| | Incorrect frame delimiters | Ensure 0x7E at start and end |
| Reply parsing errors | Assumed automatic parsing | Pass-through mode requires manual parsing |
| | Wrong buffer read | Read UART_RX_DECODED (0x02B), not UART_TX_MEM |

---

### Appendix E: Revision History

| Revision | Date | Author | Description |
|----------|------|--------|-------------|
| 1.0 | 2025-11-17 | Auto-generated | Initial release |

---

### Appendix F: Glossary

| Term | Definition |
|------|------------|
| ABH | Ability Hand - Psyonic's prosthetic hand device |
| CAN | Controller Area Network - Serial communication protocol |
| DARTT | Direct Access Register Table Transfer - Block memory protocol over CAN |
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
