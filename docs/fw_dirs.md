## mnp.js项目目录组织

* c代码入口: "./mnp.js/src_c/main.c"
* js代码入口: "./mnp.js/src/libmnp.js"

```
mnp.js
├─docs ------------------------- 文档
│  |fw_dirs.md ----------------- 项目目录组织
│  └fw_modules.md -------------- 项目模块组织
├─emcc_include ----------------- 从emscripten编译器中拷贝的头文件,仅用于提示函数参考
├─inc -------------------------- 从klb项目拷贝过来基础头文件
│  |klb_type.h ----------------- 基础类型等
│  ├─hash ---------------------- hash算法
│  ├─list ---------------------- 链表等
│  ├─log ----------------------- 打印
│  ├─mem ----------------------- 内存
│  ├─mnp ----------------------- 私有MNP协议
│  └─thread -------------------- pthread等线程相关
├─lib -------------------------- 生成的js库及测试html文件目录
├─proj ------------------------- IDE工程环境
├─src -------------------------- js代码目录
│  |libmnp.js ------------------ 封装c提供原始接口
│  |libmnpfetch.h -------------- 使用fetch提供给c头文件定义
│  |libmnpfetch.js ------------- 使用fetch提供给c做流媒体的连接
│  |libutil.js ----------------- js部分常用函数
│  |libmnpxhr.h ---------------- 使用xhr提供给c头文件定义
│  └libmnpxhr.js --------------- 使用xhr提供给c做流媒体的连接
├─src_c ------------------------ c代码目录
│  ├─demux --------------------- 应用层解复用协议
│  │  ├─em_flv_demux ----------- flv解复用
│  │  └─em_mnp_demux ----------- mnp解复用
│  ├─em_audio ------------------ 播放音频
│  │  └─em_audio_ctx ----------- 音频播放设备
│  ├─em_fetch ------------------ fetch方式连接
│  │  ├─em_fetch_flv ----------- fetch-flv连接
│  │  ├─em_fetch_mnp ----------- fetch-mnp连接
│  │  └─em_fetch_stream -------- fetch方式通用流处理
│  ├─em_gl --------------------- webgl的基础c封装
│  │  ├─gl_ctx ----------------- gl环境
│  │  ├─gl_program ------------- glsl程序
│  │  └─gl_yuv ----------------- 显示yuv纹理
│  ├─em_net -------------------- 网络部分基础封装及管理
│  │  ├─em_conn_manage --------- 所有连接管理
│  │  ├─em_socket -------------- bsd/unix风格,非阻塞socket封装(弃用)
│  │  └─em_socket_manage ------- socket管理(websocket)
│  ├─em_util ------------------- emscripten通用函数
│  │  ├─em_buf ----------------- 音视频帧等中间缓存
│  │  ├─em_buf_mnp ------------- mnp协议buf辅助函数
│  │  ├─em_frame_yuv_wav ------- yuv(图像)/wav(音频)帧定义
│  │  ├─em_log ----------------- 打印输出
│  │  └─em_timer --------------- 时间相关封装
│  ├─em_ws --------------------- websocket方式连接
│  │  ├─em_ws ------------------ websocket方式通用流处理
│  │  ├─em_ws_flv -------------- websocket-flv连接
│  │  └─em_ws_mnp -------------- websocket-mnp连接
│  ├─ff_dec -------------------- ffmpeg解码模块
│  │  ├─ff_dec_single ---------- 单路音视频解码
│  │  ├─ff_rate_ctrl* ---------- 播放帧率/延时等控制策略
│  │  ├─ffmepg_dec ------------- ffmpeg解码视频
│  │  └─ffmepg_dec_audio ------- ffmpeg解码音频
│  ├─hash ---------------------- hash算法
│  │  ├─klb_hash* -------------- 基础hash算法
│  │  ├─klb_hlist -------------- hash table与list组合使用
│  │  └─klb_htab --------------- hash table查找算法
│  ├─list ---------------------- 链表
│  │  └─klb_list --------------- 仿std::list非侵入式精简双向链表
│  ├─main_app ------------------ 主APP
│  │  └─main_app* -------------- 主APP"类"
│  ├─mem ----------------------- 内存
│  ├─test_res ------------------ 测试使用的资源
│  ├─thread -------------------- pthread等线程相关
│  └main.c --------------------- main函数入口及导出给js的接口函数
├─third ------------------------ 使用到的第三方库
|make_define ------------------- 交叉编译工具链选择
|Makefile ---------------------- make脚本
└README.md --------------------- 根说明文件
```