OOzdb是一个轻量级，高效，易用，可扩展的database连接池库，简要讲解可以参考我的博客 <http://www.thinkingyu.com/projects/OOzdb/>。

这里说一下如何使用，在工程一级目录下有个makefile，那个是ST的，对应的主函数所在源程序是test/selectTest/testPrepared.cc。如果想要跑UT，需要把用刚刚的makefile编译生成的目标文件移动到test/poolTest下，然后用test/poolTest下的makefile编译生成测试程序，这个过程有些蛋痛，因为暂时还没有时间用automake工具生成可部署的软件包，之后会有更新的。

####使用的几个注意事项：

(1)用ConnectionPool类的setInitialConnections和setReaper方法一定要在start之前调用，否则是无效的。setMaxConnections方法在连接池启动之前之后调用都无所谓。其余操作均按常规进行即可，所有对象的生命周期都由库进行自动管理，无需用户显示析构。给一个使用的example。
