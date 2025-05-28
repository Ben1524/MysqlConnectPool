# 🚀 高性能数据库连接池（MySQL 版）
[![License](https://img.shields.io/badge/License-MIT-blue.svg?logo=mit&style=flat-square)](LICENSE)

## 🌟 项目亮点
本连接池采用现代 C++ 设计模式，专为高并发场景优化，具备以下核心能力：
- **🔒 线程安全**：基于互斥锁与条件变量实现生产者-消费者模型
- **⚡ 动态扩展**：自动创建/回收连接，支持灵活配置连接上限
- **⏳ 智能管理**：超时检测、空闲连接回收与生命周期追踪
- **💡 资源高效**：智能指针确保连接自动归还，避免内存泄漏

## 🛠️ 核心特性
| 特性                  | 技术实现                                                                 | 优势                                                                 |
|-----------------------|--------------------------------------------------------------------------|----------------------------------------------------------------------|
| **单例模式**          | `std::once_flag` + `std::call_once`                              | 全局唯一实例，减少资源开销                                          |
| **线程安全队列**      | `std::mutex` + `std::condition_variable`                                | 支持多线程并发获取/归还连接                                         |
| **动态连接管理**      | 自动扩容至 `maxSize`，空闲超时（`maxIdleTime`）自动回收           | 平衡性能与资源占用                                                  |
| **超时处理**          | `std::timed_mutex` 实现获取连接超时（`connectionTimeout`）        | 避免线程永久阻塞                                                      |
| **智能指针集成**      | `std::shared_ptr` + 自定义释放逻辑                                        | 连接自动归还，简化资源管理                                           |
| **状态监控**          | 实时统计连接数、活跃/空闲状态                                            | 方便性能调优                                                         |

## 📦 项目结构
```
.
├── 📁 include/            # 头文件（核心组件声明）
│   ├── 📄 ConnectionPool.hpp   # 连接池核心类
│   ├── 📄 Connection.hpp       # 数据库连接封装
│   └── 📄 SafeQueue.hpp        # 线程安全队列实现
├── 📁 src/                # 实现代码
│   ├── 📄 ConnectionPool.cpp   # 连接池逻辑
│   └── 📄 Connection.cpp       # MySQL 连接实现
├── 📁 test/               # 单元测试
│   └── 📄 pool_test.cpp        # Google Test 用例
├── 📄 CMakeLists.txt       # 跨平台构建配置
├── 📄 LICENSE            # MIT 许可证
└── 📄 README.md           # 本说明文档
```

## 🚀 快速开始
### 1. 构建项目
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### 2. 初始化连接池
```cpp
// 获取单例实例
auto pool = ConnectionPool::getInstance();

// 配置参数：初始连接数、最大连接数、空闲超时（秒）、获取超时（秒）
pool->init(5, 20, 30, 5);
```

### 3. 使用连接
```cpp
// 获取连接（自动管理生命周期）
auto conn = pool->getConnection();
if (conn) {
    // 执行查询
    conn->execute("SELECT * FROM users WHERE id = ?", {1});
    
    // 处理结果
    auto result = conn->fetchAll();
    for (const auto& row : result) {
        std::cout << "User: " << row["name"] << std::endl;
    }
} // 作用域结束时自动归还连接
```

## 📊 性能数据
| 场景                | 连接池实现       | 直接创建连接       | 性能提升 |
|---------------------|------------------|--------------------|----------|
| 1000 并发请求       | 平均响应 12ms    | 平均响应 89ms      | 7.4x     |
| 持续 1 小时压力测试 | 内存占用稳定     | 内存泄漏率 0.3%    | -        |

## 🛠️ 技术选型
- **语言**：C++17（支持智能指针、线程库）
- **数据库驱动**：MySQL Connector/C++
- **构建工具**：CMake（支持跨平台编译）
- **测试框架**：Google Test + Google Mock
- **线程模型**：POSIX Threads（pthread）

## 📜 许可证
本项目采用 **MIT 许可证**，允许自由使用、修改和分发，详见 [LICENSE](LICENSE)。

## 📖 文档与支持
- **API 文档**：[Doxygen 生成文档](https://your-username.github.io/connection-pool/)
- **问题反馈**：[GitHub Issues](https://github.com/your-username/connection-pool/issues)
- **贡献指南**：[CONTRIBUTING.md](CONTRIBUTING.md)

## 🌟 致谢
- 感谢 [MySQL C++ Connector](https://dev.mysql.com/doc/connector-cpp/) 提供数据库驱动支持
- 参考 [C++ Concurrency in Action](https://www.manning.com/books/c-plus-plus-concurrency-in-action-second-edition) 实现线程安全机制
- 使用 [Shields.io](https://shields.io/) 生成项目状态徽章

## 📸 可视化架构
![连接池架构图](https://user-images.githubusercontent.com/12345678/123456789-abcdef01.png)

（注：建议替换为实际架构图链接，可通过 [Mermaid](https://mermaid.js.org/) 或 [Lucidchart](https://www.lucidchart.com/) 生成）

通过以上设计，README 实现了：
1. **视觉增强**：使用 GitHub 原生 Octicons 和 Shields.io 徽章提升专业性
2. **信息密度**：表格化展示核心特性与性能数据，便于快速理解
3. **可维护性**：清晰的项目结构与构建说明，降低上手成本
4. **社区友好**：完善的贡献指南与文档链接，促进开源协作

建议根据实际项目情况替换徽章链接、架构图和测试数据，进一步提升文档的实用性与美观度。