#!/usr/bin/python3

###############################################################################################
##
##  @file: server-assistant.py
##  @brief：生成路由注册代码、生成DTO序列化和反序列化代码
##
##  使用示例: (-h 查看帮助)
##    ./server-assistant.py /path/to/dir/controller /path/to/file/output -p controller
##
##  安装第三方库：(CppHeaderParser)
##    pip3 install robotpy-cppheaderparser
##
##  详细说明请参考文档 server-assistant/README.md
##
###############################################################################################

import argparse
import hashlib
import json
import os
import re
import tempfile
from enum import Enum
from functools import cmp_to_key
from typing import List, Dict, Set

# pip3 install robotpy-cppheaderparser
from CppHeaderParser import CppHeader, CppClass


###############################################################################################
##
##                                     (1) 辅助函数
##
###############################################################################################

class Helper:
    @staticmethod
    def FormatPath(dir: str):
        dir = dir.strip()
        dir = dir.replace('\\', '/')
        return dir

    @staticmethod
    def FormatDir(dir: str):
        ''' 格式化文件夹字符串，以/结尾 '''
        dir = Helper.FormatPath(dir)
        if len(dir) > 0 and dir[-1] != '/':
            dir += '/'
        return dir

    @staticmethod
    def FormatNamespace(ns: str):
        return ns if len(ns) == 0 or ns.endswith('::') else ns + '::'

    @staticmethod
    def FindFirstOf(input: str, chars: str, start_pos: int = 0):
        index = next((i for i, c in enumerate(input[start_pos:]) if c in chars), -1)
        return -1 if index < 0 else start_pos + index

    @staticmethod
    def ListHeaderFiles(dir: str):
        ''' 列出文件夹下的所有头文件(.h .hpp) '''
        files: List[str] = []
        Helper.__ListHeaderFiles(dir, len(dir), files)
        files.sort()
        return files

    @staticmethod
    def __ListHeaderFiles(dir: str, root_len: int, files: List[str]):
        ''' 列出文件夹下的所有头文件(.h .hpp) '''
        for file in os.listdir(dir):
            path = os.path.join(dir, file)
            if os.path.isdir(path):
                Helper.__ListHeaderFiles(path, root_len, files)
            else:
                if path.endswith('.h') or path.endswith('.hpp'):
                    files.append(path[root_len:])

    @staticmethod
    def ParseDoxygen(doxygen: str) -> Dict[str, List[str]]:
        ''' 解析Doxygen格式的注释 '''
        res: Dict[str, List[str]] = {}
        lines: List[str] = doxygen.split('\n')
        for line in lines:
            line = line.strip()
            at_pos = line.find('@')
            if at_pos < 0:
                continue
            blank_pos = Helper.FindFirstOf(line, ' \t', at_pos)
            blank_pos = blank_pos if blank_pos >= 0 else len(line)
            attr_name = line[at_pos:blank_pos].strip()
            attr_value = line[blank_pos+1:].strip()
            if attr_name not in res:
                res[attr_name] = []
            res[attr_name].append(attr_value)
        return res


###############################################################################################
##
##                                   (2) 生成路由注册代码
##
###############################################################################################

valid_http_methods = set(['GET', 'HEAD', 'POST', 'PUT', 'DELETE', 'CONNECT, OPTIONS', 'TRACE', 'PATCH'])

class Route:
    ''' 路由 '''
    def __init__(self):
        self.is_regex = False
        self.header_file_abspath = ''
        self.header_file_relpath = ''
        self.description = ''
        self.pathes: List[str] = []
        self.function = ''
        self.http_methods: List[str] = []
        self.config = {}

    def __ConfigToCppMap(self):
        str = ''
        for key in sorted(self.config.keys()):
            if len(str) > 0:
                str += ', '
            str += '{"%s", "%s"}' % (key.replace('"', '\\"'), self.config[key].replace('"', '\\"'))
        return '{' + str + '}'

    def __ToCppHttpMethods(self):
        cpp_http_methods = ''
        for method in self.http_methods:
            cpp_http_methods += '%sHttpMethod::k%s' % ((' | ' if len(cpp_http_methods) > 0 else ''), method)
        return cpp_http_methods if len(cpp_http_methods) > 0 else 'GET'

    def __GetHttpMethodsStr(self):
        return ','.join(self.http_methods).rjust(4)

    def ToCppParamerterList(self, route_path: str):
        return '"%s", %s, %s, "%s", %s' % (route_path, self.__ToCppHttpMethods(), self.function, self.description, self.__ConfigToCppMap())

    def Print(self):
        prefix = '@regex ' if self.is_regex else '@static'
        for r in self.pathes:
            print('%s %s %s' % (prefix, self.__GetHttpMethodsStr(), r))

    def PrintDebug(self):
        suffix = ' @regex' if self.is_regex else ''
        for r in self.pathes:
            print('route: %s %s' % (r, suffix))
        print('    method: %s' % self.__GetHttpMethodsStr())
        print('   include: %s' % self.header_file_relpath)
        print('  function: %s' % self.function)

    def ParseFunction(self, function: dict):
        name = function['name']
        namespace = Helper.FormatNamespace(function['namespace'])
        self.function = namespace + name
        if not self.ParseDoxygen(function.get('doxygen', '')):
            print('-------------------------------------')
            print('Invalid route:')
            print('     header: %s' % self.header_file_relpath)
            print('  namespace: %s' % namespace)
            print('   function: %s' % name)
            print('   line num: %d' % function['line_number'])
            return False
        return True

    def ParseClassMethod(self, method: dict, class_name: str):
        name = method['name']
        namespace = Helper.FormatNamespace(method.get('namespace', ''))
        self.function = namespace + class_name + '::' + name
        if not self.ParseDoxygen(method.get('doxygen', '')):
            print('-------------------------------------')
            print('Invalid route:')
            print('     header: %s' % self.header_file_relpath)
            print('  namespace: %s' % namespace)
            print('      class: %s' % class_name)
            print('     method: %s' % name)
            print('   line num: %d' % method['line_number'])
            return False
        return True

    def ParseDoxygen(self, doxygen: str):
        ''' 解析doxygen格式注释 '''
        doxygen_attrs = Helper.ParseDoxygen(doxygen)
        if '@route' in doxygen_attrs:
            for route_path in doxygen_attrs['@route']:
                if not self.__ParseDoxygen_Route(route_path):
                    return False
        if '@method' in doxygen_attrs:
            for method_str in doxygen_attrs['@method']:
                if not self.__ParseDoxygen_Method(method_str):
                    return False
            self.http_methods.sort()
        if '@config' in doxygen_attrs:
            for config_str in doxygen_attrs['@config']:
                if not self.__ParseDoxygen_Config(config_str):
                    return False
        if '@brief' in doxygen_attrs:
            self.description = doxygen_attrs['@brief'][0]
        return True

    def __ParseDoxygen_Route(self, route_path: str):
        ''' 解析 @route '''
        route_path_tmp = route_path.replace('\\', '\\\\')
        is_regex = False
        if len(route_path_tmp) > 0 and route_path_tmp[0] == '~':  # 正则路由
            route_path_tmp = route_path_tmp[1:].strip()
            is_regex = True
        if len(route_path_tmp) == 0 or route_path_tmp[0] != '/':
            print('Invalid @route: [%s]' % route_path)
            return False
        if len(self.pathes) > 0 and self.is_regex != is_regex:
            # 多个route必须全是static或全是regex，不能混合
            print('Invalid @route: [%s], mixed route type' % route_path)
            return False
        self.pathes.append(route_path_tmp)
        self.is_regex = is_regex
        return True

    def __ParseDoxygen_Method(self, method_str: str):
        ''' 解析 @method '''
        methods = re.split(r'[,\s]+', method_str.upper())
        for method in methods:
            if len(method) == 0 or method in self.http_methods:
                continue
            if method not in valid_http_methods:
                print('Invalid @method: [%s]' % method_str)
                return False
            self.http_methods.append(method)
        return True

    def __ParseDoxygen_Config(self, config_str: str):
        ''' 解析 @config '''
        left_pos = config_str.find('(')
        if left_pos < 0:
            self.config[config_str] = ''
            return True
        right_pos = config_str.find(')', left_pos)
        if right_pos < 0:
            print('Invalid @config: [%s]' % config_str)
            return False
        name = config_str[0:left_pos].strip()
        value = config_str[left_pos+1:right_pos].strip()
        self.config[name] = value
        return True

class RouteCollector:
    ''' 路由收集器 '''
    def __init__(self, output_file: str, overwrite: bool):
        self.output_file = output_file
        self.overwirte = overwrite
        self.routes_dict: Dict[str, List[Route]] = {}  # { header_file_abspath: List[Route] }

    @staticmethod
    def __CompareRoute(r1: Route, r2: Route):
        result = (r1.header_file_relpath > r2.header_file_relpath) - (r1.header_file_relpath < r2.header_file_relpath)
        return result if result != 0 else (r1.pathes[0] > r2.pathes[0]) - (r1.pathes[0] < r2.pathes[0])

    def ParseHeaderFile(self, header_file_abspath, header_file_relpath: str, parser: CppHeader):
        ''' 解析头文件 '''
        route_list: List[Route] = []
        # 1. 普通函数
        for function in parser.functions:
            route = Route()
            route.header_file_abspath = header_file_abspath
            route.header_file_relpath = header_file_relpath
            if not route.ParseFunction(function):
                return False
            if len(route.pathes) > 0:
                route_list.append(route)
        # 2. 类成员方法
        for class_name in parser.classes:
            methods = parser.classes[class_name]['methods']
            for public_method in methods['public']: # 公有方法
                route = Route()
                route.header_file_abspath = header_file_abspath
                route.header_file_relpath = header_file_relpath
                if not route.ParseClassMethod(public_method, class_name):
                    return False
                if len(route.pathes) > 0:
                    route_list.append(route)
        if len(route_list) > 0:
            self.routes_dict[header_file_abspath] = route_list
        return True

    def WriteToOutputFile(self):
        # 按指定规则排序
        routes: List[Route] = []
        for route_list in self.routes_dict.values():
            routes += route_list
        routes.sort(key = cmp_to_key(RouteCollector.__CompareRoute))
        print('------------------- begin route list --------------------')
        for route in routes:
            #route.PrintDebug()
            route.Print()
        print('-------------------- end route list --------------------')
        count_regex_routes  = sum([(len(route.pathes) if route.is_regex else 0) for route in routes])
        count_static_routes = sum([(len(route.pathes) if not route.is_regex else 0) for route in routes])
        print('count(static routes):', count_static_routes)
        print('count(regex  routes):', count_regex_routes)
        print('total:', count_static_routes + count_regex_routes)
        if os.path.exists(self.output_file) and not self.overwirte:
            operation = input('Output file is exist already. Overwrite? (y/N) ')
            if operation != 'y' and operation != 'Y':
                print('Canceled')
                return False
        return self.__WriteToOutputFile(routes)

    def __WriteToOutputFile(self, routes: List[Route]):
        header_lines = [
            '// Generated by script server-assistant.py',
            '// DO NOT EDIT',
            '#include <server/router.h>'
        ]
        headers_set = set()
        for route in routes:
            if route.header_file_relpath not in headers_set:
                header_lines.append('#include "%s"' % route.header_file_relpath)
                headers_set.add(route.header_file_relpath)

        body_lines = [
            'bool register_routes(std::shared_ptr<ic::server::Router> router) {',
            '    using namespace ic::server;',
            '    bool ret = true;'
        ]
        last_include_header_file_relpath = ''
        for route in routes:
            if route.header_file_relpath != last_include_header_file_relpath:
                body_lines.append('')
                body_lines.append('    // %s'  % (route.header_file_relpath))
                last_include_header_file_relpath = route.header_file_relpath
            if route.is_regex:
                for r in route.pathes:
                    body_lines.append('    ret &= router->AddRegexRoute(%s);' % (route.ToCppParamerterList(r)))
            else:
                for r in route.pathes:
                    body_lines.append('    ret &= router->AddStaticRoute(%s);' % (route.ToCppParamerterList(r)))
        body_lines.append('')
        body_lines.append('    return ret;')
        body_lines.append('} // end register_routes')
        with open(self.output_file, mode='w', encoding='utf-8') as f:
            for line in header_lines:
                f.write(line + '\n')
            f.write('\n')
            for line in body_lines:
                f.write(line + '\n')
        return True


###############################################################################################
##
##                           (3) 生成DOT序列化、反序列化方法的实现代码
##
###############################################################################################

dto_valid_cpp_builtin_types = {
    'bool', 'int', 'signed int', 'int32_t', 'int64_t', 'unsigned int', 'uint32_t', 'uint64_t', 'size_t', 'float', 'double', 'string',
    'std::int32_t', 'std::int64_t', 'std::uint32_t', 'std::uint64_t', 'std::size_t', 'std::string'
}

dto_valid_cpp_stl_types = {
    "std::vector" : { "reserve": True,  "method_add_element": "push_back" },
    "std::deque"  : { "reserve": False, "method_add_element": "push_back" },
    "std::list"   : { "reserve": False, "method_add_element": "push_back" },
    "std::set"    : { "reserve": False, "method_add_element": "emplace"   },
    "std::unordered_set" : { "reserve": False, "method_add_element": "emplace" },
}

class DtoFieldType(Enum):
    NONE = 0
    BuiltInType = 1
    UserType = 2
    STL_BuiltInType = 3
    STL_UserType = 4

class DtoField:
    def __init__(self):
        self.name: str = ''
        self.alias: str = ''
        self.stl_type: str = ''
        self.stl_attr: dict = {}
        self.value_type: str = ''
        self.type: DtoFieldType = DtoFieldType.NONE
        self.description: str = ''
        self.required: bool = True
        self.ignore: bool = False

class Dto:
    class DtoType(Enum):
        NONE = 0
        IN = 1
        OUT = 2
        IN_OUT = 3

    def __init__(self, user_types: Set[str]):
        self.name: str = ''
        self.namespace: str = ''
        self.description: str = ''
        self.fields: List[DtoField] = []
        self.required_field_count = 0
        self.dto_type: int = Dto.DtoType.NONE.value
        self.user_types = user_types

    def Parse(self, cpp_class: CppClass):
        ''' 返回值：-1(出错), 0(成功), 1(忽略) '''
        self.name = cpp_class['name']
        self.namespace = Helper.FormatNamespace(cpp_class.get('namespace', ''))
        self.fields.clear()
        # 解析doxygen注释
        doxygen_attrs = Helper.ParseDoxygen(cpp_class.get('doxygen', ''))
        if '@brief' in doxygen_attrs:
            self.description = doxygen_attrs['@brief'][0]
        for property in cpp_class['properties']['public']:
            if property['static']: # 忽略静态成员变量
                continue
            if not self.__ParsePublicProperty(property):
                print('Error: parse public property failed. <%s> <%d>' % (property['filename'], property['line_number']))
                return -1
        # 判断是否有有 DTO_IN、DOT_OUT、DOT_IN_OUT 这三个宏
        if self.dto_type > 0:
            self.user_types.add(self.namespace + self.name)
            return 0
        return 1

    def __ParsePublicProperty(self, property: dict):
        ''' 解析public成员变量 '''
        field: DtoField = DtoField()
        field.name = property['name']
        field.alias = field.name
        field.value_type = property['type']
        # 判断成员变量类型
        if not self.ValidateType(field):
            return True
        # 解析doxygen注释
        doxygen_attrs = Helper.ParseDoxygen(property.get('doxygen', ''))
        if '@alias' in doxygen_attrs:
            aliases = doxygen_attrs['@alias']
            if len(aliases) > 1:
                print('Error: @alias should be occur once at most. <%s> <%d>' % (property['filename'], property['line_number']))
                return False
            field.alias = aliases[0]
        field.description = doxygen_attrs['@brief'] if '@brief' in doxygen_attrs else ''
        field.ignore = True if '@ignore' in doxygen_attrs else False
        field.required = False if '@optional' in doxygen_attrs else True
        self.fields.append(field)
        self.required_field_count += int(field.required)
        return True

    def ValidateType(self, field: DtoField):
        ''' 验证类型是否正确 '''
        # 特殊类型
        if field.value_type == 'DTO_IN':
            self.dto_type = self.dto_type | Dto.DtoType.IN.value
            return False
        elif field.value_type == 'DTO_OUT':
            self.dto_type = self.dto_type | Dto.DtoType.OUT.value
            return False
        elif field.value_type == 'DTO_IN_OUT':
            self.dto_type = self.dto_type | Dto.DtoType.IN_OUT.value
            return False
        # 非容器类型
        [value_type, type] = self.__ValidatePlainType(field.value_type)
        if type != DtoFieldType.NONE:
            field.value_type = value_type
            field.type = type
            return True
        # 容器类型
        type_arr = re.split('<|>', field.value_type)
        type_arr = [item.strip() for item in type_arr if item.strip()]
        if len(type_arr) == 2:
            if not type_arr[0].startswith('std::'):
                type_arr[0] = 'std::' + type_arr[0]
            if type_arr[0] in dto_valid_cpp_stl_types.keys():
                [value_type, type] = self.__ValidatePlainType(type_arr[1])
                field.value_type = value_type
                field.stl_type = type_arr[0]
                field.stl_attr = dto_valid_cpp_stl_types[type_arr[0]]
                if type == DtoFieldType.BuiltInType:
                    field.type = DtoFieldType.STL_BuiltInType
                    return True
                elif type == DtoFieldType.UserType:
                    field.type = DtoFieldType.STL_UserType
                    return True
        return False

    def __ValidatePlainType(self, type: str):
        if type in dto_valid_cpp_builtin_types:
            return [type, DtoFieldType.BuiltInType]
        if type in self.user_types:
            return [type, DtoFieldType.UserType]
        if (self.namespace + type) in self.user_types:
            return [self.namespace + type, DtoFieldType.UserType]
        return ['', DtoFieldType.NONE]

    def CreateImplementationCode(self):
        ''' 生成序列化、反序列化实现代码 '''
        lines: List[str] = []
        if self.dto_type & Dto.DtoType.IN.value:
            lines += self.__CreateImplementationCode_Deserialize()
        if self.dto_type & Dto.DtoType.OUT.value:
            lines += self.__CreateImplementationCode_Serialize()
        return lines

    def __CreateImplementationCode_Serialize(self):
        ''' 生成DTO序列化代码 '''
        lines: List[str] = []
        lines.append('Json::Value %s%s::Serialize() const {' % (self.namespace, self.name))
        lines.append('    Json::Value json;')
        for field in self.fields:
            if field.ignore:
                lines.append('    // ignore "%s"' % field.name)
                continue
            if field.type == DtoFieldType.BuiltInType:
                lines.append('    json["%s"] = %s;' % (field.alias, field.name))
            elif field.type == DtoFieldType.UserType:
                lines.append('    json["%s"] = %s.Serialize();' % (field.alias, field.name))
            elif field.type == DtoFieldType.STL_BuiltInType:
                lines.append('    Json::Value& _%s_node_temp_ = json["%s"]; for (auto& item : %s) { _%s_node_temp_.append(item); }' % (field.name, field.alias, field.name, field.name))
            elif field.type == DtoFieldType.STL_UserType:
                lines.append('    Json::Value& _%s_node_temp_ = json["%s"]; for (auto& item : %s) { _%s_node_temp_.append(item.Serialize()); }' % (field.name, field.alias, field.name, field.name))
        lines.append('    return json;')
        lines.append('}\n')
        return lines

    def __CreateImplementationCode_Deserialize(self):
        ''' 生成DTO反序列化代码 '''
        lines: List[str] = []
        lines.append('bool %s%s::Deserialize(const Json::Value& json) {' % (self.namespace, self.name))
        lines.append('    if (!json.isObject()) { return %s; }' % ('true' if self.required_field_count == 0 else 'false'))
        for field in self.fields:
            if field.ignore:
                lines.append('    // ignore "%s"' % field.name)
                continue
            node = '_%s_node_temp_' % field.name
            line = '    const Json::Value& %s = json["%s"];' % (node, field.alias)
            if field.type == DtoFieldType.BuiltInType:
                line += ' if (%s.is<%s>()) { %s = %s.as<%s>(); }' % (node, field.value_type, field.name, node, field.value_type)
            elif field.type == DtoFieldType.UserType:
                line += ' if (%s.isObject()) { if (!%s.Deserialize(json)) { return false; } }' % (node, field.name)
            elif field.type == DtoFieldType.STL_BuiltInType:
                line += ' if (%s.isArray()) { unsigned int size = %s.size();' % (node, node)
                if field.stl_attr['reserve']:
                    line += ' %s.reserve(size);' % (field.name)
                line +=     ' for (unsigned int i = 0; i < size; ++i) {'
                line +=         ' if (%s[i].is<%s>()) {' % (node, field.value_type)
                line +=             ' %s.%s(%s.as<%s>());' % (field.name, field.stl_attr['method_add_element'], node, field.value_type)
                line +=         ' } else { return false; }'
                line +=     ' }'
                line += ' }'
            elif field.type == DtoFieldType.STL_UserType:
                line += ' if (%s.isArray()) { unsigned int size = %s.size();' % (node, node)
                if field.stl_attr['reserve']:
                    line += ' %s.reserve(size);' % (field.name)
                line +=     ' for (unsigned int i = 0; i < size; ++i) {'
                line +=         ' %s item;' % field.value_type
                line +=         ' if (item.Deserialize(%s[i])) {' % node
                line +=             ' %s.%s(item);' % (field.name, field.stl_attr['method_add_element'])
                line +=         ' } else { return false; }'
                line +=     ' }'
                line += ' }'
            # 字段是否必须存在
            if field.required:
                line += ' else { return false; }'
            else:
                line += ' else if (!_%s_node_temp_.isNull()) { return false; }' % field.name
            lines.append(line)
        lines.append('    return true;')
        lines.append('}\n')
        return lines

class DtoParser:
    ''' DTO解析器 '''
    def __init__(self, dto_dir: str):
        self.header_files_modify_time: Dict[str, int] = {} # { filename: modify_time }
        self.dtos_dict: Dict[str, List[Dto]] = {}  # { filename: dtos }
        self.cache_file: str = tempfile.gettempdir() + '/server-assistant_dto-parser-' + hashlib.md5(os.path.abspath(dto_dir).encode()).hexdigest()
        self.__LoadCacheFile()

    def ParseHeaderFile(self, header_file_abspath: str, parser: CppHeader):
        ''' 解析头文件 '''
        modify_time = int(os.path.getmtime(header_file_abspath))
        # 判断文件是否修改过，如果未修改则不再解析
        if header_file_abspath in self.header_files_modify_time and modify_time == self.header_files_modify_time[header_file_abspath]:
            return True
        dto_list: List[Dto] = []
        user_types: Set[str] = set()
        for cpp_class in parser.CLASSES.values():
            dto = Dto(user_types)
            ret = dto.Parse(cpp_class)
            if ret < 0:
                return False
            elif ret > 0:
                continue
            dto_list.append(dto)
        if len(dto_list) > 0:
            self.dtos_dict[header_file_abspath] = dto_list
        self.header_files_modify_time[header_file_abspath] = modify_time
        return True

    def GenerateImplementationFiles(self):
        for header_file_abspath, dto_list in self.dtos_dict.items():
            if not self.__GenerateImplementationFile(header_file_abspath, dto_list):
                return False
        self.__WriteCacheFile()
        return True

    def __GenerateImplementationFile(self, header_file_abspath: str, dto_list: List[Dto]):
        header_filename = os.path.basename(header_file_abspath)
        source_filename = os.path.splitext(header_file_abspath)[0] + '.impl_dto.cpp'
        header_lines = [
            '// Generated by script server-assistant.py',
            '// DO NOT EDIT',
            '#include "%s"' % header_filename
        ]
        body_lines: List[str] = []
        for dto in dto_list:
            body_lines += dto.CreateImplementationCode()
        with open(source_filename, mode='w', encoding='utf-8') as f:
            for line in header_lines:
                f.write(line + '\n')
            for line in body_lines:
                f.write('\n')
                f.write(line)
        return True

    def __LoadCacheFile(self):
        try:
            with open(self.cache_file, 'r') as file:
                data = json.load(file)
                for key, value in data.items():
                    if isinstance(key, str) and isinstance(value, int):
                        self.header_files_modify_time[key] = value
        except:
            self.header_files_modify_time = {}

    def __WriteCacheFile(self):
        try:
            with open(self.cache_file, 'w') as file:
                json.dump(self.header_files_modify_time, file)
        except:
            pass


###############################################################################################
##
##                                    Server Assistant
##
###############################################################################################

class ServerAssistant:
    def __init__(self, controller_dir: str, include_prefix: str, output_file: str, overwrite: bool):
        self.controller_dir = Helper.FormatDir(controller_dir)
        self.include_prefix = Helper.FormatDir(include_prefix)
        self.route_collector = RouteCollector(output_file, overwrite)
        self.dto_parser = DtoParser(self.controller_dir)

    def Run(self):
        # 列出所有的头文件(按文件名排序)
        files = Helper.ListHeaderFiles(self.controller_dir)
        # 遍历所有头文件
        for file in files:
            header_file_abspath = os.path.abspath(self.controller_dir + file)
            parser = CppHeader(header_file_abspath, encoding='utf-8')
            # 路由注册
            header_file_relpath = Helper.FormatPath(self.include_prefix + file)
            if not self.route_collector.ParseHeaderFile(header_file_abspath, header_file_relpath, parser):
                return False
            # DTO序列化和反序列化
            if not self.dto_parser.ParseHeaderFile(header_file_abspath, parser):
                return False
        # 生成代码文件
        return self.route_collector.WriteToOutputFile() and self.dto_parser.GenerateImplementationFiles()



###############################################################################################
##
##                                     Main Function
##
###############################################################################################

def ExecCMD_ParseController(args):
    assistant = ServerAssistant(args.controller_dir, args.prefix, args.output_file, args.yes)
    if assistant.Run():
        print('Success!')
        exit(0)
    else:
        print('Fail!')
        exit(1)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Assistant for http-server')
    subparsers = parser.add_subparsers(dest='command')

    # 命令1：parse-controller
    parser_cmd_parse_controller = subparsers.add_parser('parse-controller', help='Parse controllers and DTOs')
    parser_cmd_parse_controller.add_argument('controller_dir', type=str, help='controller directory')
    parser_cmd_parse_controller.add_argument('output_file', type=str, help='output filepath of routes registration')
    parser_cmd_parse_controller.add_argument('-p', '--prefix', type=str, required=False, default='', help='prefix of include path')
    parser_cmd_parse_controller.add_argument('-y', '--yes',    action='store_true', help='overwrite the existing file')

    # 命令2：
    # ...

    args = parser.parse_args()

    if args.command == 'parse-controller':
        ExecCMD_ParseController(args)
