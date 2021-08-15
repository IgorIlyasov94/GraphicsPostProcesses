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
%dxcCmd% /Zi /E"main" /Vn"screenQuadVS" /Tvs_6_0 /Fh"ScreenQuadVS.hlsl.h" /nologo ScreenQuadVS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"antiAliasingPS" /Tps_6_0 /Fh"AntiAliasingPS.hlsl.h" /nologo AntiAliasingPS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"gaussianBlurXPS" /Tps_6_0 /Fh"GaussianBlurXPS.hlsl.h" /nologo GaussianBlurXPS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"gaussianBlurYPS" /Tps_6_0 /Fh"GaussianBlurYPS.hlsl.h" /nologo GaussianBlurYPS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"toneMappingPS" /Tps_6_0 /Fh"HDRToneMappingPS.hlsl.h" /nologo HDRToneMappingPS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"brightPassPS" /Tps_6_0 /Fh"HDRBrightPassPS.hlsl.h" /nologo HDRBrightPassPS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"meshStandardVS" /Tvs_6_0 /Fh"MeshStandardVS.hlsl.h" /nologo MeshStandardVS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"meshStandardPS" /Tps_6_0 /Fh"MeshStandardPS.hlsl.h" /nologo MeshStandardPS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"setPointLightCS" /Tcs_6_0 /Fh"SetPointLightCS.hlsl.h" /nologo SetPointLightCS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"distributePointLightCS" /Tcs_6_0 /Fh"DistributePointLightCS.hlsl.h" /nologo DistributePointLightCS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"calculateClusterCoordinatesCS" /Tcs_6_0 /Fh"CalculateClusterCoordinatesCS.hlsl.h" /nologo CalculateClusterCoordinatesCS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"updateParticleSystemCS" /Tcs_6_0 /Fh"UpdateParticleSystemCS.hlsl.h" /nologo UpdateParticleSystemCS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"sortParticleSystemCS" /Tcs_6_0 /Fh"SortParticleSystemCS.hlsl.h" /nologo SortParticleSystemCS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"particleStandardVS" /Tvs_6_0 /Fh"ParticleStandardVS.hlsl.h" /nologo ParticleStandardVS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"particleStandardGS" /Tgs_6_0 /Fh"ParticleStandardGS.hlsl.h" /nologo ParticleStandardGS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"particleStandardPS" /Tps_6_0 /Fh"ParticleStandardPS.hlsl.h" /nologo ParticleStandardPS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"uiSpriteStandardVS" /Tvs_6_0 /Fh"UISpriteStandardVS.hlsl.h" /nologo UISpriteStandardVS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"uiSpriteStandardPS" /Tps_6_0 /Fh"UISpriteStandardPS.hlsl.h" /nologo UISpriteStandardPS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"uiTextStandardVS" /Tvs_6_0 /Fh"UITextStandardVS.hlsl.h" /nologo UITextStandardVS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"uiTextStandardPS" /Tps_6_0 /Fh"UITextStandardPS.hlsl.h" /nologo UITextStandardPS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"clothApplyConstaintsCS" /Tcs_6_0 /Fh"ClothApplyConstaintsCS.hlsl.h" /nologo ClothApplyConstaintsCS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"clothApplyExternalConstraintsCS" /Tcs_6_0 /Fh"ClothApplyExternalConstraintsCS.hlsl.h" /nologo ClothApplyExternalConstraintsCS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"clothApplyForcesCS" /Tcs_6_0 /Fh"ClothApplyForcesCS.hlsl.h" /nologo ClothApplyForcesCS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"clothApplySelfCollisionCS" /Tcs_6_0 /Fh"ClothApplySelfCollisionCS.hlsl.h" /nologo ClothApplySelfCollisionCS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"clothRecalculateTangentsCS" /Tcs_6_0 /Fh"ClothRecalculateTangentsCS.hlsl.h" /nologo ClothRecalculateTangentsCS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"emptyVS" /Tvs_6_0 /Fh"EmptyVS.hlsl.h" /nologo EmptyVS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"onlyVertexIdOutputVS" /Tvs_6_0 /Fh"OnlyVertexIdOutputVS.hlsl.h" /nologo OnlyVertexIdOutputVS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"clothStandardVS" /Tvs_6_0 /Fh"ClothStandardVS.hlsl.h" /nologo ClothStandardVS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
%dxcCmd% /Zi /E"main" /Vn"clothStandardPS" /Tps_6_0 /Fh"ClothStandardPS.hlsl.h" /nologo ClothStandardPS.hlsl
@IF %ERRORLEVEL% NEQ 0 (EXIT /b %ERRORLEVEL%)
ECHO Done.