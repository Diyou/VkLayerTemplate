# C++ Vulkan Layer Template

##### Minimal Vulkan Layer c++ code with vscode support for beginning and debugging.

### How-to

- Configure & Build using CMakePresets
  - VSCode: Use CMake Tools UI
  - CLI:
    ```sh
    cmake --preset=debug && cmake --build --preset=debug
    ```
- Debug
  - VSCode: [F5] Or manually launch the configuration (launch.json)
  - CLI: From the output directory Load the debug.env file and start Launcher with an argument like:
    - bash
      ```bash
      export $(grep -v '^#' debug.env | xargs) && Launcher/Launcher vkgears
      ```
    - powershell
      ```powershell
      Get-Content debug.env | Where-Object { $_ -notmatch '^\s*#' -and $_ -notmatch '^\s*$' } | ForEach-Object { $k, $v = $_.Split('=', 2); [System.Environment]::SetEnvironmentVariable($k, $v) }; .\Launcher\Launcher.exe vkgears.exe
      ```
