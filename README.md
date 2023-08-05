# rocket

## 1.总览
rocket 是基于 C++11 开发的一款多线程的异步 RPC 框架，它旨在高效、简洁的同时，又保持至极高的性能。

rocket 同样是基于主从 Reactor 架构，底层采用 epoll 实现 IO 多路复用。应用层则基于 protobuf 自定义 rpc 通信协议，同时也将支持简单的 HTTP 协议。


这里额外提一句 tinyrpc，看过我之前文章的同学们应该都知道这个。然而很多交流群里的同学都反映 tinyrpc 不是那么容易理解，有种无从下手的感觉。

因此，就引入了 tinyrpc 的简洁版，去掉 tinyrpc 一些晦涩或者不必要的地方，如协程等，但保留其核心的架构，命名为 rocket-rpc.

同时，专门为此项目开设视频课程，将从零开始一步步搭建 rpc 框架，帮助各位同学更好的理解 rpc 的原理。

附：tinyrpc - https://github.com/Gooddbird/tinyrpc


### 1.1 技术栈关键词
- c++11
- protobuf
- rpc
- reactor
- http

### 1.2 学习本课程之前，你需要掌握什么知识？
- 掌握 C++ 语言，至少能熟悉主要的语法、面向对象编程方法等
- 掌握 Linux 环境，熟悉 Linux 网络编程、Socket 编程 等
- 熟悉 Reactor 架构
- 了解 Git 及相关命令
- 了解计算机网络相关知识，如 Tcp 等
- 了解 Protobuf，可以能看懂和编写简单的 protobuf 协议文件
- 了解 rpc 通信原理

### 1.3 学习本课程之后，你将得到哪些知识？
至少当你跟着视频完整开发一遍之后，你将学习到以下技术：
- 一键搭建高性能 rpc 服务，可用于个人网站建站
- 熟悉 RPC 通信的原理，能举一反三，看懂 grpc 等框架
- 熟悉 Reactor 架构，并横向扩展，理解 muduo 等网络框架
- 熟悉 Linux 下 C++ 工程项目开发的通用套路

### 1.4 课程计划
整个项目按以下大纲进行：
- 前置准备，包括环境搭建、依赖库安装
- 日志及配置类开发
- Reactor 核心模块
- Tcp 模块封装
- 序列化、编解码模块
- Rpc 模块封装
- 脚手架搭建
- 简单性能测试

```
1. 环境安装与项目开发
1.1 环境搭建和依赖库安装
1.2 日志模块开发
1.3 配置模块开发

2. EventLoop 模块封装
2.1 EventLoop 核心类构建
2.2 FdEvent 封装以及测试
2.3 定时器 Timer
2.4 主从 Reactor 
2.5 EventLoop 模块整体测试

3. Tcp 模块封装
3.1 TcpBuffer
3.2 TcpConnection 
3.3 TcpServer (一)
3.4 TcpServer (二)
3.4 TcpClient
3.5 Tcp 模块测试 (一)
3.5 Tcp 模块测试 (二)



4. RPC 协议封装
4.1 TinyPB 协议编码
4.2 TinyPB 协议解码
4.3 编解码模块测试

5. RPC 通信模块封装
5.1 RpcController 以及 RcpClosure 等基础类
5.2 RpcDispatcher 分发器
5.3 RpcChannel
5.4 RpcAsyncChannel
5.5 Rpc 模块集成测试

6. RPC 脚手架封装
6.1 代码生成器开发
6.2 项目的构建与测试

```


## 2. 前置准备
### 2.1 环境搭建
- 开发环境：Linux，可以是虚拟机。推荐跟我保持一致，Centos8 的虚拟机. 包含必要的 C++ 开发工具，如 GCC/G++(至少能支持到 C++11 语法的版本)
- 开发工具：VsCode，通过 ssh 远程连接 linux 机器

### 2.2 依赖库的安装
#### 2.2.1 protobuf
protobuf 推荐使用 3.19.4 及其以上：

安装过程：
```
wget  https://github.com/protocolbuffers/protobuf/releases/download/v3.19.4/protobuf-cpp-3.19.4.tar.gz

tar -xzvf protobuf-cpp-3.19.4.tar.gz
```

我们需要指定 安装路径在 `/usr` 目录下:
```
cd protobuf-cpp-3.19.4

./configure -prefix=/usr/local

make -j4 

sudo make install
```

安装完成后，你可以找到头文件将位于 `/usr/include/google` 下，库文件将位于 `/usr/lib` 下。

#### 2.2.2 tinyxml
项目中使用到了配置模块，采用了 xml 作为配置文件。因此需要安装 libtinyxml 解析 xml 文件。

```
wget https://udomain.dl.sourceforge.net/project/tinyxml/tinyxml/2.6.2/tinyxml_2_6_2.zip

unzip tinyxml_2_6_2.zip

```

### 2.3 日志模块开发
首先需要创建项目：

日志模块：
```
1. 日志级别
2. 打印到文件，支持日期命名，以及日志的滚动。
3. c 格式化风控
4. 线程安全
```

LogLevel:
```
Debug
Info
Error
```

LogEvent:
```
文件名、行号
MsgNo
进程号
Thread id
日期，以及时间。精确到 ms
自定义消息
```

日志格式
```
[Level][%y-%m-%d %H:%M:%s.%ms]\t[pid:thread_id]\t[file_name:line][%msg]
```

Logger 日志器
1.提供打印日志的方法
2.设置日志输出的路径


### 2.4 Reactor
Reactor，又可以称为 EventLoop，它的本质是一个事件循环模型。

Rractor(或称 EventLoop)，它的核心逻辑是一个 loop 循环，使用伪代码描述如下：

```c++{.line-numbers}
void loop() {
  while(!stop) {
      foreach (task in tasks) {
        task();
      }

      // 1.取得下次定时任务的时间，与设定time_out去较大值，即若下次定时任务时间超过1s就取下次定时任务时间为超时时间，否则取1s
      int time_out = Max(1000, getNextTimerCallback());
      // 2.调用Epoll等待事件发生，超时时间为上述的time_out
      int rt = epoll_wait(epfd, fds, ...., time_out); 
      if(rt < 0) {
          // epoll调用失败。。
      } else {
          if (rt > 0 ) {
            foreach (fd in fds) {
              // 添加待执行任务到执行队列
              tasks.push(fd);
            }
          }
      }
      
      
  }
}
```

在 rocket 里面，使用的是主从 Reactor 模型，如下图所示：

![](./imgs/main-sub-reactor.drawio.png)

服务器有一个mainReactor和多个subReactor。

mainReactor由主线程运行，他作用如下：通过epoll监听listenfd的可读事件，当可读事件发生后，调用accept函数获取clientfd，然后随机取出一个subReactor，将cliednfd的读写事件注册到这个subReactor的epoll上即可。也就是说，mainReactor只负责建立连接事件，不进行业务处理，也不关心已连接套接字的IO事件。

subReactor通常有多个，每个subReactor由一个线程来运行。subReactor的epoll中注册了clientfd的读写事件，当发生IO事件后，需要进行业务处理。

#### 2.4.1 TimerEvent 定时任务
```
1. 指定时间点 arrive_time
2. interval, ms。
3. is_repeated 
4. is_cancled
5. task


cancle()
cancleRepeated()
```

#### 2.4.2 Timer
定时器，他是一个 TimerEvent 的集合。
Timer 继承 FdEvent
```

addTimerEvent();
deleteTimerEvent();

onTimer();    // 当发生了 IO 事件之后，需要执行的方法


reserArriveTime()

multimap 存储 TimerEvent <key(arrivetime), TimerEvent>
```

#### 2.5 IO 线程
创建一个IO 线程，他会帮我们执行：
1. 创建一个新线程（pthread_create）
2. 在新线程里面 创建一个 EventLoop，完成初始化
3. 开启 loop
```
class {



 pthread_t m_thread;
 pid_t m_thread_id;
 EventLoop event_loop;
}

```

RPC 服务端流程
```
启动的时候就注册OrderService 对象。


1. 从buufer读取数据，然后 decode 得到请求的 TinyPBProtobol 对象。然后从请求的 TinyPBProtobol 得到 method_name, 从 OrderService 对象里根据 service.method_name 找到方法 func
2. 找到对应的 requeset type 以及 response type
3. 将请求体 TinyPBProtobol 里面的 pb_date 反序列化为 requeset type 的一个对象, 声明一个空的 response type 对象
4. func(request, response)
5. 将 reponse 对象序列为 pb_data。 再塞入到 TinyPBProtobol 结构体中。做 encode 然后塞入到buffer里面，就会发送回包了
```



# 常见问题
## 1. 重要！！！ 库文件安装路径
### 1.1 tinyxml
以tinyxml 为例，tinyxml 分为库文件 `libtinyxml.a` 和头文件 `tinyxml/*.h`

其中库文件一定需要安装在 `/usr/lib` 目录下，即绝对路径为 `/usr/lib/libtinyxml.a` ，如果不一致，请拷贝过去

而头文件，所有 `*.h` 的头文件，必须位于 `tinyxml/` 目录下，而整个 `tinyxml` 目录需要放在 `usr/include` 下，即绝对路径为 `/usr/include/tinyxml`, `tinyxml` 下包含所有的 `.h` 结尾的头文件

### 1.2 protobuf
同 tinyxml，库文件在 `/usr/lib/libprotobuf.a`, 所有头文件 `*.h` 在 `/usr/include/google/protobuf/` 下


## 2. 如何确保 protobuf 库安装成功？
1. 确保头文件、库文件安装无误
2. 确保能 执行 protoc 命令
```
[ikerli@localhost protobuf]$ protoc --version
libprotoc 3.19.4
```


## 3. invalid version 3(max 0)
出现此问题，一般是安装的 libtinyxml.a 有问题，使用以下代码替换原始 tinyxml 的makefile, 然后重装 libtinyxml.a
```
#****************************************************************************
#
# Makefile for TinyXml test.
# Lee Thomason
# www.grinninglizard.com
#
# This is a GNU make (gmake) makefile
#****************************************************************************

# DEBUG can be set to YES to include debugging info, or NO otherwise
DEBUG          := NO

# PROFILE can be set to YES to include profiling info, or NO otherwise
PROFILE        := NO

# TINYXML_USE_STL can be used to turn on STL support. NO, then STL
# will not be used. YES will include the STL files.
TINYXML_USE_STL := NO

#****************************************************************************

CC     := gcc
CXX    := g++
LD     := g++
AR     := ar rc
RANLIB := ranlib

DEBUG_CFLAGS     := -Wall -Wno-format -g -DDEBUG
RELEASE_CFLAGS   := -Wall -Wno-unknown-pragmas -Wno-format -O3

LIBS		 :=

DEBUG_CXXFLAGS   := ${DEBUG_CFLAGS} 
RELEASE_CXXFLAGS := ${RELEASE_CFLAGS}

DEBUG_LDFLAGS    := -g
RELEASE_LDFLAGS  :=

ifeq (YES, ${DEBUG})
   CFLAGS       := ${DEBUG_CFLAGS}
   CXXFLAGS     := ${DEBUG_CXXFLAGS}
   LDFLAGS      := ${DEBUG_LDFLAGS}
else
   CFLAGS       := ${RELEASE_CFLAGS}
   CXXFLAGS     := ${RELEASE_CXXFLAGS}
   LDFLAGS      := ${RELEASE_LDFLAGS}
endif

ifeq (YES, ${PROFILE})
   CFLAGS   := ${CFLAGS} -pg -O3
   CXXFLAGS := ${CXXFLAGS} -pg -O3
   LDFLAGS  := ${LDFLAGS} -pg
endif

#****************************************************************************
# Preprocessor directives
#****************************************************************************

ifeq (YES, ${TINYXML_USE_STL})
  DEFS := -DTIXML_USE_STL
else
  DEFS :=
endif

#****************************************************************************
# Include paths
#****************************************************************************

#INCS := -I/usr/include/g++-2 -I/usr/local/include
INCS :=


#****************************************************************************
# Makefile code common to all platforms
#****************************************************************************

CFLAGS   := ${CFLAGS}   ${DEFS}
CXXFLAGS := ${CXXFLAGS} ${DEFS}

#****************************************************************************
# Targets of the build
#****************************************************************************

OUTPUT := libtinyxml.a 

all: ${OUTPUT}


#****************************************************************************
# Source files
#****************************************************************************

SRCS := tinyxml.cpp tinyxmlparser.cpp xmltest.cpp tinyxmlerror.cpp tinystr.cpp

# Add on the sources for libraries
SRCS := ${SRCS}

OBJS := $(addsuffix .o,$(basename ${SRCS}))

#****************************************************************************
# Output
#****************************************************************************

${OUTPUT}: ${OBJS}
	${AR} $@ ${LDFLAGS} ${OBJS} ${LIBS} ${EXTRA_LIBS}

#****************************************************************************
# common rules
#****************************************************************************

# Rules for compiling source files to object files
%.o : %.cpp
	${CXX} -c ${CXXFLAGS} ${INCS} $< -o $@

%.o : %.c
	${CC} -c ${CFLAGS} ${INCS} $< -o $@

dist:
	bash makedistlinux

clean:
	-rm -f core ${OBJS} ${OUTPUT}

depend:
	#makedepend ${INCS} ${SRCS}

tinyxml.o: tinyxml.h tinystr.h
tinyxmlparser.o: tinyxml.h tinystr.h
xmltest.o: tinyxml.h tinystr.h
tinyxmlerror.o: tinyxml.h tinystr.h
```

## 4. multiple definition of __TMC_END__
编译时出现以下多重定义错误
![](./imgs/multiple_definition.jpg)

处理方法同问题3，重新安装 libtinyxml.a 即可


## 5. testcases/order.pb.cc: No such file or directory
```
-Wno-unused-but-set-variable -I./ -Irocket  -Irocket/common -Irocket/net -Irocket/net/tcp -Irocket/net/coder -Irocket/net/rpc testcases/test_rpc_client.cc testcases/order.pb.cc -o bin/test_rpc_client lib/librocket.a /usr/lib/libprotobuf.a    /usr/lib/libtinyxml.a -ldl -pthread
g++: error: testcases/order.pb.cc: No such file or directory
make: *** [makefile:71: bin/test_rpc_client] Error 1
make: *** Waiting for unfinished jobs....
```
出现此错误时，是因为本地没有用 `protoc` 命令生成文件，需要执行：
```
cd testcases
protoc --cpp_out=./ order.proto
```
观察是否在当前目录生成了 `order.pb.h` 和 `order.pb.cc` 两个文件，生成成功后重新编译

## 6. 重要! 目录层次结构
**请一定保持一样的目录结构，最外层的文件夹命名可以任意.**
```
rocket
  - bin
    -- 存放测试程序，可执行程序
  - conf
    -- 存放测试用的xml配置文件
  - lib
    -- 存放编译完成的静态库 librocket.a
  - obj
    -- 存放所有编译主键文件，*.o
  - rocket
    -- 存放所有源代码
  - testcases
    -- 存放测试代码
```


## x. 其他问题？如果是代码崩溃异常退出，请提供 gdb 的堆栈信息
假设可执行文件为 x
```
执行 gdb x
执行 r 命令，直到崩溃此时 gdb 会暂停
执行 bt 命令，打印出函数调用堆栈信息，尝试自己先分析，如果分析不出来可以提供给我
```

# 代码设计问题
## 1. RPC 框架是啥，这个项目最终到底产出了什么，作用是啥？
RPC 原理不再解释，需要的看一看博客：
https://www.zhihu.com/question/41609070/answer/2394467493

整个项目的产出，是一个库文件 `librocket.a` 和一系列头文件`rocket/*.h`. 库文件不是可执行程序，不能直接运行。

如何使用这个框架，其实就跟使用libtinyxml 一样，需要写一个服务，编译时引入其头文件，链接时使用其库文件。

当然，rocket 提供了更加优质的封装，使用了代码生成器 `generator`，能够一键生成 RPC 服务代码，帮助我们写好 makefile、框架代码等等逻辑。

## 2. 为什么要有配置文件？为什么用 xml 不用 json？

配置文件是为了跟代码解耦开，将一些常量，如 ip, port 等写入到配置文件中，程序启动时加载配置，实现代码与配置分离。
如果没有配置文件，假设你需要修改端口，就必须修改代码，然后重新make编译，再生成可执行文件，再重启。

而有了配置文件，只需要修改配置文件，然后重启服务即可。

为什么用 xml 不用其他？
方便好用，用其他配置效果是一样的，而且本身是启动时加载配置，效率不是重点，简单起见就用 xml 好了。

## 3. 为什么用 Reactor? 原理是什么？
https://zhuanlan.zhihu.com/p/428693405

## 4. wakeup fd 作用？
先搞清楚 Reactor 的原理，再来看这个问题。
在某些场景下，例如主从 Reactor 时。mainReactor 复制 listenfd 的监听，当客户端新连接过来时，accept获取 clientfd，然后把 clientfd 随机分配给一个 subReactor，由这个 subReactor 负责clientfd的后续读写操作。

而对于 subReactor 来说，它一直在不断执行 loop 循环，没有 IO 事件发生时都会沉睡在 epoll_wait 上。假设 mainReactor 向某个 subReactor 添加 fd 的时候，subReactor正在 epoll_wait 沉睡，就会造成处理这个clientfd 延迟。
为了解决这个延迟，mainReactor 在添加clientfd给 subReactor 时，需要某种机制，把 subReactor 从 epoll_wait 唤醒，这个唤醒动作就叫 wakeup

如何实现唤醒，就是用 IO 事件触发，因此每个 Reactor 需要监听一个特殊的 wakeupfd 的可读事件，当需要唤醒时，只需要往这个 wakeupfd 里面写入数据，wakeupfd 马上变得可读，epoll_wait 就马上监听到可读事件，立马返回了。返回之后，就可以处理当前待执行任务。

## 5. timer 作用？
定时器作用不再多数，网络程序中经常需要一些定时任务、周期任务，就需要靠 tiemr 实现。
定时器原理是 timerfd，建议 google 搜索一下 timerfd 了解其原理。
本质上他就是一个 fd，给他设置一个时间戳，到达这个时间戳后 fd 变得可读，epoll_wait 监听到可读后马上返回，即可执行其上面的定时任务。

定时任务按照触发时间戳升序排列，即越靠前的任务，其越早被执行。因此，我们只需要动态维护 timerfd 的触发时间戳为定时任务队列中第一个任务的时间即可。