#  Password Maker

> A high-performance, customizable, cross-platform and modern C ++ password dictionary generator
  
[![GitHub issues](https://img.shields.io/github/issues/bluewingtan/password_maker?style=flat-square)](https://github.com/bluewingtan/password_maker/issues) [![GitHub pull requests](https://img.shields.io/github/issues-pr/bluewingtan/password_maker?style=flat-square)](https://github.com/bluewingtan/password_maker/pulls) [![GitHub release (latest SemVer)](https://img.shields.io/github/v/release/bluewingtan/password_maker?style=flat-square)](https://github.com/bluewingtan/password_maker/releases) [![GitHub tag (latest SemVer)](https://img.shields.io/github/v/tag/bluewingtan/password_maker?style=flat-square)](https://github.com/bluewingtan/password_maker/tags) ![PoweredBy](https://img.shields.io/badge/powered_by-BlueWingTan-blue?style=flat-square) ![PoweredBy](https://img.shields.io/badge/language-C%2B%2B-blueviolet?style=flat-square)

BlueWingTan is pleased to support the cyber security open source community by making Password Maker available.
  
Copyright (C) 2020 BlueWingTan. All rights reserved.
  
- [Password Maker GitHub](https://bluewingtan.github.io/password_maker/ )
- Password Maker Document
  - English
  - [简体中文](./maker/doc/Chinese.md )
  
##  Table of Contents

- [Introduction](#introduction )
- [Compatibility](#compatibility )
- [Compile](#compile )
- [File Organization](#file-organization )
- [Basic Usage](#basic-usage )
  - [Parameter configuration](#parameter-configuration )
    - [Configuration file](#configuration-file )
    - [Field description](#field-description )
  - [Run the program](#run-the-program )
  - [Error handling](#error-handling )
- [Contributing](#contributing )
  - [Issues](#issues )
  - [Workflow](#workflow )
- [Copyright and Licensing](#copyright-and-licensing )
  
##  Introduction
  
  
Password Maker is a password dictionary generator written in modern C++，It was inspired by [passmaker](https://github.com/bit4woo/passmaker ). Mainly used for information security practitioners to conduct internal audits and RED TEAMs for penetration testing.
  
- Password Maker is **high performance**. Using reasonable multi-thread acceleration, it can quickly generate a long password dictionary.
  
- Password Maker is **customizable**. Using the configuration file, a password dictionary can be customized.
  
- Password Maker is **cross-platform**. Use standard-compliant C++, cross-platform support library, and compilation environment for cross-platform compilation.
  
- Password Maker is **modern**. Written in C++ 11 standard, the next step will support C++ 17/20.
  
In cryptanalysis and computer security, a dictionary attack is a form of brute force attack technique for defeating a cipher or authentication mechanism by trying to determine its decryption key or passphrase by trying hundreds or sometimes millions of likely possibilities, such as words in a dictionary. More information about password dictionary can be obtained at
  
- [Dictionary attack](https://www.wikiwand.com/en/Dictionary_attack )
  
- [Brute-force attack](https://www.wikiwand.com/en/Brute-force_attack )
  
##  Compatibility
  
  
Password Maker can be compiled across platforms, and the following platforms/compilers are expected to complete the compilation:
  
- **Windows (32/64-bit)**
  - MSVC 14.0 and above
  - Clang (llvm) 3.5 and above
  - GNU C++ (Cygwin) 4.8 and above
  
- **Linux (32/64-bit)**
  - Clang (llvm) 3.5 and above
  - GNU C++ 4.8 and above
  
The tested platform is Windows 10 (64-bit)/MSVC 16.5.
  
##  Compile
  
  
Password Maker compile with Cmake (requires version 3.14 and above), no other dependencies.
  
On `Linux` you can use `make` to compile, on `Windows` you can use `ninja` to compile.
  
After the default compilation is complete, the generated binary files will be stored in the `./Bin` directory, and the metadata folder and related files will be copied together.
  
This program is currently not accompanied by related tests and will be added in subsequent versions.
  
##  File Organization
  
  
The files before compilation are organized as follows:
  
- password_maker    (Project folder)
  - maker           (Compile folder)
    - config        (Configuration folder)
    - dist          (Seed folder)
    - doc           (Document folder)
    - inc           (Header file folder)
    - src           (Source code folder)
  
The output directory file after compilation is organized as follows:
  
- bin               (Output folder)
  - config          (Configuration folder)
  - dist            (Seed folder)
  - generated       (Output folder)
  
##  Basic Usage
  
  
###  Parameter configuration
  
  
####  Configuration file
  
  
The default parameter configuration (`./config/config.json`) is as follows:
  
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
                "content": [ "common_number domain year" ],
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
  
####  Field description
  
  
**`generate_seed` Seed configuration**
  
Generate a password dictionary based on the following seeds
  
- `file_seed` **`object`** The file names of various seeds need to be stored in the `./dist` directory
- `special_letter` **`array`** Special character seed list
- `OTHER FIELDS` **`array`** User-defined seed content list
  
**`generate_rule` Generate rule configuration**
  
Generate a password dictionary according to the following rules:
  
- `formation` Generate format configuration
  - `content` **`array`** The format of the password to be generated, which is separated by spaces, defined in `file_seed`,`special_letter` or `OTHER FIELDS` and a continuous format, multiple target formats can be set, and the output does not contain the space in format
  - `keep_in_order` **`boolean`** Whether it needs to be output "as is" according to the defined format, if `false`, the entire arrangement of the defined format is output
- `capitalize` **`boolean`** Whether to capitalize the first letter
- `transform` **`object`** Output password conversion, add to new output instead of modifying original value
  - `active` **`boolean`** Whether to enable transform
  - `rules` **`object`** Transformation rules, the `key` is the character (string) in the password, and the`value` is the replacement content
  
**`generate_rule` Filter rule configuration**
  
Filter password dictionaries that do not meet the requirements according to the following rules：
  
- `minimum_length` **`unsigned number`** Password minimum length
- `optional` **`object`** Optional filtering requirements, does the generated password need to contain the following elements
  - `number` **`boolean`** Contains numbers
  - `lower_letter` **`boolean`** Contains lowercase letters
  - `upper_letter` **`boolean`** Contains capital letters
  - `special_letter` **`boolean`** Contains special characters defined in the `special_letter` field
- `achieve_optional` **`unsigned number`** How many optional rules must the password meet
  
**`generate_additional` Additional dictionary configuration**
  
Attach other password dictionaries according to the following rules:
  
**`array`** The additional password dictionary name needs to be stored in the `./dist` directory
  
###  Run the program
  
  
After the configuration is complete, simply run the program. The input parameter frame is reserved in the code and can be expanded.
  
The generated dictionary is stored in the `.\generated` directory in the format `yyyy-mm-dd-HH-mm-ss.txt`.
  
###  Error handling
  
  
A more detailed log output is performed in the code, and if an error occurs, it can be processed according to the output content.
  
##  Contributing
  
  
Password Maker welcomes contributions. When contributing, please follow the code below.
  
###  Issues
  
  
Feel free to submit issues and enhancement requests.
  
- Please help us by providing **minimal reproducible examples**, because source code is easier to let other people understand what happens. For crash problems on certain platforms, please bring stack dump content with the detail of the OS, compiler, etc.
  
- Please try breakpoint debugging first, tell us what you found, see if we can start exploring based on more information been prepared.
  
###  Workflow
  
  
In general, we follow the "fork-and-pull" Git workflow.
  
1. **Fork** the repo on GitHub
2. **Clone** the project to your own machine
3. **Checkout** a new branch on your fork, start developing on the branch
4. **Test** the change before commit, Make sure the changes pass all the tests, please add test case for each new feature or bug-fix if needed.
5. **Commit** changes to your own branch
6. **Push** your work back up to your fork
7. Submit a **Pull** request so that we can review your changes
  
NOTE: Be sure to merge the latest from "upstream" before making a pull request!
  
##  Copyright and Licensing
  
  
You can copy and paste the license summary from below.
  
    BlueWingTan is pleased to support the cyber security open source community by making Password Maker available.
  
    Copyright (C) 2020 BlueWingTan. All rights reserved.
  
    Licensed under the MIT License (the "License"); you may not use this file except
    in compliance with the License. You may obtain a copy of the License at
  
    http://opensource.org/licenses/MIT
  
    Unless required by applicable law or agreed to in writing, software distributed 
    under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
    CONDITIONS OF ANY KIND, either express or implied. See the License for the 
    specific language governing permissions and limitations under the License.
  