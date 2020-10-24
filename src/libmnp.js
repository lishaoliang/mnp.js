/*! 
Copyright(c) 2020, LGPLV3
https://github.com/lishaoliang/mnp.js
https://gitee.com/lishaoliang/mnp.js
*/

var mnpjs = (function(){

var _hello = function(n/*number*/){console.log('mnpjs hello:[', n, ']');return n;};
var _set_hidden = function(hidden){console.log('mnpjs_set_hidden', hidden);};
var _quit = function(status/*number*/){console.log('mnpjs quit');return status;};
var _control = function(cmd/*string*/, lparam/*string*/, wparam/*string*/){return ''};
var _open = function(name/*string*/, param/*string*/){return 0;};
var _close = function(name/*string*/){return 0;};
var _request = function(name/*string*/, req/*string*/){return 0;};
var _get_result = function(id/*number*/){return ''};

// 
var cbMaps = new Map()


var _preRun = function(){
    // the first
    //console.log('mnpjs preRun')

};

var _onRuntimeInitialized = function(){
    // before main			

    _hello = Module.cwrap('mnpjs_hello', 'number', ['number']);
    _set_hidden = Module.cwrap('mnpjs_set_hidden', 'number', ['number']);
    _quit = Module.cwrap('mnpjs_quit', 'number', ['number']);
    _control = Module.cwrap('mnpjs_control', 'string', ['string','string','string']);
    _open = Module.cwrap('mnpjs_open', 'number', ['string', 'string']);
    _close = Module.cwrap('mnpjs_close', 'number', ['string']);
    _request = Module.cwrap('mnpjs_request', 'number', ['string', 'string']);
    _get_result = Module.cwrap('mnpjs_get_result', 'string', ['number']);


    mnpjs.hello(200);
};

var _postRun = function(){
    // after main
    //console.log('mnpjs postRun')

    // Test
    mnpjs.hello(300);

    // {"ip"="127.0.0.1","port"=8000,"protocol"="WS-MNP","path"="/stream/wsmnp/chnn0/sidx1"}
    //mnpjs.open('123456', JSON.stringify({
    //    ip : window.location.hostname,
    //    port : parseInt(window.location.port),
    //    protocol : 'WS-MNP',
    //    path : '/stream/wsmnp/chnn0/sidx1'
    //}), function(result/*obj*/){
    //    console.log('open ' + window.location.hostname + ':' + window.location.port + '/stream/wsmnp/*', result);
    //
    //    setTimeout(function(){
    //        // test 10s 
    //        //mnpjs.close('123456');
    //    }, 10000);
    //});
};

var _print = (function() {
    return function(text) {
        if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
        // These replacements are necessary if you render to raw HTML
        //text = text.replace(/&/g, "&amp;");
        //text = text.replace(/</g, "&lt;");
        //text = text.replace(/>/g, "&gt;");
        //text = text.replace('\n', '<br>', 'g');
        console.log(text);
    };
  })();

var _printErr = function(text){
    if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
    console.log(text);
};

var _canvas = (function() {
    var canvas = document.getElementById('canvas');

    // As a default initial behavior, pop up an alert when webgl context is lost. To make your
    // application robust, you may want to override this behavior before shipping!
    // See http://www.khronos.org/registry/webgl/specs/latest/1.0/#5.15.2
    canvas.addEventListener("webglcontextlost", function(e) { alert('WebGL context lost. You will need to reload the page.'); e.preventDefault(); }, false);

    return canvas;
  })();

var _setStatus = function(text){
    //if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
    //console.log(text);
};

var getResult = function(id){
    var ptr = _get_result(id);

    if (ptr) {
        var s = new String(ptr);
        _free(ptr);
        return s;
    };

    return '';
};

// 监听页面是否激活
// 非激活情况下: 解码,显示可能丢弃部分数据
var listenerVisibility = function() {
    // https://www.cnblogs.com/csuwujing/p/10315309.html
    var hiddenProperty = 'hidden' in document ? 'hidden' :    
                        'webkitHidden' in document ? 'webkitHidden' :    
                        'mozHidden' in document ? 'mozHidden' :    
                        null;

    var visibilityChangeEvent = hiddenProperty.replace(/hidden/i, 'visibilitychange');

    document.addEventListener(visibilityChangeEvent, function(){
        if (!document[hiddenProperty]) {    
            //console.log('document show');
            _set_hidden(false);
        }else{
            //console.log('document hidden');
            _set_hidden(true);
        }
    });
};

listenerVisibility();

return {
    quit : _quit,
    preRun : _preRun,
    onRuntimeInitialized : _onRuntimeInitialized,
    postRun : _postRun,
    print : _print,
    printErr : _printErr,
    canvas : _canvas,
    setStatus : _setStatus,

    callback : function(id/*number*/, status/*number*/){

        var result = JSON.parse(getResult(id));
        var cb = cbMaps.get(id.toString());
        cbMaps.delete(id.toString());

        cb(result);
        //console.log('mnpjs.callback', id, status, s);

        return 0;
    },

    hello : function(n/*number*/){ _hello(n); },

    control : function(cmd/*string*/,lparam/*string*/,wparam/*string*/){
        var ptr = _control(cmd, lparam, wparam);

        if (ptr) {
            var s = new String(ptr);
            _free(ptr);
            return s;
        };

        return '';
    },

    open : function(name/*string*/, param/*obj*/, cb/*function*/){
        var json = JSON.stringify(param)
        var id = _open(name, json);
        if (0 < id) {
            cbMaps.set(id.toString(), cb)
        } else {
            cb({code:1})
        }
    },

    close : function(name/*string*/){
        var ret = _close(name);

        return ret;
    },

    request : function(name/*string*/, req/*string*/, cb/*function*/){
        var id = _request(name, req);
    }
};

})();

var Module = typeof Module !== 'undefined' ? Module : mnpjs;
