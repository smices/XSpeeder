// xldl.dll导出函数的封装类

#pragma once

// 定义函数指针
typedef BOOL	(*fn_Init)(void);
typedef BOOL	(*fn_UnInit)(void);
typedef HANDLE  (*fn_TaskCreate)(DownTaskParam &stParam);
typedef BOOL    (*fn_TaskDelete)(HANDLE hTask);
typedef BOOL	(*fn_TaskStart) (HANDLE hTask);
typedef BOOL	(*fn_TaskPause) (HANDLE hTask);
typedef BOOL	(*fn_TaskQuery) (HANDLE hTask,DownTaskInfo &stTaskInfo);
typedef BOOL	(*fn_TaskQueryEx) (HANDLE hTask,DownTaskInfo &stTaskInfo);
typedef void	(*fn_LimitSpeed)(INT32 nKBps);
typedef void	(*fn_LimitUploadSpeed)(INT32 nTcpBps,INT32 nOtherBps);
typedef BOOL	(*fn_DelTempFile)(DownTaskParam &stParam);
typedef BOOL	(*fn_SetProxy)(DOWN_PROXY_INFO &stProxy);
typedef void    (*fn_SetUserAgent)( const TCHAR* pszUserAgent );
typedef BOOL	(*fn_GetFileSizeWithUrl)(const wchar_t * lpURL, INT64& iFileSize);
typedef BOOL    (*fn_ParseThunderPrivateUrl)(const TCHAR *pszThunderUrl, TCHAR *normalUrlBuffer, INT32 bufferLen);
typedef LONG	(*fn_SetAdditionInfo)( HANDLE task_id, WSAPROTOCOL_INFOW *sock_info, CHAR *http_resp_buf, LONG buf_len );
typedef BOOL	(*fn_SetFileIdAndSize)(HANDLE hTask, char szFileId[40], unsigned __int64 nFileSize);

#define CHECKFUNC(f, ret) if (f == NULL) return ret;
#define CHECKFUNC_(f) if (f == NULL) return;

class DownWrapper
{
public:

	DownWrapper(LPCWSTR sPath)
	{
		m_hModule = ::LoadLibraryW(sPath);
		if (m_hModule == NULL)
		{
			throw L"can not load xldl.dll";
		}
		_Init					= (fn_Init)						::GetProcAddress(m_hModule, "XL_Init");
		_UnInit					= (fn_UnInit)					::GetProcAddress(m_hModule, "XL_UnInit");
		_TaskCreate				= (fn_TaskCreate)				::GetProcAddress(m_hModule, "XL_CreateTask");
		_TaskDelete				= (fn_TaskDelete)				::GetProcAddress(m_hModule, "XL_DeleteTask");
		_TaskStart				= (fn_TaskStart)				::GetProcAddress(m_hModule, "XL_StartTask");
		_TaskPause				= (fn_TaskPause)				::GetProcAddress(m_hModule, "XL_StopTask");
		_TaskQuery				= (fn_TaskQuery)				::GetProcAddress(m_hModule, "XL_QueryTaskInfo");
		_TaskQueryEx			= (fn_TaskQueryEx)				::GetProcAddress(m_hModule, "XL_QueryTaskInfoEx");
		_LimitSpeed				= (fn_LimitSpeed)				::GetProcAddress(m_hModule, "XL_SetSpeedLimit");
		_LimitUploadSpeed		= (fn_LimitUploadSpeed)			::GetProcAddress(m_hModule, "XL_SetUploadSpeedLimit");
		_DelTempFile			= (fn_DelTempFile)				::GetProcAddress(m_hModule, "XL_DelTempFile");
		_SetProxy				= (fn_SetProxy)					::GetProcAddress(m_hModule, "XL_SetProxy");
		_SetUserAgent			= (fn_SetUserAgent)				::GetProcAddress(m_hModule, "XL_SetUserAgent");
		_GetFileSizeWithUrl		= (fn_GetFileSizeWithUrl)		::GetProcAddress(m_hModule, "XL_GetFileSizeWithUrl");
		_ParseThunderPrivateUrl = (fn_ParseThunderPrivateUrl)	::GetProcAddress(m_hModule, "XL_ParseThunderPrivateUrl");
		_SetAdditionInfo		= (fn_SetAdditionInfo)			::GetProcAddress(m_hModule, "XL_SetAdditionInfo");
		_SetFileIdAndSize		= (fn_SetFileIdAndSize)			::GetProcAddress(m_hModule, "XL_SetFileIdAndSize");
	}

	virtual ~DownWrapper(void)
	{
		if (m_hModule)
		{
			::FreeLibrary(m_hModule);
		}
	}

	BOOL Init()
	{CHECKFUNC(_Init, FALSE); return _Init();}
	BOOL UnInit()
	{CHECKFUNC(_UnInit, FALSE); return _UnInit();}
	HANDLE TaskCreate(DownTaskParam &param)
	{CHECKFUNC(_TaskCreate, NULL); return _TaskCreate(param);}
	BOOL TaskDelete(HANDLE hTask)
	{CHECKFUNC(_TaskDelete, FALSE); return _TaskDelete(hTask);}
	BOOL TaskStart(HANDLE hTask)
	{CHECKFUNC(_TaskStart, FALSE); return _TaskStart(hTask);}
	BOOL TaskPause(HANDLE hTask)
	{CHECKFUNC(_TaskPause, FALSE); return _TaskPause(hTask);}
	BOOL TaskQuery(HANDLE hTask, DownTaskInfo & stTaskInfo)
	{CHECKFUNC(_TaskQuery, FALSE); return _TaskQuery(hTask,stTaskInfo);}
	BOOL TaskQueryEx(HANDLE hTask, DownTaskInfo & stTaskInfo)
	{CHECKFUNC(_TaskQueryEx, FALSE); return _TaskQueryEx(hTask,stTaskInfo);}
	void LimitSpeed(INT32 nBps)
	{CHECKFUNC_(_LimitSpeed);  _LimitSpeed(nBps);}
	void LimitUploadSpeed(INT32 nTcpBps,INT32 nOtherBps)
	{CHECKFUNC_(_LimitUploadSpeed);  _LimitUploadSpeed(nTcpBps, nOtherBps);}
	BOOL DelTempFile(DownTaskParam &stParam)
	{CHECKFUNC(_DelTempFile,FALSE);return _DelTempFile(stParam);}
	BOOL SetProxy(DOWN_PROXY_INFO &stProxy)
	{CHECKFUNC(_SetProxy, FALSE); return _SetProxy(stProxy);}
	void SetUserAgent(const TCHAR *pszUserAgent)
	{CHECKFUNC_(_SetUserAgent);_SetUserAgent(pszUserAgent);}
	BOOL GetFileSizeWithUrl(const wchar_t * lpURL, INT64& iFileSize)
	{CHECKFUNC(_GetFileSizeWithUrl, FALSE);return _GetFileSizeWithUrl(lpURL, iFileSize);}
	BOOL ParseThunderPrivateUrl(const TCHAR *pszThunderUrl, TCHAR *normalUrlBuffer, INT32 bufferLen)
	{if (_ParseThunderPrivateUrl == NULL ) return FALSE; return _ParseThunderPrivateUrl(pszThunderUrl, normalUrlBuffer, bufferLen);}
	LONG SetAdditionInfo(HANDLE task_id, WSAPROTOCOL_INFOW *sock_info, CHAR *http_resp_buf, LONG buf_len)
	{CHECKFUNC(_SetAdditionInfo, FALSE); return _SetAdditionInfo(task_id, sock_info, http_resp_buf, buf_len);}
	LONG SetFileIdAndSize(HANDLE hTask, char szFileId[40], unsigned __int64 nFileSize)
	{CHECKFUNC(_SetFileIdAndSize, FALSE); return _SetFileIdAndSize(hTask, szFileId, nFileSize);}

private:
	HMODULE						m_hModule;

	fn_Init						_Init;
	fn_UnInit					_UnInit;
	fn_TaskCreate				_TaskCreate;
	fn_TaskDelete				_TaskDelete;
	fn_TaskStart				_TaskStart;
	fn_TaskPause				_TaskPause;
	fn_TaskQuery				_TaskQuery;
	fn_TaskQueryEx				_TaskQueryEx;
	fn_LimitSpeed				_LimitSpeed;
	fn_LimitUploadSpeed			_LimitUploadSpeed;
	fn_DelTempFile				_DelTempFile;
	fn_SetProxy					_SetProxy;
	fn_SetUserAgent				_SetUserAgent;
	fn_GetFileSizeWithUrl		_GetFileSizeWithUrl;
	fn_ParseThunderPrivateUrl	_ParseThunderPrivateUrl;
	fn_SetAdditionInfo			_SetAdditionInfo;
	fn_SetFileIdAndSize			_SetFileIdAndSize;
};

