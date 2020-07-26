# C 汇编和机器码

## C到Object Code

假设存在 p1.c 和 p2.c两个源文件。  
+ 编译命令
+ 使用-Og优化编译
+ 生成二进制文件p

```
gcc -Og p1.c p2.c -o p
```

+ C源文件 -> 汇编源文件
```
gcc -Og -S p1.c p2.c
```
+ 汇编源文件->二进制文件
```
gcc p1.s p2.s || as p1.s p2.s
```
+ 二进制文件连接
```
gcc p1.o p2.o -o p || ld p1.o p2.o -o p
```

## 汇编｜数据类型

+ Interger -> 1, 2, 4, 8 bytes, 存放数据和指针
+ Float -> 4, 8, 10 bytes
+ Code -> Byte序列
+ 没有如数组一样的数据类型，仅仅是一段在内存中连续分配的bytes

## 汇编｜操作

+ 执行数学运算
+ 在memory和register之间转换数据
+ 执行跳转

## 机器级别的指令

```
*dest = t;         //C语言
movq %rax, (%rbx)  //汇编语言
0x40059e: 48 89 03 //机器码
```

## 反编译Object code

```
objdump -d （文件名.o or 可执行文件）e
```

# 汇编基础

## x86-64 Interger Registers
16个Interger寄存器
|||
|----|----|
|%rax|%r8|
|%rbx|%r9|
|%rcx|%r10|
|%rdx|%r11|
|%rsi|%r12|
|%rdi|%r13|
|%rsp|%r4|
|%rbp|%r5|

## 汇编| Moving Data

```
movq Source， Dest；
```
+ 移动常量 movq $0x4, %rax
+ 在寄存器内移动数据 movq %rax, %r13

+ 移动寄存器内指针指向的内存数据movq (%rax), %r13

<font size=4> **🏁无法在一条指令内完成从内存到内存的操作**</font>

## 汇编｜ Simple Memory Addressing Modes

C语言中是指针解引用操作  
(R)  -> Mem[Reg[R]]
```
  movq (%rcx), %rax
```
D(R) -> Mem[Reg[R]+D]
```
  movq 8(%rbp),%rdx
```

## 汇编｜ Complete Memory Addressing Modes

D(Rb,Ri,S) -> Mem[Reg[Rb]+S*Reg[Ri]+D]
```
(%rdx,%rcx)
(%rdx,%rcx,4)
0x80(,%rdx,2)
```

## 汇编｜ 运算

### 地址运算命令 leaq

+ 计算地址
```
translation p = &x[i];
```

+ 数学运算 x+k*y
```
long m12(long x){
    return x*12;
}
------------------
leaq(%rdi,%rdi,2),%rax //t <- x+x*2
salq $2,%rax //t<<2 
```

### 数学运算命令

|Format|Computation|
|----|----|
|addq|Dest = Dest + Src|
|subq|Dest = Dest - Src|
|imulq|Dest = Dest * Src|
|salq|Dest = Dest << Src|
|sarq|Dest = Dest >> Src|
|shrq|Dest = Dest >> Src logic shift|
|xorq|Dest = Dest ^ Src|
|andq|Dest = Dest & Src|
|orqq|Dest = Dest | Src|

