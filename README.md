# ATOM-liu

# 项目概述
这是使用C/C++多线程为室友写的一个处理分子和原子的程序。

# 需求
在其中ke_atom.100.dump文件中，每一列代表的是
原子的序号 原则的类型 原子的x坐标 原子的y坐标 原子的z坐标 原子的动能

现在要计算所有原子的平均动能。
其中每一个原子的动能计算方法是和这个原子相距不超过10的距离里，的所有原子的动能和除以原子的个数，表示这个原子的平均动能。


## 实现1
暴力法实现，为每一个原子都计算一次，这个耗时太长, 算法是O(n^2)级别

## 实现2
为所有的原子按照x坐标进行排序，这样如果x坐标超过10，就可以退出内层循环，提前结束循环，小于10则计算距离是否满足。

## 实现3
在实现2的基础上，检查y z 坐标，如果y z 坐标相差大于10，则不计算距离，直接continue，否则再进算距离，比步骤二速度提升很大。


## 实现4
按照实现3的想法，使用多线程，每个线程处理一段数据。这样效率会进一步加大很多。

# 待改进部分

## 有关数据预处理
现在只按照x坐标进行排序的话，符合x坐标的原子数量和满足距离小于10的比例大概是100比1。所以如何快速的排序原子坐标，或者有一种其他方法，直接选出范围更小的原子数量，再计算是否满足距离小于10的条件。

## 使用多进程
如果使用多进程程序，目前我遇到的问题是多进程如何按照顺序写入到一个文件中。
这里初步的想法是在父进程中打开文件，并建立共享内存给子进程，每个子进程中对共享内存的一部分进行处理，最后再由父进程按照id排序，再写入文件中。
