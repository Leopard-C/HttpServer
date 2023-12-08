#!/usr/bin/python3

###############################################################################################
##
##  收集控制器中定义的方法，生成注册路由的代码
##
##  使用示例: (-h 查看帮助)
##    ./route_collector.py /path/to/dir/controller /path/to/file/output -p controller
##
##  安装第三方库：(CppHeaderParser)
##    pip3 install robotpy-cppheaderparser
##
##  详细说明请参考文档 route_collector/README.md
##
###############################################################################################

import os
import re
import argparse
from CppHeaderParser import CppHeader
from typing import List
from functools import cmp_to_key

class_method_attr_blacklist = [
    'const', 'final', 'override', 'deleted', 'pure_virtual', 'operator', 'constructor',
    'destructor', 'extern', 'template', 'virtual', 'friend', 'default'
]

valid_http_methods = [
    'GET', 'HEAD', 'POST', 'PUT', 'DELETE', 'CONNECT, OPTIONS', 'TRACE', 'PATCH'
]

def FormatPath(dir):
    dir = dir.strip()
    dir = dir.replace('\\', '/')
    return dir

def FormatDir(dir):
    dir = FormatPath(dir)
    if len(dir) > 0 and dir[-1] != '/':
        dir += '/'
    return dir

##
## 路由
##
class Route:
    def __init__(self):
        self.is_regex = False
        self.include_header_file = ''
        self.description = ''
        self.routes: List[str] = []
        self.function = ''
        self.http_methods: List[str] = []
        self.config = {}

    def __ConfigToCppMap(self):
        str = ''
        for key, value in self.config.items():
            if len(str) > 0:
                str += ', '
            str += '{"%s", "%s"}' % (key.replace('"', '\\"'), value.replace('"', '\\"'))
        return '{' + str + '}'

    def __ToCppHttpMethods(self):
        cpp_http_methods = ''
        if len(self.http_methods) == 0:
            self.http_methods.append('GET')
        for method in self.http_methods:
            if len(cpp_http_methods) > 0:
                cpp_http_methods += ' | '
            cpp_http_methods += 'HttpMethod::k%s' % method
        return cpp_http_methods

    def __GetHttpMethodsStr(self):
        return ','.join(self.http_methods).rjust(4)

    def ToCppParamerterList(self, route_path: str):
        return '"%s", %s, %s, "%s", %s' % (route_path, self.__ToCppHttpMethods(), self.function, self.description, self.__ConfigToCppMap())

    def Print(self):
        if self.is_regex:
            for r in self.routes:
                print('@regex  %s %s' % (self.__GetHttpMethodsStr(), r))
        else:
            for r in self.routes:
                print('@static %s %s' % (self.__GetHttpMethodsStr(), r))

    def PrintDebug(self):
        if self.is_regex:
            for r in self.routes:
                print('route: %s @regex' % r)
        else:
            for r in self.routes:
                print('route: %s' % r)
        print('    method: %s' % self.__GetHttpMethodsStr())
        print('   include: %s' % self.include_header_file)
        print('  function: %s' % self.function)

# Route对象排序函数
def CompareRoute(r1: Route, r2: Route):
    if r1.include_header_file < r2.include_header_file:
        return -1
    elif r1.include_header_file > r2.include_header_file:
        return 1
    else:
        if r1.routes[0] < r2.routes[0]:
            return -1
        elif r1.routes[0] > r2.routes[0]:
            return 1
        else:
            return 0

##
## 路由收集器
##
class RouteCollector:
    def __init__(self, controller_dir: str, include_prefix: str, output_file: str, overwrite: bool):
        self.controller_dir = FormatDir(controller_dir)
        self.include_prefix = FormatDir(include_prefix)
        self.output_file = output_file
        self.overwirte = overwrite

    def Run(self):
        files: List[str] = []
        routes: List[Route] = []
        # 列出所有的头文件，并按文件名称排序
        self.__ListHeaderFiles(self.controller_dir, len(self.controller_dir), files)
        files.sort()
        # 列出所有路由
        for file in files:
            include_header_file = FormatPath(self.include_prefix + file)
            parser = CppHeader(os.path.abspath(self.controller_dir + file), encoding='utf-8')
            if not self.__ParseRoute(include_header_file, parser, routes):
                return False
        # 按指定规则排序
        routes.sort(key = cmp_to_key(CompareRoute))

        count_static_routes = 0
        count_regex_routes = 0
        print('------------------- begin route list --------------------')
        for route in routes:
            #route.PrintDebug()
            route.Print()
            if route.is_regex:
                count_regex_routes += len(route.routes)
            else:
                count_static_routes += len(route.routes)
        print('-------------------- end route list --------------------')
        print('count(static routes):', count_static_routes)
        print('count(regex  routes):', count_regex_routes)
        print('total:', count_static_routes + count_regex_routes)
        if not self.__WriteToOutputFile(routes):
            return False
        return True

    ##
    ## 列出文件夹下的所有头文件(.h .hpp)
    ##
    def __ListHeaderFiles(self, dir: str, root_len: int, list: List[str]):
        for file in os.listdir(dir):
            path = os.path.join(dir, file)
            if os.path.isdir(path):
                self.__ListHeaderFiles(path, root_len, list)
            else:
                if path.endswith('.h') or path.endswith('.hpp'):
                    list.append(path[root_len:])

    def __ParseRoute(self, include_header_file: str, parser: CppHeader, routes: List[Route]):
        # 1. 函数
        for function in parser.functions:
            #print(function)
            route = Route()
            route.include_header_file = include_header_file
            ret = self.__ParseRoute_Function(function, route)
            if ret == 0:
                routes.append(route)
            elif ret > 0:
                continue
            else:
                return False
        # 2. 类成员函数
        for class_name in parser.classes:
            methods = parser.classes[class_name]['methods']
            ## 2.1 公有方法
            ret = -1
            for public_method in methods['public']:
                route = Route()
                route.include_header_file = include_header_file
                ret = self.__ParseRoute_ClassMethod(public_method, class_name, True, route)
                if ret == 0:
                    routes.append(route)
                elif ret < 0:
                    return False
            ## 2.2 保护方法
            for protected_method in methods['protected']:
                route = Route()
                route.include_header_file = include_header_file
                ret = self.__ParseRoute_ClassMethod(protected_method, class_name, False, route)
                if ret < 0:
                    return False
            ## 2.3 私有方法
            for private_method in methods['private']:
                route = Route()
                route.include_header_file = include_header_file
                ret = self.__ParseRoute_ClassMethod(private_method, class_name, False, route)
                if ret < 0:
                    return False
        return True

    def __ParseRoute_Function(self, function: object, route: Route):
        name = function['name']
        namespace = function['namespace']
        line_num = function['line_number']
        route.function = namespace + name
        try:
            ret = self.__ParseDoxygen(function['doxygen'], route)
        except:
            ret = 1
        if ret < 0:
            print('-------------------------------------')
            print('Invalid route:')
            print('     header: %s' % route.include_header_file)
            print('  namespace: %s' % namespace)
            print('   function: %s' % name)
            print('   line num: %d' % line_num)
        return ret

    def __ParseRoute_ClassMethod(self, method: object, class_name: str, is_public: bool, route: Route):
        name = method['name']
        namespace = method['namespace']
        line_num = method['line_number']
        route.function = namespace + class_name + '::' + name
        try:
            ret = self.__ParseDoxygen(method['doxygen'], route)
        except:
            ret = 1
        def PrintError(msg = ''):
            print('-------------------------------------')
            print('Invalid route: %s' % msg)
            print('     header: %s' % route.include_header_file)
            print('  namespace: %s' % namespace)
            print('      class: %s' % class_name)
            print('     method: %s' % name)
            print('   line num: %d' % line_num)
        if ret > 0:
            return 1
        elif ret < 0:
            PrintError()
            return -1
        else:
            if not is_public:
                PrintError('<NOT public>')
                return -1
            if not method['static']:
                PrintError('<NOT static>')
                return -1
            if method['returns'] != 'void':
                PrintError('<NOT return void>')
                return -1
            for attr in class_method_attr_blacklist:
                if method[attr]:
                    PrintError('<%s>' % attr)
                    return -1
        return 0

    ##
    ## 解析Doxygen格式注释
    ## @retval  0 正常的路由
    ## #retval  1 非路由，忽略
    ## @retval -1 错误
    ##
    def __ParseDoxygen(self, doxygen, route: Route):
        if doxygen.find('@route ') < 0:
            return 1
        tokens = doxygen.split('\n')
        for token in tokens:
            # 1. brief
            ret = self.__ParseDoxygenSection_Brief(token, route)
            if ret == 0:
                continue
            elif ret == -1:
                return -1
            # 2. route
            ret = self.__ParseDoxygenSection_Route(token, route)
            if ret == 0:
                continue
            elif ret == -1:
                return -1
            # 3. method
            ret = self.__ParseDoxygenSection_Method(token, route)
            if ret == 0:
                continue
            elif ret == -1:
                return -1
            # 4. config
            ret = self.__ParseDoxygenSection_Config(token, route)
            if ret == 0:
                continue
            elif ret == -1:
                return -1

        route.routes.sort()

        def PrintError(msg):
            print('===========================================')
            print(msg)

        if len(route.http_methods) == 0:
            PrintError('Missing @method')
            return -1
        if len(route.routes) == 0:
            PrintError('Missing @route')
            return -1
        if route.is_regex:
            for r in route.routes:
                try:
                    re.compile(r)
                except re.error:
                    PrintError('Invalid regex route: "%s"' % r)
                    return -1
        return 0

    # 解析 @brief
    def __ParseDoxygenSection_Brief(self, token: str, route: Route):
        pos = token.find('@brief ')
        if pos < 0:
            return 1
        if len(route.description) == 0:
            route.description = token[pos+7:].strip()
        return 0

    # 解析 @route
    def __ParseDoxygenSection_Route(self, token: str, route: Route):
        pos = token.find('@route ')
        if pos < 0:
            return 1
        str = token[pos+7:].strip().replace('\\', '\\\\')
        route_tmp = ''
        is_regex_tmp = False
        if str[0] == '/':  # 静态路由
            route_tmp = str
        elif str[0] == '~':  # 正则路由
            route_tmp = str[1:].strip()
            is_regex_tmp = True
            if route_tmp[0] != '/':
                print('Invalid @route: [%s]' % str)
                return -1
        else:
            print('Invalid @route: [%s]' % str)
            return -1
        if len(route_tmp) == 0:
            print('@route is empty')
            return -1
        if len(route.routes) == 0:
            route.routes.append(route_tmp)
            route.is_regex = is_regex_tmp
        else:
            # 多个route必须全是static或全是regex，不能混合
            if route.is_regex != is_regex_tmp:
                print('Invalid @route: [%s]' % str)
                return -1
            route.routes.append(route_tmp)
        return 0

    # 解析 @method
    def __ParseDoxygenSection_Method(self, token: str, route: Route):
        pos = token.find('@method ')
        if pos < 0:
            return 1
        token = token[pos+8:].strip()
        methods = re.split(r'[,\s]+', token.upper())
        for method in methods:
            if len(method) == 0 or method in route.http_methods:
                continue
            if method not in valid_http_methods:
                print('Invalid @method: [%s]' % token)
                return -1
            route.http_methods.append(method)
        return 0

    # 解析 @config
    def __ParseDoxygenSection_Config(self, token: str, route: Route):
        pos = token.find('@config ')
        if pos < 0:
            return 1
        config = token[pos+8:].strip()
        left_pos = config.find('(')
        if left_pos < 0:
            route.config[config] = ''
            return 0
        right_pos = config.find(')', left_pos)
        if right_pos < 0:
            print('Invalid @config: [%s]' % config)
            return -1
        name = config[0:left_pos].strip()
        value = config[left_pos+1:right_pos].strip()
        route.config[name] = value
        return 0

    def __WriteToOutputFile(self, routes: List[Route]):
        header_lines = [
            '// Generated by script route_collector.py',
            '#include <server/router.h>'
        ]
        headers_set = set()
        for route in routes:
            if route.include_header_file not in headers_set:
                header_lines.append('#include "%s"' % route.include_header_file)
                headers_set.add(route.include_header_file)

        function_lines = [
            'bool register_routes(std::shared_ptr<ic::server::Router> router) {',
            '    using namespace ic::server;',
            '    bool ret = true;'
        ]
        last_include_header_file = ''
        for route in routes:
            if route.include_header_file != last_include_header_file:
                function_lines.append('')
                function_lines.append('    // %s'  % (route.include_header_file))
                last_include_header_file = route.include_header_file
            if route.is_regex:
                for r in route.routes:
                    function_lines.append('    ret &= router->AddRegexRoute(%s);' % (route.ToCppParamerterList(r)))
            else:
                for r in route.routes:
                    function_lines.append('    ret &= router->AddStaticRoute(%s);' % (route.ToCppParamerterList(r)))
        function_lines.append('')
        function_lines.append('    return ret;')
        function_lines.append('}')

        if os.path.exists(self.output_file) and not self.overwirte:
            operation = input('Output file is exist already. Overwrite? (y/N) ')
            if operation != 'y' and operation != 'Y':
                print('Canceled')
                return False
        with open(self.output_file, mode='w', encoding='utf-8') as f:
            for line in header_lines:
                f.write(line + '\n')
            f.write('\n')
            for line in function_lines:
                f.write(line + '\n')
        return True

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Collect all routes in controllers')
    parser.add_argument('controller_dir', type=str, help='controller directory')
    parser.add_argument('output_file',    type=str, help='output filepath')
    parser.add_argument('-p', '--prefix', type=str, required=False, default='', help='prefix of include path')
    parser.add_argument('-y', '--yes',    action='store_true', help='overwrite the existing file')
    args = parser.parse_args()
    rc = RouteCollector(args.controller_dir, args.prefix, args.output_file, args.yes)
    if rc.Run():
        print('Success!')
        exit(0)
    else:
        print('Fail!')
        exit(1)
