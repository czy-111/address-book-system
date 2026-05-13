#include<iostream>
using namespace std;
#include<string>
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

//添加联系人
void addPerson(Addressbooks * abs)
{
    if(abs->m_size==MAX)
        {
            cout<<"通讯录已满，无法添加！"<<endl;
            return;
        }else{
            string name;
            cout<<"请输入姓名："<<endl;
            cin>>name;
            abs->parray[abs->m_size].m_name=name;

            cout<<"请输入性别：1--男；2--女"<<endl;
            int sex=0;
            while(1)
            {
                cin>>sex;
                if(sex==1||sex==2)
                {
                    abs->parray[abs->m_size].m_sex=sex;   
                    break;
                }
                cout<<"输入有误，请重新输入"<<endl;
            }

            cout<<"请输入年龄："<<endl;
            int age=0;
            cin>>age;
            abs->parray[abs->m_size].m_age=age;

            cout<<"请输入电话："<<endl;
            string phone;
            cin>>phone;
            abs->parray[abs->m_size].m_phone=phone;

            cout<<"请输入住址："<<endl;
            string address;
            cin>>address;
            abs->parray[abs->m_size].m_addr=address;

            abs->m_size++;

            cout<<"添加成功"<<endl;

            system("pause");
            system("cls");//清屏
            
        }
}

//显示联系人
void showPerson(Addressbooks * abs)
{
    if(abs->m_size==0)
    {
        cout<<"当前记录为空"<<endl;
    }else{
        for(int i=0;i<abs->m_size;i++)
        {
            cout<<"姓名："<<abs->parray[i].m_name<<"\t";
            cout<<"性别："<<(abs->parray[i].m_sex==1?"男":"女")<<"\t";
            cout<<"年龄："<<abs->parray[i].m_age<<"\t";
            cout<<"电话："<<abs->parray[i].m_phone<<"\t";
            cout<<"住址："<<abs->parray[i].m_addr<<endl;
        }
    }
    system("pause");
    system("cls");
}


//检测联系人是否存在
int isExist(Addressbooks * abs,string name)
{
    for(int i=0;i<abs->m_size;i++)
    {
        if(abs->parray[i].m_name==name)
        {
            return i;
        }
    }
    return -1;
}

//删除联系人
void deletePerson(Addressbooks * abs)
{
    cout<<"输入删除的联系人的姓名："<<endl;
    string name;
    cin>>name;
    int ret=isExist(abs,name);//这里的abs是子函数中的abs指针
    if(ret!=-1)
    {
        for(int i=ret;i<abs->m_size;i++)
        {
            abs->parray[i]=abs->parray[i+1];
        }
        abs->m_size--;
        cout<<"删除成功"<<endl;
    }else{
        cout<<"查无此人"<<endl;
    }

    system("pause");
    system("cls");

}

//查找联系人
void findPerson(Addressbooks * abs)
{
    cout<<"输入删除的联系人的姓名："<<endl;
    string name;
    cin>>name;
    int ret=isExist(abs,name);
    if(ret!=-1)
    {
        cout<<"姓名："<<abs->parray[ret].m_name<<"\t";
        cout<<"性别："<<abs->parray[ret].m_name<<"\t";
        cout<<"年龄："<<abs->parray[ret].m_name<<"\t";
        cout<<"电话："<<abs->parray[ret].m_name<<"\t";
        cout<<"住址："<<abs->parray[ret].m_name<<endl;
    }else{
        cout<<"查无此人"<<endl;
    }

    system("pause");
    system("cls");
}

//修改联系人
void modifyPerson(Addressbooks * abs)
{
    cout<<"输入修改的联系人的姓名："<<endl;
    string name;
    cin>>name;
    int ret=isExist(abs,name);
    if(ret!=-1)
    {
        cout<<"请输入姓名："<<endl;
        cin>>name;
        abs->parray[ret].m_name=name;

        cout<<"请输入性别：1--男；2--女"<<endl;
            int sex=0;
            while(1)
            {
                cin>>sex;
                if(sex==1||sex==2)
                {
                    abs->parray[ret].m_sex=sex;   
                    break;
                }
                cout<<"输入有误，请重新输入"<<endl;
            }

            cout<<"请输入年龄："<<endl;
            int age=0;
            cin>>age;
            abs->parray[ret].m_age=age;

            cout<<"请输入电话："<<endl;
            string phone;
            cin>>phone;
            abs->parray[ret].m_phone=phone;

            cout<<"请输入住址："<<endl;
            string address;
            cin>>address;
            abs->parray[ret].m_addr=address;

            cout<<"修改成功"<<endl;
    }else{
        cout<<"查无此人"<<endl;
    }

    system("pause");
    system("cls");
}

//清空联系人
void cleanPerson(Addressbooks * abs)
{
    cout<<"是否清空全部联系人？1--是；2--否"<<endl;
    int a=0;
    cin>>a;
    while(1)
            {
                cin>>a;
                if(a==1)
                {
                    abs->m_size=0;
                    cout<<"已清空"<<endl;
                    break;
                }else if(a==2)
                {
                    break;
                }else{
                    cout<<"输入有误，请重新输入"<<endl;
                }
                
            }
   
    system("pause");
    system("cls");
}


int main()
{
    Addressbooks abs;//创建通讯录
    abs.m_size=0;//初始化人数
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
                return 0;//退出系统
                break;
            default:
                break;
        }//选择功能
    }
    
   
    system("pause");
    return 0;
}