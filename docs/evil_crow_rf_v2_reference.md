# Evil Crow RF v2 - Reference Notes

Source used:
- `Referencia-EvlCrwRf/firmware.ino`
- `Referencia-EvlCrwRf/EvilCrowRF-V2/README.md`
- `Referencia-EvlCrwRf/ELECHOUSE_CC1101_SRC_DRV.*`

## Confirmed pinout (radio path)

Shared CC1101 SPI bus:
- `SCK = GPIO14`
- `MISO = GPIO12`
- `MOSI = GPIO13`

CC1101 #0:
- `CS = GPIO5`
- `GDO0/RX edge = GPIO4`
- `TX raw pin = GPIO2`

CC1101 #1:
- `CS = GPIO27`
- `GDO0/RX edge = GPIO26`
- `TX raw pin = GPIO25`

Notes:
- The reference firmware uses both CC1101 modules on the same SPI lines and switches by CS.
- Interrupt-driven raw RX is attached on `GPIO4` and `GPIO26` (one per module).

## Other confirmed board wiring (non-radio core)

MicroSD SPI in reference firmware:
- `SCK = GPIO18`
- `MISO = GPIO19`
- `MOSI = GPIO23`
- `CS = GPIO22`

## Reference flash/tool settings

From upstream README (Arduino IDE settings):
- Board: `ESP32 Dev Module`
- Flash Size: `4MB`
- CPU Frequency: `80MHz`
- Flash Frequency: `40MHz`
- Flash Mode: `DIO`

## Upload issue seen in this workspace

Current device consistently fails during flash write with:
- `Failed to communicate with the flash chip`
- `Serial data stream stopped`

This usually indicates hardware-level flashing instability (USB cable/port noise, CH340 link quality, board power, or interference from attached peripherals during ROM flashing), not an application compile problem.

Recommended on-device checks before next upload:
- Use a short high-quality USB cable and direct USB port (no hub).
- Disconnect external RF peripherals/wires if any are attached.
- Hold BOOT while starting upload, release after connection.
- Keep upload speed at `115200`.

