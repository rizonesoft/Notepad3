This directory contains files used by the portable app and should generally not be accessed directly by users except in specific instances of using plugins or other documented additions to a given app (see below).

User files, data, or settings should not be stored within the App directory or its subdirectories. Any data stored within the App directory structure will likely be deleted on upgrades.

An exception to this is apps that use plugins. Generally, these directories will be preserved on upgrades. You can confirm which directories are preserved within the installer.ini file inside the App\AppInfo directory. If there are any plugin directories which are not preserved which should be, please let the publisher of this portable app know so they can address the issue.