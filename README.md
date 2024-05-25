﻿# Portable Launcher

The Portable Launcher is designed to set the `AppData` and `LocalAppData` environment variables for the target application before launch. This ensures that all data generated by the application is centralized, making it portable and easy to manage.

## How to Use

1. Choose the appropriate launcher based on the type of target application:
    - Window launcher for GUI applications
    - Console launcher for command-line tools

2. Rename the launcher to `<TargetApplicationName>.launcher.exe`, where `<TargetApplicationName>` is the name of the application you wish to launch.

3. Execute the launcher to start your application with the new environment settings.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE.md) file for details.