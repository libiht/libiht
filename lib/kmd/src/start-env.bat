# Enable Test Signing Mode
bcdedit /set testsigning on

# Enable Debug Mode
bcdedit -debug on

# Enable Boot Debugging
bcdedit /bootdebug on

# Set debug settings to network debugging, please change the xxxxx parts to the IP and port of your machine, KEY can be set or left unset
bcdedit /dbgsettings NET HOSTIP:192.168.xx.xx PORT:xxxxx KEY:xxxxx

