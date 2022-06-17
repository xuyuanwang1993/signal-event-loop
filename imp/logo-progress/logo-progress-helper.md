主要功能点:
1.多屏logo显示
2.debug模式下显示可用ip->10.* 172.* 192.168.*
3.进度条


屏幕配置:
{
{
name : hdmi
is_primary : true
logo_path : /var/logo/***.bmp
flip_type : 2
flip_angle : 90
},
{
name : DSI
is_primary : false
logo_path : /var/logo/***.bmp
flip_type : 0
flip_angle : 0
}
}
1.设备名称如HDMI DSI    当设置为空时，代表使用任意可用屏
2.是否是主屏 仅主屏显示额外的信息，副屏只显示logo
3.logo路径

环境变量:
LOGO_PROGRESS_RED  进度条及文字颜色
LOGO_PROGRESS_GREEN 进度条及文字颜色
LOGO_PROGRESS_BLUE 进度条及文字颜色
LOGO_PROGRESS_DEBUG 是否开启debug模式，仅debug模式下会显示ip
渲染处理逻辑:
渲染任务采用任务队列的方式进行驱动，默认调度间隔40ms，有任务时立即唤醒
1.初始化时加载底图logo，创建suface_logo
2.渲染外部任务处理
1)重新初始化
2)显示ip
3)进度条处理
4)退出
3.渲染执行
底图->ip->进度条


热插拔处理:ip检测:


程序外部控制:使用本地socket处理
