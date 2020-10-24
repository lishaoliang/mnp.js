# sudo apt install nodejs
# sudo npm install -g uglify-js
# uglifyjs libmnp.js mnp.js -o mnp-1.0.0.min.js -minify
#
# 编译命令 : make

SHELL = /bin/bash


# 引入一些环境参数
-include ./make_define
MY_LIB_PATH = ./$(MY_LIB_NAME)


# 从目录检索需要编译的c文件
MY_DIRS := ./src_c ./src_c/mem ./src_c/hash ./src_c/list ./src_c/thread ./src_c/em_util ./src_c/em_ws ./src_c/em_gl
MY_DIRS += ./src_c/em_net ./src_c/ff_dec ./src_c/main_app ./src_c/em_fetch ./src_c/demux ./src_c/em_audio
#MY_DIRS += ./src_c/test_res


# 第三方库
MY_DIRS += ./third/cJSON-1.7.13
#MY_DIRS += ./third/quickjs


# 编译选项
#MY_CFLAGS := $(my_flags)

# -x c++ 选项 强制使用g++检查静态错误
#MY_CFLAGS += -std=gnu99
#MY_CFLAGS += -x c++


# 使用wasm文件
MY_CFLAGS += -s WASM=1 
MY_CFLAGS += -s TOTAL_MEMORY=67108864


# 优化选项
MY_CFLAGS += -Oz


# 支持FETCH
#MY_CFLAGS += -s FETCH=1


# 使用多线程库; Firefox暂不支持
#MY_CFLAGS += -s USE_PTHREADS=1
#MY_CFLAGS += -s PTHREAD_POOL_SIZE=4


# 导出包装函数
MY_CFLAGS += -s 'EXTRA_EXPORTED_RUNTIME_METHODS=["ccall", "cwrap"]'


# 引用头文件
MY_INCLUDES := -I ./inc -I ./src_c -I ./src
MY_INCLUDES += -I ./src_c/em_fetch -I ./src_c/em_gl -I ./src_c/em_net -I ./src_c/em_util -I ./src_c/em_ws
MY_INCLUDES += -I ./src_c/ff_dec  -I ./src_c/main_app -I ./src_c/demux -I ./src_c/em_audio
#MY_INCLUDES += -I ./src_c/test_res


# 第三方库头文件
MY_INCLUDES += -I ./third/cJSON-1.7.13
MY_INCLUDES += -I ./third/emcc_ffmpeg345_disable_pthreads/include


# 引用的静态库
MY_LIB_STATIC := -L $(MY_LIB_PATH) -Bstatic
MY_LIB_STATIC += -L ./third/emcc_ffmpeg345_disable_pthreads/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswresample -lswscale
MY_LIB_STATIC += -lwebsocket.js
MY_LIB_STATIC += --js-library ./src/libmnpxhr.js --js-library ./src/libmnpfetch.js

# 引用的动态库
MY_LIB_DYNAMIC := -L $(MY_LIB_PATH) -Bdynamic


# 编译目标名称
MY_TARGET_NAME := mnp-1.0.0
MY_TARGET_EXE := $(MY_LIB_PATH)/$(MY_TARGET_NAME).js
MY_LIBMNP_JS := ./src/libmnp.js
MY_LIBMNPUTIL_JS := ./src/libmnputil.js


# 所有编译文件
MY_FIND_FILES_C = $(wildcard $(dir)/*.c)
MY_SOURCES = $(foreach dir, $(MY_DIRS), $(MY_FIND_FILES_C))


MY_LIB_A_OBJS := $(addsuffix .o, $(MY_SOURCES))
#MY_A_PARAMS := $(MY_INCLUDES) $(MY_CFLAGS) $(MY_LIB_STATIC) $(MY_LIB_DYNAMIC)
MY_A_PARAMS := $(MY_INCLUDES) $(MY_CFLAGS)
MY_LIB_ALL := $(MY_LIB_STATIC) $(MY_LIB_DYNAMIC)

.PHONY: all clean js

all: exe

exe: $(MY_TARGET_EXE)
lib: $(MY_TARGET_A)
so: $(MY_TARGET_SO)

%.c.o: %.c
	$(CC) $(MY_A_PARAMS) -c -o $@ $<

$(MY_TARGET_EXE): $(MY_LIB_A_OBJS)
	$(my_tip)
	$(CC) -o $@ $(MY_LIB_A_OBJS) $(MY_A_PARAMS) $(MY_LIB_ALL)
	uglifyjs --comments /^!/ -minify -o $(MY_LIB_PATH)/$(MY_TARGET_NAME).min.js $(MY_LIBMNPUTIL_JS) $(MY_LIBMNP_JS) $(MY_TARGET_EXE)
#	uglifyjs --comments /^!/ -o $(MY_LIB_PATH)/$(MY_TARGET_NAME).min.js $(MY_LIBMNPUTIL_JS) $(MY_LIBMNP_JS) $(MY_TARGET_EXE)

$(MY_TARGET_A): $(MY_LIB_A_OBJS)
	$(my_tip)
	$(CAR) rs $(MY_TARGET_A) $(MY_LIB_A_OBJS)
	$(CRANLIB) $(MY_TARGET_A)

js :
	uglifyjs --comments /^!/ -o $(MY_LIB_PATH)/$(MY_TARGET_NAME).min.js $(MY_LIBMNPUTIL_JS) $(MY_LIBMNP_JS) $(MY_TARGET_EXE)

clean:
	@echo "++++++ make clean ++++++"
	@echo "+ MY_DIRS = $(MY_DIRS)"
	@echo "++ RM = $(RM)"
	$(RM) $(MY_LIB_A_OBJS)
	$(RM) $(MY_TARGET_EXE)
	$(RM) $(MY_LIB_PATH)/$(MY_TARGET_NAME).html
	$(RM) $(MY_LIB_PATH)/$(MY_TARGET_NAME).min.js
	$(RM) $(MY_LIB_PATH)/$(MY_TARGET_NAME).wasm
	@echo "+++++++++++++++++++++++++"


define my_tip
	@echo "++++++ make tip ++++++"
	@echo "+ CC = $(CC)"
	@echo "+ CXX = $(CXX)"
	@echo "+ MY_SOURCES = $(MY_SOURCES)"
	@echo "+ MY_DIRS = $(MY_DIRS)"
	@echo "+ MY_CFLAGS = $(MY_CFLAGS)"
	@echo "+ MY_TARGET_EXE = $(MY_TARGET_EXE)"
	@echo "++++++++++++++++++++++"
endef
