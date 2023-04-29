#pragma once
#include <cassert>
#include <cstdlib>
#include <new>
#include <memory>
#include <utility>
#include <algorithm>

//----------------------------------------RawMemory----------------------------------------------
template <typename T>
class RawMemory {
private:
    // Выделяет сырую память под n элементов и возвращает указатель на неё
    static T* Allocate(size_t n);

    // Освобождает сырую память, выделенную ранее по адресу buf при помощи Allocate
    static void Deallocate(T* buf) noexcept;
public:
    RawMemory() = default;

    explicit RawMemory(size_t capacity)
            : buffer_(Allocate(capacity))
            , capacity_(capacity) {}

    RawMemory(const RawMemory&) = delete;
    RawMemory& operator=(const RawMemory& rhs) = delete;
    RawMemory(RawMemory&& other) noexcept;
    RawMemory& operator=(RawMemory&& rhs) noexcept;


    T* operator+(size_t offset) noexcept;
    const T* operator+(size_t offset) const noexcept;
    const T& operator[](size_t index) const noexcept;
    T& operator[](size_t index) noexcept;

    void Swap(RawMemory& other) noexcept;
    const T* GetAddress() const noexcept;
    T* GetAddress() noexcept;
    size_t Capacity() const;

    ~RawMemory();
private:
    T* buffer_ = nullptr;
    size_t capacity_ = 0;
};

template <typename T>
RawMemory<T>::RawMemory(RawMemory&& other) noexcept {
    Swap(other);
}

template <typename T>
RawMemory<T>& RawMemory<T>::operator=(RawMemory&& rhs) noexcept {
    Swap(rhs);
    return *this;
}

template <typename T>
RawMemory<T>::~RawMemory() {
    Deallocate(buffer_);
}

template <typename T>
T* RawMemory<T>::operator+(size_t offset) noexcept {
    assert(offset <= capacity_);
    return buffer_ + offset;
}

template <typename T>
const T* RawMemory<T>::operator+(size_t offset) const noexcept {
    return const_cast<RawMemory&>(*this) + offset;
}

template <typename T>
const T& RawMemory<T>::operator[](size_t index) const noexcept {
    return const_cast<RawMemory&>(*this)[index];
}

template <typename T>
T& RawMemory<T>::operator[](size_t index) noexcept {
    assert(index < capacity_);
    return buffer_[index];
}

template <typename T>
void RawMemory<T>::Swap(RawMemory& other) noexcept {
    std::swap(buffer_, other.buffer_);
    std::swap(capacity_, other.capacity_);
}

template <typename T>
const T* RawMemory<T>::GetAddress() const noexcept {
    return buffer_;
}

template <typename T>
T* RawMemory<T>::GetAddress() noexcept {
    return buffer_;
}

template <typename T>
size_t RawMemory<T>::Capacity() const {
    return capacity_;
}

template <typename T>
T* RawMemory<T>::Allocate(size_t n) {
    return n != 0 ? static_cast<T*>(operator new(n * sizeof(T))) : nullptr;
}

template <typename T>
void RawMemory<T>::Deallocate(T* buf) noexcept {
    operator delete(buf);
}

//-----------------------------------------Vector------------------------------------------------

template <typename T>
class Vector {
private:
    // Создаёт копию объекта elem в сырой памяти по адресу buf
    static void CopyConstruct(T* buf, const T& elem);
    void CopyAndSwapForArray(T *from, size_t size, T *to);

    template <typename... Args>
    void ReallocateForEmplace(size_t index, Args&&... args);

public:
    using iterator = T*;
    using const_iterator = const T*;

    Vector() = default;
    explicit Vector(size_t size);
    Vector(const Vector& other);
    Vector(Vector&& other) noexcept;

    Vector& operator=(const Vector& rhs);
    Vector& operator=(Vector&& rhs) noexcept;

    void Swap(Vector& other) noexcept;

    iterator begin() noexcept;
    iterator end() noexcept;
    const_iterator begin() const noexcept;
    const_iterator end() const noexcept;
    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;

    size_t Size() const noexcept;
    size_t Capacity() const noexcept;

    const T& operator[](size_t index) const noexcept;
    T& operator[](size_t index) noexcept;

    void Reserve(size_t new_capacity);
    void Resize(size_t new_size);

    template <typename... Args>
    iterator Emplace(const_iterator pos, Args&&... args);

    template <typename Type>
    iterator Insert(const_iterator pos, Type&& value);

    template <typename Type>
    void PushBack(Type&& value);

    template <typename... Args>
    T& EmplaceBack(Args&&... args);

    void PopBack() noexcept;
    iterator Erase(const_iterator pos) noexcept(std::is_nothrow_move_assignable_v<T>);

    ~Vector();

private:
    RawMemory<T> data_;
    size_t size_ = 0;
};

template <typename T>
void Vector<T>::CopyConstruct(T* buf, const T& elem) {
    new (buf) T(elem);
}

template <typename T>
void Vector<T>::CopyAndSwapForArray(T *from, size_t size, T *to){
    if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
        std::uninitialized_move_n(from, size, to);
    } else {
        std::uninitialized_copy_n(from, size, to);
    }
    std::destroy_n(from, size);
}

template <typename T>
template <typename... Args>
void Vector<T>::ReallocateForEmplace(size_t index, Args&&... args){
    RawMemory<T> new_data{size_ == 0 ? 1 : size_ * 2};
    new (new_data + index) T(std::forward<Args>(args)...);
    try {
        CopyAndSwapForArray(data_.GetAddress(), index, new_data.GetAddress());
    }  catch (...) {
        new_data[index].~T();
        throw;
    }

    try {
        CopyAndSwapForArray(data_+index, size_ - index, new_data + (index+1));
    }  catch (...) {
        std::destroy_n(new_data.GetAddress(), index + 1);
        throw;
    }
    data_.Swap(new_data);
}

template <typename T>
Vector<T>::Vector(size_t size): data_(size), size_(size) {
    std::uninitialized_value_construct_n(data_.GetAddress(), size);
}

template <typename T>
Vector<T>::Vector(const Vector& other)
        : data_(other.size_){
    std::uninitialized_copy_n(other.data_.GetAddress(), other.size_, data_.GetAddress());
    size_ = other.size_;
}

template <typename T>
Vector<T>::Vector(Vector&& other) noexcept{
    Swap(other);
}

template <typename T>
Vector<T>& Vector<T>::operator=(const Vector& rhs) {
    if (this != &rhs) {
        if (rhs.size_ > data_.Capacity()) {
            Vector<T> rhs_copy(rhs);
            Swap(rhs_copy);
        } else {
            for (size_t i = 0; i < size_ && i < rhs.size_; ++i) {
                data_[i] = rhs[i];
            }
            if(size_ < rhs.size_){
                std::uninitialized_copy_n(rhs.data_.GetAddress() + size_,
                                          rhs.size_ - size_, data_.GetAddress() + size_);
            } else if(size_ > rhs.size_){
                std::destroy_n(data_.GetAddress() + rhs.size_, size_ - rhs.size_);
            }
            size_ = rhs.size_;
        }
    }
    return *this;
}

template <typename T>
Vector<T>& Vector<T>::operator=(Vector&& rhs) noexcept{
    Swap(rhs);
    return *this;
}

template <typename T>
void Vector<T>::Swap(Vector<T>& other) noexcept{
    data_.Swap(other.data_);
    std::swap(size_, other.size_);
}

template <typename T>
typename Vector<T>::iterator Vector<T>::begin() noexcept{
    return data_.GetAddress();
}

template <typename T>
typename Vector<T>::iterator Vector<T>::end() noexcept{
    return data_.GetAddress() + size_;
}

template <typename T>
typename Vector<T>::const_iterator Vector<T>::begin() const noexcept{
    return data_.GetAddress();
}

template <typename T>
typename Vector<T>::const_iterator Vector<T>::end() const noexcept{
    return data_.GetAddress() + size_;
}

template <typename T>
typename Vector<T>::const_iterator Vector<T>::cbegin() const noexcept{
    return data_.GetAddress();
}

template <typename T>
typename Vector<T>::const_iterator Vector<T>::cend() const noexcept{
    return data_.GetAddress() + size_;
}

template <typename T>
size_t Vector<T>::Size() const noexcept {
    return size_;
}

template <typename T>
size_t Vector<T>::Capacity() const noexcept {
    return data_.Capacity();
}

template <typename T>
const T& Vector<T>::operator[](size_t index) const noexcept {
    return const_cast<Vector&>(*this)[index];
}

template <typename T>
T& Vector<T>::operator[](size_t index) noexcept {
    return data_[index];
}

template <typename T>
void Vector<T>::Reserve(size_t new_capacity) {
    if (new_capacity <= data_.Capacity()) {
        return;
    }
    RawMemory<T> new_data(new_capacity);
    CopyAndSwapForArray(data_.GetAddress(), size_, new_data.GetAddress());
    data_.Swap(new_data);
}

template <typename T>
void Vector<T>::Resize(size_t new_size){
    Reserve(new_size);
    if(new_size < size_){
        std::destroy_n(data_.GetAddress() + new_size, size_ - new_size);
    }else if(new_size > size_){
        std::uninitialized_value_construct_n(data_.GetAddress() + size_, new_size - size_);
    }
    size_ = new_size;
}

template <typename T>
template <typename... Args>
typename Vector<T>::iterator Vector<T>::Emplace(const_iterator pos, Args&&... args){
    if (pos == end()) {
        return &EmplaceBack(std::forward<Args>(args)...);
    }
    size_t index = pos - begin();
    if(size_ == Capacity()){
        ReallocateForEmplace(index, std::forward<Args>(args)...);
    } else{
        T temp_elem(std::forward<Args>(args)...);
        new (data_ + size_) T(std::move(data_[size_ - 1]));
        std::move_backward(begin() + index, end() - 1, end());
        data_[index] = std::move(temp_elem);
    }
    ++size_;
    return begin() + index;
}

template <typename T>
template <typename Type>
typename Vector<T>::iterator Vector<T>::Insert(const_iterator pos, Type&& value){
    return Emplace(pos, std::forward<Type>(value));
}

template <typename T>
template <typename Type>
void Vector<T>::PushBack(Type&& value){
    EmplaceBack(std::forward<Type>(value));
}

template <typename T>
template <typename... Args>
T& Vector<T>::EmplaceBack(Args&&... args){
    if (size_ == Capacity()) {
        RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);
        new(new_data + size_) T(std::forward<Args>(args)...);
        CopyAndSwapForArray(data_.GetAddress(), size_, new_data.GetAddress());
        data_.Swap(new_data);
    }else {
        new(data_ + size_) T(std::forward<Args>(args)...);
    }
    ++size_;
    return this->data_[size_ - 1];
}

template <typename T>
void Vector<T>::PopBack() noexcept{
    std::destroy_at(data_.GetAddress() + size_ - 1);
    --size_;
}

template <typename T>
typename Vector<T>::iterator Vector<T>::Erase(const_iterator pos) noexcept(std::is_nothrow_move_assignable_v<T>){
    size_t index = pos - begin();
    //https://en.cppreference.com/w/cpp/algorithm/move
    std::move(begin() + index + 1, end(), begin() + index);
    data_[size_ - 1].~T();
    --size_;
    return begin() + index;
}

template <typename T>
Vector<T>::~Vector() {
    std::destroy_n(data_.GetAddress(), size_);
}