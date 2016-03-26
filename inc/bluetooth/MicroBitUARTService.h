#ifndef MICROBIT_UART_SERVICE_H
#define MICROBIT_UART_SERVICE_H

#include "mbed.h"
#include "ble/UUID.h"
#include "ble/BLE.h"
#include "MicroBitConfig.h"
#include "MicroBitSerial.h"

#define MICROBIT_UART_S_DEFAULT_BUF_SIZE    20

#define MICROBIT_UART_S_EVT_DELIM_MATCH     1
#define MICROBIT_UART_S_EVT_HEAD_MATCH      2
#define MICROBIT_UART_S_EVT_RX_FULL         3

class MicroBitUARTService
{
    uint8_t* rxBuffer;

    uint8_t* txBuffer;

    uint8_t rxBufferHead;
    uint8_t rxBufferTail;
    uint8_t rxBufferSize;

    uint8_t txBufferSize;

    uint32_t txCharacteristicHandle;

    // Bluetooth stack we're running on.
    BLEDevice           &ble;

    //delimeters used for matching on receive.
    ManagedString delimeters;

    //a variable used when a user calls the eventAfter() method.
    int rxBuffHeadMatch;

    /**
      * A callback function for whenever a Bluetooth device writes to our TX characteristic.
      */
    void onDataWritten(const GattWriteCallbackParams *params);

    /**
      * An internal method that copies values from a circular buffer to a linear buffer.
      *
      * @param circularBuff a pointer to the source circular buffer
      * @param circularBuffSize the size of the circular buffer
      * @param linearBuff a pointer to the destination linear buffer
      * @param tailPosition the tail position in the circular buffer you want to copy from
      * @param headPosition the head position in the circular buffer you want to copy to
      *
      * @note this method assumes that the linear buffer has the appropriate amount of
      *       memory to contain the copy operation
      */
    void circularCopy(uint8_t *circularBuff, uint8_t circularBuffSize, uint8_t *linearBuff, uint16_t tailPosition, uint16_t headPosition);

    public:

    /**
     * Constructor for the UARTService.
     * @param _ble an instance of BLEDevice
     * @param rxBufferSize the size of the rxBuffer
     * @param txBufferSize the size of the txBuffer
     *
     * @note defaults to 20
     */
    MicroBitUARTService(BLEDevice &_ble, uint8_t rxBufferSize = MICROBIT_UART_S_DEFAULT_BUF_SIZE, uint8_t txBufferSize = MICROBIT_UART_S_DEFAULT_BUF_SIZE);

    /**
      * Retreives a single character from our RxBuffer.
      *
      * @param mode the selected mode, one of: ASYNC, SYNC_SPINWAIT, SYNC_SLEEP. Each mode
      *        gives a different behaviour:
      *
      *            ASYNC - Will attempt to read a single character, and return immediately
      *
      *            SYNC_SPINWAIT - will return MICROBIT_INVALID_PARAMETER
      *
      *            SYNC_SLEEP - Will configure the event and block the current fiber until the
      *                         event is received.
      *
      * @return MICROBIT_INVALID_PARAMETER if the mode given is SYNC_SPINWAIT, a character or MICROBIT_NO_DATA
      */
    int getc(MicroBitSerialMode mode = SYNC_SLEEP);

    /**
      * places a single character into our transmission buffer,
      *
      * @param c the character to transmit
      *
      * @return the number of characters written (0, or 1).
      */
    int putc(char c);

    /**
      * Copies characters into the buffer used for Transmitting to the central device.
      *
      * @param buf a buffer containing length number of bytes.
      * @param length the size of the buffer.
      *
      * @return the number of characters copied into the buffer
      *
      * @note no modes for sending are available at the moment, due to interrupt overhead.
      */
    int send(const uint8_t *buf, int length);

    /**
      * Copies characters into the buffer used for Transmitting to the central device.
      *
      * @param s the string to transmit
      *
      * @return the number of characters copied into the buffer
      *
      * @note no modes for sending are available at the moment, due to interrupt overhead.
      */
    int send(ManagedString s);

    /**
      * Reads a number of characters from the rxBuffer and fills user given buffer.
      *
      * @param buf a pointer to a buffer of len bytes.
      * @param len the size of the user allocated buffer
      * @param mode the selected mode, one of: ASYNC, SYNC_SPINWAIT, SYNC_SLEEP. Each mode
      *        gives a different behaviour:
      *
      *            ASYNC - Will attempt to read all available characters, and return immediately
      *                    until the buffer limit is reached
      *
      *            SYNC_SPINWAIT - will return MICROBIT_INVALID_PARAMETER
      *
      *            SYNC_SLEEP - Will first of all determine whether the given number of characters
      *                         are available in our buffer, if not, it will set an event and sleep
      *                         until the number of characters are avaialable.
      *
      * @return the number of characters digested
      */
    int read(uint8_t *buf, int len, MicroBitSerialMode mode = SYNC_SLEEP);

    /**
      * Reads a number of characters from the rxBuffer and returns them as a ManagedString
      *
      * @param len the number of characters to read.
      * @param mode the selected mode, one of: ASYNC, SYNC_SPINWAIT, SYNC_SLEEP. Each mode
      *        gives a different behaviour:
      *
      *            ASYNC - Will attempt to read all available characters, and return immediately
      *                    until the buffer limit is reached
      *
      *            SYNC_SPINWAIT - will return MICROBIT_INVALID_PARAMETER
      *
      *            SYNC_SLEEP - Will first of all determine whether the given number of characters
      *                         are available in our buffer, if not, it will set an event and sleep
      *                         until the number of characters are avaialable.
      *
      * @return an empty ManagedString on error, or a ManagedString containing characters
      */
    ManagedString read(int len, MicroBitSerialMode mode = SYNC_SLEEP);

    /**
      * Reads characters until a character matches one of the given delimeters
      *
      * @param delimeters the number of characters to match against
      * @param mode the selected mode, one of: ASYNC, SYNC_SPINWAIT, SYNC_SLEEP. Each mode
      *        gives a different behaviour:
      *
      *            ASYNC - Will attempt read the immediate buffer, and look for a match.
      *                    If there isn't, an empty ManagedString will be returned.
      *
      *            SYNC_SPINWAIT - will return MICROBIT_INVALID_PARAMETER
      *
      *            SYNC_SLEEP - Will first of all consider the characters in the immediate buffer,
      *                         if a match is not found, it will block on an event, fired when a
      *                         character is matched.
      *
      * @return an empty ManagedString on error, or a ManagedString containing characters
      */
    ManagedString readUntil(ManagedString delimeters, MicroBitSerialMode mode = SYNC_SLEEP);

    /**
      * Configures an event to be fired on a match with one of the delimeters.
      *
      * @param delimeters the characters to match received characters against e.g. ManagedString("\r\n")
      * @param mode the selected mode, one of: ASYNC, SYNC_SPINWAIT, SYNC_SLEEP. Each mode
      *        gives a different behaviour:
      *
      *            ASYNC - Will configure the event and return immediately.
      *
      *            SYNC_SPINWAIT - will return MICROBIT_INVALID_PARAMETER
      *
      *            SYNC_SLEEP - Will configure the event and block the current fiber until the
      *                         event is received.
      *
      * @return MICROBIT_INVALID_PARAMETER if the mode given is SYNC_SPINWAIT, otherwise MICROBIT_OK.
      *
      * @note delimeters are matched on a per byte basis.
      */
    int eventOn(ManagedString delimeters, MicroBitSerialMode mode = ASYNC);

    /**
      * Configures an event to be fired after "len" characters.
      *
      * @param len the number of characters to wait before triggering the event
      * @param mode the selected mode, one of: ASYNC, SYNC_SPINWAIT, SYNC_SLEEP. Each mode
      *        gives a different behaviour:
      *
      *            ASYNC - Will configure the event and return immediately.
      *
      *            SYNC_SPINWAIT - will return MICROBIT_INVALID_PARAMETER
      *
      *            SYNC_SLEEP - Will configure the event and block the current fiber until the
      *                         event is received.
      *
      * @return MICROBIT_INVALID_PARAMETER if the mode given is SYNC_SPINWAIT, otherwise MICROBIT_OK.
      */
    int eventAfter(int len, MicroBitSerialMode mode = ASYNC);

    /**
      * Determines if we have space in our rxBuff.
      *
      * @return 1 if we have space, 0 if we do not.
      */
    int isReadable();

    /**
      * @return The currently buffered number of bytes in our rxBuff.
      */
    int rxBufferedSize();

    /**
      * @return The currently buffered number of bytes in our txBuff.
      */
    int txBufferedSize();
};

extern const uint8_t  UARTServiceBaseUUID[UUID::LENGTH_OF_LONG_UUID];
extern const uint16_t UARTServiceShortUUID;
extern const uint16_t UARTServiceTXCharacteristicShortUUID;
extern const uint16_t UARTServiceRXCharacteristicShortUUID;

extern const uint8_t  UARTServiceUUID[UUID::LENGTH_OF_LONG_UUID];
extern const uint8_t  UARTServiceUUID_reversed[UUID::LENGTH_OF_LONG_UUID];

extern const uint8_t  UARTServiceTXCharacteristicUUID[UUID::LENGTH_OF_LONG_UUID];
extern const uint8_t  UARTServiceRXCharacteristicUUID[UUID::LENGTH_OF_LONG_UUID];

#endif