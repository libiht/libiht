# 启用测试签名模式
bcdedit /set testsigning on

# 启用调试模式
bcdedit -debug on

# 启用启动调试功能
bcdedit /bootdebug on

# 设置调试设置为网络调试，请将 xxxxx 部分更改为你的机器的 IP 和端口，KEY 可以设置也可以不设置
bcdedit /dbgsettings NET HOSTIP:192.168.xx.xx PORT:xxxxx KEY:xxxxx
