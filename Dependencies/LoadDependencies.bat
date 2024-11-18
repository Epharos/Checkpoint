REM Vulkan SDK download
echo "Downloading Vulkan SDK. You will have to install it manually."
curl -O https://sdk.lunarg.com/sdk/download/1.3.296.0/windows/VulkanSDK-1.3.296.0-Installer.exe
cls

REM GLFW SETUP
echo "Downloading GLFW."
git clone https://github.com/glfw/glfw.git
curl -L https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.bin.WIN64.zip > glfw_libs.zip
unzip glfw_libs.zip
del glfw_libs.zip
cd glfw-3.4.bin.WIN64\lib-vc2022
mkdir ..\..\glfw\libs
move glfw3.lib ..\..\glfw\libs
cd ..\..
rmdir glfw-3.4.bin.WIN64
cls

echo "Dependencies downloaded. You can now build the project."