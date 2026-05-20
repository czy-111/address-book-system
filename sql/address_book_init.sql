-- 创建通讯录用的数据库
CREATE DATABASE IF NOT EXISTS address_book;//数据库名，可改
USE address_book;

-- 创建联系人表
CREATE TABLE IF NOT EXISTS contact (
    id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(50) NOT NULL,
    sex INT,
    age INT,
    phone VARCHAR(20),
    addr VARCHAR(100)
);