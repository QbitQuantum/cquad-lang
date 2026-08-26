# cquad-lang
CQuad — это формальный диалект C++ со строго определенными синтаксическими ограничениями. Спецификация устраняет синтаксическую неоднозначность, присущую стандартному C++, и устанавливает единообразные правила объявления типов, инициализации и преобразования типов.

Основные принципы
Все модификаторы типа (const, *, &, &&, []) являются частью типа, а не переменной. Тип применяется ко всем переменным в групповом объявлении. Круглые скобки () используются ТОЛЬКО для вызова функций. Язык предоставляет фиксированный набор из 9 базовых типов без избыточных комбинаций. Приведение типов выполняется только через cast<T>(expression).

Система типов
Базовые типы:

```cpp
int // 32-битное целое знаковое
long // 64-битное целое знаковое
llong // 128-битное целое знаковое
char // 8-битный символ/байт
float // 32-битное число с плавающей точкой
double // 64-битное число с плавающей точкой
bool // логический тип (true/false)
void // пустота
auto // автоматический вывод типа
```
Множественные спецификаторы типа запрещены.
Модификаторы знака: unsigned int, signed int, unsigned long, signed long, unsigned char, signed char.

Модификаторы типа:

```cpp
const T // константный тип
T* // указатель
const T* // указатель на константу
T& // ссылка
const T& // константная ссылка
T&& // rvalue-ссылка
const T&& // константная rvalue-ссылка
T[N] // массив из N элементов
const T[N] — массив из N константных элементов
T[N]* // указатель на массив
T*[N] // массив указателей
T*[N]* // указатель на массив указателей
```

Пользовательские типы: имена классов и структур, пространства имён (namespace::Type), шаблоны (std::vector<int>).

Инициализация
Переменные:
```cpp
T name;           // неинициализированная
T name = value;   // копирующая инициализация
T name = {value}; // инициализация списком
T name {value};   // прямая инициализация списком
```

Массивы:
```cpp
T[N] name;                    // неинициализированный
T[N] name = {v1, v2, ...};    // инициализация списком
T[N] name = {};               // нулевая инициализация
T name[] = {v1, v2, ...};     // вывод размера массива
```

Запрещено:
```cpp
T name(value);     // синтаксис вызова функции
T name = (value);  // лишние скобки
```

Примеры:
```cpp
int a = 10;           // копирующая инициализация
int a = {10};         // инициализация списком
int a {10};           // прямая инициализация списком

int[3] arr = {1, 2, 3};     // полная инициализация
int[3] arr = {1, 2};        // {1, 2, 0}
int[3] arr = {};            // {0, 0, 0}
int[3] arr;                 // не инициализирован

int arr[] = {1, 2, 3};      // размер выведен как 3
```

Функции
Объявление:
```cpp
return_type name(parameters) { body }
return_type name(parameters);  // прототип
```

Параметры:
```cpp
void func(int a, int& b, const int& c, int* d) { /* ... */ }
```



Возврат:
```cpp
int getValue() { return 10; }
int& getRef() { return ref; }
const int& getConstRef() { return ref; }
int* getPtr() { return &value; }
```

Вызов:
```cpp
int result = add(5, 10);    // вызов функции
int result = add(5);        // если есть значение по умолчанию
int result = add();         // если нет параметров
```

Параметры по умолчанию:
```cpp
int add(int a, int b) {
    return a + b;
}

void print(const std::string& msg) {
    std::cout << msg;
}
```

Классы. Объявление:
```cpp
class ClassName {
    // поля и методы
    // Модификаторы доступа:
    
public: // открытый доступ
private: // закрытый доступ (по умолчанию)
protected: // защищённый доступ

}
```

Поля класса инициализируются как обычные переменные, групповые объявления работают, модификаторы типа являются частью типа.

Методы объявляются как функции внутри класса, могут быть константными:
```cpp
void method() const { /* ... */ }
```

Конструкторы:
```cpp
ClassName(int x, int y) {
    this.x = x;
    this.y = y;
}
```

// ИЛИ с инициализацией списком:
```cpp
ClassName(int x, int y) : x(x), y(y) {}
```

Деструктор:

```cpp
~ClassName() { /* ... */ }
```

Наследование:

```cpp
class Child : public Parent {
    // ...
}
```

Пример:

```cpp
class Point {
private:
    int x = 0;
    int y = 0;
    
public:
    Point(int x, int y) {
        this.x = x;
        this.y = y;
    }
    
    int getX() const {
        return x;
    }
    
    void setX(int value) {
        x = value;
    }
}
```
Примеры правильного синтаксиса

Одиночные объявления:
```cpp
int a = 10;
long b;
char c = 'A';
float f = 3.14f;
double d = 3.14;
bool flag = true;
```

С модификаторами:
```cpp
const int a = 10;
int* ptr = nullptr;
const int* ptr2 = &a;
int& ref = a;
const int& ref2 = a;
int[10] arr;
const int[5] arr2;
```

Групповые объявления:
```cpp
int x, y, z;                        // все три - int
unsigned int u1, u2;                // оба - unsigned int
const int* p1, p2;                  // оба - const int*
int[10] arr1, arr2;                 // оба - int[10]
Data* ptr1, ptr2 = nullptr, ptr3;   // все три - Data*
```

Пользовательские типы:
```cpp
std::vector<int> vec;
MyClass obj;
namespace::Type var;
```
Запрещённые конструкции

Недопустимые комбинации:
```cpp
const int* ptr1, *ptr2;     // звёздочка только в типе
int a = 10, const b = 20;   // модификаторы только в типе
Data* ptr1, Data* ptr2;     // тип указывается один раз
```

Недопустимая инициализация:
```cpp
int a(5);          // синтаксис вызова функции
int a = (5);       // лишние скобки
int[3] arr(1,2,3); // синтаксис вызова функции
```

Приведения типов:
```cpp
long b = cast<long>(a); C-style cast
int a = 10;
long b = a;          неявное преобразование
```

Пример полного класса
```cpp
class Example {
private:
    // Поля класса с инициализацией
    int x = 10;
    const int y = 20;
    int* ptr = nullptr;
    std::vector<int> data;
    
    // Групповые объявления
    int a, b, c;
    const int[5] arr1, arr2;
    Data* ptr1, ptr2 = nullptr;
    
public:
    // Конструктор
    Example(int x, int y) {
        this.x = x;
        this.y = y;
    }
    
    // Методы
    int getX() const {
        return x;
    }
    
    void setX(int value) {
        x = value;
    }
    
    // Статический метод
    static void printHello() {
        std::cout << "Hello";
    }
}
```

Ключевые слова
```cpp
class, public, private, protected,
const, unsigned, signed,
int, long, char, float, double, bool,
true, false, nullptr,
if, else, for, while, do, switch, case, default,
return, break, continue, goto,
new, delete,
static, virtual, override, final,
try, catch, throw,
namespace, using,
template, typename,
auto, decltype
```
Отношение к C++: является спецификацией, определяющей подмножество конструкций C++, допустимых к использованию. Код на cpp транспилируется в стандартный C++ и компилируется любым компилятором, поддерживающим C++20 и выше. Язык не добавляет новых возможностей, а только накладывает ограничения на существующий синтаксис C++ для повышения надёжности и предсказуемости кода.
