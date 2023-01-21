Setup
  1. Follow the installation guide(https://code.visualstudio.com/docs/cpp/config-mingw) to download MinGW C++ compiler.
  2. Using VSCode, edit the task.json file to include all C++ file by adding the following args:
	"args": [
                "-g",
                "${workspaceFolder}/*.cpp",
                "-o",
                "${fileDirname}\\${fileBasenameNoExtension}.exe"
		]
  3. Compile and run main.cpp.