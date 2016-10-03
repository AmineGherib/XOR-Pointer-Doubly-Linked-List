#define CXX_XOR_PROJECT_USE_REINTERPRET_CAST
#ifndef DLLIST_HXX
#define DLLIST_HXX

#include <algorithm>
#include <iterator>
#include <limits>
#include <initializer_list>

#include "xorptr.hxx"

template <typename T>
class dllist_node;      // doubly-linked list node type

template <typename T>
class dllist;           // doubly-linked list container type

template <typename T>
class dllist_iter;      // doubly-linked list iterator type

template <typename T>
class dllist_citer;     // doubly-linked list const_iterator type

template <typename T>
class dllist_node_ptr_only
{
    public:
        using xorptr_type = xorptr<dllist_node_ptr_only>;

    private:
        xorptr_type xorptr_;

    public:
        dllist_node_ptr_only() = delete;
        dllist_node_ptr_only(dllist_node_ptr_only const&) = delete;
        dllist_node_ptr_only& operator =(dllist_node_ptr_only const&) = delete;

        ~dllist_node_ptr_only() = default;
        dllist_node_ptr_only(dllist_node_ptr_only&&) = default;
        dllist_node_ptr_only& operator =(dllist_node_ptr_only&&) = default;

        explicit dllist_node_ptr_only(
                dllist_node_ptr_only* prev,
                dllist_node_ptr_only* next
                ):
            xorptr_{prev,next}
        {}

        explicit dllist_node_ptr_only(xorptr<dllist_node_ptr_only> const& xp):
            xorptr_{xp}
        {}
        explicit dllist_node_ptr_only(xorptr<dllist_node_ptr_only>&& xp):
            xorptr_{std::move(xp)}
        {}

        void swap(dllist_node_ptr_only& b)
        {
            std::swap(xorptr_, b.xorptr_);
        }

        dllist_node<T>& to_node() &
        {
            return static_cast<dllist_node<T>&>(*this);
        }

        dllist_node<T> const& to_node() const&
        {
            return static_cast<dllist_node<T> const&>(*this);
        }

        dllist_node<T>&& to_node() &&
        {
            return static_cast<dllist_node<T>&&>(*this);
        }

        dllist_node_ptr_only* nextptr(dllist_node_ptr_only* prev)
        {
            return xorptr_ ^ prev;
        }

        dllist_node_ptr_only const* nextptr(dllist_node_ptr_only const* prev) const
        {
            return xorptr_ ^ prev;
        }

        void updateptr(
                dllist_node_ptr_only* oldptr,
                dllist_node_ptr_only* newptr
                )
        {
            xorptr_ = xorptr_type{ xorptr_ ^ oldptr, newptr };
        }

        void setptr(dllist_node_ptr_only* ptr1, dllist_node_ptr_only* ptr2)
        {
            xorptr_ = xorptr_type{ ptr1, ptr2 };
        }

        static dllist_node_ptr_only* insert(
                dllist_node_ptr_only* prev,
                dllist_node_ptr_only* before,
                dllist_node_ptr_only* new_node
                )
        {
            auto oldnext = before->xorptr_ ^ prev;

            oldnext->updateptr(before, new_node);

            before->updateptr(oldnext, new_node);

            new_node->xorptr_ = xorptr_type{ before, oldnext };

            return oldnext;
        }

        static dllist_node_ptr_only* remove(
                dllist_node_ptr_only* prev,
                dllist_node_ptr_only* before
                )
        {
            auto oldnext = before->xorptr_ ^ prev;

            auto newnext = oldnext->nextptr(before);

            newnext->updateptr(oldnext, before);

            before->updateptr(oldnext, newnext);

            oldnext->xorptr_ = xorptr_type{};

            return oldnext;
        }
};

template <typename T>
class dllist_node final : public dllist_node_ptr_only<T>
{
    T datum_;
    public:
    using xorptr_type = typename dllist_node_ptr_only<T>::xorptr_type;

    ~dllist_node() = default;

    explicit dllist_node(T const& datum):
        dllist_node_ptr_only<T>(nullptr,nullptr),
        datum_{datum}
    {}

    explicit dllist_node(T&& datum):
        dllist_node_ptr_only<T>(nullptr,nullptr),
        datum_{std::move(datum)}
    {}

    dllist_node(T const& datum, dllist_node* prev, dllist_node* next):
        dllist_node_ptr_only<T>(prev,next),
        datum_{datum}
    {}

    dllist_node(T&& datum, dllist_node* prev, dllist_node* next):
        dllist_node_ptr_only<T>(prev,next),
        datum_{std::move(datum)}
    {}

    dllist_node(T const& datum, xorptr_type const& xp):
        dllist_node_ptr_only<T>(xp),
        datum_{datum}
    {}

    dllist_node(T&& datum, xorptr_type const& xp):
        dllist_node_ptr_only<T>(xp),
        datum_{std::move(datum)}
    {}

    dllist_node(T&& datum, xorptr_type&& xp):
        dllist_node_ptr_only<T>{std::move(xp)},
        datum_{std::move(datum)}
    {}

    dllist_node(dllist_node&& n):
        dllist_node_ptr_only<T>(std::move(n)),
        datum_{std::move(n.datum_)}
    {}

    dllist_node& operator =(dllist_node&& n)
    {
        dllist_node_ptr_only<T>::operator =( std::move(n) );
        datum_ = std::move(n.datum_);
        return *this;
    }

    void swap(dllist_node& b)
    {
        dllist_node_ptr_only<T>::swap(static_cast<dllist_node_ptr_only<T>>(b));
        std::swap(datum_, b.datum_);
        swap(datum_, b.datum_);
    }

    T& datum()
    {
        return datum_;
    }

    T const& datum() const
    {
        return datum_;
    }
};

//===========================================================================

template <typename T>
inline void swap(dllist_node_ptr_only<T>& a, dllist_node_ptr_only<T>& b)
{
    a.swap(b);
}

    template <typename T>
inline void swap(dllist_node<T>& a, dllist_node<T>& b)
{
    a.swap(b);
}

//===========================================================================

template <typename T>
class dllist
{
    private:
        std::size_t size_;
        dllist_node_ptr_only<T> front_;
        dllist_node_ptr_only<T> back_;

    public:
        using value_type = T;

        using reference = value_type&;
        using const_reference = value_type const&;

        using pointer = value_type*;
        using const_pointer = value_type const*;

        using iterator = dllist_iter<T>;
        using const_iterator = dllist_citer<T>;

        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        dllist():
            size_{},
            front_{&back_, &back_},
            back_{&front_, &front_}
        {}

        dllist(size_type n, T const& value = T{}):
            dllist<T>{}
        {
            std::fill_n(std::back_inserter(*this), n, value);
        }

dllist(std::initializer_list<T> il):
    dllist<T>{}
{
    std::copy(il.begin(), il.end(), std::back_inserter(*this));
}

template <typename InIter>
dllist(InIter const& first, InIter const& last):
    dllist<T>{}
{
    std::copy(first, last, std::back_inserter(*this));
}

dllist(dllist const& l):
    dllist<T>{}
{
    std::copy(l.begin(), l.end(), std::back_inserter(*this));
}

dllist(dllist&& l) :
    dllist<T>{}
{
    swap(l);
}

~dllist() {
    while(!empty()) {
        try {
            clear();
        } catch(...) {}
    }
}

dllist& operator =(dllist const& l) {
    dllist<T> tmp{l};
    this->swap(tmp);
    return *this;
}

dllist& operator =(dllist&& l) {
    dllist<T> tmp{std::move(l)};
    this->swap(tmp);
    return *this;
}

void assign(std::initializer_list<T> il) {
    dllist<T> tmp{il};
    this->swap(tmp);
}

void assign(size_type n, value_type const& value) {
    dllist<T> tmp(n,value);
    this->swap(tmp);
}

template <typename InIter>
void assign(InIter const& first, InIter const& last) {
    dllist<T> tmp(first,last);
    this->swap(tmp);
}

bool empty() const
{
    return size_ == 0;
}

size_type size() const
{
    return size_;
}

size_type max_size() const
{
    return std::numeric_limits<size_type>::max();
}

reference front()
{
    return front_.nextptr(&back_)->to_node().datum();
}

const_reference front() const
{
    return front_.nextptr(&back_)->to_node().datum();
}

reference back()
{
    return back_.nextptr(&front_)->to_node().datum();
}

const_reference back() const
{
    return back_.nextptr(&front_)->to_node().datum();
}

iterator begin()
{
    return dllist_iter<T>{&front_, front_.nextptr(&back_)};
}

iterator end()
{
    return dllist_iter<T>(back_.nextptr(&front_), &back_);
}

const_iterator begin() const
{
    return dllist_citer<T>(&front_, front_.nextptr(&back_));
}
const_iterator end() const
{
    return dllist_citer<T>{back_.nextptr(&front_), &back_};
}
const_iterator cbegin() const
{
    return begin();
}
const_iterator cend() const
{
    return end();
}
reverse_iterator rbegin()
{
    return reverse_iterator{this->end()};
}

reverse_iterator rend()
{
    return reverse_iterator{this->begin()};
}

const_reverse_iterator rbegin() const
{
    return const_reverse_iterator{this->end()};
}

const_reverse_iterator rend() const
{
    return const_reverse_iterator{this->begin()};
}

const_reverse_iterator crbegin() const
{
    return const_reverse_iterator{this->cend()};
}

const_reverse_iterator crend() const
{
    return const_reverse_iterator{this->cbegin()};
}

void clear()
{
    while(!empty())
        pop_back();
}

void swap(dllist& l)
{
    if (size_ == 0)
    {
        front_.setptr(&l.back_, &l.back_);
        back_.setptr(&l.front_, &l.front_);
    }
    else if (size_ == 1)
    {
        auto element = front_.nextptr(&back_);

        front_.setptr(&l.back_, element);
        back_.setptr(element, &l.front_);

        element->setptr(&l.back_, &l.front_);
    }
    else
    {
        auto first = front_.nextptr(&back_);
        auto second = first->nextptr(&front_);
        auto last = back_.nextptr(&front_);
        auto before_last = last->nextptr(&back_);

        front_.setptr(&l.back_, first);
        back_.setptr(&l.front_, last);

        first->setptr(&l.front_, second);
        last->setptr(&l.back_, before_last);
    }

    if (l.size_ == 0)
    {
        l.front_.setptr(&back_, &back_);
        l.back_.setptr(&front_, &front_);
    }
    else if (l.size_ == 1)
    {
        auto l_element = l.front_.nextptr(&l.back_);

        l.front_.setptr(&back_, l_element);
        l.back_.setptr(l_element, &front_);

        l_element->setptr(&back_, &front_);
    }
    else
    {
        auto l_first = l.front_.nextptr(&l.back_);
        auto l_second = l_first->nextptr(&l.front_);
        auto l_last = l.back_.nextptr(&l.front_);
        auto l_before_last = l_last->nextptr(&l.back_);

        l.front_.setptr(&back_, l_first);
        l.back_.setptr(&front_, l_last);

        l_first->setptr(&front_, l_second);
        l_last->setptr(&back_, l_before_last);
    }

    front_.swap(l.front_);
    back_.swap(l.back_);

    std::swap(size_, l.size_);
}

void push_front(value_type const& v)
{
    dllist_node<T>::insert(&back_, &front_, new dllist_node<T>(v));
    size_++;
}

void push_front(value_type&& v)
{
    dllist_node<T>::insert(&back_, &front_, new dllist_node<T>(std::move(v)));
    size_++;
}

void pop_front()
{
    auto old = dllist_node<T>::remove(&back_, &front_);
    size_--;
    delete old;
}

void push_back(value_type const& v)
{
    dllist_node<T>::insert(&front_, &back_, new dllist_node<T>(v));
    size_++;
}
void push_back(value_type&& v)
{
    dllist_node<T>::insert(&front_, &back_, new dllist_node<T>(std::move(v)));
    size_++;
}
void pop_back()
{
    auto old = dllist_node<T>::remove(&front_, &back_);
    size_--;
    delete old;
}

    template <typename... Args>
iterator emplace(iterator pos, Args&&... args)
{
    --pos;
    auto newnode = new dllist_node<T>{T(std::forward<Args>(args)...)};
    auto nextnode = dllist_node<T>::insert(pos.prevptr_, pos.nodeptr_, newnode);
    size_++;
    return iterator{newnode, nextnode};
}

    template <typename... Args>
void emplace_front(Args&&... args)
{
    this->emplace(begin(), std::forward<Args>(args)...);
}

    template <typename... Args>
void emplace_back(Args&&... args)
{
    this->emplace(end(), std::forward<Args>(args)...);
}

iterator insert(iterator pos, value_type const& value)
{
    --pos;
    auto newnode = new dllist_node<T>{value};
    auto nextnode = dllist_node<T>::insert(pos.prevptr_, pos.nodeptr_, newnode);
    size_++;
    return iterator{newnode, nextnode};
}

    template <typename InIter>
void insert(iterator pos, InIter first, InIter const& last)
{
    for(;first != last; ++first) {
        pos = insert(pos, *first);
    }
}

iterator erase(iterator pos)
{
    --pos;
    auto oldnode{dllist_node<T>::remove(pos.prevptr_, pos.nodeptr_)};
    size_--;
    delete oldnode;
    ++pos;
    return pos;
}

iterator erase(iterator first, iterator const& last)
{
    int i = 0;
    for(;first != last;i++) {
        first = erase(first);
    }
    return first;
}
};

//===========================================================================

    template <typename T>
inline void swap(dllist<T>& a, dllist<T>& b)
{
    a.swap(b);
}

    template <typename T>
inline bool operator ==(dllist<T> const& a, dllist<T> const& b)
{
    using std::equal;
    return equal( a.begin(), a.end(), b.begin(), b.end());
}

    template <typename T>
inline bool operator !=(dllist<T> const& a, dllist<T> const& b)
{
    return !operator ==(a,b);
}

    template <typename T>
inline bool operator <(dllist<T> const& a, dllist<T> const& b)
{
    using std::lexicographical_compare;
    return lexicographical_compare( a.begin(), a.end(), b.begin(), b.end() );
}

    template <typename T>
inline bool operator <=(dllist<T> const& a, dllist<T> const& b)
{
    return !(a > b);
}

    template <typename T>
inline bool operator >=(dllist<T> const& a, dllist<T> const& b)
{
    return !(a < b);
}

    template <typename T>
inline bool operator >(dllist<T> const& a, dllist<T> const& b)
{
    return b < a;
}

    template <typename T>
inline auto begin(dllist<T>& a)
{
    return a.begin();
}

    template <typename T>
inline auto begin(dllist<T> const& a)
{
    return a.begin();
}

    template <typename T>
inline auto cbegin(dllist<T> const& a)
{
    return a.begin();
}

    template <typename T>
inline auto rbegin(dllist<T>& a)
{
    return a.rbegin();
}

    template <typename T>
inline auto rbegin(dllist<T> const& a)
{
    return a.rbegin();
}

    template <typename T>
inline auto crbegin(dllist<T> const& a)
{
    return a.crbegin();
}
    template <typename T>
inline auto end(dllist<T>& a)
{
    return a.end();
}

    template <typename T>
inline auto end(dllist<T> const& a)
{
    return a.end();
}

    template <typename T>
inline auto cend(dllist<T> const& a)
{
    return a.end();
}

    template <typename T>
inline auto rend(dllist<T>& a)
{
    return a.rend();
}

    template <typename T>
inline auto rend(dllist<T> const& a)
{
    return a.rend();
}

    template <typename T>
inline auto crend(dllist<T> const& a)
{
    return a.crend();
}

//===========================================================================

template <typename T>
class dllist_iter :
    public std::iterator<
    std::bidirectional_iterator_tag,
    T,
    std::ptrdiff_t,
    T*,
    T&
    >
{
    private:
        friend class dllist<T>;
        friend class dllist_citer<T>;

        dllist_node_ptr_only<T>* prevptr_;
        dllist_node_ptr_only<T>* nodeptr_;

    public:
        dllist_iter() :
            prevptr_{nullptr},
            nodeptr_{nullptr}
        {
        }

        dllist_iter(dllist_iter const&) = default;
        dllist_iter& operator =(dllist_iter const&) = default;

        dllist_iter(dllist_iter&) = default;
        dllist_iter& operator =(dllist_iter&) = default;

        ~dllist_iter() = default;


        dllist_iter(
                dllist_node_ptr_only<T>* prev,
                dllist_node_ptr_only<T>* xornode
                ) :
            prevptr_{prev},
            nodeptr_{xornode}
        {
        }


        bool operator ==(dllist_iter const& i) const
        {
            return nodeptr_ == i.nodeptr_;
        }


        bool operator !=(dllist_iter const& i) const
        {
            return !operator ==(i);
        }

        T& operator *() const
        {
            return nodeptr_->to_node().datum();
        }

        T* operator ->() const
        {
            return &nodeptr_->to_node().datum();
        }

        dllist_iter& operator ++()
        {
            auto next_nodeptr_ = nodeptr_->nextptr(prevptr_);
            prevptr_ = nodeptr_;
            nodeptr_ = next_nodeptr_;
            return *this;
        }

        dllist_iter operator ++(int)
        {
            dllist_iter<T> tmp(*this);
            operator ++();
            return tmp;
        }

        dllist_iter& operator --()
        {
            auto prev_prevptr_ = prevptr_->nextptr(nodeptr_);
            nodeptr_ = prevptr_;
            prevptr_ = prev_prevptr_;
            return *this;
        }

        dllist_iter operator --(int)
        {
            dllist_iter<T> tmp(*this);
            operator --();
            return tmp;
        }
};

//===========================================================================

template <typename T>
class dllist_citer :
    public std::iterator<
    std::bidirectional_iterator_tag,
    T const,
    std::ptrdiff_t,
    T const*,
    T const&
    >
{
    private:
        friend class dllist<T>;

        dllist_node_ptr_only<T> const* prevptr_; // Used to compute next node address
        dllist_node_ptr_only<T> const* nodeptr_; // Cur node; for xorptr_ value

    public:
        dllist_citer() :
            prevptr_{nullptr},
            nodeptr_{nullptr}
        {
        }

        dllist_citer(dllist_citer const&) = default;
        dllist_citer& operator =(dllist_citer const&) = default;

        dllist_citer(dllist_citer&) = default;
        dllist_citer& operator =(dllist_citer&) = default;

        ~dllist_citer() = default;

        dllist_citer(
                dllist_node_ptr_only<T> const* prev,
                dllist_node_ptr_only<T> const* xornode
                ) :
            prevptr_{prev},
            nodeptr_{xornode}
        {
        }

        dllist_citer(dllist_iter<T> const& i) :
            nodeptr_{i.nodeptr_},
            prevptr_{i.prevptr_}
        {
        }

        dllist_citer& operator =(dllist_iter<T> const& i)
        {
            nodeptr_ = i.nodeptr_;
            prevptr_ = i.prevptr_;
            return *this;
        }

        bool operator ==(dllist_iter<T> const& i) const
        {
            return nodeptr_ == i.nodeptr_;
        }

        bool operator !=(dllist_iter<T> const& i) const
        {
            return !operator ==(i);
        }

        bool operator ==(dllist_citer const& i) const
        {
            return nodeptr_->nextptr(prevptr_) == i.nodeptr_->nextptr(i.prevptr_);
        }

        bool operator !=(dllist_citer const& i) const
        {
            return !(*this == i);
            return !this->operator ==(i);
        }

        T const& operator *() const
        {
            return nodeptr_->to_node().datum();
        }

        T const* operator ->() const
        {
            return &nodeptr_->to_node().datum();
        }

        dllist_citer& operator ++()
        {
            auto next_nodeptr_ = nodeptr_->nextptr(prevptr_);
            prevptr_ = nodeptr_;
            nodeptr_ = next_nodeptr_;
            return *this;
        }

        dllist_citer operator ++(int)
        {
            dllist_citer<T> tmp(*this);
            operator ++();
            return tmp;
        }

        dllist_citer& operator --()
        {
            auto prev_prevptr_ = prevptr_->nextptr(nodeptr_);
            nodeptr_ = prevptr_;
            prevptr_ = prev_prevptr_;
            return *this;
        }

        dllist_citer operator --(int)
        {
            dllist_citer<T> tmp(*this);
            operator --();
            return tmp;
        }
};

    template <typename T>
inline bool operator ==(dllist_iter<T> const& i, dllist_citer<T> const& j)
{
    return j == i;
}
    template <typename T>
inline bool operator !=(dllist_iter<T> const& i, dllist_citer<T> const& j)
{
    return j != i;
}

#endif // ifndef DLLIST_HXX
