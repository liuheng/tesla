#######################################################
# automake options = foreign
AUTOMAKE_OPTIONS=foreign

#######################################################
# libraries
lib_LIBRARIES=libtesla.a libtesla_base.a

#######################################################
# programs
bin_PROGRAMS=echo chat_client chat_server

#######################################################
# libraries

#libtesla.a
libtesla_a_SOURCES=tesla/base/AsyncLogger.cc tesla/base/Condition.cc tesla/base/CountdownLatch.cc tesla/base/Date.cc tesla/base/Exception.cc tesla/base/FileLogger.cc tesla/base/FileUtils.cc tesla/base/Logger.cc tesla/base/LogStream.cc tesla/base/ProcessInfo.cc tesla/base/Thread.cc tesla/base/Threadpool.cc tesla/base/Time.cc tesla/base/Timestamp.cc tesla/net/Acceptor.cc tesla/net/Buffer.cc tesla/net/Channel.cc tesla/net/Connector.cc tesla/net/EventLoop.cc tesla/net/EventLoopThread.cc tesla/net/EventLoopThreadpool.cc tesla/net/InetAddress.cc tesla/net/Socket.cc tesla/net/SockOps.cc tesla/net/TcpClient.cc tesla/net/TcpConnection.cc tesla/net/TcpServer.cc tesla/net/Timer.cc tesla/net/TimerQueue.cc tesla/net/UdpClient.cc tesla/net/backend/DefaultBackend.cc tesla/net/backend/EpollBackend.cc tesla/net/backend/PollBackend.cc

#libtesla_base.a
libtesla_base_a_SOURCES=tesla/base/AsyncLogger.cc tesla/base/Condition.cc tesla/base/CountdownLatch.cc tesla/base/Date.cc tesla/base/Exception.cc tesla/base/FileLogger.cc tesla/base/FileUtils.cc tesla/base/Logger.cc tesla/base/LogStream.cc tesla/base/ProcessInfo.cc tesla/base/Thread.cc tesla/base/Threadpool.cc tesla/base/Time.cc tesla/base/Timestamp.cc

#######################################################
# programs

#echo
echo_SOURCES=examples/echo/main.cc examples/echo/echo.cc
echo_LDADD=libtesla.a
echo_LDFLAGS=-D_GNU_SOURCE

#chat_client
chat_client_SOURCES=examples/chat/client.cc
chat_client_LDADD=libtesla.a
chat_client_LDFLAGS=-D_GNU_SOURCE

#chat_server
chat_server_SOURCES=examples/chat/server.cc
chat_server_LDADD=libtesla.a
chat_server_LDFLAGS=-D_GNU_SOURCE

########################################################
#common includes and libs
INCLUDES=-I$(CURRENTPATH) -I/usr/include
LIBS=-lpthread
CFLAGS=-g -O0
CPPFLAGS=-g -O0
LDFLAGS=-g -O0
