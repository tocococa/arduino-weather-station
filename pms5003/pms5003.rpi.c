// PMS5003 interface for Raspberry Pi (WiringPi + wiringSerial)
// Build: gcc -O2 -o pms5003 pms5003.rpi.c -lwiringPi

#include <wiringPi.h>
#include <wiringSerial.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#define SERIAL_PRT       "/dev/serial0"   // Primary UART alias
#define SERIAL_BAUD_RATE 9600
#define EN_PIN           7                // wPi 7 (BCM 4) if you gate SET via transistor
#define MSG_LEN          32
#define READ_TIMEOUT_MS  1500

struct pms5003data {
    uint16_t framelen;
    uint16_t pm10_standard, pm25_standard, pm100_standard;        // PM1.0, PM2.5, PM10 (CF=1)
    uint16_t pm10_env, pm25_env, pm100_env;                        // PM1.0, PM2.5, PM10 (ATM)
    uint16_t particles_03um, particles_05um, particles_10um;
    uint16_t particles_25um, particles_50um, particles_100um;
    uint16_t unused;
    uint16_t checksum;
};

static bool readFrame(int fd, uint8_t buf[MSG_LEN], unsigned int timeoutMs) {
    unsigned int start = millis();
    int state = 0; // 0=looking for 0x42, 1=looking for 0x4D

    while ((millis() - start) < timeoutMs) {
        int avail = serialDataAvail(fd);
        if (avail > 0) {
            int b = serialGetchar(fd);
            if (state == 0) {
                if (b == 0x42) {
                    buf[0] = 0x42;
                    state = 1;
                }
            } else {
                if (b == 0x4D) {
                    buf[1] = 0x4D;
                    // Read remaining 30 bytes
                    int i = 2;
                    unsigned int innerStart = millis();
                    while (i < MSG_LEN && (millis() - innerStart) < timeoutMs) {
                        if (serialDataAvail(fd) > 0) {
                            buf[i++] = (uint8_t)serialGetchar(fd);
                        } else {
                            delay(1);
                        }
                    }
                    return i == MSG_LEN;
                } else {
                    state = 0; // false start, restart sync
                }
            }
        } else if (avail < 0) {
            // serialDataAvail error
            return false;
        } else {
            delay(1);
        }
    }
    return false;
}

static bool parseAndValidate(struct pms5003data *data, const uint8_t buf[MSG_LEN]) {
    // Checksum: sum of bytes [0..29] equals big-endian at [30..31]
    uint16_t sum = 0;
    for (int i = 0; i < 30; i++) sum += buf[i];
    uint16_t checksum = ((uint16_t)buf[30] << 8) | buf[31];
    if (sum != checksum) return false;

    #define BE16(i) ( (uint16_t)((uint16_t)buf[(i)] << 8 | (uint16_t)buf[(i)+1]) )
    data->framelen        = BE16(2);
    data->pm10_standard   = BE16(4);   // PM1.0 (CF=1)
    data->pm25_standard   = BE16(6);   // PM2.5 (CF=1)
    data->pm100_standard  = BE16(8);   // PM10  (CF=1)
    data->pm10_env        = BE16(10);  // PM1.0 (ATM)
    data->pm25_env        = BE16(12);  // PM2.5 (ATM)
    data->pm100_env       = BE16(14);  // PM10  (ATM)
    data->particles_03um  = BE16(16);
    data->particles_05um  = BE16(18);
    data->particles_10um  = BE16(20);
    data->particles_25um  = BE16(22);
    data->particles_50um  = BE16(24);
    data->particles_100um = BE16(26);
    data->unused          = BE16(28);
    data->checksum        = checksum;
    #undef BE16

    return true;
}

bool readPMSdata(struct pms5003data *data, int fd) {
    if (fd < 0) {
        fprintf(stderr, "Serial port not open: %s\n", strerror(errno));
        return false;
    }
    uint8_t buf[MSG_LEN];
    if (!readFrame(fd, buf, READ_TIMEOUT_MS)) {
        return false;
    }
    return parseAndValidate(data, buf);
}

int main(void) {
    // WiringPi (wPi) numbering
    if (wiringPiSetup() < 0) {
        fprintf(stderr, "wiringPiSetup failed\n");
        return 1;
    }

    // Drive SET/enable line if wired through a transistor to BCM4 (wPi 7).
    pinMode(EN_PIN, OUTPUT);
    digitalWrite(EN_PIN, HIGH); // keep sensor enabled (SET high)

    int fd = serialOpen(SERIAL_PRT, SERIAL_BAUD_RATE);
    if (fd < 0) {
        fprintf(stderr, "Unable to open serial port %s: %s\n", SERIAL_PRT, strerror(errno));
        return 1;
    }

    printf("PMS5003 reader started on %s (baud %d)\n", SERIAL_PRT, SERIAL_BAUD_RATE);

    struct pms5003data data;
    for (;;) {
        if (readPMSdata(&data, fd)) {
            printf("\n---------------------------------------\n");
            printf("Concentration Units (standard)\n");
            printf("PM 1.0: %u\n",  data.pm10_standard);
            printf("PM 2.5: %u\n",  data.pm25_standard);
            printf("PM 10 : %u\n",  data.pm100_standard);
            printf("---------------------------------------\n");
            printf("Concentration Units (environmental)\n");
            printf("PM 1.0: %u\n",  data.pm10_env);
            printf("PM 2.5: %u\n",  data.pm25_env);
            printf("PM 10 : %u\n",  data.pm100_env);
            printf("---------------------------------------\n");
            printf(">0.3um:  %u\n", data.particles_03um);
            printf(">0.5um:  %u\n", data.particles_05um);
            printf(">1.0um:  %u\n", data.particles_10um);
            printf(">2.5um:  %u\n", data.particles_25um);
            printf(">5.0um:  %u\n", data.particles_50um);
            printf(">10.0um: %u\n", data.particles_100um);
            printf("---------------------------------------\n");
        } else {
            // no complete frame or checksum fail — retry shortly
            delay(50);
        }
    }
    // Not reached; add cleanup if you plan to exit:
    // serialClose(fd); digitalWrite(EN_PIN, LOW);
    // return 0;
}