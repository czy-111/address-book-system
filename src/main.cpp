#include<mysql.h>
#include<iostream>
#include<windows.h>
using namespace std;
#include<string>
#include <cstdlib>  // 必须加这个头文件，才能用getenv
#define MAX 1000

//联系人结构体
struct Person
{
    string m_name;//姓名
    int m_sex;//性别，1男2女
    int m_age;//年龄
    string m_phone;//电话
    string m_addr;//住址
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
bool connectMySQL() {
    // 初始化连接
    mysql_conn = mysql_init(NULL);
    if (!mysql_conn) {
        cout << "MySQL初始化失败！" << endl;
        return false;
    }

    // 从环境变量读取密码，避免明文泄露
    const char* db_pass = getenv("MYSQL_PASSWORD");
    if (!db_pass) {
        cerr << "错误：请先设置环境变量 MYSQL_PASSWORD！" << endl;
        cerr << "Windows: set MYSQL_PASSWORD=你的密码" << endl;
        cerr << "Linux/macOS: export MYSQL_PASSWORD=你的密码" << endl;//cerr 是为了专门标记错误信息，更规范、更不容易丢
        return false;
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
        return false;
    }

    // 设置编码，防止中文乱码
    // 连接MySQL后立即设置字符集
    mysql_query(mysql_conn, "SET NAMES 'utf8mb4'");
    mysql_query(mysql_conn, "SET CHARACTER SET utf8mb4");
    mysql_query(mysql_conn, "SET character_set_client = utf8mb4");
    mysql_query(mysql_conn, "SET character_set_results = utf8mb4");
    mysql_query(mysql_conn, "SET character_set_connection = utf8mb4");
    cout << "? MySQL数据库连接成功！" << endl;
    return true;
}

// 关闭MySQL连接
void closeMySQL() {
    if (mysql_conn) {
        mysql_close(mysql_conn);
        cout << "数据库连接已关闭" << endl;
    }
}

// 从MySQL加载所有联系人到Addressbooks
void loadFromDB(Addressbooks *abs) {
    // 清空当前数组
    abs->m_size = 0;
    string sql = "SELECT name, sex, age, phone, addr FROM contact";
    
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
        abs->parray[abs->m_size].m_name = row[0];
        abs->parray[abs->m_size].m_sex = atoi(row[1]); // 字符串转int
        abs->parray[abs->m_size].m_age = atoi(row[2]);
        abs->parray[abs->m_size].m_phone = row[3];
        abs->parray[abs->m_size].m_addr = row[4];
        abs->m_size++;
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
void addPerson(Addressbooks* abs)
{
    if (abs->m_size >= MAX)
    {
        cout << "通讯录已满，无法添加！" << endl;
        system("pause");
        system("cls");
        return;
    }

    string name;
    cout << "请输入姓名：";
    cin >> name;

    if (isExist(abs, name) != -1)
    {
        cout << "联系人已存在，无需重复添加！" << endl;
        system("pause");
        system("cls");
        return;
    }

    int sex;
    cout << "请输入性别 1-男 2-女：";
    while (1)
    {
        cin >> sex;
        if (sex == 1 || sex == 2) break;
        cout << "输入错误，请重新输入：";
    }

    int age;
    string phone, addr;
    cout << "请输入年龄："; cin >> age;
    cout << "请输入电话："; cin >> phone;
    cout << "请输入住址："; cin >> addr;

    // 1. 先更新内存缓存
    abs->parray[abs->m_size].m_name = name;
    abs->parray[abs->m_size].m_sex  = sex;
    abs->parray[abs->m_size].m_age  = age;
    abs->parray[abs->m_size].m_phone= phone;
    abs->parray[abs->m_size].m_addr = addr;
    abs->m_size++;

    // 2. 单条SQL插入数据库
    string sql = "INSERT INTO contact(name,sex,age,phone,addr) VALUES('"
        + name + "'," + to_string(sex) + "," + to_string(age) + ",'"
        + phone + "','" + addr + "')";
    int ret=mysql_query(mysql_conn, sql.c_str());
    if (ret != 0) {
    cout << "❌ 添加失败！错误信息：" << mysql_error(mysql_conn) << endl;
    } else {
         cout << "✅ 添加成功！数据已写入数据库" << endl;
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

    cout << "姓名\t性别\t年龄\t电话\t\t住址" << endl;
    for (int i = 0; i < abs->m_size; i++)
    {
        cout << abs->parray[i].m_name << "\t";
        cout << (abs->parray[i].m_sex == 1 ? "男" : "女") << "\t";
        cout << abs->parray[i].m_age << "\t";
        cout << abs->parray[i].m_phone << "\t";
        cout << abs->parray[i].m_addr << endl;
    }
    system("pause");
    system("cls");
}


//删除联系人
void deletePerson(Addressbooks* abs)
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
void modifyPerson(Addressbooks* abs)
{
    cout << "请输入要修改的联系人姓名：";
    string oldName;
    cin >> oldName;

    int index = isExist(abs, oldName);
    if (index == -1)
    {
        cout << "查无此人！" << endl;
        system("pause");
        system("cls");
        return;
    }

    string newName, phone, addr;
    int sex, age;
    cout << "请输入新姓名："; cin >> newName;
    cout << "请输入新性别 1-男 2-女："; cin >> sex;
    cout << "请输入新年龄："; cin >> age;
    cout << "请输入新电话："; cin >> phone;
    cout << "请输入新住址："; cin >> addr;

    // 1. 更新内存
    abs->parray[index].m_name = newName;
    abs->parray[index].m_sex  = sex;
    abs->parray[index].m_age  = age;
    abs->parray[index].m_phone= phone;
    abs->parray[index].m_addr = addr;

    // 2. 单条更新数据库
    string sql = "UPDATE contact SET name='" + newName + "',sex=" + to_string(sex)
        + ",age=" + to_string(age) + ",phone='" + phone + "',addr='" + addr
        + "' WHERE name='" + oldName + "'";
    mysql_query(mysql_conn, sql.c_str());

    cout << "修改成功！" << endl;
    system("pause");
    system("cls");
}

//清空联系人
void cleanPerson(Addressbooks* abs)
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
    connectMySQL();

    Addressbooks abs;//创建通讯录
    abs.m_size=0;//初始化人数
    
    loadFromDB(&abs);

    int select=0;//选择功能初始化
    while(1)
    {
        showMenu();//展示菜单
        cin>>select;
        switch(select)
        {
            case 1://添加联系人
                addPerson(&abs);
                break;
            case 2://显示联系人
                showPerson(&abs);
                break;
            case 3://删除联系人
                deletePerson(&abs);
                break;
            case 4://查找联系人
                findPerson(&abs);
                break;
            case 5://修改联系人
                modifyPerson(&abs);
                break;
            case 6://清空联系人
                cleanPerson(&abs);
                break;
            case 0://0.退出通讯录
                cout<<"欢迎下次使用"<<endl;
                break;
            default:
                break;
        }//选择功能
    }
    
    closeMySQL();
   
    system("pause");
    return 0;
}