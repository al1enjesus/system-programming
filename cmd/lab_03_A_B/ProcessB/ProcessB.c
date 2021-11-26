#include <windows.h>
#include <stdio.h>

int fail()
{
	printf("FAIL!\n");
	system("pause");
	return -1;
}
int main(void) {
	LPCTSTR pipename = "\\\\.\\pipe\\mynamedpipe";					// им€ канала
	HANDLE hNamedPipe;												// дескриптор именованного канала
	DWORD szAddress;												// адрес выделенной пам€ти
	DWORD cbRead, cbWritten;										// буфферы (имеют промежуточное значение)
	char* msg;														// получаема€ строка
	BOOL Success;													// флаг успешного открыти€/считывани€/записи

	printf("Creating pipe...");
	hNamedPipe = CreateNamedPipe(
		pipename,													// им€ канала
		PIPE_ACCESS_DUPLEX,											// канал двунаправленный; серверные и клиентские процессы могут читать с канала и записывать в него
		PIPE_TYPE_MESSAGE |											// данные записываютс€ в канал и считываютс€ из него как поток сообщений
		PIPE_READMODE_MESSAGE |
		PIPE_WAIT,													// неограниченное ожидание в течение выполнени€ действий клиентским процессом
		PIPE_UNLIMITED_INSTANCES,									// максимальное число экземпл€ров
		sizeof(DWORD),												// резерв дл€ выводимого буфера
		sizeof(DWORD),												// резерв дл€ вводимого буфера
		100,														// спецификатор значени€ time-out по умолчанию (не используетс€)
		NULL);														// атрибуты защиты
	if (hNamedPipe == INVALID_HANDLE_VALUE) return fail();

	printf("\nConnecting to pipe...");
	ConnectNamedPipe(hNamedPipe, NULL);

	printf("\nReading from pipe...");
	Success = ReadFile(
		hNamedPipe,													// им€ именованного канала
		&szAddress,													// буфер дл€ считывани€
		sizeof(szAddress),											// объЄм считываемых данных
		&cbRead,													// количество считанных данных (не используем)
		NULL);														// асинхронный буфер (не используем)
	if (Success == FALSE) return fail();

	printf("\nYour string: ");
	msg = (char*)szAddress;
	printf_s("%s\n", msg);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	printf("Sending answer to ProccessA...");
	Success = WriteFile(
		hNamedPipe,													// дескриптор канала 
		&szAddress,													// данные дл€ записи 
		sizeof(szAddress),											// объЄм записываемых данных
		&cbWritten,													// количество записанных данных (не используем)
		NULL);														// асинхронный буфер (не используем)
	if (Success == FALSE) return fail();

	printf("\nDisconnecting from named pipe...");
	Success = DisconnectNamedPipe(hNamedPipe);
	if (Success == FALSE) return fail();

	printf("\nClosing named pipe...");
	Success = CloseHandle(hNamedPipe);
	if (Success == FALSE) return fail();

	printf("\nProcessB was finished...\n");
	system("pause");
	return 0;
}