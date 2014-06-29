7z a -tzip wp.zip .\www -xr!ios -xr!android
7z a -tzip android.zip .\www -xr!ios -xr!windows-phone
7z a -tzip ios.zip .\www -xr!windows-phone -xr!android