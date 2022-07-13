#!/bin/bash
# 在Linux下查看内存我们一般用free命令：
# [root@ tmp]# free
#              total       used       free     shared    buffers     cached
# Mem:       3266180    3250004      16176          0     110652    2668236
# -/+ buffers/cache:     471116    2795064
# Swap:      2048276      80160    1968116

# 下面是对这些数值的解释：
# total:总计物理内存的大小。
# used:已使用多大。
# free:可用有多少。
# Shared:多个进程共享的内存总额。
# Buffers/cached:磁盘缓存的大小。
# 第三行(-/+ buffers/cached):
# used:已使用多大。
# free:可用有多少。
# 第四行就不多解释了。
# 区别：第二行(mem)的used/free与第三行(-/+ buffers/cache) used/free的区别。 这两个的区别在于使用的角度来看，第一行是从OS的角度来看，因为对于OS，buffers/cached 都是属于被使用，所以他的可用内存是16176KB,已用内存是3250004KB,其中包括，内核（OS）使用+Application(X, oracle,etc)使用的+buffers+cached.
# 第三行所指的是从应用程序角度来看，对于应用程序来说，buffers/cached 是等于可用的，因为buffer/cached是为了提高文件读取的性能，当应用程序需在用到内存的时候，buffer/cached会很快地被回收。
# 所以从应用程序的角度来说，可用内存=系统free memory+buffers+cached。
# 如上例：
# 2795064=16176+110652+2668236
echo "monitor memory info"
echo ""
echo "#   free"
free
echo ""
echo "#   cat /proc/meminfo"
cat /proc/meminfo