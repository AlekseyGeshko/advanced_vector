# advanced_vector

Аналог стандартного контейнера vector. В основе динамический массив.

# Описание
+ RawMemory - класс обертка над массивом. Выделяет и освобождает сырую память.  
+ explicit Vector(size_t size) - конструктор, создает вектор размера size.  
+ Vector(const Vector& other) - конструктор копирования. Создает аналог Vector& other.  
+ Vector(Vector&& other) noexcept - перемещающий конструктор копирования. Создает новый массив, перемещая элементы из other. noexcept говорит нам о том, что исключения не возникнут.  
+ Vector& operator=(const Vector& rhs) - копирующий оператор присваивания. Инициализирует вектор элементами rhs.
+ Vector& operator=(Vector&& rhs) noexcept - перемещающий оператор присваивания.  
+ iterator begin() noexcept - возвращает итератор на начало вектора O(1).  
+ iterator end() noexcept - возвращает итератор на следующий за последним элементом вектора O(1).  
+ const_iterator cbegin() const noexcept - возвращает константный итератор на начало вектора O(1).  
+ const_iterator cend() const noexcept - возвращает константный итератор на следующий за последним элементом вектора O(1).  
+ size_t Size() const noexcept - возвращает текущий размер вектора O(1).  
+ size_t Capacity() const noexcept - возвращает зарезервированный размер вектора O(1).  
+ const T& operator[](size_t index) const noexcept - возвращает элемент по индексу index O(1).
+ void Reserve(size_t new_capacity) - резервирует память размера new_capacity. Если текущий размер больше, то ничего не делает. Иначе создает новый массив размера new_capacity и переносит туда все элементы O(n).  
+ void Resize(size_t new_size) - создает массив размера new_size. Если текущий размер больше, то удаляет все элементы за new_size. Если размер меньше, то добавляет элементы по умолчанию в интервал (size + 1, new_size).  
+ iterator Emplace(const_iterator pos, Args&&... args) - принимает на вход параметры конструктора и создает новый элемент на позиции pos. 
+ iterator Insert(const_iterator pos, Type&& value) - вставляет элемент value на позицию pos.
+ void PushBack(Type&& value) - вставляет элемент value в конец вектора. Если capacity == size, то происходит реаллокация. Амортизированная константа.  
+ T& EmplaceBack(Args&&... args) - принимает параметры конструктора, создает элемент и вставляет его в конец вектора. Амортизированная константа.  
+ void PopBack() noexcept - удаляет последний элемент.  
+ iterator Erase(const_iterator pos) noexcept(std::is_nothrow_move_assignable_v<T>) - удаляет элемент на заданной позиции. O(n).  

# Системные требования
Компилятор С++ с поддержкой стандарта C++17
