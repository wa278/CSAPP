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



# float

$$value = (-1)^sM2^E$$

+ s表示正负
+ M取值范围[1.0,2.0)
+ E=2的次方

单精度浮点数32bits ： s:1位，exp:8位，frac：23位

双精度浮点数64bits ： s:1位，exp：11位，frac：52位  

## Normalized Value

### Exponent coded
$$E = exp - Bias$$
$$Bias = 2^{k-1}-1$$

Eg: 

+ Single precision:127(Exp:1...254, E:-126,127)

+ Double precision:1023(Exp:1...2046,E:-1022...1023)

### significand coded

M = 1.xxxxxxxx

+ xxxxxx:代表frac字段
+ Minimun frac = 000000(M=1.0)
+ Maximum frac = 111111(M=2.0-微小量)
+ 节省了1.xxx中的1字段

### Denormalized Values

exp = 000000000
+ E = 1-Bias(instead of E = 0-Bias)
+ Significand coded M = 0.xxxxxxx 
+ case : exp = 000 frac = 000代表0
+ case： exp = 000 frac ！= 000,代表数字非常接近0.0

exp = 1111..1

+ case:  exp = 111..1,frac = 000,可以代表无穷大∞，算数Overflow，$1.0/0.0 = +∞$，$1.0/-0.0 = -∞$

+ case: exp = 111..1,frac!=000..0,代表非数字（NaN），比如：sqrt(-1),$∞-∞$,$∞*0$等等。

## Rounding

4舍6入， 5遵循最近偶数原则。

## 乘法

$$(-1)^{s_1}M_12^{E_1} * (-1)^{s_2}M_22^{E_2}$$

+ Sign s:  $S1 异或 S2$
+ Significand M : $M_1 * M_2$
+ Exponent E : $E_1 + E_2$

+ if M>=2, 右移M，增加E
+ E outof range, Overflow

## 加法

$$(-1)^{s_1}M_12^{E_1} + (-1)^{s_2}M_22^{E_2}, E_1 > E_2$$

+ Sign s,significand M: M位数根据E对齐，相加.

+ Exponent E: E1

+ if M>=2 ,M右移，E++
+ if M<1, M左移k位，E-=K
+ E outof range, Overflow
+ round M

## C语言中的浮点数


