#include <tchar.h>
#include <stdio.h>
#include "serialport.h"
#include "winerror.h"

///////////////////////////////// defines /////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

class _SERIAL_PORT_DATA
{
public:
//Constructors /Destructors
  _SERIAL_PORT_DATA();
  ~_SERIAL_PORT_DATA();

  HINSTANCE m_hKernel32;
  typedef BOOL (CANCELIO)(HANDLE);
  typedef CANCELIO* LPCANCELIO;
  LPCANCELIO m_lpfnCancelIo;
};

_SERIAL_PORT_DATA::_SERIAL_PORT_DATA()
{
  m_hKernel32 = LoadLibrary(TEXT("KERNEL32.DLL"));
  if (m_hKernel32 != NULL);
  m_lpfnCancelIo = (LPCANCELIO) GetProcAddress(m_hKernel32, "CancelIo");
}

_SERIAL_PORT_DATA::~_SERIAL_PORT_DATA()
{
  FreeLibrary(m_hKernel32);
  m_hKernel32 = NULL;
}

//The local variable which handle the function pointers
_SERIAL_PORT_DATA _SerialPortData;

////////// The actual serial port code
CSerialPort::CSerialPort()
{
  m_hComm = INVALID_HANDLE_VALUE;
  m_bOverlapped = FALSE;
}

CSerialPort::~CSerialPort()
{
  Close();
}

BOOL CSerialPort::Open(int nPort, DWORD dwBaud, Parity parity, BYTE DataBits, StopBits stopbits, FlowControl fc, BOOL bOverlapped)
{
  //Validate our parameters
  if (!(nPort>0 && nPort<=255)) return FALSE;
  //Call CreateFile to open up the comms port
  WCHAR sPort[32];
  swprintf (sPort, L"\\\\.\\COM%d", nPort);
  m_hComm = CreateFile(sPort, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL,
                       OPEN_EXISTING, bOverlapped ? FILE_FLAG_OVERLAPPED : 0, NULL);
  //if (nPort==5)
    //m_hComm = CreateFile((LPCSTR)_T("\\\\.\\COM5"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL,
                                      //OPEN_EXISTING, bOverlapped ? FILE_FLAG_OVERLAPPED : 0, NULL);
  if (m_hComm == INVALID_HANDLE_VALUE)
    return FALSE;

  m_bOverlapped = bOverlapped;

  //Get the current state prior to changing it
  DCB dcb;
  GetState(dcb);

  //Setup the baud rate
  dcb.BaudRate = dwBaud;

  //Setup the Parity
  switch (parity)
  {
    case EvenParity:  dcb.Parity = EVENPARITY;  break;
    case MarkParity:  dcb.Parity = MARKPARITY;  break;
    case NoParity:    dcb.Parity = NOPARITY;    break;
    case OddParity:   dcb.Parity = ODDPARITY;   break;
    case SpaceParity: dcb.Parity = SPACEPARITY; break;
    default:          return FALSE;
  }

  //Setup the data bits
  dcb.ByteSize = DataBits;

  //Setup the stop bits
  switch (stopbits)
  {
    case OneStopBit:           dcb.StopBits = ONESTOPBIT;   break;
    case OnePointFiveStopBits: dcb.StopBits = ONE5STOPBITS; break;
    case TwoStopBits:          dcb.StopBits = TWOSTOPBITS;  break;
    default:                   return FALSE;
  }

  //Setup the flow control
  dcb.fDsrSensitivity = FALSE;
  switch (fc)
  {
    case NoFlowControl:
    {
      dcb.fOutxCtsFlow = FALSE;
      dcb.fOutxDsrFlow = FALSE;
      dcb.fOutX = FALSE;
      dcb.fInX = FALSE;
      break;
    }
    case CtsRtsFlowControl:
    {
      dcb.fOutxCtsFlow = TRUE;
      dcb.fOutxDsrFlow = FALSE;
      dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
      dcb.fOutX = FALSE;
      dcb.fInX = FALSE;
      break;
    }
    case CtsDtrFlowControl:
    {
      dcb.fOutxCtsFlow = TRUE;
      dcb.fOutxDsrFlow = FALSE;
      dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
      dcb.fOutX = FALSE;
      dcb.fInX = FALSE;
      break;
    }
    case DsrRtsFlowControl:
    {
      dcb.fOutxCtsFlow = FALSE;
      dcb.fOutxDsrFlow = TRUE;
      dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
      dcb.fOutX = FALSE;
      dcb.fInX = FALSE;
      break;
    }
    case DsrDtrFlowControl:
    {
      dcb.fOutxCtsFlow = FALSE;
      dcb.fOutxDsrFlow = TRUE;
      dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
      dcb.fOutX = FALSE;
      dcb.fInX = FALSE;
      break;
    }
    case XonXoffFlowControl:
    {
      dcb.fOutxCtsFlow = FALSE;
      dcb.fOutxDsrFlow = FALSE;
      dcb.fOutX = TRUE;
      dcb.fInX = TRUE;
      dcb.XonChar = 0x11;
      dcb.XoffChar = 0x13;
      dcb.XoffLim = 100;
      dcb.XonLim = 100;
      break;
    }
    default:
    {
      return FALSE;
      break;
    }
  }
  //Now that we have all the settings in place, make the changes
  SetState(dcb);
  return TRUE;
}

BOOL CSerialPort::Close()
{
  if (IsOpen())
  {
    BOOL bSuccess = CloseHandle(m_hComm);
    m_hComm = INVALID_HANDLE_VALUE;
    if (!bSuccess)
      return FALSE;
    m_bOverlapped = FALSE;
  }
  return TRUE;
}

void CSerialPort::Attach(HANDLE hComm)
{
  Close();
  m_hComm = hComm;
}

HANDLE CSerialPort::Detach()
{
  HANDLE hrVal = m_hComm;
  m_hComm = INVALID_HANDLE_VALUE;
  return hrVal;
}

DWORD CSerialPort::Read(void* lpBuf, DWORD dwCount)
{
  if (!IsOpen())  return -1;
  if (m_bOverlapped) return -1;

  DWORD dwBytesRead = 0;
  if (!ReadFile(m_hComm, lpBuf, dwCount, &dwBytesRead, NULL))
      return -1;
  return dwBytesRead;
}

BOOL CSerialPort::Read(void* lpBuf, DWORD dwCount, OVERLAPPED& overlapped)
{
  if (!IsOpen()) return 0;
  if (!m_bOverlapped) return 0;
  if (!overlapped.hEvent) return 0;

  DWORD dwBytesRead = 0;
  BOOL bSuccess = ReadFile(m_hComm, lpBuf, dwCount, &dwBytesRead, &overlapped);
  if (!bSuccess)
  {
    if (GetLastError() != ERROR_IO_PENDING)
      return FALSE;
  }
  return bSuccess;
}

DWORD CSerialPort::Write(const void* lpBuf, DWORD dwCount)
{
  if (!IsOpen()) return -1;
  if (m_bOverlapped) return -1;

  DWORD dwBytesWritten = 0;
  if (!WriteFile(m_hComm, lpBuf, dwCount, &dwBytesWritten, NULL))
      return -1;
  return dwBytesWritten;
}

BOOL CSerialPort::Write(const void* lpBuf, DWORD dwCount, OVERLAPPED& overlapped)
{
  if (!IsOpen()) return 0;
  if (!m_bOverlapped) return 0;
  if (overlapped.hEvent) return 0;

  DWORD dwBytesWritten = 0;
  BOOL bSuccess = WriteFile(m_hComm, lpBuf, dwCount, &dwBytesWritten, &overlapped);
  if (!bSuccess)
  {
    if (GetLastError() != ERROR_IO_PENDING)
      return FALSE;
  }
  return bSuccess;
}

void CSerialPort::GetOverlappedResult(OVERLAPPED& overlapped, DWORD& dwBytesTransferred, BOOL bWait)
{
  if (!IsOpen()) return;
  if (!m_bOverlapped) return;
  if (!overlapped.hEvent) return;

  DWORD dwBytesWritten = 0;
  if (!::GetOverlappedResult(m_hComm, &overlapped, &dwBytesTransferred, bWait))
  {
    if (GetLastError() != ERROR_IO_PENDING)
      return;
  }
}

void CSerialPort::OnCompletion(DWORD /*dwErrorCode*/, DWORD /*dwCount*/, LPOVERLAPPED lpOverlapped)
{
  //Just free up the memory which was previously allocated for the OVERLAPPED structure
  delete lpOverlapped;

  //Your derived classes can do something useful in OnCompletion, but don't forget to
  //call CSerialPort::OnCompletion to ensure the memory is freed up
}

void CSerialPort::CancelIo()
{
  if (!IsOpen()) return;

  if (_SerialPortData.m_lpfnCancelIo == NULL)
      return;

  if (!::_SerialPortData.m_lpfnCancelIo(m_hComm))
      return;
}

void CSerialPort::TransmitChar(char cChar)
{
  if (!IsOpen()) return;

  if (!TransmitCommChar(m_hComm, cChar))
      return;
}

void CSerialPort::GetConfig(COMMCONFIG& config)
{
  if (!IsOpen()) return;

  DWORD dwSize = sizeof(COMMCONFIG);
  if (!GetCommConfig(m_hComm, &config, &dwSize))
      return;
}

void CSerialPort::SetConfig(COMMCONFIG& config)
{
  if (!IsOpen()) return;

  DWORD dwSize = sizeof(COMMCONFIG);
  if (!SetCommConfig(m_hComm, &config, dwSize))
      return;
}

void CSerialPort::SetBreak()
{
  if (!IsOpen()) return;

  if (!SetCommBreak(m_hComm))
      return;
}

void CSerialPort::ClearBreak()
{
  if (!IsOpen()) return;

  if (!ClearCommBreak(m_hComm))
      return;
}

void CSerialPort::ClearError(DWORD& dwErrors)
{
  if (!IsOpen()) return;

  if (!ClearCommError(m_hComm, &dwErrors, NULL))
      return;
}

void CSerialPort::GetDefaultConfig(int nPort, COMMCONFIG& config)
{
  //Validate our parameters
  if (nPort>0 && nPort<=255) return;

  //Create the device name as a string
  WCHAR sPort[32];
  swprintf (sPort, L"COM%d", nPort);

  DWORD dwSize = sizeof(COMMCONFIG);
  if (!GetDefaultCommConfig(sPort, &config, &dwSize))
      return;
}

void CSerialPort::SetDefaultConfig(int nPort, COMMCONFIG& config)
{
  //Validate our parameters
  if (nPort>0 && nPort<=255) return;

  //Create the device name as a string
  WCHAR sPort[32];
  swprintf (sPort, L"COM%d", nPort);

  DWORD dwSize = sizeof(COMMCONFIG);
  if (!SetDefaultCommConfig(sPort, &config, dwSize))
      return;
}

void CSerialPort::GetStatus(COMSTAT& stat)
{
  if (!IsOpen()) return;

  DWORD dwErrors;
  if (!ClearCommError(m_hComm, &dwErrors, &stat))
      return;
}

void CSerialPort::GetState(DCB& dcb)
{
  if (!IsOpen()) return;

  if (!GetCommState(m_hComm, &dcb))
      return;
}

void CSerialPort::SetState(DCB& dcb)
{
  if (!IsOpen()) return;

  if (!SetCommState(m_hComm, &dcb))
      return;
}

void CSerialPort::Escape(DWORD dwFunc)
{
  if (!IsOpen()) return;

  if (!EscapeCommFunction(m_hComm, dwFunc))
      return;
}

void CSerialPort::ClearDTR()
{
  Escape(CLRDTR);
}

void CSerialPort::ClearRTS()
{
  Escape(CLRRTS);
}

void CSerialPort::SetDTR()
{
  Escape(SETDTR);
}

void CSerialPort::SetRTS()
{
  Escape(SETRTS);
}

void CSerialPort::SetXOFF()
{
  Escape(SETXOFF);
}

void CSerialPort::SetXON()
{
  Escape(SETXON);
}

void CSerialPort::GetProperties(COMMPROP& properties)
{
  if (!IsOpen()) return;

  if (!GetCommProperties(m_hComm, &properties))
      return;
}

void CSerialPort::GetModemStatus(DWORD& dwModemStatus)
{
  if (!IsOpen()) return;

  if (!GetCommModemStatus(m_hComm, &dwModemStatus))
      return;
}

void CSerialPort::SetMask(DWORD dwMask)
{
  if (!IsOpen()) return;

  if (!SetCommMask(m_hComm, dwMask))
      return;
}

void CSerialPort::GetMask(DWORD& dwMask)
{
  if (!IsOpen()) return;

  if (!GetCommMask(m_hComm, &dwMask))
      return;
}

void CSerialPort::Flush()
{
  if (!IsOpen()) return;

  if (!FlushFileBuffers(m_hComm))
      return;
}

void CSerialPort::Purge(DWORD dwFlags)
{
  if (!IsOpen()) return;

  if (!PurgeComm(m_hComm, dwFlags))
      return;
}

void CSerialPort::TerminateOutstandingWrites()
{
  Purge(PURGE_TXABORT);
}

void CSerialPort::TerminateOutstandingReads()
{
  Purge(PURGE_RXABORT);
}

void CSerialPort::ClearWriteBuffer()
{
  Purge(PURGE_TXCLEAR);
}

void CSerialPort::ClearReadBuffer()
{
  Purge(PURGE_RXCLEAR);
}

void CSerialPort::Setup(DWORD dwInQueue, DWORD dwOutQueue)
{
  if (!IsOpen()) return;

  if (!SetupComm(m_hComm, dwInQueue, dwOutQueue))
      return;
}

void CSerialPort::SetTimeouts(COMMTIMEOUTS& timeouts)
{
  if (!IsOpen()) return;

  if (!SetCommTimeouts(m_hComm, &timeouts))
      return;
}

void CSerialPort::GetTimeouts(COMMTIMEOUTS& timeouts)
{
  if (!IsOpen()) return;

  if (!GetCommTimeouts(m_hComm, &timeouts))
      return;
}

void CSerialPort::Set0Timeout()
{
  COMMTIMEOUTS Timeouts;
  ZeroMemory(&Timeouts, sizeof(COMMTIMEOUTS));
  Timeouts.ReadIntervalTimeout = MAXDWORD;
  Timeouts.ReadTotalTimeoutMultiplier = 0;
  Timeouts.ReadTotalTimeoutConstant = 0;
  Timeouts.WriteTotalTimeoutMultiplier = 0;
  Timeouts.WriteTotalTimeoutConstant = 0;
  SetTimeouts(Timeouts);
}

void CSerialPort::Set0WriteTimeout()
{
  COMMTIMEOUTS Timeouts;
  GetTimeouts(Timeouts);
  Timeouts.WriteTotalTimeoutMultiplier = 0;
  Timeouts.WriteTotalTimeoutConstant = 0;
  SetTimeouts(Timeouts);
}

void CSerialPort::Set0ReadTimeout()
{
  COMMTIMEOUTS Timeouts;
  GetTimeouts(Timeouts);
  Timeouts.ReadIntervalTimeout = MAXDWORD;
  Timeouts.ReadTotalTimeoutMultiplier = 0;
  Timeouts.ReadTotalTimeoutConstant = 0;
  SetTimeouts(Timeouts);
}

void CSerialPort::WaitEvent(DWORD& dwMask)
{
  if (!IsOpen()) return;
  if (m_bOverlapped) return;
  WaitCommEvent(m_hComm, &dwMask, NULL);
}

void CSerialPort::WaitEvent(DWORD& dwMask, OVERLAPPED& overlapped)
{
  if (!IsOpen()) return;
  if (!m_bOverlapped) return;
  if (!overlapped.hEvent) return;

  if (!WaitCommEvent(m_hComm, &dwMask, &overlapped))
  {
    if (GetLastError() != ERROR_IO_PENDING)
        return;
  }
}

