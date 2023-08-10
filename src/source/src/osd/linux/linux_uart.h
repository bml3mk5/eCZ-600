/** @file linux_uart.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2021.05.05 -

	@brief [ linux uart ]
*/

#include "../../vm/vm_defs.h"

#ifdef USE_UART

#ifndef LINUX_UART_H
#define LINUX_UART_H

#include "../../emu_osd.h"
#include <termios.h>
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
	int    fd;
	char   sName[128];
	struct termios ios;
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
	int WriteToDevice();
	int RecvData(char *buf, int size);

	void SetName(const char *name);
	void SetDescription(const char *desc);
	void SetDevice(DEVICE *dev);
	void SetBaudRate(int rate);
	void SetDataBit(int bit);
	void SetParity(int parity);
	void SetStopBit(int bit);
	void SetFlowControl(int flow);

	int GetName(char *name, size_t size) const;
	int GetDescription(char *desc, size_t size) const;
	int GetNameDescription(char *str, size_t size) const;
	DEVICE *GetDevice() const;

	bool CompareName(const char *name) const;

	void Lock();
	void Unlock();
};

/**
	@brief Manage CommPort list on host
*/
class CommPorts : public CPtrList<CommPort>
{
private:

public:
	CommPorts();
	virtual ~CommPorts();

	int Enum();
	int Find(const char *name);

	int GetName(int idx, char *name, size_t size) const;
	int GetDescription(int idx, char *name, size_t size) const;
	int GetNameDescription(int idx, char *str, size_t size) const;
};

#endif /* LINUX_UART_H */
#endif /* USE_UART */
