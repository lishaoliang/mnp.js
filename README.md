# mnp.js

c, js, wasm, ffmpeg, webgl, http-flv, ws-flv, http-mnp, ws-mnp

基于wasm、ffmpeg解码、webgl显示, 播放http-flv/ws-flv等实时流(直播流)

特点:
1. 使用Ffmpeg的CPU软解码H264/H265等
2. 使用WebGl显示
3. 支持Windows,iOS,Android等系统新wasm标准的任何浏览器
4. 支持Web底层连接: http-fetch, http-xhr, websocket
5. 应用层分包协议: FLV, MNP(私有分包)
6. 可扩展自定义分包协议: 添加"./mnp.js/src_c/demux/xxx"解复用,并加入支持协议集
7. 极低延时: 局域网(高速网络)播放端延时100ms(缓存2-3帧/30fps), 总延时可低至200-300ms(30fps)
8. 延时场景可调配: 200ms, 500ms, 1000ms, 2000ms, 4000ms, 6000ms

性能:

* PC windows
* CPU: i5 3.00 GHz
* 室内 H264@1080P@30fps

```
Google Chrome 84.0.4147.105(64位)
CPU: 约占 18%
GPU: 约占 11%
内存: 200M
```

```
Wasm文档:
https://emscripten.org/index.html

安装emcc:
https://emscripten.org/docs/getting_started/downloads.html

安装uglifyjs:
sudo npm install -g uglify-js
```

## 编译
* Ubuntu 16.04.6 LTS

```
cd ./mnp.js
make
```

## 生成的模块文件
```
./mnp.js/lib/mnp-1.0.0.wasm
./mnp.js/lib/mnp-1.0.0.min.js
```

## 使用流程简介
### Step1. 页面加载完成后,将模块指定为mnpjs

```
$(document).ready(function(){
  var Module = mnpjs
  ...
}
```

### Step2. 重写模块的"postRun"函数
* 此函数在模块初始化完成之后调用

```
mnpjs.postRun = function() {
  ...
}
```

### Step3. 打开实时流媒体链接

```
mnpjs.postRun = function() {
  mnpjs.open('123456', {
    ip : window.location.hostname,
    port : parseInt(window.location.port),
    protocol : 'WS-FLV',
    path : '/stream/wsflv/chnn0/sidx1'
  }, function(result/*obj*/){
    console.log('open /stream/wsflv/*', result);
  });

  ... //
}
```

## API
* 参见"./mnp.js/src/libmnp.js"

## 测试API


## 设计


## H5相关播放器

```
https://github.com/bilibili/flv.js
https://github.com/langhuihui/jessibuca
https://github.com/sonysuqin/WasmVideoPlayer
```
