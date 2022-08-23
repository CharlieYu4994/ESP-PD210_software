# Copyright (c) 2022 CharlieYu4994
# 
# This software is released under the MIT License.
# https://opensource.org/licenses/MIT

idf_component_register(SRCS "i2c_manager/i2c_manager.c" "ssd1316.c"
                    INCLUDE_DIRS "./includes")