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
%dxcCmd% /Zi /E"main" /Vn"antiAliasingPixelShader" /Tps_6_0 /Fh"AntiAliasing.psh.h" /nologo AntiAliasing.psh
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"gaussianBlurXPixelShader" /Tps_6_0 /Fh"GaussianBlurX.psh.h" /nologo GaussianBlurX.psh
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"gaussianBlurYPixelShader" /Tps_6_0 /Fh"GaussianBlurY.psh.h" /nologo GaussianBlurY.psh
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"toneMappingPixelShader" /Tps_6_0 /Fh"HDRToneMapping.psh.h" /nologo HDRToneMapping.psh
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"brightPassPixelShader" /Tps_6_0 /Fh"HDRBrightPass.psh.h" /nologo HDRBrightPass.psh
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"meshStandardVertexShader" /Tvs_6_0 /Fh"MeshStandard.vsh.h" /nologo MeshStandard.vsh
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"meshStandardPixelShader" /Tps_6_0 /Fh"MeshStandard.psh.h" /nologo MeshStandard.psh
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"addPointLightToBufferCS" /Tcs_6_0 /Fh"AddPointLightToBufferCS.hlsl.h" /nologo AddPointLightToBufferCS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
ECHO Done.