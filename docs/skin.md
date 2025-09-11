## 已知问题
1. 没有使用C++智能指针（刚开始做的时候用不太习惯智能指针，所以没用，但是有ASAN检查，再加上节点的创建和释放都有固定方式，应该问题不大）
2. 生成的代码m维度必须对齐到16（原因是从OpenBLAS里偷的kernel对A打包时不足16的部分进行了更细粒度的切分，生成的打包操作没有对不足16的部分继续切分，但是不论是把打包操作对齐到kernel或者是改kernel对齐到打包操作都还挺麻烦）
3. kernel里的预取指令没根据数据布局的变化进行修改
4. 缺乏专业严肃的PL、编译器或者设计模式知识支撑，很多地方包括各种类型、语法树和所谓的`pass`都秉持着“想一出是一出，怎么方便怎么来”的设计原则
5. 没有用llvm（刚开始确实没想到国产平台上有llvm，当然老平台目前确实也找不到现成能用的，不用llvm的好处是现在几乎没有任何其它依赖，有cmake/make/gcc/g++/omp/python就行）
6. tuner坑有点深，就不提了

以上问题受限于精力、动力、能力所以一直存在着。尤其是2，3两条，感兴趣可以看一眼[从OpenBLAS里抠出来的kernel](../runtime/skylakex/dgemm_kernel_16x2_skylakex.c)，相信大部分人都是能理解我的，当时把这个kernel改成支持ABC是否重排的8种组合已经快燃尽了（重写又不敢保证性能差不多）。
## 感谢
不管是整体思路、性能优化的思想还是部分细节的实现、甚至是源代码，以下项目都或多或少的帮助到了我。
- [OpenBLAS](https://github.com/OpenMathLib/OpenBLAS)
- [tvm](https://github.com/apache/tvm)
- [mlir-toy](https://github.com/llvm/llvm-project/tree/main/mlir/examples/toy)