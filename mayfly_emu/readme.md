### 作为模拟器模式
首先Makefile中SHARE = 0，然后common.h中设置 #define CONFIG_SHARE_SO 0。
common.h中CONFIG_FLASH宏定义用于判断为flash模式还是Mem模式区别在刚开始执行指令的地址。flash模式下指令执行的初始地址为0x3000_0000。mem模式下指令执行的初始地址为0x8000_0000

### 作为参考模型模式
首先Makefile中SHARE = 1，然后common.h中设置 #define CONFIG_SHARE_SO 1。