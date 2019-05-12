
Bangumi娘的使用指南
===================

### 用途

用于在QQ平台便捷地查寻条目，更新条目进度

### 使用说明

GitHub说明文档: <a href="bangumi.md">Bangumi娘使用指南</a>

网页说明文档: <a href="http://bangumi.irisu.cc/" target="_blank">Bangumi娘的食用指南</a>

### Bangumi娘

Bangumi娘QQ号: 272242684

讨论群: 477741212

---


### 搭建自己的Bangumi娘

+ 在`ToBuild`文件夹中存放了搭建所需要的必要结构和配置

---

    ToBuild
    │  db_bangumi.sql			//数据库表
    │
    ├─data					//酷Q目录下的data
    │  ├─app				//插件目录
    │  │  └─cc.sirokuma.Bangumi		//本插件的目录
    │  │      └─Bangumi			//插件设定的目录（需要自己创建）
    │  │              Bangumi.ini		//插件的配置文件[内有所有参数说明]
    │  │
    │  └─image				//data下的图片文件夹
    │      └─Cache				//插件设定的目录（需要自己创建，同时要递归赋予文件夹写入权限）
    │          │  404.png			//条目的缺省封面
    │          │  404_ava.png		//用户的缺省头像
    │          │
    │          ├─Character			//存放角色的图片
    │          ├─Subject			//存放条目的封面
    │          ├─Tag			//存放条目的Tag压缩封面
    │          └─User			//存放用户头像
    └─Redirect_URL
            bangumi.php  			//绑定功能需要的回调页面

---
+ 依据实际情况填写配置文件，同时创建以上文件夹并给予Cache及其子文件夹写入权限
+ 本插件还需要WEB环境【PHP 5.6+ 及 MySQL 5.6+】
+ Linux下推荐使用LNMP搭建WEB及MYSQL环境
+ Windows下推荐使用WampServer搭建WEB及MYSQL环境
