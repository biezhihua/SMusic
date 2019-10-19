# 设置库名称
project(splayer_andorid_impl)

# 设置cmake最低版本
cmake_minimum_required(VERSION 3.4.1)

# 设置播放器引擎目录
set(ANDROID_SPLAYER_ENGINE_DIR ${ROOT_DIR}/splayer_engine)

# 添加引擎子模块目录
# add_subdirectory(${ROOT_DIR}/splayer_engine alias_splayer_engine)