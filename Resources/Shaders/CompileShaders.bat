@ECHO OFF
:: Search dxc.exe on current PATH. 
where dxc.exe >nul 2>nul
IF %ERRORLEVEL%==0 (
    SET dxcCmd=dxc.exe
	GOTO COMPILE_SHADER
)

:: Search dxc.exe on 15063 SDK installtion path.
dir "%PROGRAMFILES(x86)%\Windows Kits\10\bin\10.0.17763.0\x86\dxc.exe" >nul 2>nul
IF %ERRORLEVEL%==0 (
	SET dxcCmd="%PROGRAMFILES(x86)%\Windows Kits\10\bin\10.0.17763.0\x86\dxc.exe"
	GOTO COMPILE_SHADER
)

:DXC_NOT_FOUND
ECHO Error: dxc.exe does not exist somewhere on PATH or on %PROGRAMFILES(x86)%\Windows Kits\10\bin\10.0.17763.0\x86\
EXIT /b 1

:COMPILE_SHADER
ECHO DXC Path: %dxcCmd%
ECHO Start compiling shaders...
ECHO ON
%dxcCmd% /Zi /E"main" /Vn"quadVertexShader" /Tvs_6_0 /Fh"ScreenQuad.vsh.h" /nologo ScreenQuad.vsh
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"toneMappingPixelShader" /Tps_6_0 /Fh"HDRToneMapping.psh.h" /nologo HDRToneMapping.psh
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
ECHO Done.