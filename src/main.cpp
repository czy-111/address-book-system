#include<mysql.h>
#include<iostream>
#include<windows.h>
using namespace std;
#include<string>
#include <cstdlib>  // 必须加这个头文件，才能用getenv
#include <cctype>
#define MAX 1000

//联系人结构体
struct Person
{
    int id=0;       // 新增：数据库主键id
    string m_name;//姓名
    int m_sex=-1;//性别，1男2女,用-1表示未输入（NULL）
    int m_age=-1;//年龄,用-1表示未输入（NULL）
    string m_phone;//电话
    string m_addr="";//住址,空字符串表示未输入（NULL）
};

//通讯录结构体
struct Addressbooks
{
    struct Person parray[MAX];//联系人数组
    int m_size;//通讯录里的人数
};

MYSQL *mysql_conn;

//数据库配置常量（可修改）
const char* DB_HOST = "localhost";//数据库地址，公网部署时改为服务器IP
const char* DB_USER = "root";//数据库用户名
const int DB_PORT = 3306;//数据库端口
const char* DB_NAME = "address_book";// 刚才创建的数据库名

// 连接MySQL数据库
MYSQL* connectMySQL() {
    // 初始化连接
    mysql_conn = mysql_init(NULL);
    if (!mysql_conn) {
        cout << "MySQL初始化失败！" << endl;
        exit(1);
    }

    // 从环境变量读取密码，避免明文泄露
    const char* db_pass = getenv("MYSQL_PASSWORD");
    if (!db_pass) {
        cerr << "错误：请先设置环境变量 MYSQL_PASSWORD！" << endl;
        cerr << "Windows: set MYSQL_PASSWORD=你的密码" << endl;
        cerr << "Linux/macOS: export MYSQL_PASSWORD=你的密码" << endl;//cerr 是为了专门标记错误信息，更规范、更不容易丢
        exit(1);
    }

    // 连接MySQL
    if (!mysql_real_connect(
        mysql_conn,
        DB_HOST,
        DB_USER,        
        db_pass,
        DB_NAME,
        DB_PORT,
        NULL, 0
    )) {
        cout << "MySQL连接失败：" << mysql_error(mysql_conn) << endl;
        exit(1);
    }

    // 设置编码，防止中文乱码
    // 连接MySQL后立即设置字符集
    mysql_query(mysql_conn, "SET NAMES 'utf8mb4'");
    cout << " MySQL数据库连接成功！" << endl;
    return mysql_conn;
}

// 关闭MySQL连接
void closeMySQL(MYSQL* mysql_conn) {
    if (mysql_conn) {
        mysql_close(mysql_conn);
        cout << "数据库连接已关闭" << endl;
    }
}

// 从MySQL加载所有联系人到Addressbooks
void loadFromDB(Addressbooks *abs,MYSQL* mysql_conn) {
    // 清空当前数组
    abs->m_size = 0;
    string sql = "SELECT id, name, sex, age, phone, addr FROM contact";
    
    // 执行查询
    if (mysql_query(mysql_conn, sql.c_str()) != 0) {
        cout << "查询失败：" << mysql_error(mysql_conn) << endl;
        return;
    }

    // 获取结果集
    MYSQL_RES *res = mysql_store_result(mysql_conn);
    if (!res) {
        cout << "获取结果集失败：" << mysql_error(mysql_conn) << endl;
        return;
    }

    // 遍历结果，存入你的数组
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != NULL && abs->m_size < MAX) {
        Person p;

        // 关键修正：安全读取 id
        if (row[0] != nullptr) {
            // 用 stoi 代替 atoi，更安全，能正确处理数字字符串
            p.id = stoi(string(row[0]));
        } else {
            // 兜底，防止 row[0] 为 NULL 时程序崩溃
            p.id = 0;
        }
        p.m_name = row[1];
        p.m_sex = row[2] ? atoi(row[2]) : -1; // NULL转-1
        p.m_age = row[3] ? atoi(row[3]) : -1;
        p.m_phone = row[4];
        p.m_addr = row[5] ? row[5] : "";     // NULL转空字符串

        abs->parray[abs->m_size++] = p;
  
    }

    // 释放结果集
    mysql_free_result(res);
    cout << "已从数据库加载 " << abs->m_size << " 条联系人" << endl;
}


//显示菜单界面函数
void showMenu()
{
    cout<<"~~~~~~~~~~~~~~~~~~~~~"<<endl;
    cout<<"*****1.添加联系人*****"<<endl;
    cout<<"*****2.显示联系人*****"<<endl;
    cout<<"*****3.删除联系人*****"<<endl;
    cout<<"*****4.查找联系人*****"<<endl;
    cout<<"*****5.修改联系人*****"<<endl;
    cout<<"*****6.清空联系人*****"<<endl;
    cout<<"*****0.退出通讯录*****"<<endl;
    cout<<"~~~~~~~~~~~~~~~~~~~~~"<<endl;
}

// 1. 校验姓名：非空即可（对应数据库name NOT NULL）
bool checkName(const string& name) {
    if (name.empty()) {
        return false;
    }
    // 可选：你可以加额外限制，比如不能包含特殊字符
    return true;
}

// 2. 校验手机号：11位纯数字 + 大陆号段（对应数据库phone NOT NULL UNIQUE）
bool checkPhone(const string& phone) {
    // 长度必须是11位
    if (phone.size() != 11) {
        return false;
    }
    // 必须全是数字
    for (char c : phone) {
        if (!isdigit(c)) {
            return false;
        }
    }
    // 必须是大陆手机号号段（1开头，第二位3-9）
    if (phone[0] != '1') {
        return false;
    }
    switch (phone[1]) {
        case '3': case '4': case '5': case '6':
        case '7': case '8': case '9':
            return true;
        default:
            return false;
    }
}

// 3. 校验性别：只能是1或2（可选输入，输入了才校验）
bool checkSex(int sex) {
    return (sex == 1 || sex == 2);
}

// 4. 校验年龄：0-150之间（可选输入，输入了才校验）
bool checkAge(int age) {
    return (age >= 0 && age <= 150);
}

// 5. 校验地址：如果输入了就不能是空字符串（可选输入）
bool checkAddr(const string& addr) {
    // 这里只校验：如果用户输入了内容，就不能全是空格
    for (char c : addr) {
        if (!isspace(c)) {
            return true;
        }
    }
    return false;
}

//6.校验是否重复手机号
bool isPhoneDuplicate(Addressbooks* abs, const string& phone) {
    for (int i = 0; i < abs->m_size; i++) {
        if (abs->parray[i].m_phone == phone) {
            return true;
        }
    }
    return false;
}

//检测联系人是否存在
int isExist(Addressbooks* abs, string name)
{
    for (int i = 0; i < abs->m_size; i++)
    {
        if (abs->parray[i].m_name == name)
        {
            return i;
        }
    }
    return -1;
}

//添加联系人
void addPerson(Addressbooks* abs,MYSQL* mysql_conn)
{
    if (abs->m_size >= MAX)
    {
        cout << "通讯录已满，无法添加！" << endl;
        system("pause");
        system("cls");
        return;
    }

    Person p;
    // 初始化可选字段为“未输入”状态
    p.m_sex = -1;
    p.m_age = -1;
    p.m_addr = "";

    // ===================== 1. 强制输入：姓名（调用checkName）=====================
    while (true) {
        cout <<"请输入姓名：";
        cin >> p.m_name;
        if (checkName(p.m_name)) {
            break;
        }
        cout << "❌ 姓名不能为空，请重新输入！" << endl;
    }

    // ===================== 2. 强制输入：手机号（调用checkPhone）=====================
    while (true) {
        cout << "请输入电话：";
        cin >> p.m_phone;
        if (checkPhone(p.m_phone)) {
            break;
        }
        cout << "❌ 手机号不合法（需11位纯数字且号段正确），请重新输入！" << endl;
    }
    cin.ignore();

    // ===================== 3. 可选输入：性别（调用checkSex，输入了才校验）=====================
    cout << "请输入性别（1-男 2-女，直接回车跳过）：";
    string sexStr;
    getline(cin, sexStr); // 直接用getline读取，空回车会得到空字符串
    if (!sexStr.empty()) {
        // 先判断是不是纯数字，防止stoi崩溃
        bool isNum = true;
        for (char c : sexStr) {
            if (!isdigit(c)) {
                isNum = false;
                break;
            }
        }
        if (!isNum) {
            cout << "❌ 性别只能输入数字，本次跳过！" << endl;
            p.m_sex = -1;
        } else {
            int sex = stoi(sexStr);
            while (!checkSex(sex)) {
                cout << "❌ 性别只能是1或2，请重新输入：";
                getline(cin, sexStr);
                // 重新校验输入
                while (sexStr.empty() || !isdigit(sexStr[0])) {
                    cout << "❌ 性别只能输入数字，请重新输入：";
                    getline(cin, sexStr);
                }
                sex = stoi(sexStr);
            }
            p.m_sex = sex;
        }
    } else {
        p.m_sex = -1; // 直接回车，设为-1表示NULL
    }

    // ===================== 4. 可选输入：年龄（调用checkAge，输入了才校验）=====================
    cout << "请输入年龄（直接回车跳过）：";
    string ageStr;
    getline(cin, ageStr);
    if (!ageStr.empty()) {
        bool isNum = true;
        for (char c : ageStr) {
            if (!isdigit(c)) {
                isNum = false;
                break;
            }
        }
        if (!isNum) {
            cout << "❌ 年龄只能输入数字，本次跳过！" << endl;
            p.m_age = -1;
        } else {
            int age = stoi(ageStr);
            while (!checkAge(age)) {
                cout << "❌ 年龄必须在0-150之间，请重新输入：";
                getline(cin, ageStr);
                while (ageStr.empty() || !isdigit(ageStr[0])) {
                    cout << "❌ 年龄只能输入数字，请重新输入：";
                    getline(cin, ageStr);
                }
                age = stoi(ageStr);
            }
            p.m_age = age;
        }
    } else {
        p.m_age = -1;
    }


    // ===================== 5. 可选输入：地址（调用checkAddr，输入了才校验）=====================
    cout << "请输入新地址（直接回车跳过）：";
    string newAddr;
    getline(cin, newAddr);
    if (!newAddr.empty()) {
        while (!checkAddr(newAddr)) {
            cout << "❌ 地址不能全为空格，请重新输入：";
            getline(cin, newAddr);
        }
        p.m_addr = newAddr;
    }


    // 单条SQL插入数据库
    // 处理各个字段，适配SQL语句
    string sql_name = "'" + p.m_name + "'";
    string sql_phone = "'" + p.m_phone + "'";

    // 性别：未输入(-1) 转 NULL
    string sql_sex;
    if (p.m_sex == -1) {
        sql_sex = "NULL";
    } else {
        sql_sex = to_string(p.m_sex);
    }

    // 年龄：未输入(-1) 转 NULL
    string sql_age;
    if (p.m_age == -1) {
        sql_age = "NULL";
    } else {
        sql_age = to_string(p.m_age);
    }

    // 地址：未输入("") 转 NULL
    string sql_addr;
    if (p.m_addr.empty()) {
        sql_addr = "NULL";
    } else {
        sql_addr = "'" + p.m_addr + "'";
    }

    // 拼接最终SQL
    string sql = "INSERT INTO contact(name, sex, age, phone, addr) VALUES ("
                + sql_name + ", "
                + sql_sex + ", "
                + sql_age + ", "
                + sql_phone + ", "
                + sql_addr + ");";

    // 执行SQL
    int ret = mysql_query(mysql_conn, sql.c_str());
    if (ret != 0) {
        cout << "❌ 添加失败！错误信息：" << mysql_error(mysql_conn) << endl;
    } else {
        // ✅ 关键：读取MySQL刚生成的自增id
        p.id = (int)mysql_insert_id(mysql_conn);
        cout << "✅ 添加成功！数据已写入数据库" << endl;

        // 把数据存入通讯录数组
        abs->parray[abs->m_size] = p;
        abs->m_size++;
    }
    
    system("pause");
    system("cls");
}

//显示联系人
void showPerson(Addressbooks* abs)
{
    if (abs->m_size == 0)
    {
        cout << "当前记录为空" << endl;
        system("pause");
        system("cls");
        return;
    }

    cout << "id\t姓名\t性别\t年龄\t电话\t\t住址" << endl;
    for (int i = 0; i < abs->m_size; i++)
    {
        Person& p = abs->parray[i];
        cout << p.id << "\t"
            << p.m_name << "\t"
            // 性别：-1显示为“未填写”，1显示男，2显示女
            << (p.m_sex == -1 ? "未填写" : (p.m_sex == 1 ? "男" : "女")) << "\t"
            // 年龄：-1显示为“未填写”
            << (p.m_age == -1 ? "未填写" : to_string(p.m_age)) << "\t"
            << p.m_phone << "\t"
            << (p.m_addr.empty() ? "未填写" : p.m_addr) << endl;
    }
    system("pause");
    system("cls");
}


//删除联系人
void deletePerson(Addressbooks* abs, MYSQL* mysql_conn)
{
    cout << "请输入要删除的联系人姓名：";
    string name;
    cin >> name;

    int index = isExist(abs, name);
    if (index == -1)
    {
        cout << "查无此人！" << endl;
        system("pause");
        system("cls");
        return;
    }

    // 1. 先删数据库
    string sql = "DELETE FROM contact WHERE name='" + name + "'";
    mysql_query(mysql_conn, sql.c_str());

    // 2. 再删内存数组
    for (int i = index; i < abs->m_size - 1; i++)
    {
        abs->parray[i] = abs->parray[i + 1];
    }
    abs->m_size--;

    cout << "删除成功！" << endl;
    system("pause");
    system("cls");
}

//查找联系人
void findPerson(Addressbooks * abs)
{
    cout<<"输入查找的联系人的姓名："<<endl;
    string name;
    cin>>name;
    int ret=isExist(abs,name);
    if(ret!=-1)
    {
        cout<<"姓名："<<abs->parray[ret].m_name<<"\t";
        cout<<"性别："<<(abs->parray[ret].m_sex==1?"男":"女")<<"\t";
        cout<<"年龄："<<abs->parray[ret].m_age<<"\t";
        cout<<"电话："<<abs->parray[ret].m_phone<<"\t";
        cout<<"住址："<<abs->parray[ret].m_addr<<endl;
    }else{
        cout<<"查无此人"<<endl;
    }

    system("pause");
    system("cls");
}

//修改联系人
void modifyPerson(Addressbooks* abs, MYSQL* mysql_conn) {
    cout << "请输入要修改的联系人姓名：";
    string oldName;
    cin >> oldName;
    cin.ignore(); // 清除cin留下的换行符，避免后面getline出问题

    int index = isExist(abs, oldName);
    if (index == -1) {
        cout << "❌ 查无此人！" << endl;
        system("pause");
        system("cls");
        return;
    }

    Person& p = abs->parray[index];

    // -------------------------- 1. 输入新数据（和addPerson逻辑一样） --------------------------
    // 姓名修改
    cout << "请输入新姓名（直接回车不修改）：";
    string newName;
    getline(cin, newName);
    if (!newName.empty()) {
        while (!checkName(newName)) {
            cout << "❌ 姓名不能为空，请重新输入：";
            getline(cin, newName);
        }
        p.m_name = newName;
    }

    // 手机号修改
    cout << "请输入新电话（直接回车不修改）：";
    string newPhone;
    getline(cin, newPhone);
    if (!newPhone.empty()) {
        while (!checkPhone(newPhone)) {
            cout << "❌ 手机号不合法，请重新输入：";
            getline(cin, newPhone);
        }
        p.m_phone = newPhone;
    }

    // 性别修改
    cout << "请输入新性别（1-男 2-女，直接回车不修改）：";
    string newSexStr;
    getline(cin, newSexStr);
    if (!newSexStr.empty()) {
        bool isNum = true;
        for (char c : newSexStr) {
            if (!isdigit(c)) {
                isNum = false;
                break;
            }
        }
        if (isNum) {
            int newSex = stoi(newSexStr);
            while (!checkSex(newSex)) {
                cout << "❌ 性别只能是1或2，请重新输入：";
                getline(cin, newSexStr);
                while (newSexStr.empty() || !isdigit(newSexStr[0])) {
                    cout << "❌ 性别只能输入数字，请重新输入：";
                    getline(cin, newSexStr);
                }
                newSex = stoi(newSexStr);
            }
            p.m_sex = newSex;
        }
    }

    // 年龄修改
    cout << "请输入新年龄（直接回车不修改）：";
    string newAgeStr;
    getline(cin, newAgeStr);
    if (!newAgeStr.empty()) {
        bool isNum = true;
        for (char c : newAgeStr) {
            if (!isdigit(c)) {
                isNum = false;
                break;
            }
        }
        if (isNum) {
            int newAge = stoi(newAgeStr);
            while (!checkAge(newAge)) {
                cout << "❌ 年龄必须在0-150之间，请重新输入：";
                getline(cin, newAgeStr);
                while (newAgeStr.empty() || !isdigit(newAgeStr[0])) {
                    cout << "❌ 年龄只能输入数字，请重新输入：";
                    getline(cin, newAgeStr);
                }
                newAge = stoi(newAgeStr);
            }
            p.m_age = newAge;
        }
    }

    // 地址修改
    cout << "请输入新地址（直接回车不修改）：";
    string newAddr;
    getline(cin, newAddr);
    if (!newAddr.empty()) {
        while (!checkAddr(newAddr)) {
            cout << "❌ 地址不能全为空格，请重新输入：";
            getline(cin, newAddr);
        }
        p.m_addr = newAddr;
    }

    // -------------------------- 2. 生成UPDATE语句，处理NULL值 --------------------------
    string sql_name = "'" + p.m_name + "'";
    string sql_phone = "'" + p.m_phone + "'";

    string sql_sex = (p.m_sex == -1) ? "NULL" : to_string(p.m_sex);
    string sql_age = (p.m_age == -1) ? "NULL" : to_string(p.m_age);
    string sql_addr = p.m_addr.empty() ? "NULL" : "'" + p.m_addr + "'";

    string sql = "UPDATE contact SET "
                "name = " + sql_name + ", "
                "phone = " + sql_phone + ", "
                "sex = " + sql_sex + ", "
                "age = " + sql_age + ", "
                "addr = " + sql_addr + " "
                "WHERE id = " + to_string(p.id) + ";";

    // -------------------------- 3. 执行SQL --------------------------
    int ret = mysql_query(mysql_conn, sql.c_str());
    if (ret != 0) {
        cout << "❌ 修改失败！错误：" << mysql_error(mysql_conn) << endl;
    } else {
        cout << "✅ 修改成功！数据库已更新" << endl;
    }

    system("pause");
    system("cls");
}

//清空联系人
void cleanPerson(Addressbooks* abs, MYSQL* mysql_conn)
{
    cout << "确定清空所有联系人？1-确认 0-取消：";
    int op;
    cin >> op;
    if (op == 1)
    {
        // 清空数据库表
        mysql_query(mysql_conn, "TRUNCATE TABLE contact");
        // 清空内存缓存
        abs->m_size = 0;
        cout << "已清空所有联系人！" << endl;
    }
    else if(op == 0)
    {
        cout << "已取消清空操作" << endl;
    }else
    {
        cout<<"输入有误"<<endl;
    }
    system("pause");
    system("cls");
}


int main()
{
    SetConsoleOutputCP(65001); // 设置控制台输出编码为UTF-8
    SetConsoleCP(65001);       // 设置控制台输入编码为UTF-8
    MYSQL* mysql_conn=connectMySQL();

    Addressbooks abs;//创建通讯录
    abs.m_size=0;//初始化人数
    
    loadFromDB(&abs,mysql_conn);

    int select=0;//选择功能初始化
    while(1)
    {
        showMenu();//展示菜单
        cin>>select;
        switch(select)
        {
            case 1://添加联系人
                addPerson(&abs,mysql_conn);
                break;
            case 2://显示联系人
                showPerson(&abs);
                break;
            case 3://删除联系人
                deletePerson(&abs,mysql_conn);
                break;
            case 4://查找联系人
                findPerson(&abs);
                break;
            case 5://修改联系人
                modifyPerson(&abs,mysql_conn);
                break;
            case 6://清空联系人
                cleanPerson(&abs,mysql_conn);
                break;
            case 0://0.退出通讯录
                cout<<"欢迎下次使用"<<endl;
                break;
            default:
                break;
        }//选择功能
    }
    
    closeMySQL(mysql_conn);
   
    system("pause");
    return 0;
}