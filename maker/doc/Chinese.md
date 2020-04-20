#  Password Maker

> 一款高性能、可定制、跨平台和使用现代C++的密码字典生成器
  
[![GitHub issues](https://img.shields.io/github/issues/bluewingtan/password_maker?style=flat-square)](https://github.com/bluewingtan/password_maker/issues) [![GitHub pull requests](https://img.shields.io/github/issues-pr/bluewingtan/password_maker?style=flat-square)](https://github.com/bluewingtan/password_maker/pulls) [![GitHub tag (latest SemVer)](https://img.shields.io/github/v/tag/bluewingtan/password_maker?style=flat-square)](https://github.com/bluewingtan/password_maker/tags) ![PoweredBy](https://img.shields.io/badge/powered_by-BlueWingTan-blue?style=flat-square) ![PoweredBy](https://img.shields.io/badge/language-C%2B%2B-blueviolet?style=flat-square)

能够为信息安全开源界做出一点微小的贡献，我感到很开心。
  
版权所有 (C) 2020 BlueWingTan。保留所有权利。
  
- [Password Maker GitHub](https://bluewingtan.github.io/password_maker/ )
- Password Maker 文档
  - [English](../../README.md)
  - 简体中文
  
##  目录
  
- [简介](#简介 )
- [变更](#变更 )
- [兼容性](#兼容性 )
- [编译](#编译 )
- [文件组织](#文件组织 )
- [基本使用](#基本使用 )
  - [参数配置](#参数配置 )
    - [配置文件](#配置文件 )
    - [字段说明](#字段说明 )
  - [运行程序](#运行程序 )
  - [错误处理](#错误处理 )
- [贡献](#贡献 )
  - [Issues](#issues )
  - [Workflow](#workflow )
- [版权及许可](#版权及许可 )
  
##  简介
  
  
Password Maker 是一款使用现代C++编写的密码字典生成器，它的灵感来自于[passmaker](https://github.com/bit4woo/passmaker )。主要用于信息安全从业人员进行内部审计和红队进行渗透测试。
  
- Password Maker 是**高性能**的。使用合理的多线程加速，能够快速生成长密码字典。
  
- Password Maker 是**可定制**的。使用配置文件，可客制化的生成密码字典。
  
- Password Maker 是**跨平台**的。使用符合标准的C++、跨平台支撑库和编译环境，可进行跨平台编译。
  
- Password Maker 是**现代化**的。使用C++ 11标准编写，下步将支持C++17/20。
  
在密码分析和计算机安全性中，字典攻击是一种蛮力攻击技术，通过尝试多种可能的密码来确定其解密密钥或口令，以击败密码或身份验证机制。有关密码字典的更多信息，请访问
  
- [Dictionary attack](https://www.wikiwand.com/en/Dictionary_attack )
  
- [Brute-force attack](https://www.wikiwand.com/en/Brute-force_attack )
  
## 变更

- 修复了错误的序列化时机导致无输出的问题 (#1)
- 使用 `cbegin`/`cend` 来代替常量循环中的 `begin`/`end`
- 移除无用的 `_appedLock`
- 添加了`PasswordMaker`构造函数的`线程数量`参数
- 增加了命令行参数 `-c/--config` 和 `-t/--thread`

更多变更日志请参阅 [change log (英文)](CHANGELOG.md).

##  兼容性
  
  
Password Maker 能够跨平台编译，下列平台/编译器预期能够完成编译：
  
- **Windows (32/64-bit)**
  - MSVC 14.0 及以上版本
  - Clang (llvm) 3.5 及以上版本
  - GNU C++ (Cygwin) 4.8 及以上版本
  
- **Linux (32/64-bit)**
  - Clang (llvm) 3.5 及以上版本
  - GNU C++ 4.8 及以上版本
  
经过测试的平台为Windows 10 (64-bit)/MSVC 16.5。
  
##  编译
  
  
Password Maker 使用 [CMake](https://cmake.org/ ) 编译（要求 3.14 及以上版本），无其它依赖项。
  
在`Linux`上可使用`make`进行编译，在`Windows`上可使用`ninja`进行编译。
  
默认编译完成后，其生成的二进制文件将存放在`./bin`目录下，元数据文件夹及相关文件将一并复制。
  
本程序目前未附带相关测试，在后续版本中将添加。
  
##  文件组织
  
  
编译前文件组织如下：
  
- password_maker    (项目文件夹)
  - maker           (编译文件夹)
    - config        (配置文件夹)
    - dist          (种子文件夹)
    - doc           (文档文件夹)
    - inc           (头文件文件夹)
    - src           (源代码文件夹)
  
编译后输出目录文件组织如下：
  
- bin               (输出文件夹)
  - config          (配置文件夹)
  - dist            (种子文件夹)
  - generated       (输出文件夹)
  
##  基本使用
  
  
###  参数配置
  
  
####  配置文件
  
  
默认参数配置（`./config/config.json`）如下：
  
```json
{
    "config": {
        "generate_seed": {
            "file_seed": {
                "chinese_last_name": "chinese_last_name_top100.txt",
                "common_english_name": "english_name.txt",
                "common_number": "common_number.txt",
                "keyboard_walk": "4_keyboard_walk.txt",
                "year_4": "4_years.txt",
                "year_2": "2_years.txt",
                "english_name_with_chinese_last_name": "english_name_with_chinese_last_name.txt"
            },
            "domain": [ "baidu.com", "badidu" ],
            "special_letter": [ "~", "`", "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "-", "_", "=", "+" ],
            "year": [ "2018", "2019", "2020" ]
        },
        "generate_rule": {
            "formation": {
                "content": [ "keyboard_walk year chinese_last_name" ],
                "keep_in_order": true
            },
            "capitalize": false,
            "transform": {
                "active": false,
                "rules": {
                    "a": "4",
                    "e": "3",
                    "g": "9",
                    "i": "1",
                    "o": "0",
                    "s": "5",
                    "t": "7",
                    "z": "2"
                }
            }
        },
        "generate_filter": {
            "minimum_length": 6,
            "optional": {
                "number": true,
                "lower_letter": true,
                "upper_letter": true,
                "special_letter": true
            },
            "achieve_optional": 2
        },
        "generate_additional": [ "weak_pass_chinese.txt", "weak_pass_top100.txt", "weak_pass_keyboard_walk.txt" ]
    }
}
```
  
####  字段说明
  
  
**`generate_seed`种子配置**
  
根据以下种子生成密码字典
  
- `file_seed` **`object`** 各类种子的文件名称，需存放在`./dist`目录下
- `special_letter` **`array`** 特殊字符种子列表
- `其它字段` **`array`** 用户自定义的种子内容列表
  
**`generate_rule`生成规则配置**
  
根据以下规则生成密码字典：
  
- `formation` 生成格式配置
  - `content` **`array`** 需要生成的密码格式，其为以空格为分隔符的，在`file_seed`、`special_letter`或`其它字段`中定义的，连续的格式，可设置多种目标格式，输出中不含有格式中的空格
  - `keep_in_order` **`boolean`** 是否需要按照定义格式“源样”输出，如为`false`则输出定义格式的全排列
- `capitalize` **`boolean`** 是否首字母大写
- `transform` **`object`** 输出密码转换，添加到新的输出而不是修改原有值
  - `active` **`boolean`** 是否启用转换
  - `rules` **`object`** 转换规则，其`key`为密码内字符（串），其`value`为替换内容
  
**`generate_rule`过滤规则配置**
  
根据以下规则过滤不符合要求的密码字典：
  
- `minimum_length` **`unsigned number`** 密码最小长度
- `optional` **`object`** 可选择的过滤要求，生成的密码中是否需要含有以下要素
  - `number` **`boolean`** 含有数字
  - `lower_letter` **`boolean`** 含有小写字母
  - `upper_letter` **`boolean`** 含有大写字母
  - `special_letter` **`boolean`** 含有定义在`special_letter`字段中的特殊字符
- `achieve_optional` **`unsigned number`** 密码需要满足以上多少条可选规则
  
**`generate_additional`附加字典配置**
  
根据以下规则附加其它的密码字典：
  
**`array`** 附加的密码字典名称，需存放在`./dist`目录下
  
###  运行程序
  
  
配置完成后，直接运行程序即可。代码内预留了输入参数框架，可进行扩展。
  
生成的字典以`yyyy-mm-dd-HH-mm-ss.txt`格式存放在`.\generated`目录下。
  
###  错误处理
  
  
代码中进行了较为详细的log输出，如出现错误可按照输出内容进行处理。
  
##  贡献
  
Password Maker 欢迎您的贡献。在贡献时，请遵循以下规则：
  
###  Issues

欢迎随时提交问题和增强要求。
  
- 请通过提供**最少的可复现示例**来帮助我们，因为源代码更容易让其他人理解会发生什么。对于某些平台上的崩溃问题，请提交堆栈转储内容以及操作系统，编译器等的详细信息。
  
- 请先尝试断点调试，告诉我们您发现了什么，看看我们是否可以根据准备的更多信息开始探索。
  
###  Workflow
  
  
通常，我们遵循`fork-and-pull`的 Workflow：
  
1. `fork`本 repo
2. 将项目`clone`到您自己的计算机上
3. 在您的`fork`上签出一个新分支，开始在该分支上进行开发
4. 在提交之前测试更改，确保更改通过所有测试，请为每个新功能或错误修复添加测试用例（如果需要）
5. 将更改`commit`到您自己的分支
6. `push`您的本地更改到`fork`
7. 提交`pull request`，以便我们可以检查您的更改

在发出拉取请求之前，请确保合并来自“上游”的最新消息！
  
##  版权及许可
  
  
您可以从下面复制并粘贴许可证摘要。
  
    BlueWingTan is pleased to support the cyber security open source community by making Password Maker available. 
  
    Copyright (C) 2020 BlueWingTan. All rights reserved.
  
    Licensed under the MIT License (the "License"); you may not use this file except
    in compliance with the License. You may obtain a copy of the License at
  
    http://opensource.org/licenses/MIT
  
    Unless required by applicable law or agreed to in writing, software distributed 
    under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
    CONDITIONS OF ANY KIND, either express or implied. See the License for the 
    specific language governing permissions and limitations under the License.
  