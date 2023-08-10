/** @file win_uart.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2021.04.29 -

	@brief [ win32 uart ]
*/

#include "../../vm/vm_defs.h"

#ifdef USE_UART

#ifndef WIN_UART_H
#define WIN_UART_H

#include "../../emu_osd.h"
#include <Windows.h>
#include <tchar.h>
#include "../../cptrlist.h"
#include "../../cmutex.h"

/**
	@brief Manage a serial port on host
*/
class CommPort
{
protected:
	int    m_ch;
	DEVICE *device;
	HANDLE hComm;
	TCHAR  sName[8];
	TCHAR  sDesc[120];
	DCB    sDcb;
	int    iNeedRetry;
	CMutex *mux;

public:
	CommPort(int ch);
	virtual ~CommPort();

	CommPort &operator=(const CommPort &src);

	int Open();
	void Close();
	bool IsOpened() const;
	void Clear();
	int ReadFromDevice();
	int SendData(const char *buf, int size);
	int SendData(const wchar_t *buf, int size);
	int WriteToDevice();
	int RecvData(char *buf, int size);
	int RecvData(wchar_t *buf, int size);

	void SetName(const TCHAR *name);
	void SetDescription(const TCHAR *desc);
//	int SetProperty(const DCB &dcb);
	void SetDevice(DEVICE *dev);
	void SetBaudRate(int rate);
	void SetDataBit(int bit);
	void SetParity(int parity);
	void SetStopBit(int bit);
	void SetFlowControl(int flow);

	int GetName(TCHAR *name, size_t size) const;
	int GetDescription(TCHAR *desc, size_t size) const;
	int GetNameDescription(TCHAR *str, size_t size) const;
//	int GetProperty(DCB &dcb);
	DEVICE *GetDevice() const;

	bool CompareName(const TCHAR *name) const;

	void Lock();
	void Unlock();
};

/**
	@brief Manage CommPort list on host
*/
class CommPorts : public CPtrList<CommPort>
{
public:
	CommPorts();
	virtual ~CommPorts();
//	CommPort &operator[](int idx);

	int Enum();
	int Find(const TCHAR *name);

	int GetName(int idx, TCHAR *name, size_t size) const;
	int GetDescription(int idx, TCHAR *name, size_t size) const;
	int GetNameDescription(int idx, TCHAR *str, size_t size) const;
//	int GetProperty(int idx, DCB &dcb);
//	int SetProperty(int idx, const DCB &dcb);
};

#endif /* WIN_UART_H */
#endif /* USE_UART */
