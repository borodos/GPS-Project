#include <iostream>
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <proj.h>

using namespace std;

#define TextBufferSize 255
unsigned char Buffer[TextBufferSize]; // -- ������ ��� ������ ������ � �����

LPCSTR fileName = "test.txt";

// ===========================================================================
HANDLE connectedPort, handleFile;
DWORD BytesIterated;
ofstream out;
ifstream in;

int selectedPort = 3;
bool isConnected = false;
int targerBaudRate = 9600;
int k = 0;
// ===========================================================================

int SerialBegin(int BaudRate, int comport) {
	CloseHandle(connectedPort);

	std::string portName = "COM" + std::to_string(comport);

	// -- �������� �����
	connectedPort = CreateFileA(
		portName.c_str(),
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL);

	if (connectedPort == INVALID_HANDLE_VALUE) { return -4; } // -- ���������� ����������

	// -- ��������� �����
	DCB SerialParams;
	SerialParams.DCBlength = sizeof(SerialParams);
	if (!GetCommState(connectedPort, &SerialParams)) { return -3; } // -- ������ GetState: ��������� �� ��������

	SerialParams.BaudRate = BaudRate;
	SerialParams.ByteSize = 8;
	SerialParams.StopBits = ONESTOPBIT;
	SerialParams.Parity = NOPARITY;
	if (!SetCommState(connectedPort, &SerialParams)) { return -2; } // -- ������ SetState: ����� ��������� �� �����������

	// -- �������� �����
	COMMTIMEOUTS SerialTimesouts;
	SerialTimesouts.ReadIntervalTimeout = 0;
	SerialTimesouts.ReadTotalTimeoutConstant = 0;
	SerialTimesouts.ReadTotalTimeoutMultiplier = 0;
	SerialTimesouts.WriteTotalTimeoutConstant = 0;
	SerialTimesouts.WriteTotalTimeoutMultiplier = 0;
	if (!SetCommTimeouts(connectedPort, &SerialTimesouts)) { return -1; } // -- ������ SetCommTimesouts: �� ������� ������ ������������ ��������

	return 0;
}

void ConnectRequest(void) {
	if (isConnected) {
		CloseHandle(connectedPort);
		std::cout << "��������!" << std::endl;
		isConnected = false;
		return;
	}

	switch (SerialBegin(targerBaudRate, selectedPort))
	{
	case -4: std::cout << "The device is missing" << std::endl; break;
	case -3: std::cout << "Error GetState: settings not received" << std::endl;
	case -2: std::cout << "Error SetState: no new settings have been set" << std::endl;
	case -1: std::cout << "Error SetCommTimesouts: could not set the waiting time" << std::endl; break;
	case 0:
		std::cout << "Connect to COM" + std::to_string(selectedPort) << std::endl;
		isConnected = true;
		return;
	}

	CloseHandle(connectedPort);
}

void convertToUTM(double numberLongitude, double numberLatitude) {
	PJ_CONTEXT* C;
	PJ* P;
	PJ* norm;
	PJ_COORD a, b;

	// -- �������� ���������
	C = proj_context_create();

	// -- �������� ������� ��� ������������� ���������
	P = proj_create_crs_to_crs(C,
		"EPSG:4326",
		"+proj=utm +zone=32 +datum=WGS84", /* or EPSG:32632 */
		NULL);

	// -- �������� �� �������� �������� �������
	if (0 == P) {
		std::cout << "Failed to create transformation object" << endl;
		return;
	}

	// -- ����������� ������ ������ ��� �������������, ����� � ������ ��������� �� ������ ���.
	norm = proj_normalize_for_visualization(C, P);

	// -- �������� �� �������� �������� ������ �������
	if (0 == norm) {
		std::cout << "Failed to normalize transformation object." << endl;
		return;
	}

	// -- ������� ������ ������
	proj_destroy(P);

	// -- ����������� ��������������� ������ ���������� P
	P = norm;

	// -- ���� ��������� (������� � ������)
	a = proj_coord(38.939990, 47.207359, 0, 0);

	// -- �������������� ���������� � UTM 32N
	b = proj_trans(P, PJ_FWD, a);

	cout << "LongitudeUTM:\t" << b.enu.e << '\t' << "LatitudeUTM:\t" << b.enu.n << endl;

	// -- ������� �������
	proj_destroy(P);
	proj_context_destroy(C);

	// -- ���������� � ���� ��������������� ����������
	out << "LongitudeUTM:\t" << b.enu.e << endl << "LatitudeUTM:\t" << b.enu.n << endl << endl;
}

void checkFile() {
	out.close();
	in.open(fileName);
	string str, strLongitude, strLatitude, valueLongitude, valueLatitude;
	double numberLongitude = 0, numberLatitude = 0;

	in.seekg(k);
	while (getline(in, str)) {
		k = in.tellg();
		cout << str << endl;
		if (str.find("Latitude:") != string::npos) {
			if (strLatitude == "") {
				strLatitude = str;
			}
		}

		if (str.find("Longitude:") != string::npos) {
			if (strLongitude == "") {
				strLongitude = str;
			}
		}
	}

	// -- ���������� ������� � ����������
	valueLongitude.resize(11);

	if (strLongitude != "") {
		for (size_t i = strLongitude.size(), j = 0; i <= strLongitude.size(); i--, j++)
		{
			if (strLongitude[i] == '\t') {
				break;
			}
			else {
				valueLongitude[j] = strLongitude[i];
			}
		}
	}

	// -- ���������� ������ � ����������
	valueLatitude.resize(11);

	if (strLatitude != "") {
		for (size_t i = strLatitude.size(), j = 0; i <= strLatitude.size(); i--, j++)
		{
			if (strLatitude[i] == '\t') {
				break;
			}
			else {
				valueLatitude[j] = strLatitude[i];
			}
		}
	}

	// -- ������������� ������ valueLongitude
	if (strLongitude != "") {
		for (size_t i = 0; i < valueLongitude.size() / 2; i++)
			swap(valueLongitude[i], valueLongitude[valueLongitude.size() - i - 1]);
	}

	// -- ������������� ������ valueLatitude
	if (strLatitude != "") {
		for (size_t i = 0; i < valueLatitude.size() / 2; i++)
			swap(valueLatitude[i], valueLatitude[valueLatitude.size() - i - 1]);
	}

	// -- ����������� string valueLongitude � float
	if (strLongitude != "") {
		numberLongitude = std::stod(valueLongitude);
	}

	// -- ����������� string valueLongitude � float
	if (strLatitude != "") {
		numberLatitude = std::stod(valueLatitude);
	}
	in.close();
	out.open(fileName, ios::app);
	if (strLongitude != "" && strLatitude != "") {
		convertToUTM(numberLongitude, numberLatitude);
	}

}

void ReadPrinting(const int size)
{
	string textBuffer;
	textBuffer.resize(size);
	for (int i = 0; i < size; i++)
	{
		textBuffer[i] = Buffer[i];
	}
	//std::cout << textBuffer;
	out << textBuffer;
	memset(Buffer, 0, TextBufferSize);
}

DWORD WINAPI ReadSerialPort(LPVOID)
{
	COMSTAT comstat = { 0 };
	DWORD btr = comstat.cbInQue;
	const int READ_TIME = 500;
	OVERLAPPED sync = { 0 };
	unsigned long wait = 0, read = 0, state = 0;

	/* ������� ������ ������������� */
	sync.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	while (1)
	{

		/* ������������� ����� �� ������� ����� */
		if (SetCommMask(connectedPort, EV_RXCHAR | EV_TXEMPTY)) {
			/* ��������� ���� � ������ �������������*/
			WaitCommEvent(connectedPort, &state, &sync);
			/* �������� �������� ������*/
			wait = WaitForSingleObject(sync.hEvent, READ_TIME);
			/* ������ �������� */
			if (wait == WAIT_OBJECT_0) {
				/* �������� ������ ������ */
				ClearCommError(connectedPort, &BytesIterated, &comstat);
				btr = comstat.cbInQue;
				if (btr) {
					ReadFile(connectedPort, Buffer, btr, &BytesIterated, &sync);
					ReadPrinting(btr);
				}
			}
			else {
				checkFile();
			}
		}
	}

	return 0;
}

void SerialRead(void) {

	if (!isConnected) { return; }
	if (!SetCommMask(connectedPort, EV_RXCHAR)) { ConnectRequest(); return; }
	out.open(fileName, ios::trunc);
	PurgeComm(connectedPort, PURGE_RXCLEAR | PURGE_TXCLEAR);
	ReadSerialPort(0);

}

int main() {
	ConnectRequest();
	SerialRead();
	CloseHandle(connectedPort);

	return 0;
}