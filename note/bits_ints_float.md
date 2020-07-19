# bit

## 64位系统数据类型

char    1-bit   
short   2-bit   
int     4-bit  
long    8-bit  
float   4-bit  
double  8-bit  
pointer 8-bit  

## C语言比特操作符

& ｜ ～ ^

## C语言逻辑操作符

&&  ｜｜  ！

## C语言位移操作符

左移<<    
丢弃高位bit，低位bit填充0  
右移>>  
逻辑右移：高位填充0  
算数右移：高位填充原来的算数值

# Integer

## int编码

### Unsigned转换公式

$$B2U(X) = \sum_{i=0}^{w-1}x_i2^i$$

$$UMin = 0$$

$$UMax = 2^{w}-1$$

### Two‘s Complement转换公式

$$B2T(X) = -x_{w-1}2^{w-1}*\sum_{i=0}^{w-2}x_i2^i$$

$$TMin = -2^{w-1}$$
$$TMax = 2^{w-1}-1$$

$$Minus1 = 111...1$$

### 2'sComplement->Unsigned 

Negative -> Big Positive

### C语言中的Signed、Unsigned

一般来说都是Signed interger  
用后缀U表示Unsigned. Eg: 0U.

### Signed , Unsigend 转换

**Unsiged 和 Signed在同一个表达式中时，Signed表达式会隐式转换为Unsigend**

符号包括： <,>,==,<=,>=

### Sign扩展

Task:  
+ W-bit Signed Interger -> W+k-bit Interger (same value)

Rule:
+ 复制k-bit标志位

### Truncating 

+ 截断到固定位数


## 加法

+ Unsigned  超过$2^w$溢出
+ Signed 超过$2^{w-1}-1$,$-2^{w-1}$均会溢出。

## 乘法

+ Unsigned,Signed 均会溢出截断

## Byte Ordering

+ Big Endian : Internet Least significant byte has highest address
+ Little Endian: Least significant byte at lowest address





