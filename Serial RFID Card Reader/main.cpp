/* Serial RFID Card Reader
 *
 * File:
 *   main.cpp
 *
 * Description:
 *   Handles program initialization, processes command-line argument, and opens
 *   the serial reader.
 *
 * Author:
 *   Wes Hampson
 */

#include <cstdio>
#include <Windows.h>

#include "SerialReader.h"

const char* APP_NAME = "RFIDReader";

SerialReader* reader = NULL;

/// <summary>
/// Prints usage information to STDOUT.
///</summary>
void ShowUsage()
{
    printf("%s - Reads data from a serial RFID scanner.\n", APP_NAME);
    printf("\nUsage:\n    %s com_port\n", APP_NAME);
    printf("\nOptions:\n");
    printf("    com_port      The COM port number on which to listen (1 - 9).\n");
    printf("\n\nWritten by Wes Hampson (2016).\n");
}

/// <summary>
/// Handles console control events, such as CTRL+C.
/// </summary>
BOOL WINAPI ConsoleHandler(DWORD dwSignal)
{
    switch (dwSignal)
    {
    case CTRL_C_EVENT:
        if (reader != NULL)
        {
            printf("CTRL+C detected! Closing...\n");
            reader->Close();
            exit(0);
        }
        break;
    }
    return TRUE;
}

/// <summary>
/// Prints the contents of the character pointer to STDOUT as a string.
/// Printing stops once a NUL character is reached.
/// </summary>
void PrintData(char* data)
{
    printf("%s\n", data);
}

/// <summary>
/// Program entry point.
/// </summary>
int main(int argc, char* argv[])
{
    int commPort = -1;
    int baudRate = 9600;
    int dataBits = 8;
    int stopBits = ONESTOPBIT;
    int parity = NOPARITY;
    size_t bufSize = 8;
    bool success;

    // Process command-line arguments
    if (argc < 2)
    {
        ShowUsage();
        return 0;
    }
    else
    {
        commPort = atoi(argv[1]);
    }

    // Set CTRL+C handler.
    success = SetConsoleCtrlHandler(ConsoleHandler, TRUE);
    if (!success)
    {
        printf("Error setting console handler! (%d)", GetLastError());
        return 1;
    }

    // Create and initialize serial reader
    reader = new SerialReader(commPort);
    success = reader->Initialize(baudRate, dataBits, stopBits, parity);
    if (!success)
    {
        printf("Error opening COM port! (%d)\n", GetLastError());
        return 1;
    }

    // Read data from serial port, pass PrintData() as a callback function 
    reader->Read(bufSize, PrintData);

    return 0;
}
