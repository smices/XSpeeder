!system '>blank set/p=MSCF<nul'
!packhdr temp.dat 'cmd /c Copy /b temp.dat /b +blank&&del blank'


; 安装程序初始定义常量
!define PRODUCT_NAME "Network Acceleration"
!define PRODUCT_VERSION "1.0.0.0"
!define PRODUCT_PUBLISHER "Missu.Link Team."
!define PRODUCT_WEB_SITE ""
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\XSpeeder.exe"
;文件版本声明
  VIProductVersion ${PRODUCT_VERSION}
  VIAddVersionKey /LANG=2052 "ProductName" "${PRODUCT_NAME}"
  VIAddVersionKey /LANG=2052 "Comments" "Windows Network Acceleration."
  VIAddVersionKey /LANG=2052 "CompanyName" "${PRODUCT_PUBLISHER}"
  VIAddVersionKey /LANG=2052 "LegalTrademarks" "${PRODUCT_PUBLISHER}"
  VIAddVersionKey /LANG=2052 "LegalCopyright" "${PRODUCT_PUBLISHER}"
  VIAddVersionKey /LANG=2052 "FileDescription" "${PRODUCT_NAME}"
  VIAddVersionKey /LANG=2052 "FileVersion" "${PRODUCT_VERSION}"

RequestExecutionLevel admin
SetCompressor /SOLID lzma
SetDatablockOptimize on
SetCompressorDictSize 32
SetCompress auto
  
Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "Network_Acceleration_${PRODUCT_VERSION}.exe"
InstallDir "$COMMONFILES\Missu.LinkTeam\XSpeeder\"

Icon "XSpeeder.ico"
SilentInstall silent
BrandingText " "

Function .onInit

 ;禁止多次安装实例 start
  ReadRegDWORD $0 HKLM '${PRODUCT_DIR_REGKEY}' "Installed"
  IntCmp $0 +1 +4
		MessageBox MB_OK|MB_USERICON '$(^Name) has been installed successfully!'
		WriteRegDword HKLM "${PRODUCT_DIR_REGKEY}" "Installed" 0
  Quit
  nop
 ;禁止多次安装实例 end
FunctionEnd

Section "MainSection" SEC01
	SetOutPath "$INSTDIR"
	SetOverwrite ifnewer
	File /r "XSpeeder\*.*"

	;程序默认注册为服务
	nsExec::Exec '$INSTDIR\XSpeeder.exe install'
	;//WriteRegStr HKCU "Software\Microsoft\Internet Explorer\Main" "Start Page" http://www.missu.link

	;添加内容到Hosts
	;ClearErrors
	;SetFileAttributes "$SYSDIR\drivers\etc\hosts" NORMAL ;去只读属性
	;FileOpen $9 $SYSDIR\drivers\etc\hosts a ;追加数据
	;FileSeek $9 0 END ;指针定位到相对于文件结尾
	;下面自己加
	;SetFileAttributes "$SYSDIR\drivers\etc\hosts" READONLY ;加只读属性
	;FileClose $9 ;关闭打开的文件

SectionEnd

Section -Post
	WriteRegDword HKLM "${PRODUCT_DIR_REGKEY}" "Installed" 1
SectionEnd


Function .onInstSuccess
  SelfDel::Del
  SetAutoClose true
FunctionEnd
