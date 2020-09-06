说明：

1、此目录用来存放slic芯片相关配置参数的profile文件，china文件里是全集，其他文件是地区或者国家有其他要求时生成的差异部分参数文件。

2、新增国家码时，请在slic_profile.h里添加对应国家头文件，不要在slic_ctl.c里增加。
   同时在slic_ctl.c里按照国家码枚举顺序增加slic_customize_ex_parm和slic_profile对应参数，否则功能不正常。