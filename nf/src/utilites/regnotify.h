#pragma once

class CRegNotify
{
	HANDLE m_hEvent;
	HKEY m_hKey;
public:
	CRegNotify(wchar_t const* RegKey) {
		m_hEvent = CreateEvent(0, FALSE, FALSE, NULL);
		RegOpenKeyEx(HKEY_CURRENT_USER
			, RegKey
			, REG_OPTION_NON_VOLATILE
			, KEY_NOTIFY | KEY_QUERY_VALUE
			, &m_hKey
		);
		Start();
	}
	~CRegNotify() {
		CloseHandle(m_hEvent);
		RegCloseKey(m_hKey);
	}

	bool Check() {	//проверить, не возникло ли событие
		return WAIT_OBJECT_0 == WaitForSingleObject(m_hEvent, 0);
	}
	void Start() {	//дать команду генерировать Event при внесении изменений в реестр
		RegNotifyChangeKeyValue(m_hKey
			, TRUE
			, REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET
			, m_hEvent
			, TRUE);
	}
};