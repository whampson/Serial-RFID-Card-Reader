/* Serial RFID Card Reader
*
* File:
*   SerialReader.h
*
* Description:
*   Defines SerialReader class and related constants.
*
* Author:
*   Wes Hampson
*/

#include <Windows.h>

#ifndef SERIAL_READER_H
#define SERIAL_READER_H

const int SERIAL_READ_INTERVAL_TIMEOUT          = 50;
const int SERIAL_READ_TOTAL_TIMEOUT_CONSTANT    = 50;
const int SERIAL_READ_TOTAL_TIMEOUT_MULTIPLIER  = 10;

/// <summary>
/// Provides basic functionality for reading data from a serial port.
/// </summary>
class SerialReader
{
private:
    HANDLE m_hSerial;
    bool m_isReading;
    int m_commPortNumber;

    /// <summary>
    /// Gets a HANDLE from the Win32 API which cooresponds to a COM port.
    /// </summary>
    static HANDLE GetSerialPortHandle(int commPortNumber);

public:
    /// <summary>
    /// Constructor. Takes a COM port number.
    /// </summary>
    SerialReader(int commPortNumber);

    /// <summary>
    /// Opens a connection to the COM port specified in the constructor with
    /// the specified baud rate, data byte size, stop bits, and parity.
    /// </summary>
    bool Initialize(int baudRate, int dataBits, int stopBits, int parity);

    /// <summary>
    /// Closes the connection to the COM port, if open.
    /// </summary>
    void Close();

    /// <summary>
    /// Reads a set number of characters from the COM port, then calls
    /// OnRead() and passes in the character buffer.
    /// </summary>
    void Read(size_t bufSize, void(*OnRead)(char*));
};

#endif
