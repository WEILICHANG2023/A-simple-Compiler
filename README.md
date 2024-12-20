# A-simple-Compiler

这是一个简单的编译器项目。包含词法分析、语法分析、语义分析、错误处理、中间代码生成（Pcode）和代码解释运行。

以下是使用该编译器的步骤：

## 使用方法



### 1. 构建项目



首先，创建一个 `build` 目录并进入该目录：

```
mkdir build
cd build
```



然后，使用 `cmake` 生成构建文件：

```
cmake ..
```



接下来，编译项目：

```
make
```



### 2. 编写测试代码



在 `build` 目录下创建一个测试文件 `testfile.txt`，并使用 `vim` 或其他编辑器编写代码：

```
vim build/testfile.txt
```



在 `testfile.txt` 中编写你的代码。

代码的运行结果可以在`pcoderesult.txt`中查看到，

如有输入则从命令行中输入。



 `testfile.txt` t示例：

```
// 单行注释：定义全局变量和常量
const int MAX = 10; // 常量声明
int globalVar = 5;  // 全局变量声明

/* 多行注释：
定义一个函数，
计算数组元素的和 */
int sumArray(int arr[], int size) {
    int sum = 0;
    int i = 0;
    for (i = 0; i < size; i = i + 1) {
        sum = sum + arr[i];
    }
    return sum;
}

/* 多行注释：定义一个函数，判断数组中是否有偶数 */
int hasEven(int arr[], int size) {
    int i = 0;
    for (i = 0; i < size; i = i + 1) {
        if (arr[i] % 2 == 0) {
            return 1; // 返回1表示有偶数
        }
    }
    return 0; // 返回0表示没有偶数
}

int main() {
    int array[MAX] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}; // 数组声明并初始化
    int localVar = 10; // 局部变量声明

    // 调用函数计算数组和
    int sum = sumArray(array, MAX);
    printf("Sum of array elements: %d\n", sum);

    // 调用函数判断数组中是否有偶数
    if (hasEven(array, MAX)) {
        printf("Array contains even numbers.\n");
    } else {
        printf("Array does not contain even numbers.\n");
    }

    // 使用for循环打印数组元素
    int i = 0;
    for (i = 0; i < MAX; i = i + 1) {
        printf("Element %d: %d\n", i, array[i]);
    }

    // 使用if-else语句判断全局变量和局部变量的大小
    if (globalVar > localVar) {
        printf("Global variable is greater than local variable.\n");
    } else {
        printf("Local variable is greater than or equal to global variable.\n");
    }

    return 0;
}
```

`pcoderesult.txt`输出示例：

```
Sum of array elements: 55
Array contains even numbers.
Element 0: 1
Element 1: 2
Element 2: 3
Element 3: 4
Element 4: 5
Element 5: 6
Element 6: 7
Element 7: 8
Element 8: 9
Element 9: 10
Local variable is greater than or equal to global variable.
```



