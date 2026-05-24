-- 创建通讯录数据库
CREATE DATABASE IF NOT EXISTS address_book;
USE address_book;

-- 创建联系人表
CREATE TABLE IF NOT EXISTS contact (
    id INT PRIMARY KEY AUTO_INCREMENT COMMENT '主键ID，自增',
    name VARCHAR(50) NOT NULL COMMENT '联系人姓名（必填）',
    sex INT COMMENT '性别：1-男，2-女，NULL表示未填写',
    age INT COMMENT '年龄，NULL表示未填写',
    phone VARCHAR(20) NOT NULL UNIQUE COMMENT '手机号（必填，不重复）',
    addr VARCHAR(100) COMMENT '地址，NULL表示未填写'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;