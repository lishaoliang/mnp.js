// libmnpfetch.js

mergeInto(LibraryManager.library, {

    mnpfetch_open : function (url, json_param, user_data, cb_open, cb_recv, cb_error) {

        var open = function(url, json_param, user_data, cb_open, cb_recv, cb_error) {

            // https://www.cnblogs.com/wonyun/p/fetch_polyfill_timeout_jsonp_cookie_progress.html
            // https://jakearchibald.com/2016/streams-ftw/
            fetch(url, {method: "GET"}).then(function(response){
                response.status     //=> number 100–599
                response.statusText //=> String
                response.headers    //=> Headers
                response.url        //=> String
  
                if (200 == response.status) {
                    dynCall_iii(cb_open, user_data, 0); // 连接成功
                } else {
                    dynCall_iii(cb_open, user_data, 1); // 连接失败
                }

                // response.body is a readable stream.
                // Calling getReader() gives us exclusive access to the stream's content
                var reader = response.body.getReader();
                var bytesReceived = 0;
  
                // read() returns a promise that resolves when a value has been received
                reader.read().then(function processResult(result) {
                    // Result objects contain two properties:
                    // done  - true if the stream has already given you all its data.
                    // value - some data. Always undefined when done is true.
                    if (result.done) {
                        console.log("fetch complete");
                        return;
                    }
    
                    // result.value for fetch streams is a Uint8Array
                    bytesReceived += result.value.length;
                    //console.log("result.value", result.value)
                    //console.log('Received', bytesReceived, 'bytes of data so far');
                    
                    var len = result.value.length;
                    var buf = _malloc(len);
                    HEAP8.set(new Uint8Array(result.value), buf);
                    dynCall_iiii(cb_recv, user_data, buf, len);
                    _free(buf);
    
                    // Read some more, and call this function again
                    return reader.read().then(processResult);
                });
            }, function(error){
                console.log("fetch error", error);
            }).catch(function(error){
                console.log("fetch catch", error);
            });
        };
        
        open(UTF8ToString(url), UTF8ToString(json_param), user_data, cb_open, cb_recv, cb_error);

        return 0;
    },

    mnpfetch_close : function(id) {

        return
    }
})
