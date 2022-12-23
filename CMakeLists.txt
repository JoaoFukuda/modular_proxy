cmake_minimum_required(VERSION 3.20)
project(mproxy C)

add_executable(mproxy src/mproxy.c src/r_monitor.c src/r_module.c src/r_proxy.c src/m_module.c)
