@ECHO OFF
cd %~dp0
ECHO Installing OpenPL

cd External

ECHO Installing JUCE

if exist JUCE\ (
	ECHO JUCE is already installed
) else (
	mkdir JUCE
	git clone https://github.com/juce-framework/JUCE.git --branch 6.0.7 --depth 1 JUCE
)

ECHO Installing libigl

if exist libigl (
	ECHO libigl is already installed
) else (
	mkdir libigl

	git clone https://github.com/libigl/libigl.git --branch v2.3.0 --depth 1 libigl
    
    	cd libigl
   	mkdir external
    	cd external

    	ECHO Installing lbigl dependencies
    	mkdir eigen
    	mkdir glad
    	mkdir glfw

    	ECHO Installing eigen
    	git clone https://github.com/libigl/eigen.git --depth 1 eigen
    
    	ECHO Installing glad
    	git clone https://github.com/libigl/libigl-glad.git --depth 1 glad
    
    	ECHO Installing glfw
    	git clone https://github.com/glfw/glfw.git --depth 1 glfw

	cd ..\..\
)

ECHO Installing matplotplusplus

if exist matplotplusplus (
	ECHO matplot++ installed
) else (
	mkdir matplotplusplus
	git clone https://github.com/alandefreitas/matplotplusplus.git --branch v1.0.1 --depth 1 matplotplusplus
)

if exist vcpkg (
	ECHO Vcpkg manager installed
) else (
	mkdir vcpkg
	git clone https://github.com/microsoft/vcpkg vcpkg
	cd vcpkg
	call bootstrap-vcpkg.bat

	cd ..
)

ECHO Using vcpkg to install dependencies

if exist vcpkg (

	ECHO Downloading dependencies
	cd vcpkg

	.\vcpkg.exe install yasm-tool:x86-windows
	.\vcpkg.exe install cgal:x64-windows
	.\vcpkg.exe install boost:x64-windows
	.\vcpkg.exe install glfw3:x64-windows
	
	.\vcpkg integrate install
	
	cd ..
	ECHO Done downloading
) else (
	ECHO Error! Can't find vcpkg
)

PAUSE
