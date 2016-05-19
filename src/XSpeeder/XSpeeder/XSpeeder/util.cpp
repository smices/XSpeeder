
#include <iostream>

#include "xbStructDef.h"
#include "util.h"
#include "curl\curl.h"
#include "snappy\snappy.h"
#include "InitConf.h"

DownWrapper* g_pWapper = NULL;
std::string strHDSerial;

int GlobalInitialize(CurlInitialize &curl){
	// check and init our dir
	InitDir();

	// Initialize curl
	if (!curl.GetStatus())
		return 1;
	// Load download engine
	g_pWapper = LoadDll();

	if (!g_pWapper)
		return 2;

	// Initialize download engine
	if (!g_pWapper->Init()) {
		UnloadDll(&g_pWapper);
		return 3;
	}
	g_pWapper->LimitUploadSpeed(1024,30);
	return 0;
}
// Load xldl.dll
DownWrapper* LoadDll()
{
	WCHAR szDllpath[512] = {0};
	std::string szLoadPath = GetAppdataPath("HurricaneTeam");
	szLoadPath.append("\\xbSpeed\\xldl.dll");

	std::wstring wstr;
	StringToWString(szLoadPath,wstr);
	StrCpyW(szDllpath,wstr.data());
	DownWrapper* pWapper = new DownWrapper(szDllpath);
	return pWapper;
}

void UnloadDll(DownWrapper** Wapper){
	if (!Wapper) {
		return ;
	}
	if ((*Wapper)!=NULL) {
		(*Wapper)->UnInit();
		delete (*Wapper);
		(*Wapper) = NULL;
	}
}

// Print task info
void PrintTaskInfo(DownTaskInfo &info,bool bTitle)
{
	using namespace std;
	std::wstring stat(L"unknown");
	std::wostringstream  ostr;
	if ( bTitle ) {
		ostr<< endl 
			<< setiosflags(ios::left) <<setw(35) << "Filename"<< setw(15) << "Status" << setw(15) << "Progress " 
			<< setw(15) << "File Size (B)" << setw(15) << "Download Size (B)" << endl; 
		std::wcout<<ostr.str();
		bTitle = false;
		return ;
	}
	switch(info.stat)
	{
	case NOITEM: stat = L"NOITEM";
		break;
	case TSC_ERROR: stat = L"ERROR";
		break;
	case TSC_PAUSE: stat = L"PAUSE";
		break;
	case TSC_DOWNLOAD: stat = L"DOWNLOAD";
		break;
	case TSC_COMPLETE: stat = L"COMPLETE";
		break;
	case TSC_STARTPENDING: stat = L"STARTPENDING";
		break;
	case TSC_STOPPENDING : stat = L"STOPPENDING";
		break;
	}
	// output task detail
	{
		ostr<< setiosflags(ios::left) <<setw(35)<< info.szFilename  << setw(15)<< stat << setiosflags(ios::fixed) << setprecision(2) << setw(15) << info.fPercent * 100 
			<< setw(15) << info.nTotalSize << setw(15) << info.nTotalDownload << endl; 
		std::wcout<<ostr.str();
	}
}

BOOL StringToWString(const std::string &str,std::wstring &wstr)
{    
	int nLen = (int)str.length();    
	wstr.resize(nLen,L' ');

	int nResult = MultiByteToWideChar(CP_ACP,0,(LPCSTR)str.c_str(),nLen,(LPWSTR)wstr.c_str(),nLen);

	if (nResult == 0)
	{
		return FALSE;
	}

	return TRUE;
}
//wstring高字节不为0，返回FALSE
BOOL WStringToString(const std::wstring &wstr,std::string &str)
{    
	int nLen = (int)wstr.length();    
	str.resize(nLen,' ');

	int nResult = WideCharToMultiByte(CP_ACP,0,(LPCWSTR)wstr.c_str(),nLen,(LPSTR)str.c_str(),nLen,NULL,NULL);

	if (nResult == 0)
	{
		return FALSE;
	}

	return TRUE;
}

// --------------------------------------
CurlInitialize::CurlInitialize(){
	m_bStatus = FALSE;
	if (::curl_global_init(CURL_GLOBAL_WIN32)==CURLE_OK)
		m_bStatus = TRUE;
}

CurlInitialize::~CurlInitialize(){
	::curl_global_cleanup();
	m_bStatus = FALSE;
}
BOOL CurlInitialize::GetStatus(){
	return m_bStatus;
}

DWORD WINAPI CommonThreadProc(LPVOID Parameter) {
	PThreadCtrl pThreadCtrl=(PThreadCtrl)Parameter;
	if (!Parameter)
		return 1;
	return pThreadCtrl->ThreadProc(pThreadCtrl);
}

PThreadCtrl CreateXbThread(PVOID Parameter,DWORD (*ThreadProc)(PThreadCtrl)){
	int i=0;
	DWORD dwThreadId=0;
	HANDLE hThread=INVALID_HANDLE_VALUE;
	HANDLE hPipe[2] ={INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE};
	HANDLE hEvent[2]={INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE};

	PThreadCtrl pThreadCtrl = new ThreadCtrl;
	if (!ThreadProc)
		return NULL;
	if (!CreatePipe(&hPipe[0],&hPipe[1],NULL,1024)) {
		return NULL;
	}
	for ( i=0; i<2; i++) {
		hEvent[i] = CreateEvent(NULL,TRUE,FALSE,NULL);
	}
	hThread=CreateThread(NULL,0,CommonThreadProc,(PVOID)pThreadCtrl,CREATE_SUSPENDED,&dwThreadId);
	if (hPipe[0]==INVALID_HANDLE_VALUE ||
		hPipe[1]==INVALID_HANDLE_VALUE ||
		hEvent[0]==INVALID_HANDLE_VALUE||
		hEvent[1]==INVALID_HANDLE_VALUE||
		hThread==INVALID_HANDLE_VALUE
		) {
			// Initial fail
			for (i=0; i<2; i++) {
				if (hPipe[i]!=INVALID_HANDLE_VALUE)
					CloseHandle(hPipe[i]);
				if (hEvent[i]!=INVALID_HANDLE_VALUE)
					CloseHandle(hEvent[i]);
			}
			if (hThread!=INVALID_HANDLE_VALUE) {
				TerminateThread(hThread,0);
				CloseHandle(hThread);
			}
			return NULL;
	}
	else {		
		pThreadCtrl->m_hThread  = hThread;
		for (i=0; i<2; i++) {
			pThreadCtrl->m_hEvent[i]= hEvent[i];
			pThreadCtrl->m_hPipe[i]= hPipe[i];
		}
		pThreadCtrl->m_Parameter = Parameter;
		pThreadCtrl->ThreadProc  = ThreadProc;
		pThreadCtrl->m_dwThreadId=dwThreadId;
		pThreadCtrl->m_wStatus = 0;
		return pThreadCtrl;
	}
}

BOOL ResumeXbThread(PThreadCtrl pThreadCtrl) {
	if (!pThreadCtrl) {
		return FALSE;
	}
	if (pThreadCtrl->m_wStatus==0) {
		pThreadCtrl->m_wStatus=1;
		ResumeThread(pThreadCtrl->m_hThread);
		return TRUE;
	}
	return FALSE;
}

VOID QuitXbThread(PThreadCtrl pThrdCtrl) {
	if (!pThrdCtrl)
		return ;
	SetEvent(pThrdCtrl->m_hEvent[0]);
}
VOID DestoryXbThread(PThreadCtrl pThrdCtrl) {
	if (!pThrdCtrl)
		return ;

	if (pThrdCtrl->m_hThread!=INVALID_HANDLE_VALUE) {
		CloseHandle(pThrdCtrl->m_hThread);
		pThrdCtrl->m_hThread = INVALID_HANDLE_VALUE;
	}
	for (int i=0; i<2; i++) {
		if (pThrdCtrl->m_hEvent[i]=INVALID_HANDLE_VALUE) {
			CloseHandle(pThrdCtrl->m_hEvent[i]);
			pThrdCtrl->m_hEvent[i] = INVALID_HANDLE_VALUE;
		}
		if (pThrdCtrl->m_hPipe[i]=INVALID_HANDLE_VALUE) {
			CloseHandle(pThrdCtrl->m_hPipe[i]);
			pThrdCtrl->m_hPipe[i] = INVALID_HANDLE_VALUE;
		}
	}
	delete pThrdCtrl;
}

int ScanLogicalDrive(char **disk) {
	int i=0,j=0;
	char *buf=NULL,*p=NULL;
	DWORD dwCount = GetLogicalDriveStringsA(0,buf);
	if (dwCount==0 )
		return 0;

	buf = new char[dwCount];
	GetLogicalDriveStringsA(dwCount,buf);

	for (i=0,j=0,p=buf; i<dwCount;i++) {
		if (buf[i]=='\0') {
			if (GetDriveTypeA(p)==DRIVE_FIXED) {
				(*disk)[j] = p[0];
				p = buf+i+1;
				j++;
			}
		}
	}
	delete []buf;
	return strlen(*disk);
}

int CompressBySnappy(std::string szSource,std::string szDestination) {
	return 0;
}

int UncompressBySnappy(std::string szSource,std::string szDestination) {
	std::string szOutput;
	std::string szInput;
	std::ifstream ifs(szSource,std::ios_base::binary|std::ios_base::in);
	std::ofstream ofs(szDestination,std::ios_base::binary|std::ios_base::out|std::ios_base::trunc);

	if (ifs.fail()||ofs.fail())
		return 1;
	ifs.seekg(0,std::ios_base::end);
	std::streamoff position = ifs.tellg();
	if (position<=0)
		return 1;
	szInput.resize(position);//szInput = new char[position];
	ifs.seekg(0,std::ios_base::beg);
	ifs.read((char *)szInput.data(),position);
	ifs.close();

	snappy::Uncompress(szInput.data(),position,&szOutput);
	ofs.write(szOutput.data(),szOutput.size());
	ofs.flush();
	ofs.close();
	return 0;
}

#include <Wincrypt.h>

#define CHECK_NULL_RET(bCondition) if (!bCondition) goto Exit0
#define BUFSIZE 1024
#define MD5LEN  16

BOOL GetContentMD5(
	BYTE *pszFilePath, 
	BOOL bFile, 
	BOOL bUpperCase, 
	CHAR *pszResult,
	DWORD &dwStatus)
{
	BOOL bResult = FALSE;
	HCRYPTPROV hProv = 0;
	HCRYPTHASH hHash = 0;
	HANDLE hFile = NULL;
	BYTE rgbFile[BUFSIZE];
	DWORD cbRead = 0;
	BYTE rgbHash[MD5LEN];
	DWORD cbHash = 0;
	CHAR rgbDigitsL[] = "0123456789abcdef";
	CHAR rgbDigitsU[] = "0123456789ABCDEF";
	CHAR *rgbDigits = bUpperCase ? rgbDigitsU : rgbDigitsL;
	CHAR szResult[MD5LEN*2+1] = {0};

	dwStatus = 0;
	bResult = CryptAcquireContext(&hProv,
		NULL,
		NULL,
		PROV_RSA_FULL,
		CRYPT_VERIFYCONTEXT);
	CHECK_NULL_RET(bResult);

	bResult = CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash);
	CHECK_NULL_RET(bResult);

	if (bFile)
	{
		hFile = CreateFileA((CHAR *)pszFilePath,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_SEQUENTIAL_SCAN,
			NULL);
		CHECK_NULL_RET(!(INVALID_HANDLE_VALUE == hFile));

		while (bResult = ReadFile(hFile, rgbFile, BUFSIZE, 
			&cbRead, NULL))
		{
			if (0 == cbRead)
			{
				break;
			}

			bResult = CryptHashData(hHash, rgbFile, cbRead, 0);
			CHECK_NULL_RET(bResult);
		}
	}
	else
	{
		bResult = CryptHashData(hHash, pszFilePath, strlen((CHAR *)pszFilePath), 0);
		CHECK_NULL_RET(bResult);
	}

	cbHash = MD5LEN;
	if (bResult = CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0))
	{
		CHAR szTmpBuff[3] = {0};
		for (DWORD i = 0; i < cbHash; i++)
		{
			sprintf(szTmpBuff, "%c%c", rgbDigits[rgbHash[i] >> 4],rgbDigits[rgbHash[i] & 0xf]);
			lstrcatA(szResult, szTmpBuff);
		}
		bResult = TRUE;
	}

Exit0:
	dwStatus = GetLastError();
	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);
	CloseHandle(hFile);

	lstrcpyA(pszResult, szResult);

	return bResult; 
}


std::string AlgorithemMD5(std::string szFile) {
	std::string szMD5;
	std::string szInput;
	DWORD dwStatus = 0;
	CHAR szResult[MD5LEN*2+1] = {0};

	GetContentMD5((BYTE *)szFile.data(), TRUE, TRUE, szResult, dwStatus);
	szMD5.append(szResult);
	return szMD5;

/*	HCRYPTPROV hCryptProv = NULL;
	HCRYPTHASH hHash=NULL;
	std::ifstream ifs(szFile,std::ios_base::binary|std::ios_base::in);

	if (ifs.fail()) {
		return szMD5;
	}

	ifs.seekg(0,std::ios_base::end);
	std::streamoff position = ifs.tellg();
	if (position<=0)
		return szMD5;
	szInput.resize(position);
	ifs.seekg(0,std::ios_base::beg);
	ifs.read((char *)szInput.data(),position);
	ifs.close();

	// Get a handle to the default PROV_RSA_FULL provider.
	if(!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, 0)) {
		return szMD5;
	}

	if(!CryptCreateHash( hCryptProv, CALG_MD5, 0, 0, &hHash)) {
		CryptReleaseContext(hCryptProv,0);
		return szMD5;
	}

	if (CryptHashData(hHash, (const BYTE*)(szInput.data()), szInput.size(), 0) ) {
		BYTE MD5Hash[16];
		DWORD dwHashLen=16;
		if(CryptGetHashParam(hHash, HP_HASHVAL, MD5Hash, &dwHashLen, 0)) {
			std::ostringstream ostr;
			for(int i=0;i<dwHashLen;i++) {
				ostr<<std::setw(2)<<std::setfill('0')<<std::hex<<(int)MD5Hash[i];
			}
			ostr<<std::ends;
			szMD5 = ostr.str();
			if (szMD5.size()>32) {
				szMD5.erase(32);
			}
		}
	}
	if(hHash) 
		CryptDestroyHash(hHash);
	if(hCryptProv) 
		CryptReleaseContext(hCryptProv,0);
	return szMD5;*/
}

DWORD GetPhysicalDriveFromPartitionLetter()
{
	HANDLE hDevice;                 // handle to the drive to be examined
	DWORD readed;                   // discard results
	STORAGE_DEVICE_NUMBER number;   // use this to get disk numbers

	CHAR path[512];
	CHAR szSystemDrive[10];
	GetEnvironmentVariableA("SystemDrive",szSystemDrive,10);
	sprintf(path, "\\\\.\\%s", szSystemDrive);
	hDevice = CreateFileA(path,	GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	if (hDevice == INVALID_HANDLE_VALUE) {
		return DWORD(-1);
	}

	if (!DeviceIoControl(hDevice, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &number, sizeof(number), &readed, NULL)) {
		(void)CloseHandle(hDevice);
		return (DWORD)-1;
	}
	(void)CloseHandle(hDevice);
	return number.DeviceNumber;
}

std::string GetSystemRootHDSerialNumber() {
	std::string szHDSerialNumber;
	IWbemLocator *pLoc = NULL;
	IWbemServices *pSvc = NULL;
	IWbemClassObject* pclsObj = NULL;
	IEnumWbemClassObject* pEnum = NULL;
	HRESULT hres;
	hres = CoInitializeSecurity( NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL );
	if( FAILED( hres ) ) {
		goto err;
	}
	hres = CoCreateInstance( CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, ( LPVOID* )&pLoc );
	if( FAILED( hres ) ) {
		goto err;
	}
	hres = pLoc->ConnectServer( _bstr_t( "ROOT\\CIMV2" ), NULL, NULL, 0, NULL, 0, 0, &pSvc );
	if( FAILED( hres ) )
	{
		pLoc->Release(); 
		goto err;
	}
	int ret = 0;
	char szSql[1024]={'\0'};
	int nDiskDeviceID=GetPhysicalDriveFromPartitionLetter();
	if (nDiskDeviceID==((DWORD)-1)){
		pLoc->Release(); 
		goto err;
	}
	sprintf(szSql,"SELECT * FROM Win32_PhysicalMedia where Tag=\"\\\\\\\\.\\\\PHYSICALDRIVE%d\"",nDiskDeviceID);
	hres = pSvc->ExecQuery( bstr_t( "WQL" ), bstr_t( szSql ), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnum );
	if( FAILED( hres ) ) {
		pSvc->Release();
		pLoc->Release(); 
		goto err;
	}
	ULONG uReturn = 0;
	while( pEnum )
	{
		HRESULT hr = pEnum->Next( WBEM_INFINITE, 1, &pclsObj, &uReturn );
		if( 0 == uReturn )
		{
			break;
		}
		VARIANT vtProp;
		VariantInit( &vtProp );
		vtProp.vt = VT_BSTR;
		vtProp.bstrVal = SysAllocString( L"" );
		hres = pclsObj->Get( bstr_t( "SerialNumber" ), 0, &vtProp, 0, 0 );
		if( FAILED( hres ) ) {
			break;
		}
		char* strNumber = _com_util::ConvertBSTRToString( vtProp.bstrVal );
		StrTrimA(strNumber," ");
		szHDSerialNumber.append(strNumber);
		delete [] strNumber;
		VariantClear( &vtProp );
	}
	pEnum->Release();
	pSvc->Release();
	pLoc->Release(); 
err:
	return szHDSerialNumber;
}