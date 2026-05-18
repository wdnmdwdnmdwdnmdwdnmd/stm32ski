https://nvlabs.github.io/sionna/rk/quickstart.html

# Checkout the Github repository
git clone --recurse-submodules https://github.com/NVlabs/sionna-rk.git
cd sionna-rk

# 装usrp驱动不用这个也行
./scripts/install-usrp.sh
uhd_find_devices
uhd_usrp_probe

# 拉oai镜像做容器
./scripts/quickstart-oai.sh

# Generate config files
./scripts/generate-configs.sh
# Build plugins specific components (e.g., TRT engines) （）
./plugins/common/build_all_plugins.sh --host       这个可能报错但不影响
./plugins/common/build_all_plugins.sh --container  这个正常就行


# 修改配置文件以适配硬件（这块不确定 可能有问题）
   # config/b200/.env
   - 填入你在 uhd_find_devices 中记下的 **USRP serial number (序列号)**。
   - 确认所需的 PRB（物理资源块）配置，默认是 24（对应 8.64MHz 带宽）。
   # config/common/gnb.sa.band78.24prbs.conf
   - mcc(001) mnc(01) 可调信号相关的参数 
   # config/common/mini_nonrf_config.yaml
   - mcc mnc
   # config/common/nrue.uicc.conf
   - imsi key opc （均与sim卡一致）
   # oai_db.sql
   - 添加一条UE数据
   # sys_config.yaml
   - mcc mnc
   

# For real hardware setup using the USRP 看到容器都healthy代表成功
./scripts/start_system.sh b200

# Or for RF simulations without using real hardware
./scripts/start_system.sh rfsim



### Quectel 常用 AT 命令小抄
AT+CPIN? // 看 SIM 是否识别成功
AT+CIMI // 看 IMSI
AT+CCID // 看 ICCID
AT+QSCAN=2 // 扫描周围 5G 网络
AT+QENG="servingcell" // 看当前实际挂在哪个小区
AT+C5GREG? // 看 5G 注册状态
AT+COPS? // 看当前选网状态
AT+COPS=? // 看模块认为可选的运营商列表
AT+COPS=0 // 恢复自动选网
AT+COPS=1,2,"00101" // 手动选到实验网 001/01
AT+CFUN=0 // 关闭无线功能
AT+CFUN=1 // 重新开启无线功能