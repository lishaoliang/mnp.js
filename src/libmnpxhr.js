// libmnpxhr.js

mergeInto(LibraryManager.library, {

    mnpxhr_open: function (url, json_param, user_data, cb_open, cb_recv, cb_error) {
        var open = function(url) {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", url, true);
            //xhr.responseType = 'moz-chunked-arraybuffer';
            xhr.responseType = "text";	
            xhr.url = url;
            xhr.onreadystatechange = function(e) {
                //console.log("xhr.onreadystatechange", xhr.readyState);
                if (xhr.readyState == 4) {

                }
            };

            xhr.addEventListener('progress', function(e){
                var chunk = e.target.response;
                console.log("xhr.onprogress", chunk.byteLength);
            });

            //xhr.onprogress = function(e) {
            //    var chunk = xhr.response;
            //    console.log("xhr.onprogress", chunk.byteLength);
            //};

            //xhr.onloadend = function(e){
            //    console.log("xhr.onloadend")
            //};

            xhr.onerror = function(e){
                console.log("xhr.onerror")
            };

            xhr.send();
        };

        return open(UTF8ToString(url));
    },

    mnpxhr_close : function(id){
        return
    }
})
