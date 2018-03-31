#ifndef Z3DDEBUGSYSTEMCONFIG_H
#define Z3DDEBUGSYSTEMCONFIG_H

/* This file is a part of Zavod3D engine project.
It's licensed unser the MIT license (see "License.txt" for details).*/




// Установка уровня отладки (раскомментировать только один из макросов).
//#define Z3D_DEBUG_LEVEL 0 // Отладка не производится.
//#define Z3D_DEBUG_LEVEL 1 // Затраты процессорного времени на отладку несущественны (допустимо оставить в релизе).
//#define Z3D_DEBUG_LEVEL 2 // Обычный режим отладки при разработке(для хорошо оттестированного кода).
//#define Z3D_DEBUG_LEVEL 3 // Обычный режим отладки при разработке (для ловли серьезных багов).
//#define Z3D_DEBUG_LEVEL 4 // Для крайне серьезных и опасных ситуаций.
//#define Z3D_DEBUG_LEVEL 5 // Параноидальный режим ловли багов.

// Установка уровня предупреждений (раскомментировать только один из макросов).
//#define Z3D_WARN_LEVEL 0 // предупреждения не выводятся.
//#define Z3D_WARN_LEVEL 1 // затраты процессорного времени на предупреждения несущественны.
//#define Z3D_WARN_LEVEL 2 // обычный режим вывода предупреждений при разработке (для потенциально опасных мест).

// Задание имени файла лога
// #define Z3D_LOGFILENAME "type_your_log_file_name.txt"

#endif // Z3DDEBUGSYSTEMCONFIG_H

