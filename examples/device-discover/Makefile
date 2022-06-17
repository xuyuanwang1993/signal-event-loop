#Makefile
#config
#项目名
#########################################change############################################
APP_NAME ?= device-discover
###########################################################################################

#交叉编译工具前缀
CROSS_COMPILE ?=
#安装目录前缀
PREFIX ?= /usr/local
#Sysroot
SYSROOT ?=
#编译输出目录
#########################################change############################################
OUT_PATH ?= ../out
###########################################################################################

#库前缀
LIB_PREFIX ?= lib
#静态库后缀
STATIC_LIB_SUFIX ?= a
#动态库后缀
DLL_LIB_SUFIX ?= so
#链接选项
LINK_OPTS ?= -rdynamic 
#.o文件输出目录
OBJS_OUT_PATH = $(OUT_PATH)/objs/$(APP_NAME)
#头文件输出目录
HEADER_OUT_PATH = $(OUT_PATH)/include/$(APP_NAME)
#库文件输出目录
LIB_OUT_PATH = $(OUT_PATH)/lib/$(APP_NAME)
#执行文件输出目录
EXEC_OUT_PATH = $(OUT_PATH)/bin/$(APP_NAME)
#C++编译器
CXX = $(CROSS_COMPILE)g++ --sysroot=$(SYSROOT)/
#C编译器
CC = $(CROSS_COMPILE)gcc  --sysroot=$(SYSROOT)/
#安装时去除链接符号
STRIP = $(CROSS_COMPILE)strip
#静态库编译命令
STATIC_LIBRARY_LINK =		$(CROSS_COMPILE)ar -cr 
#动态库编译命令
SHARED_LIB_COMPILE= $(CXX) -shared 
#编译搜索目录
LOCAL_INCLUDE = -I.
#外部包含目录
#########################################change############################################
EXTERN_INCLUDE += 

###########################################################################################

#本地静态链接库
#########################################change############################################
LOCAL_STATIC_LIBS =
###########################################################################################
#本地动态链接库 会被拷至运行文件目录
#########################################change############################################
LOCAL_SHARDE_LIBS = 

###########################################################################################
#本地动态链接库搜索目录
LOCAL_SHARDE_LIBS_PATH =$(addprefix -L,$(dir $(LOCAL_SHARDE_LIBS)))

#本地动态链接库链接选项
LOCAL_SHARDE_LIBS_DEPS =$(addprefix -l,$(patsubst $(LIB_PREFIX)%,%,$(notdir $(basename $(LOCAL_SHARDE_LIBS)))))

#外部库依赖
#########################################change############################################
EXTERN_LIBS_LINK += $(LOCAL_SHARDE_LIBS_PATH) $(LOCAL_SHARDE_LIBS_DEPS) -lpthread
###########################################################################################

#外部头文件依赖
#########################################change############################################
EXTERN_INCLUDE_PATH +=
###########################################################################################
#C编译选项
CFLAGS ?=
LOCAL_CFLAGS = -O2 -Wl,--gc-sections -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8 -fPIC
LOCAL_CFLAGS += $(CFLAGS)
#C++编译选项
CXXFLAGS ?=
LOCAL_CXXFLAGS = $(LOCAL_CFLAGS) -std=c++11 -ffunction-sections -fdata-sections
LOCAL_CXXFLAGS += $(CXXFLAGS)
#########################################change############################################
WORK_DIR += 	. ../../core/ 
WORK_DIR +=		../../imp  
WORK_DIR += 	../../utils 
WORK_DIR +=  	../../third_party/json 
WORK_DIR +=  	../../log   
WORK_DIR +=  	../../ 
WORK_DIR +=  	../../imp/device-discover 
WORK_DIR += 	../../imp/can-utils 

###########################################################################################
SRC_FILE += $(wildcard *.c)
SRC_FILE += $(wildcard *.cpp)
HEADER_FILE +=$(wildcard *.h)
ifneq ($(WORK_DIR), "")
SRC_FILE += $(patsubst %,%/*.c,$(WORK_DIR))
SRC_FILE += $(sort $(patsubst %,%/*.cpp,$(WORK_DIR)))
HEADER_FILE +=$(wildcard $(patsubst %,%/*.h,$(WORK_DIR)))
endif
#所有的源文件
CPP_SOURCE =$(filter %.c %.cpp,$(wildcard $(SRC_FILE)))
#编译搜索目录
LOCAL_INCLUDE += $(patsubst %,-I%,$(WORK_DIR))
LOCAL_INCLUDE += $(patsubst %,-I%,$(EXTERN_INCLUDE))
LOCAL_INCLUDE += $(patsubst %,-I%,$(EXTERN_INCLUDE_PATH))
#所有输出目标
ALL_OBJECTS = $(sort $(addprefix $(OBJS_OUT_PATH)/,$(addsuffix .o,$(basename $(notdir $(CPP_SOURCE))))))
#运行部分所需的源文件目标和头文件依赖
#########################################change############################################
MAIN_OBJECTS = $(OBJS_OUT_PATH)/main.o
MAIN_HEADER =
 ###########################################################################################
#库的源文件依赖及头文件
LIB_OBJECTS = $(filter-out $(MAIN_OBJECTS),$(ALL_OBJECTS)) 
LIB_HEADERS = $(filter-out $(MAIN_HEADER),$(HEADER_FILE)) 

ENV_FILE = $(OBJS_OUT_PATH)/.compile_env
.PHONY :all prepare lib app clean help distclean install uninstall
all : lib app
prepare : 
	-mkdir -m 755 -p $(OBJS_OUT_PATH)
	-mkdir -m 755 -p $(HEADER_OUT_PATH)
	-mkdir -m 755 -p $(LIB_OUT_PATH)
	-mkdir -m 755 -p $(EXEC_OUT_PATH)
	@rm -rf $(ENV_FILE)
	@echo  "export CC=\"$(CC)\"" >> $(ENV_FILE)
	@echo  "export CXX=\"$(CXX)\"" >> $(ENV_FILE)
	@echo  "export LOCAL_CFLAGS=\"$(LOCAL_CFLAGS)\"" >> $(ENV_FILE)
	@echo  "export LOCAL_INCLUDE=\"$(LOCAL_INCLUDE)\"" >> $(ENV_FILE)
	@echo  "export LOCAL_CXXFLAGS=\"$(LOCAL_CXXFLAGS)\"" >> $(ENV_FILE)
	@echo  "export CPP_SOURCE=\"$(CPP_SOURCE)\"" >> $(ENV_FILE)
	echo ${LIB_OBJECTS}
	echo ${ALL_OBJECTS}
lib : prepare $(LIB_OBJECTS)
	cp -ruf  $(LIB_HEADERS) $(HEADER_OUT_PATH)
	-$(SHARED_LIB_COMPILE) -o $(LIB_OUT_PATH)/$(LIB_PREFIX)$(APP_NAME).$(DLL_LIB_SUFIX) $(LIB_OBJECTS)
	-$(STATIC_LIBRARY_LINK) $(LIB_OUT_PATH)/$(LIB_PREFIX)$(APP_NAME).$(STATIC_LIB_SUFIX) $(LIB_OBJECTS)
app : prepare $(ALL_OBJECTS)
	$(CXX) -o $(EXEC_OUT_PATH)/$(APP_NAME) $(ALL_OBJECTS) $(LOCAL_STATIC_LIBS) $(LINK_OPTS) $(EXTERN_LIBS_LINK) 
	if [ ! -z "$(LOCAL_SHARDE_LIBS)" ] ; then cp -ruf $(LOCAL_SHARDE_LIBS) $(EXEC_OUT_PATH) ; fi
help :
	@echo  "support cmd {'prepare' 'lib' 'app' 'clean' 'distclean' 'install' 'uninstall'}"
	@echo  "\033[1;31moption_name \t default\t 功能描述\033[m"
	@echo  "APP_NAME\t $(APP_NAME) \t 输出文件名"
	@echo  "SYSROOT\t $(SYSROOT) \t 交叉编译系统目录"
	@echo  "CROSS_COMPILE\t $(CROSS_COMPILE) \t 交叉编译工具前缀"
	@echo  "PREFIX \t $(PREFIX) \t 安装目录前缀"
	@echo  "OUT_PATH \t $(OUT_PATH) \t 编译输出目录"
	@echo  "LIB_PREFIX \t $(LIB_PREFIX) \t 库前缀"
	@echo  "STATIC_LIB_SUFIX \t $(STATIC_LIB_SUFIX) \t 静态库后缀"
	@echo  "SHARED_LIB_COMPILE \t $(SHARED_LIB_COMPILE) \t 动态库后缀"
	@echo  "LINK_OPTS \t $(LINK_OPTS) \t 链接选项"
clean :
	-rm -rf $(OBJS_OUT_PATH)/*
distclean :
	-rm -rf $(OBJS_OUT_PATH)
	-rm -rf $(HEADER_OUT_PATH)
	-rm -rf $(LIB_OUT_PATH)
	-rm -rf $(EXEC_OUT_PATH)
install : lib app
	$(STRIP) $(EXEC_OUT_PATH)/$(APP_NAME)
	install -d $(PREFIX)/include/$(APP_NAME)
	install -d $(PREFIX)/lib/$(APP_NAME)
	install -d $(PREFIX)/bin/$(APP_NAME)
	install $(HEADER_OUT_PATH)/* $(PREFIX)/include/$(APP_NAME)
	install $(LIB_OUT_PATH)/* $(PREFIX)/lib/$(APP_NAME)
	install $(EXEC_OUT_PATH)/* $(PREFIX)/bin/$(APP_NAME)
uninstall :
	-rm -rf $(PREFIX)/include/$(APP_NAME)
	-rm -rf $(PREFIX)/lib/$(APP_NAME)
	-rm -rf $(PREFIX)/bin/$(APP_NAME)
#代码编译部分
$(OBJS_OUT_PATH)/%.o :
	@./build.sh $(ENV_FILE) $@
	
