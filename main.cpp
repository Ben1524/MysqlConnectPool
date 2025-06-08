// #include <iostream>
// #include <mysqlx/xdevapi.h>
// #include <vector>
// #include <string>
// #include <random>
// #include <chrono>
// #include <algorithm>
// #include <iomanip>
// #include <locale>
// #include <codecvt>
// using namespace std;
// using namespace mysqlx;
// bool parseDateTime(const std::string& timeStr, tm& tm) {
//     istringstream ss(timeStr);
//     ss.imbue(locale::classic()); // 使用 C 本地化
//     ss >> get_time(&tm, "%Y-%m-%d %H:%M:%S"); // 严格匹配格式
//     return !ss.fail();
// }
// // 生成随机字符串（用于测试数据）
// std::string randomString(size_t length) {
//     static const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
//     static mt19937 rng(chrono::system_clock::now().time_since_epoch().count());
//     uniform_int_distribution<> dist(0, chars.size() - 1);
//
//     std::string str;
//     str.reserve(length);
//     for (size_t i = 0; i < length; ++i) {
//         str += chars[dist(rng)];
//     }
//     return str;
// }
//
// // 格式化 MySQL 时间字符串
// std::string formatMySQLTime(const std::string& timeStr) {
//     if (timeStr.empty()) return "NULL";
//
//     std::tm tm = {};
//     std::istringstream ss(timeStr);
// //    2025-06-02 09:52:18
//     ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
//
//     if (ss.fail()) {
//         return "ERROR_FORMAT"; // 无效时间格式
//     }
//
//     std::ostringstream oss;
//     oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S"); // 输出格式：YYYY-MM-DD HH:MM:SS
//     return oss.str();
// }
//
// const char* convertChar16ToChar(const char16_t* char16Str) {
//     if (!char16Str) return nullptr;
//     std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
//     static std::string result;
//     try {
//         result = converter.to_bytes(char16Str);
//     } catch (const std::range_error&) {
//         // 转换失败时返回空字符串
//         result.clear();
//     }
//     return result.c_str();
// }
//
//
// int main() {
//         // 1. 连接数据库（需修改为您的实际配置）
//         Session session("mysqlx://root:123456@localhost:33060");
//         cout << "Connected to MySQL Server!" << endl;
//         session.sql("SET NAMES utf8mb4").execute(); // 设置字符集
//
//         // 2. 创建数据库（如果不存在）
//         Schema db = session.getSchema("test_db");
//         if (!db.existsInDatabase()) {
//             db = session.createSchema("test_db");
//             cout << "Schema 'test_db' created!" << endl;
//         }
//         session.sql("USE test_db").execute();
//
//         session.startTransaction();
//
//         session.commit(); // 提交事务
//         // 3. 创建表（使用原生 SQL）
//         session.sql(R"(
//             CREATE TABLE IF NOT EXISTS test_users (
//                 id INT PRIMARY KEY AUTO_INCREMENT,
//                 name VARCHAR(50) NOT NULL,
//                 age INT NOT NULL,
//                 email VARCHAR(100) UNIQUE NOT NULL,
//                 created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
//             )
//         )").execute();
//         cout << "Table 'test_users' created or already exists!" << endl;
//         auto table=db.getTable("test_users");
//         // 4. 插入数据（使用原生 SQL）
//         for (int i = 0; i < 10; ++i) {
//             std::string name = randomString(5);
//             int age = rand() % 50 + 18; // 随机年龄在18到67之间
//             std::string email = name + "@example.com";
// //            session.sql("INSERT INTO test_users (name, age, email) VALUES (?, ?, ?)")
// //                   .bind(name, age, email).execute();
//             table.insert("name", "age", "email")
//                  .values(name, age, email).execute();
//         }
//         auto res = table.select("id", "name", "age+100","email","DATE_FORMAT(created_at, '%Y-%m-%d %H:%i:%s') AS formatted_time   ")
//              .execute();
//         for (auto row : res) {
//             cout << "ID: " << row[0].get<int>()
//                  << ", Name: " << row[1].get<std::string>()
//                  << ", Age: " << row[2].get<int>()
//                  << ", Email: " << row[3].get<std::string>();
//             std::string datetime = row[4].get<std::string>();
//             std::cout<<datetime.c_str()[0];
//             for (int i=0;i< strlen(datetime.c_str());i++) {
//                 if (datetime.c_str()[i] == 'T') {
//                     datetime[i] = ' ';
//                 }
//                 std::cout<< datetime.c_str()[i];
//             }
//
//             std::cout << "时间字符串===========" << std::endl;
//
//         }
// }

#include <absl/time/time.h>
#include <absl/time/clock.h>
#include <absl/log/log.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <absl/strings/str_split.h>

// 将absl::Time转换为指定格式的字符串
std::string formatTime(const absl::Time& time) {
    absl::CivilSecond civil_time = absl::ToCivilSecond(time, absl::LocalTimeZone());
    absl::Duration subsecond = time - absl::FromCivil(civil_time, absl::LocalTimeZone());
    int64_t micros = absl::ToInt64Microseconds(subsecond);

    std::ostringstream oss;
    oss << std::setfill('0');

    // 输出年-月-日
    oss << absl::FormatTime("%Y-%m-%d", absl::FromCivil(civil_time,absl::LocalTimeZone()), absl::LocalTimeZone());

    // 如果时分秒不全为0，则添加时间部分
    if (civil_time.hour() != 0 || civil_time.minute() != 0 || civil_time.second() != 0) {
        oss << " " << std::setw(2) << civil_time.hour() << ":"
            << std::setw(2) << civil_time.minute() << ":"
            << std::setw(2) << civil_time.second();

        // 如果有微秒部分，则添加
        if (micros != 0) {
            oss << ":" << std::setw(6) << micros;
        }
    }

    return oss.str();
}

int main() {

    return 0;
}