#include <windows.h>
#include <WinUser.h>
// handle about Instance or module, 

int DisplayResourceNAMessagebox()
{
	int msgID = MessageBox(
		NULL,
		(LPCWSTR)L"잘가",
		(LPCWSTR)L"안녕하세요",
		MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2
	);

	return msgID;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {

	DisplayResourceNAMessagebox();
	
	return 0;
}