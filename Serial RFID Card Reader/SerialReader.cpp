/* Serial RFID Card Reader
*
* File:
*   SerialReader.cpp
*
* Description:
*   Initializes SerialReader class functions.
*
* Author:
*   Wes Hampson
*/

#include <cstdio>
#include <Windows.h>

#include "SerialReader.h"

HANDLE SerialReader::GetSerialPortHandle(int commPortNumber)
{
    // Build COM port name as a wide string (COM[n])
    wchar_t szCommPortName[5];
    swprintf(szCommPortName, L"COM%d", commPortNumber);

    // Get COM port handle
    HANDLE hComm;
    hComm = CreateFile(szCommPortName,      // COM port name
        GENERIC_READ,                       // Read data
        0,                                  // No sharing, must be opened with exclusive access
        NULL,                               // Default security attributes
        OPEN_EXISTING,                      // Open existing file (required)
        FILE_FLAG_OVERLAPPED                // Enable overlapped I/O for event-driven reading
            | FILE_FLAG_NO_BUFFERING,       // No OS buffering; we will handle buffering ourselves
        NULL);                              // Template must be null for COM devices

    return hComm;
}

SerialReader::SerialReader(int commPortNumber)
{
    m_commPortNumber = commPortNumber;
    m_hSerial = INVALID_HANDLE_VALUE;
    m_isReading = false;
}

bool SerialReader::Initialize(int baudRate, int dataBits, int stopBits, int parity)
{
    // Close existing handle (if already open)
    Close();

    // Get new handle
    m_hSerial = GetSerialPortHandle(m_commPortNumber);
    if (m_hSerial == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    // Initialize serial port parameters
    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    dcbSerialParams.BaudRate = baudRate;
    dcbSerialParams.ByteSize = dataBits;
    dcbSerialParams.StopBits = stopBits;
    dcbSerialParams.Parity = parity;

    // Initialize timeouts
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = SERIAL_READ_INTERVAL_TIMEOUT;
    timeouts.ReadTotalTimeoutConstant = SERIAL_READ_TOTAL_TIMEOUT_CONSTANT;
    timeouts.ReadTotalTimeoutMultiplier = SERIAL_READ_TOTAL_TIMEOUT_MULTIPLIER;

    // Write parameters and timeouts to port and return
    return SetCommState(m_hSerial, &dcbSerialParams)
        && SetCommTimeouts(m_hSerial, &timeouts)
        && SetCommMask(m_hSerial, EV_RXCHAR);
}

void SerialReader::Close()
{
    if (m_hSerial != INVALID_HANDLE_VALUE)
    {
        m_isReading = false;
        CloseHandle(m_hSerial);
    }
}

void SerialReader::Read(size_t bufSize, void(*OnRead)(char*))
{
    // Allocate buffer
    char* serialBuf = (char*)calloc(bufSize, sizeof(char));

    // Create read event handle and use it for read event
    HANDLE hReadEvent = CreateEvent(NULL, TRUE, FALSE, L"RxEvent");
    OVERLAPPED ovRead = { 0 };
    ovRead.hEvent = hReadEvent;

    DWORD dwBytesRead;

    // Read some data!
    m_isReading = true;
    while (m_isReading)
    {
        // Check for outstanding read event
        if (HasOverlappedIoCompleted(&ovRead))
        {
            // Read data from serial port and store in buffer
            if (!ReadFile(m_hSerial, serialBuf, bufSize, &dwBytesRead, &ovRead))
            {
                DWORD dwError = GetLastError();
                if (dwError != ERROR_IO_PENDING)
                {
                    printf("An error has occured. (%d)\n", dwError);
                    break;
                }
            }
        }

        // Continue if buffer is empty
        if (serialBuf[0] == 0)
        {
            continue;
        }

        // Checks if the overlapped read process has completed
        if (GetOverlappedResult(m_hSerial, &ovRead, &dwBytesRead, FALSE))
        {
            // Do something with data and zero-out the buffer
            OnRead(serialBuf);
            memset(serialBuf, 0, sizeof(serialBuf));
        }
    }

    // Cleanup
    free(serialBuf);
    CloseHandle(hReadEvent);
}
