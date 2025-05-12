REM VULKAN SDK DOWNLOAD
echo "Downloading Vulkan SDK. You will have to install it manually."
curl -O https://sdk.lunarg.com/sdk/download/1.4.304.0/windows/VulkanSDK-1.4.304.0-Installer.exe
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

REM SLANG
echo "Slang compiler"
curl -OL https://github.com/shader-slang/slang/releases/download/v2024.14.5/slang-2024.14.5-windows-x86_64.zip
mkdir slang
cd slang
unzip ../slang-2024.14.5-windows-x86_64.zip
del ../slang-2024.14.5-windows-x86_64.zip
cd ..
cls

vcpkg install spirv-tools

echo "Dependencies downloaded. You can now build the project."